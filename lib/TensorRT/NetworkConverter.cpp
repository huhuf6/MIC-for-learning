#include "MIC/TensorRT/NetworkConverter.h"
#include "MIC/Dialect/NNOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "NvInfer.h"

using namespace mlir;
using namespace MIC::NN;

namespace mlir {
namespace MIC {
namespace TensorRT {

class NetworkConverter::Impl {
private:
  TensorRTBuilder &builder;
  nvinfer1::INetworkDefinition *network;
  DenseMap<Value, nvinfer1::ITensor *> valueMap;

public:
  Impl(TensorRTBuilder &builder) : builder(builder) {
    network = builder.getNetwork();
  }

  LogicalResult convertOperation(Operation *op) {
    if (auto linearOp = dyn_cast<LinearOp>(op)) {
      return convertLinearOp(linearOp);
    }
    if (auto convOp = dyn_cast<Conv2DOp>(op)) {
      return convertConv2DOp(convOp);
    }
    if (auto geluOp = dyn_cast<GELUOp>(op)) {
      return convertGELUOp(geluOp);
    }
    if (auto lnOp = dyn_cast<LayerNormOp>(op)) {
      return convertLayerNormOp(lnOp);
    }
    if (auto attentionOp = dyn_cast<AttentionOp>(op)) {
      return convertAttentionOp(attentionOp);
    }
    return failure();
  }

  LogicalResult convertLinearOp(LinearOp op) {
    auto input = getOrCreateTensor(op.input());
    auto weight = getOrCreateTensor(op.weight());
    
    if (!input || !weight) {
      return failure();
    }

    // Create fully connected layer
    auto fcLayer = network->addFullyConnected(*input, weight->getDimensions().d[0], *weight, nullptr);
    if (!fcLayer) {
      return failure();
    }

    if (op.bias()) {
      auto bias = getOrCreateTensor(op.bias());
      if (bias) {
        fcLayer->setBiasWeights(*bias);
      }
    }

    valueMap[op.output()] = fcLayer->getOutput(0);
    return success();
  }

  LogicalResult convertConv2DOp(Conv2DOp op) {
    auto input = getOrCreateTensor(op.input());
    auto weight = getOrCreateTensor(op.weight());
    
    if (!input || !weight) {
      return failure();
    }

    // Create convolution layer
    nvinfer1::Dims kernelSize = weight->getDimensions();
    nvinfer1::Dims stride;
    stride.nbDims = 2;
    auto strides = op.strides();
    stride.d[0] = strides[0].cast<IntegerAttr>().getInt();
    stride.d[1] = strides[1].cast<IntegerAttr>().getInt();

    auto convLayer = network->addConvolution(*input, weight->getDimensions().d[0], kernelSize, *weight, nullptr);
    if (!convLayer) {
      return failure();
    }

    convLayer->setStride(stride);

    if (op.bias()) {
      auto bias = getOrCreateTensor(op.bias());
      if (bias) {
        convLayer->setBiasWeights(*bias);
      }
    }

    valueMap[op.output()] = convLayer->getOutput(0);
    return success();
  }

  LogicalResult convertGELUOp(GELUOp op) {
    auto input = getOrCreateTensor(op.input());
    if (!input) {
      return failure();
    }

    // Create activation layer (GELU)
    auto activationLayer = network->addActivation(*input, nvinfer1::ActivationType::kGELU);
    if (!activationLayer) {
      return failure();
    }

    valueMap[op.output()] = activationLayer->getOutput(0);
    return success();
  }

  LogicalResult convertLayerNormOp(LayerNormOp op) {
    auto input = getOrCreateTensor(op.input());
    if (!input) {
      return failure();
    }

    // For now, we'll need to implement this as a plugin
    // TensorRT doesn't have built-in layer norm
    // TODO: Implement LayerNorm plugin
    valueMap[op.output()] = input;
    return success();
  }

  LogicalResult convertAttentionOp(AttentionOp op) {
    // TODO: Implement attention conversion
    // This will likely require a custom plugin
    return success();
  }

  nvinfer1::ITensor *getOrCreateTensor(Value value) {
    if (auto it = valueMap.find(value); it != valueMap.end()) {
      return it->second;
    }

    // Handle constant values
    if (auto constantOp = value.getDefiningOp<arith::ConstantOp>()) {
      // TODO: Convert constant to TensorRT weights
      return nullptr;
    }

    // Handle function arguments
    if (auto funcArg = value.dyn_cast<BlockArgument>()) {
      auto funcOp = funcArg.getOwner()->getParentOp();
      // TODO: Create input tensor for function argument
      return nullptr;
    }

    return nullptr;
  }

  const DenseMap<Value, nvinfer1::ITensor *> &getValueMap() const {
    return valueMap;
  }
};

NetworkConverter::NetworkConverter(TensorRTBuilder &builder) : impl(std::make_unique<Impl>(builder)) {}

NetworkConverter::~NetworkConverter() = default;

LogicalResult NetworkConverter::convertOperation(Operation *op) {
  return impl->convertOperation(op);
}

const DenseMap<Value, nvinfer1::ITensor *> &NetworkConverter::getValueMap() const {
  return impl->getValueMap();
}

} // namespace TensorRT
} // namespace MIC
} // namespace mlir
