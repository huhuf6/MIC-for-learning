//===----------------------------------------------------------------------===//
// MIC Fusion Pass Implementation
//===----------------------------------------------------------------------===//

#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Support/TypeID.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "MIC/Dialect/NNOps.h"

// 使用mlir命名空间
using namespace mlir;
// 使用MIC::NN命名空间，包含神经网络相关操作
using namespace MIC::NN;

namespace {

//===----------------------------------------------------------------------===//
// FusionPass类
// 功能：实现神经网络操作融合优化，提高执行性能
// 说明：该通道会识别并融合神经网络中的常见操作组合，如Linear+GELU、Linear+LayerNorm
//===----------------------------------------------------------------------===//
class FusionPass : public PassWrapper<FusionPass, OperationPass<func::FuncOp>> {
public:
  // 定义类型ID
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(FusionPass)

  // 获取通道的命令行参数名
  StringRef getArgument() const final { return "mic-fusion";
  }
  
  // 获取通道的描述
  StringRef getDescription() const final {
    return "Fuse neural network operations for better performance";
  }
  
  // 获取通道的名称
  StringRef getName() const final {
    return "FusionPass";
  }

  //===----------------------------------------------------------------------===//
  // 运行通道的主方法
  // 功能：设置并应用操作融合模式
  // 实现：
  // 1. 创建RewritePatternSet对象用于存储融合模式
  // 2. 添加Linear+GELU和Linear+LayerNorm的融合模式
  // 3. 贪婪地应用这些模式到函数中
  //===----------------------------------------------------------------------===//
  void runOnOperation() override {
    // 获取当前操作（函数）
    auto func = getOperation();
    // 获取上下文
    auto context = func.getContext();

    // 设置模式重写器
    RewritePatternSet patterns(context);
    // 添加Linear+GELU融合模式
    patterns.add<FuseLinearGELUPattern>(context);
    // 添加Linear+LayerNorm融合模式
    patterns.add<FuseLinearLayerNormPattern>(context);

    // 应用模式
    if (failed(applyPatternsAndFoldGreedily(func, std::move(patterns)))) {
      signalPassFailure();
    }
  }

private:
  //===----------------------------------------------------------------------===//
  // FuseLinearGELUPattern结构体
  // 功能：融合Linear -> GELU操作序列
  // 说明：识别GELU操作的输入是否来自Linear操作，如果是则尝试融合这两个操作
  //===----------------------------------------------------------------------===//
  struct FuseLinearGELUPattern : public OpRewritePattern<GELUOp> {
    using OpRewritePattern::OpRewritePattern;

    //===----------------------------------------------------------------------===//
    // 匹配并重写操作的方法
    // 功能：检查GELU操作是否可以与前面的Linear操作融合
    // 参数：
    //   - geluOp: 当前要匹配的GELU操作
    //   - rewriter: 用于重写操作的PatternRewriter对象
    // 返回值：成功或失败
    //===----------------------------------------------------------------------===//
    LogicalResult matchAndRewrite(GELUOp geluOp, PatternRewriter &rewriter) const override {
      // 检查GELU的输入是否是LinearOp的输出
      auto linearOp = geluOp.input().getDefiningOp<LinearOp>();
      if (!linearOp) {
        return failure();
      }

      // 创建融合的linear+gelu操作
      SmallVector<Value, 3> operands;
      operands.push_back(linearOp.input());
      operands.push_back(linearOp.weight());
      if (linearOp.bias()) {
        operands.push_back(linearOp.bias());
      }

      // 创建新操作（目前使用现有操作）
      // 注意：在实际实现中，我们会定义一个融合操作
      auto linearResult = rewriter.create<LinearOp>(
          linearOp.getLoc(), linearOp.input(), linearOp.weight(), linearOp.bias());
      auto fusedResult = rewriter.create<GELUOp>(geluOp.getLoc(), linearResult.output());

      // 用融合后的结果替换原GELU操作
      rewriter.replaceOp(geluOp, fusedResult.output());
      return success();
    }
  };

  //===----------------------------------------------------------------------===//
  // FuseLinearLayerNormPattern结构体
  // 功能：融合Linear -> LayerNorm操作序列
  // 说明：识别LayerNorm操作的输入是否来自Linear操作，如果是则尝试融合这两个操作
  //===----------------------------------------------------------------------===//
  struct FuseLinearLayerNormPattern : public OpRewritePattern<LayerNormOp> {
    using OpRewritePattern::OpRewritePattern;

    //===----------------------------------------------------------------------===//
    // 匹配并重写操作的方法
    // 功能：检查LayerNorm操作是否可以与前面的Linear操作融合
    // 参数：
    //   - lnOp: 当前要匹配的LayerNorm操作
    //   - rewriter: 用于重写操作的PatternRewriter对象
    // 返回值：成功或失败
    //===----------------------------------------------------------------------===//
    LogicalResult matchAndRewrite(LayerNormOp lnOp, PatternRewriter &rewriter) const override {
      // 检查LayerNorm的输入是否是LinearOp的输出
      auto linearOp = lnOp.input().getDefiningOp<LinearOp>();
      if (!linearOp) {
        return failure();
      }

      // 创建融合的linear+layernorm操作
      auto linearResult = rewriter.create<LinearOp>(
          linearOp.getLoc(), linearOp.input(), linearOp.weight(), linearOp.bias());
      auto fusedResult = rewriter.create<LayerNormOp>(
          lnOp.getLoc(), linearResult.output(), lnOp.scale(), lnOp.bias(), lnOp.epsilonAttr());

      // 用融合后的结果替换原LayerNorm操作
      rewriter.replaceOp(lnOp, fusedResult.output());
      return success();
    }
  };
};

} // namespace

//===----------------------------------------------------------------------===//
// 通道注册
// 功能：注册FusionPass通道，使其可用于命令行和PassManager
//===----------------------------------------------------------------------===//
namespace MIC {
void registerFusionPass() {
  PassRegistration<FusionPass>();
}
}
