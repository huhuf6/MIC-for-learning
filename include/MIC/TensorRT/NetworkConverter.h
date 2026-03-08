#pragma once

#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"
#include "MIC/TensorRT/TensorRTBuilder.h"
#include "NvInfer.h"
#include <memory>

namespace mlir {
namespace MIC {
namespace TensorRT {

/// 将 MLIR 操作转换为 TensorRT Network Layer。
///
/// 当前后端边界约定输入为运行时风格调用：
/// `func.call @mic_trt_matmul/...`，再映射到 TensorRT API。
class NetworkConverter {
private:
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  NetworkConverter(TensorRTBuilder &builder);
  ~NetworkConverter();

  /// 转换单个 MLIR 操作；遇到不支持或非法操作返回 failure。
  LogicalResult convertOperation(Operation *op);

  /// 正向映射：MLIR SSA Value -> TensorRT ITensor*。
  const DenseMap<Value, nvinfer1::ITensor *> &getValueMap() const;
};

} // namespace TensorRT
} // namespace MIC
} // namespace mlir
