#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "MIC/Dialect/NNOps.h"

using namespace mlir;
using namespace MIC::NN;

namespace {

class FusionPass : public PassWrapper<FusionPass, OperationPass<func::FuncOp>> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(FusionPass)

  StringRef getArgument() const final { return "mic-fusion";
  }
  StringRef getDescription() const final {
    return "Fuse neural network operations for better performance";
  }
  StringRef getName() const final {
    return "FusionPass";
  }

  void runOnOperation() override {
    auto func = getOperation();
    auto context = func.getContext();

    // Pattern rewriter setup
    RewritePatternSet patterns(context);
    patterns.add<FuseLinearGELUPattern>(context);
    patterns.add<FuseLinearLayerNormPattern>(context);

    // Apply patterns
    if (failed(applyPatternsAndFoldGreedily(func, std::move(patterns)))) {
      signalPassFailure();
    }
  }

private:
  // Fuse Linear -> GELU pattern
  struct FuseLinearGELUPattern : public OpRewritePattern<GELUOp> {
    using OpRewritePattern::OpRewritePattern;

    LogicalResult matchAndRewrite(GELUOp geluOp, PatternRewriter &rewriter) const override {
      // Check if GELU's input is the output of a LinearOp
      auto linearOp = geluOp.getInput().getDefiningOp<LinearOp>();
      if (!linearOp) {
        return failure();
      }

      // Create fused linear+gelu operation
      SmallVector<Value, 3> operands;
      operands.push_back(linearOp.getInput());
      operands.push_back(linearOp.getWeight());
      if (linearOp.getBias()) {
        operands.push_back(linearOp.getBias());
      }

      // Create new operation (using existing ops for now)
      // Note: In a real implementation, we would define a fused op
      auto linearResult = rewriter.create<LinearOp>(
          linearOp.getLoc(), linearOp.getInput(), linearOp.getWeight(), linearOp.getBias());
      auto fusedResult = rewriter.create<GELUOp>(geluOp.getLoc(), linearResult.getOutput());

      rewriter.replaceOp(geluOp, fusedResult.getOutput());
      return success();
    }
  };

  // Fuse Linear -> LayerNorm pattern
  struct FuseLinearLayerNormPattern : public OpRewritePattern<LayerNormOp> {
    using OpRewritePattern::OpRewritePattern;

    LogicalResult matchAndRewrite(LayerNormOp lnOp, PatternRewriter &rewriter) const override {
      // Check if LayerNorm's input is the output of a LinearOp
      auto linearOp = lnOp.getInput().getDefiningOp<LinearOp>();
      if (!linearOp) {
        return failure();
      }

      // Create fused linear+layernorm operation
      auto linearResult = rewriter.create<LinearOp>(
          linearOp.getLoc(), linearOp.getInput(), linearOp.getWeight(), linearOp.getBias());
      auto fusedResult = rewriter.create<LayerNormOp>(
          lnOp.getLoc(), linearResult.getOutput(), lnOp.getScale(), lnOp.getBias(), lnOp.getEpsilon());

      rewriter.replaceOp(lnOp, fusedResult.getOutput());
      return success();
    }
  };
};

} // namespace

// Register the pass
namespace MIC {
void registerFusionPass() {
  PassRegistration<FusionPass>();
}
}
