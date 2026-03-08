//===----------------------------------------------------------------------===//
// MIC Neural Network (NN) Dialect Implementation
//===----------------------------------------------------------------------===//

#include "MIC/Dialect/NNDialect.h"
#include "mlir/IR/DialectImplementation.h"

// 使用mlir命名空间
using namespace mlir;

//===----------------------------------------------------------------------===//
// NNDialect初始化函数
// 功能：注册NN方言中的所有操作
// 说明：通过包含TableGen生成的操作列表，将所有NN操作添加到方言中
//===----------------------------------------------------------------------===//
namespace MIC {
namespace NN {

void NNDialect::initialize() {
  addOperations<
    #include "NN.cpp.inc"
  >();
}

} // namespace NN
} // namespace MIC

//===----------------------------------------------------------------------===//
// 方言注册
// 说明：包含TableGen生成的方言注册代码
//===----------------------------------------------------------------------===//
#include "NNDialect.cpp.inc"
