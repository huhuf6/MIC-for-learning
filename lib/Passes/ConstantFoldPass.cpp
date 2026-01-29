//===----------------------------------------------------------------------===//
// MIC Constant Fold Pass Implementation
//===----------------------------------------------------------------------===//

#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/FoldUtils.h"
#include "MIC/Dialect/NNOps.h"

// 使用mlir命名空间
using namespace mlir;
// 使用MIC::NN命名空间，包含神经网络相关操作
using namespace MIC::NN;

namespace {

//===----------------------------------------------------------------------===//
// ConstantFoldPass类
// 功能：实现常量折叠优化，通过预计算常量表达式提高性能
// 说明：该通道会遍历函数中的所有操作，尝试将常量操作折叠为单一常量值
//===----------------------------------------------------------------------===//
class ConstantFoldPass : public PassWrapper<ConstantFoldPass, OperationPass<func::FuncOp>> {
public:
  // 定义类型ID
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ConstantFoldPass)

  // 获取通道的命令行参数名
  StringRef getArgument() const final { return "mic-constant-fold";
  }
  
  // 获取通道的描述
  StringRef getDescription() const final {
    return "Fold constant operations for better performance";
  }
  
  // 获取通道的名称
  StringRef getName() const final {
    return "ConstantFoldPass";
  }

  //===----------------------------------------------------------------------===//
  // 运行通道的主方法
  // 功能：遍历函数中的所有操作，尝试折叠常量
  // 实现：
  // 1. 创建OperationFolder对象用于折叠操作
  // 2. 遍历函数中的所有操作
  // 3. 对每个有结果的操作尝试进行折叠
  // 4. 用折叠后的值替换原操作的结果
  //===----------------------------------------------------------------------===//
  void runOnOperation() override {
    // 获取当前操作（函数）
    auto func = getOperation();
    // 获取上下文
    auto context = func.getContext();

    // 创建操作折叠器
    OperationFolder folder(context);

    // 遍历操作并折叠常量
    func.walk([&](Operation *op) {
      // 跳过不产生结果的操作
      if (op->getNumResults() == 0) {
        return;
      }

      // 尝试折叠操作
      SmallVector<Value, 4> foldedValues;
      if (succeeded(folder.tryToFold(op, foldedValues))) {
        // 用折叠后的值替换原操作
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

//===----------------------------------------------------------------------===//
// 通道注册
// 功能：注册ConstantFoldPass通道，使其可用于命令行和PassManager
//===----------------------------------------------------------------------===//
namespace MIC {
void registerConstantFoldPass() {
  PassRegistration<ConstantFoldPass>();
}
}
