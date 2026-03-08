//===----------------------------------------------------------------------===//
// Planned Pipeline Passes
// ONNX -> NN -> Linalg/Tensor -> Linalg optimizations -> TensorRT calls
//===----------------------------------------------------------------------===//

#include "MIC/Passes/Passes.h"

#include "mlir/Dialect/Arithmetic/IR/Arithmetic.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Linalg/Passes.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/IR/AffineMap.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

#include <memory>
#include <string>

using namespace mlir;

namespace {

static Operation *renameOp(Operation *op, StringRef newName) {
  OpBuilder builder(op);
  OperationState st(op->getLoc(), newName);
  st.addOperands(op->getOperands());
  st.addTypes(op->getResultTypes());
  st.addAttributes(op->getAttrs());
  Operation *newOp = Operation::create(st);
  builder.insert(newOp);
  op->replaceAllUsesWith(newOp->getResults());
  op->erase();
  return newOp;
}

static RankedTensorType getSingleRankedTensorResultType(Operation *op) {
  if (op->getNumResults() != 1)
    return {};
  return op->getResult(0).getType().dyn_cast<RankedTensorType>();
}

static Value createInitTensorLikeResult(OpBuilder &builder, Location loc,
                                        RankedTensorType resultType,
                                        Value shapeSource) {
  SmallVector<Value, 4> dynamicDims;
  for (int64_t i = 0; i < resultType.getRank(); ++i) {
    if (resultType.isDynamicDim(i))
      dynamicDims.push_back(builder.create<tensor::DimOp>(loc, shapeSource, i));
  }
  return builder
      .create<linalg::InitTensorOp>(loc, dynamicDims, resultType.getShape(),
                                    resultType.getElementType())
      .result();
}

static Value createInitTensorForMatmul(OpBuilder &builder, Location loc,
                                       RankedTensorType resultType, Value lhs,
                                       Value rhs) {
  SmallVector<Value, 4> dynamicDims;
  for (int64_t i = 0; i < resultType.getRank(); ++i) {
    if (!resultType.isDynamicDim(i))
      continue;
    if (i == 0) {
      dynamicDims.push_back(builder.create<tensor::DimOp>(loc, lhs, 0));
    } else if (i == 1) {
      dynamicDims.push_back(builder.create<tensor::DimOp>(loc, rhs, 1));
    } else {
      dynamicDims.push_back(builder.create<tensor::DimOp>(loc, lhs, i));
    }
  }
  return builder
      .create<linalg::InitTensorOp>(loc, dynamicDims, resultType.getShape(),
                                    resultType.getElementType())
      .result();
}

static Value buildZeroScalar(OpBuilder &builder, Location loc, Type elemType) {
  if (auto floatTy = elemType.dyn_cast<FloatType>())
    return builder.create<arith::ConstantOp>(loc, FloatAttr::get(floatTy, 0.0))
        .getResult();
  if (auto intTy = elemType.dyn_cast<IntegerType>())
    return builder.create<arith::ConstantOp>(loc, IntegerAttr::get(intTy, 0))
        .getResult();
  return {};
}

static bool isSupportedTensorType(Type t) {
  auto rt = t.dyn_cast<RankedTensorType>();
  if (!rt)
    return false;
  Type et = rt.getElementType();
  return et.isa<FloatType>() || et.isa<IntegerType>();
}

static bool lowerBinaryElementwiseOp(Operation *op, bool isAdd) {
  if (op->getNumOperands() != 2 || op->getNumResults() != 1)
    return false;

  Value lhs = op->getOperand(0);
  Value rhs = op->getOperand(1);
  auto resultType = getSingleRankedTensorResultType(op);
  if (!resultType || !isSupportedTensorType(lhs.getType()) ||
      !isSupportedTensorType(rhs.getType()))
    return false;

  OpBuilder builder(op);
  Location loc = op->getLoc();
  Value init = createInitTensorLikeResult(builder, loc, resultType, lhs);

  int64_t rank = resultType.getRank();
  SmallVector<AffineMap, 3> maps = {
      AffineMap::getMultiDimIdentityMap(rank, op->getContext()),
      AffineMap::getMultiDimIdentityMap(rank, op->getContext()),
      AffineMap::getMultiDimIdentityMap(rank, op->getContext())};
  SmallVector<StringRef, 4> iterators(rank, getParallelIteratorTypeName());

  auto generic = builder.create<linalg::GenericOp>(
      loc, TypeRange{resultType}, ValueRange{lhs, rhs}, ValueRange{init}, maps,
      iterators,
      [&](OpBuilder &nested, Location nestedLoc, ValueRange args) {
        Value out;
        if (args[0].getType().isa<FloatType>()) {
          out = isAdd
                    ? nested.create<arith::AddFOp>(nestedLoc, args[0], args[1])
                          .getResult()
                    : nested.create<arith::MulFOp>(nestedLoc, args[0], args[1])
                          .getResult();
        } else {
          out = isAdd
                    ? nested.create<arith::AddIOp>(nestedLoc, args[0], args[1])
                          .getResult()
                    : nested.create<arith::MulIOp>(nestedLoc, args[0], args[1])
                          .getResult();
        }
        nested.create<linalg::YieldOp>(nestedLoc, out);
      });

  generic->setAttr("mic.backend.op",
                   builder.getStringAttr(isAdd ? "add" : "mul"));
  op->replaceAllUsesWith(generic->getResults());
  op->erase();
  return true;
}

static bool lowerReluOp(Operation *op) {
  if (op->getNumOperands() != 1 || op->getNumResults() != 1)
    return false;

  Value input = op->getOperand(0);
  auto resultType = getSingleRankedTensorResultType(op);
  if (!resultType || !isSupportedTensorType(input.getType()))
    return false;

  OpBuilder builder(op);
  Location loc = op->getLoc();
  Value init = createInitTensorLikeResult(builder, loc, resultType, input);

  int64_t rank = resultType.getRank();
  SmallVector<AffineMap, 2> maps = {
      AffineMap::getMultiDimIdentityMap(rank, op->getContext()),
      AffineMap::getMultiDimIdentityMap(rank, op->getContext())};
  SmallVector<StringRef, 4> iterators(rank, getParallelIteratorTypeName());

  auto generic = builder.create<linalg::GenericOp>(
      loc, TypeRange{resultType}, ValueRange{input}, ValueRange{init}, maps,
      iterators,
      [&](OpBuilder &nested, Location nestedLoc, ValueRange args) {
        Value zero = buildZeroScalar(nested, nestedLoc, args[0].getType());
        Value out;
        if (args[0].getType().isa<FloatType>())
          out = nested.create<arith::MaxFOp>(nestedLoc, args[0], zero).getResult();
        else
          out = nested.create<arith::MaxSIOp>(nestedLoc, args[0], zero).getResult();
        nested.create<linalg::YieldOp>(nestedLoc, out);
      });

  generic->setAttr("mic.backend.op", builder.getStringAttr("relu"));
  op->replaceAllUsesWith(generic->getResults());
  op->erase();
  return true;
}

static bool lowerMatmulLike(Operation *op, bool hasBias) {
  if (op->getNumOperands() < 2 || op->getNumResults() != 1)
    return false;

  Value lhs = op->getOperand(0);
  Value rhs = op->getOperand(1);
  auto lhsTy = lhs.getType().dyn_cast<RankedTensorType>();
  auto rhsTy = rhs.getType().dyn_cast<RankedTensorType>();
  auto resultType = getSingleRankedTensorResultType(op);
  if (!lhsTy || !rhsTy || !resultType)
    return false;
  if (lhsTy.getRank() != 2 || rhsTy.getRank() != 2 || resultType.getRank() != 2)
    return false;

  OpBuilder builder(op);
  Location loc = op->getLoc();

  Value init = createInitTensorForMatmul(builder, loc, resultType, lhs, rhs);
  Value zero = buildZeroScalar(builder, loc, resultType.getElementType());
  if (!zero)
    return false;

  auto fill = builder.create<linalg::FillOp>(loc, ValueRange{zero}, ValueRange{init});
  Value filled = fill->getResult(0);

  auto matmul = builder.create<linalg::MatmulOp>(loc, TypeRange{resultType},
                                                 ValueRange{lhs, rhs},
                                                 ValueRange{filled});
  Value out = matmul->getResult(0);

  if (hasBias && op->getNumOperands() >= 3 && op->getOperand(2)) {
    Value bias = op->getOperand(2);
    SmallVector<AffineMap, 3> maps = {
        AffineMap::getMultiDimIdentityMap(2, op->getContext()),
        AffineMap::get(2, 0, {builder.getAffineDimExpr(1)}, op->getContext()),
        AffineMap::getMultiDimIdentityMap(2, op->getContext())};
    SmallVector<StringRef, 2> iterators = {getParallelIteratorTypeName(),
                                           getParallelIteratorTypeName()};

    auto addBias = builder.create<linalg::GenericOp>(
        loc, TypeRange{resultType}, ValueRange{out, bias}, ValueRange{out}, maps,
        iterators,
        [&](OpBuilder &nested, Location nestedLoc, ValueRange args) {
          Value sum;
          if (args[0].getType().isa<FloatType>()) {
            sum = nested.create<arith::AddFOp>(nestedLoc, args[0], args[1])
                      .getResult();
          } else {
            sum = nested.create<arith::AddIOp>(nestedLoc, args[0], args[1])
                      .getResult();
          }
          nested.create<linalg::YieldOp>(nestedLoc, sum);
        });
    addBias->setAttr("mic.backend.op", builder.getStringAttr("linear"));
    op->replaceAllUsesWith(addBias->getResults());
    op->erase();
    return true;
  }

  matmul->setAttr("mic.backend.op",
                  builder.getStringAttr(hasBias ? "linear" : "matmul"));
  op->getResult(0).replaceAllUsesWith(out);
  op->erase();
  return true;
}

class LowerONNXToNNPass
    : public PassWrapper<LowerONNXToNNPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "lower-onnx-to-nn"; }
  StringRef getDescription() const final {
    return "Lower common ONNX ops to nn dialect ops";
  }

  void runOnOperation() override {
    static const llvm::StringMap<std::string> onnxToNN = {
        {"onnx.Gemm", "nn.linear"},
        {"onnx.Conv", "nn.conv2d"},
        {"onnx.LayerNormalization", "nn.layer_norm"},
        {"onnx.Softmax", "nn.softmax"},
        {"onnx.Relu", "nn.relu"},
        {"onnx.Add", "nn.add"},
        {"onnx.Mul", "nn.mul"},
        {"onnx.MatMul", "nn.matmul"},
        {"onnx.Reshape", "nn.reshape"},
        {"onnx.Transpose", "nn.transpose"},
    };

    SmallVector<Operation *, 64> targets;
    getOperation().walk([&](Operation *op) {
      if (onnxToNN.count(op->getName().getStringRef()))
        targets.push_back(op);
    });

    for (Operation *op : targets) {
      auto it = onnxToNN.find(op->getName().getStringRef());
      if (it == onnxToNN.end())
        continue;
      Operation *newOp = renameOp(op, it->second);
      if (it->second == "nn.softmax" && !newOp->hasAttr("axis")) {
        newOp->setAttr(
            "axis",
            IntegerAttr::get(IntegerType::get(newOp->getContext(), 64), -1));
      }
    }
  }
};

class LowerNNToLinalgPass
    : public PassWrapper<LowerNNToLinalgPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "lower-nn-to-linalg"; }
  StringRef getDescription() const final {
    return "Lower nn ops to linalg/tensor ops";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    SmallVector<Operation *, 64> nnOps;

    module.walk([&](Operation *op) {
      if (op->getName().getDialectNamespace() == "nn")
        nnOps.push_back(op);
    });

    for (Operation *op : nnOps) {
      if (!op || !op->getBlock())
        continue;

      StringRef name = op->getName().getStringRef();
      bool lowered = false;
      if (name == "nn.add")
        lowered = lowerBinaryElementwiseOp(op, /*isAdd=*/true);
      else if (name == "nn.mul")
        lowered = lowerBinaryElementwiseOp(op, /*isAdd=*/false);
      else if (name == "nn.relu")
        lowered = lowerReluOp(op);
      else if (name == "nn.matmul")
        lowered = lowerMatmulLike(op, /*hasBias=*/false);
      else if (name == "nn.linear")
        lowered = lowerMatmulLike(op, /*hasBias=*/true);

      if (!lowered)
        op->emitRemark("NN->Linalg lowering not implemented for this op");
    }
  }
};

class LinalgFusePass
    : public PassWrapper<LinalgFusePass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "linalg-fuse"; }
  StringRef getDescription() const final {
    return "Apply MLIR linalg elementwise fusion";
  }

  void runOnOperation() override {
    OpPassManager funcPM(func::FuncOp::getOperationName());
    funcPM.addPass(createLinalgElementwiseOpFusionPass());
    if (failed(runPipeline(funcPM, getOperation())))
      signalPassFailure();
  }
};

class LinalgTilePass
    : public PassWrapper<LinalgTilePass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "linalg-tile"; }
  StringRef getDescription() const final {
    return "Apply MLIR linalg tiling";
  }

  void runOnOperation() override {
    OpPassManager funcPM(func::FuncOp::getOperationName());
    funcPM.addPass(createLinalgTilingPass({16, 16}));
    if (failed(runPipeline(funcPM, getOperation())))
      signalPassFailure();
  }
};

class LinalgVectorizePass
    : public PassWrapper<LinalgVectorizePass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "linalg-vectorize"; }
  StringRef getDescription() const final {
    return "Apply MLIR linalg vectorization strategy";
  }

  void runOnOperation() override {
    OpPassManager funcPM(func::FuncOp::getOperationName());
    funcPM.addPass(createLinalgStrategyVectorizePass());
    if (failed(runPipeline(funcPM, getOperation())))
      signalPassFailure();
  }
};

class LinalgToTensorRTPass
    : public PassWrapper<LinalgToTensorRTPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "lower-linalg-to-tensorrt"; }
  StringRef getDescription() const final {
    return "Lower linalg ops to TensorRT runtime call placeholders";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    SmallVector<Operation *, 64> targets;

    module.walk([&](Operation *op) {
      StringRef name = op->getName().getStringRef();
      if (name == "linalg.matmul" || name == "linalg.generic")
        targets.push_back(op);
    });

    for (Operation *op : targets) {
      if (!op || !op->getBlock())
        continue;

      std::string callee = inferRuntimeCallee(op);
      if (callee.empty())
        continue;

      OpBuilder builder(op);
      SmallVector<Type, 8> argTypes(op->getOperandTypes().begin(),
                                    op->getOperandTypes().end());
      SmallVector<Type, 4> resultTypes(op->getResultTypes().begin(),
                                       op->getResultTypes().end());
      ensureRuntimeDeclaration(module, callee, argTypes, resultTypes);

      auto call = builder.create<func::CallOp>(op->getLoc(), callee, resultTypes,
                                               op->getOperands());
      op->replaceAllUsesWith(call.getResults());
      op->erase();
    }
  }

private:
  static std::string inferRuntimeCallee(Operation *op) {
    StringRef name = op->getName().getStringRef();
    if (name == "linalg.matmul")
      return "mic_trt_matmul";
    if (name != "linalg.generic")
      return {};

    auto backend = op->getAttrOfType<StringAttr>("mic.backend.op");
    if (!backend)
      return {};
    return ("mic_trt_" + backend.getValue()).str();
  }

  static void ensureRuntimeDeclaration(ModuleOp module, StringRef name,
                                       ArrayRef<Type> argTypes,
                                       ArrayRef<Type> resultTypes) {
    if (module.lookupSymbol<func::FuncOp>(name))
      return;

    OpBuilder builder(module.getBodyRegion());
    auto funcType = builder.getFunctionType(argTypes, resultTypes);
    auto fn = builder.create<func::FuncOp>(module.getLoc(), name, funcType);
    fn.setPrivate();
  }
};

} // namespace

namespace mlir {
namespace MIC {

std::unique_ptr<Pass> createLowerONNXToNNPass() {
  return std::make_unique<LowerONNXToNNPass>();
}

std::unique_ptr<Pass> createLowerNNToLinalgPass() {
  return std::make_unique<LowerNNToLinalgPass>();
}

std::unique_ptr<Pass> createLinalgFusePass() {
  return std::make_unique<LinalgFusePass>();
}

std::unique_ptr<Pass> createLinalgTilePass() {
  return std::make_unique<LinalgTilePass>();
}

std::unique_ptr<Pass> createLinalgVectorizePass() {
  return std::make_unique<LinalgVectorizePass>();
}

std::unique_ptr<Pass> createLinalgToTensorRTPass() {
  return std::make_unique<LinalgToTensorRTPass>();
}

} // namespace MIC
} // namespace mlir

// NOLINTNEXTLINE
static PassRegistration<LowerONNXToNNPass> passLowerONNXToNN;
// NOLINTNEXTLINE
static PassRegistration<LowerNNToLinalgPass> passLowerNNToLinalg;
// NOLINTNEXTLINE
static PassRegistration<LinalgFusePass> passLinalgFuse;
// NOLINTNEXTLINE
static PassRegistration<LinalgTilePass> passLinalgTile;
// NOLINTNEXTLINE
static PassRegistration<LinalgVectorizePass> passLinalgVectorize;
// NOLINTNEXTLINE
static PassRegistration<LinalgToTensorRTPass> passLinalgToTensorRT;
