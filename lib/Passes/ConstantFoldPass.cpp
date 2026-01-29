#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/FoldUtils.h"
#include "MIC/Dialect/NNOps.h"

using namespace mlir;
using namespace MIC::NN;

namespace {

class ConstantFoldPass : public PassWrapper<ConstantFoldPass, OperationPass<func::FuncOp>> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ConstantFoldPass)

  StringRef getArgument() const final { return "mic-constant-fold";
  }
  StringRef getDescription() const final {
    return "Fold constant operations for better performance";
  }
  StringRef getName() const final {
    return "ConstantFoldPass";
  }

  void runOnOperation() override {
    auto func = getOperation();
    auto context = func.getContext();

    // Create folder
    OperationFolder folder(context);

    // Walk through operations and fold constants
    func.walk([&](Operation *op) {
      // Skip operations that don't produce results
      if (op->getNumResults() == 0) {
        return;
      }

      // Try to fold the operation
      SmallVector<Value, 4> foldedValues;
      if (succeeded(folder.tryToFold(op, foldedValues))) {
        // Replace the operation with folded values
        if (foldedValues.size() == op->getNumResults()) {
          for (unsigned i = 0; i < foldedValues.size(); ++i) {
            if (foldedValues[i] != op->getResult(i)) {
              op->getResult(i).replaceAllUsesWith(foldedValues[i]);
            }
          }
        }
      }
    });
  }
};

} // namespace

// Register the pass
namespace MIC {
void registerConstantFoldPass() {
  PassRegistration<ConstantFoldPass>();
}
}
