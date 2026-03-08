//===----------------------------------------------------------------------===//
// NN to Runtime Call Lowering Pass
//===----------------------------------------------------------------------===//

#include "MIC/Passes/Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

using namespace mlir;

namespace {

class NNToRuntimeLoweringPass
    : public PassWrapper<NNToRuntimeLoweringPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "lower-nn-to-runtime"; }
  StringRef getDescription() const final {
    return "Lower nn.* ops to runtime func.call operations";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    SmallVector<Operation *, 32> nnOps;

    module.walk([&](Operation *op) {
      if (op->getName().getDialectNamespace() == "nn")
        nnOps.push_back(op);
    });

    for (Operation *op : nnOps) {
      if (!op || op->getBlock() == nullptr)
        continue;

      const std::string calleeName = getRuntimeCallee(op->getName().getStringRef());
      if (calleeName.empty())
        continue;

      OpBuilder builder(op);
      SmallVector<Type, 8> argTypes(op->getOperandTypes().begin(),
                                    op->getOperandTypes().end());
      SmallVector<Type, 4> resultTypes(op->getResultTypes().begin(),
                                       op->getResultTypes().end());

      ensureRuntimeDeclaration(module, calleeName, argTypes, resultTypes);

      auto call = builder.create<func::CallOp>(op->getLoc(), calleeName, resultTypes,
                                               op->getOperands());
      op->replaceAllUsesWith(call.getResults());
      op->erase();
    }
  }

private:
  static std::string getRuntimeCallee(StringRef opName) {
    if (opName == "nn.linear")
      return "mic_nn_linear";
    if (opName == "nn.conv2d")
      return "mic_nn_conv2d";
    if (opName == "nn.attention")
      return "mic_nn_attention";
    if (opName == "nn.gelu")
      return "mic_nn_gelu";
    if (opName == "nn.layer_norm")
      return "mic_nn_layer_norm";
    if (opName == "nn.relu")
      return "mic_nn_relu";
    if (opName == "nn.softmax")
      return "mic_nn_softmax";
    if (opName == "nn.add")
      return "mic_nn_add";
    if (opName == "nn.mul")
      return "mic_nn_mul";
    if (opName == "nn.reshape")
      return "mic_nn_reshape";
    if (opName == "nn.transpose")
      return "mic_nn_transpose";
    if (opName == "nn.matmul")
      return "mic_nn_matmul";
    return "";
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

std::unique_ptr<Pass> createNNToRuntimeLoweringPass() {
  return std::make_unique<NNToRuntimeLoweringPass>();
}

} // namespace MIC
} // namespace mlir

// NOLINTNEXTLINE
static PassRegistration<NNToRuntimeLoweringPass> pass;
