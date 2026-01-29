//===----------------------------------------------------------------------===//
// MIC Layout Transform Pass Implementation
//===----------------------------------------------------------------------===//

#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "MIC/Dialect/NNOps.h"

// 使用mlir命名空间
using namespace mlir;
// 使用MIC::NN命名空间，包含神经网络相关操作
using namespace MIC::NN;

namespace {

//===----------------------------------------------------------------------===//
// LayoutTransformPass类
// 功能：优化张量布局以提高性能
// 说明：该通道会根据硬件偏好转换张量布局，例如在NHWC和NCHW格式之间转换
//===----------------------------------------------------------------------===//
class LayoutTransformPass : public PassWrapper<LayoutTransformPass, OperationPass<func::FuncOp>> {
public:
  // 定义类型ID
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(LayoutTransformPass)

  // 获取通道的命令行参数名
  StringRef getArgument() const final { return "mic-layout-transform";
  }
  
  // 获取通道的描述
  StringRef getDescription() const final {
    return "Transform tensor layouts for better performance";
  }
  
  // 获取通道的名称
  StringRef getName() const final {
    return "LayoutTransformPass";
  }

  //===----------------------------------------------------------------------===//
  // 运行通道的主方法
  // 功能：设置并应用布局优化模式
  // 实现：
  // 1. 创建RewritePatternSet对象用于存储布局优化模式
  // 2. 添加Conv2D布局优化模式
  // 3. 贪婪地应用这些模式到函数中
  //===----------------------------------------------------------------------===//
  void runOnOperation() override {
    // 获取当前操作（函数）
    auto func = getOperation();
    // 获取上下文
    auto context = func.getContext();

    // 设置模式重写器
    RewritePatternSet patterns(context);
    // 添加Conv2D布局优化模式
    patterns.add<OptimizeConv2DLayoutPattern>(context);

    // 应用模式
    if (failed(applyPatternsAndFoldGreedily(func, std::move(patterns)))) {
      signalPassFailure();
    }
  }

private:
  //===----------------------------------------------------------------------===//
  // OptimizeConv2DLayoutPattern结构体
  // 功能：优化Conv2D操作的张量布局
  // 说明：检查Conv2D操作的输入布局，并根据硬件偏好进行优化
  //===----------------------------------------------------------------------===//
  struct OptimizeConv2DLayoutPattern : public OpRewritePattern<Conv2DOp> {
    using OpRewritePattern::OpRewritePattern;

    //===----------------------------------------------------------------------===//
    // 匹配并重写操作的方法
    // 功能：检查Conv2D操作的输入布局是否需要优化
    // 参数：
    //   - convOp: 当前要匹配的Conv2D操作
    //   - rewriter: 用于重写操作的PatternRewriter对象
    // 返回值：成功或失败
    //===----------------------------------------------------------------------===//
    LogicalResult matchAndRewrite(Conv2DOp convOp, PatternRewriter &rewriter) const override {
      // 检查输入是否是ranked tensor类型
      auto inputType = convOp.getInput().getType().dyn_cast<RankedTensorType>();
      if (!inputType) {
        return failure();
      }

      // 目前，我们只是验证布局
      // 在实际实现中，我们会根据硬件偏好在NHWC和NCHW之间转换
      // 例如，GPU通常偏好NCHW格式，而CPU可能偏好NHWC格式

      return success();
    }
  };
};

} // namespace

//===----------------------------------------------------------------------===//
// 通道注册
// 功能：注册LayoutTransformPass通道，使其可用于命令行和PassManager
//===----------------------------------------------------------------------===//
namespace MIC {
void registerLayoutTransformPass() {
  PassRegistration<LayoutTransformPass>();
}
}
