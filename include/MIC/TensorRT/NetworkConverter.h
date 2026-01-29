#pragma once

#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"
#include "MIC/TensorRT/TensorRTBuilder.h"
#include "NvInfer.h"
#include <memory>

namespace mlir {
namespace MIC {
namespace TensorRT {

class NetworkConverter {
private:
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  NetworkConverter(TensorRTBuilder &builder);
  ~NetworkConverter();

  LogicalResult convertOperation(Operation *op);

  const DenseMap<Value, nvinfer1::ITensor *> &getValueMap() const;
};

} // namespace TensorRT
} // namespace MIC
} // namespace mlir
