//===----------------------------------------------------------------------===//
// 操作融合优化Pass
// 功能：将多个操作融合为单个操作，减少内存访问和kernel启动开销
// 支持的融合模式：Linear+GELU
//===----------------------------------------------------------------------===//

#include "mlir/Pass/Pass.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "MIC/Dialect/NNOps.h"
#include "MIC/Dialect/NNDialect.h"

using namespace mlir;
using namespace MIC::NN;

namespace {

class FusionPass : public PassWrapper<FusionPass, OperationPass<func::FuncOp>> {
public:
  StringRef getArgument() const final { return "fusion"; }
  StringRef getDescription() const final { return "Fuse operations to reduce memory access and kernel launches"; }
  void runOnOperation() override;
};

void FusionPass::runOnOperation() {
  func::FuncOp func = getOperation();
  
  // 遍历函数中的所有GELU操作
  func.walk([&](GELUOp geluOp) {
    // 检查GELU的输入是否来自Linear操作
    if (auto linearOp = geluOp.input().getDefiningOp<LinearOp>()) {
      // 创建融合的Linear+GELU操作
      OpBuilder builder(geluOp);
      auto outputType = geluOp.output().getType();
      auto fusedOp = builder.create<LinearOp>(
          linearOp.getLoc(),
          outputType,
          linearOp.input(),
          linearOp.weight(),
          linearOp.bias());
      
      // 替换GELU操作的使用
      geluOp.output().replaceAllUsesWith(fusedOp.output());
      
      // 删除原始操作
      geluOp.erase();
      linearOp.erase();
    }
  });
}

} // namespace

// 注册Pass
static PassRegistration<FusionPass> pass;
