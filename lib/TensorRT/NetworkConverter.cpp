#include "MIC/TensorRT/NetworkConverter.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinTypes.h"

#include "llvm/ADT/StringRef.h"

#include <string>
#include <utility>

using namespace mlir;
using namespace mlir::MIC::TensorRT;

namespace {

static bool isTensorLikeValue(Value value) {
  return value.getType().isa<TensorType>();
}

static nvinfer1::DataType convertElementType(Type elemTy) {
  if (elemTy.isF32())
    return nvinfer1::DataType::kFLOAT;
  if (elemTy.isF16())
    return nvinfer1::DataType::kHALF;
  if (elemTy.isInteger(32))
    return nvinfer1::DataType::kINT32;
  if (elemTy.isInteger(8))
    return nvinfer1::DataType::kINT8;
  if (elemTy.isInteger(1))
    return nvinfer1::DataType::kBOOL;
  // Fallback for unsupported types in this prototype.
  return nvinfer1::DataType::kFLOAT;
}

static nvinfer1::Dims toTensorRTDims(RankedTensorType type) {
  nvinfer1::Dims dims;
  dims.nbDims = static_cast<int32_t>(type.getRank());
  for (int32_t i = 0; i < dims.nbDims; ++i) {
    if (type.isDynamicDim(i))
      dims.d[i] = -1;
    else
      dims.d[i] = static_cast<int32_t>(type.getDimSize(i));
  }
  return dims;
}

} // namespace

class NetworkConverter::Impl {
private:
  TensorRTBuilder &builder;
  nvinfer1::INetworkDefinition *network;

  // MLIR value -> TensorRT tensor
  DenseMap<Value, nvinfer1::ITensor *> valueToTensor;
  // TensorRT tensor -> MLIR value
  DenseMap<nvinfer1::ITensor *, Value> tensorToValue;

public:
  explicit Impl(TensorRTBuilder &builder) : builder(builder) {
    network = builder.getNetwork();
  }

  LogicalResult convertOperation(Operation *op) {
    if (!op)
      return success();

    if (auto funcOp = dyn_cast<func::FuncOp>(op))
      return convertFuncOp(funcOp);

    if (auto callOp = dyn_cast<func::CallOp>(op))
      return convertCallOp(callOp);

    if (auto retOp = dyn_cast<func::ReturnOp>(op))
      return convertReturnOp(retOp);

    return success();
  }

  const DenseMap<Value, nvinfer1::ITensor *> &getValueMap() const {
    return valueToTensor;
  }

private:
  LogicalResult convertFuncOp(func::FuncOp funcOp) {
    // Skip runtime declaration stubs: they have no body.
    if (funcOp.empty())
      return success();

    Block &entry = funcOp.getBody().front();
    for (BlockArgument arg : entry.getArguments()) {
      if (!isTensorLikeValue(arg))
        continue;
      if (failed(getOrCreateTensor(arg)))
        return failure();
    }
    return success();
  }

  LogicalResult convertReturnOp(func::ReturnOp retOp) {
    for (Value value : retOp.getOperands()) {
      if (!isTensorLikeValue(value))
        continue;
      nvinfer1::ITensor *tensor = nullptr;
      if (failed(getOrCreateTensor(value, &tensor)))
        return failure();
      if (!tensor)
        return failure();
      if (!tensor->isNetworkOutput())
        network->markOutput(*tensor);
    }
    return success();
  }

  LogicalResult convertCallOp(func::CallOp callOp) {
    StringRef callee = callOp.getCallee();
    if (!callee.startswith("mic_trt_"))
      return success();

    if (callee == "mic_trt_matmul")
      return lowerMatmul(callOp);
    if (callee == "mic_trt_relu")
      return lowerRelu(callOp);
    if (callee == "mic_trt_add")
      return lowerElementWise(callOp, nvinfer1::ElementWiseOperation::kSUM);
    if (callee == "mic_trt_mul")
      return lowerElementWise(callOp, nvinfer1::ElementWiseOperation::kPROD);
    if (callee == "mic_trt_linear")
      return lowerLinear(callOp);

    callOp.emitError() << "unsupported TensorRT runtime call: " << callee;
    return failure();
  }

  LogicalResult lowerMatmul(func::CallOp callOp) {
    if (callOp.getNumOperands() != 2 || callOp.getNumResults() != 1) {
      callOp.emitError("mic_trt_matmul expects 2 operands and 1 result");
      return failure();
    }

    nvinfer1::ITensor *lhs = nullptr;
    nvinfer1::ITensor *rhs = nullptr;
    if (failed(getOrCreateTensor(callOp.getOperand(0), &lhs)) ||
        failed(getOrCreateTensor(callOp.getOperand(1), &rhs))) {
      return failure();
    }

    auto *layer = network->addMatrixMultiply(*lhs, nvinfer1::MatrixOperation::kNONE,
                                             *rhs, nvinfer1::MatrixOperation::kNONE);
    if (!layer) {
      callOp.emitError("failed to create TensorRT MatrixMultiply layer");
      return failure();
    }

    nvinfer1::ITensor *out = layer->getOutput(0);
    bindValueAndTensor(callOp.getResult(0), out);
    return success();
  }

  LogicalResult lowerRelu(func::CallOp callOp) {
    if (callOp.getNumOperands() != 1 || callOp.getNumResults() != 1) {
      callOp.emitError("mic_trt_relu expects 1 operand and 1 result");
      return failure();
    }

    nvinfer1::ITensor *input = nullptr;
    if (failed(getOrCreateTensor(callOp.getOperand(0), &input)))
      return failure();

    auto *layer = network->addActivation(*input, nvinfer1::ActivationType::kRELU);
    if (!layer) {
      callOp.emitError("failed to create TensorRT ReLU layer");
      return failure();
    }

    nvinfer1::ITensor *out = layer->getOutput(0);
    bindValueAndTensor(callOp.getResult(0), out);
    return success();
  }

  LogicalResult lowerElementWise(func::CallOp callOp,
                                 nvinfer1::ElementWiseOperation opType) {
    if (callOp.getNumOperands() != 2 || callOp.getNumResults() != 1) {
      callOp.emitError("elementwise call expects 2 operands and 1 result");
      return failure();
    }

    nvinfer1::ITensor *lhs = nullptr;
    nvinfer1::ITensor *rhs = nullptr;
    if (failed(getOrCreateTensor(callOp.getOperand(0), &lhs)) ||
        failed(getOrCreateTensor(callOp.getOperand(1), &rhs))) {
      return failure();
    }

    auto *layer = network->addElementWise(*lhs, *rhs, opType);
    if (!layer) {
      callOp.emitError("failed to create TensorRT ElementWise layer");
      return failure();
    }

    nvinfer1::ITensor *out = layer->getOutput(0);
    bindValueAndTensor(callOp.getResult(0), out);
    return success();
  }

  LogicalResult lowerLinear(func::CallOp callOp) {
    if (callOp.getNumOperands() < 2 || callOp.getNumOperands() > 3 ||
        callOp.getNumResults() != 1) {
      callOp.emitError("mic_trt_linear expects 2 or 3 operands and 1 result");
      return failure();
    }

    nvinfer1::ITensor *lhs = nullptr;
    nvinfer1::ITensor *rhs = nullptr;
    if (failed(getOrCreateTensor(callOp.getOperand(0), &lhs)) ||
        failed(getOrCreateTensor(callOp.getOperand(1), &rhs))) {
      return failure();
    }

    auto *matmul =
        network->addMatrixMultiply(*lhs, nvinfer1::MatrixOperation::kNONE, *rhs,
                                   nvinfer1::MatrixOperation::kNONE);
    if (!matmul) {
      callOp.emitError("failed to create TensorRT MatrixMultiply layer for linear");
      return failure();
    }

    nvinfer1::ITensor *out = matmul->getOutput(0);
    if (callOp.getNumOperands() == 3) {
      nvinfer1::ITensor *bias = nullptr;
      if (failed(getOrCreateTensor(callOp.getOperand(2), &bias)))
        return failure();
      auto *add =
          network->addElementWise(*out, *bias, nvinfer1::ElementWiseOperation::kSUM);
      if (!add) {
        callOp.emitError("failed to create TensorRT bias add layer for linear");
        return failure();
      }
      out = add->getOutput(0);
    }

    bindValueAndTensor(callOp.getResult(0), out);
    return success();
  }

  LogicalResult getOrCreateTensor(Value value) {
    nvinfer1::ITensor *ignored = nullptr;
    return getOrCreateTensor(value, &ignored);
  }

  LogicalResult getOrCreateTensor(Value value, nvinfer1::ITensor **outTensor) {
    auto found = valueToTensor.find(value);
    if (found != valueToTensor.end()) {
      if (outTensor)
        *outTensor = found->second;
      return success();
    }

    // Currently we can materialize only function arguments as TensorRT inputs.
    if (auto arg = value.dyn_cast<BlockArgument>()) {
      auto *owner = arg.getOwner();
      auto *parentOp = owner ? owner->getParentOp() : nullptr;
      auto parentFunc = dyn_cast_or_null<func::FuncOp>(parentOp);
      if (!parentFunc || !owner->isEntryBlock()) {
        if (parentFunc)
          parentFunc.emitError(
              "unsupported non-entry block argument for TensorRT conversion");
        return failure();
      }

      auto ranked = value.getType().dyn_cast<RankedTensorType>();
      if (!ranked) {
        parentFunc.emitError("only ranked tensor arguments are supported");
        return failure();
      }

      nvinfer1::Dims dims = toTensorRTDims(ranked);
      nvinfer1::DataType dt = convertElementType(ranked.getElementType());

      const unsigned argNo = static_cast<unsigned>(arg.getArgNumber());
      std::string inputName = (argNo == 0) ? "input" : ("input" + std::to_string(argNo));
      nvinfer1::ITensor *tensor = network->addInput(inputName.c_str(), dt, dims);
      if (!tensor) {
        parentFunc.emitError("failed to create TensorRT network input");
        return failure();
      }

      bindValueAndTensor(value, tensor);
      if (outTensor)
        *outTensor = tensor;
      return success();
    }

    auto *def = value.getDefiningOp();
    if (def)
      def->emitError("TensorRT conversion: missing ITensor mapping for this value");
    return failure();
  }

  void bindValueAndTensor(Value value, nvinfer1::ITensor *tensor) {
    if (!tensor)
      return;
    valueToTensor[value] = tensor;
    tensorToValue[tensor] = value;
  }
};

NetworkConverter::NetworkConverter(TensorRTBuilder &builder)
    : impl(std::make_unique<Impl>(builder)) {}

NetworkConverter::~NetworkConverter() = default;

LogicalResult NetworkConverter::convertOperation(Operation *op) {
  return impl->convertOperation(op);
}

const DenseMap<Value, nvinfer1::ITensor *> &NetworkConverter::getValueMap() const {
  return impl->getValueMap();
}
