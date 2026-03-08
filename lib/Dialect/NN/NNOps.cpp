//===----------------------------------------------------------------------===//
// MIC Neural Network (NN) Operations Implementation
//===----------------------------------------------------------------------===//

#include "MIC/Dialect/NNOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/TypeUtilities.h"

// 使用mlir命名空间
using namespace mlir;
// 使用MIC::NN命名空间，包含神经网络相关操作
using namespace MIC::NN;

//===----------------------------------------------------------------------===//
// TableGen生成的实现
// 说明：包含TableGen生成的操作实现代码
//===----------------------------------------------------------------------===//
#include "NN.cpp.inc"
