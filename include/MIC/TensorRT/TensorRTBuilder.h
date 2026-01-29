#pragma once

#include "mlir/IR/Value.h"
#include "NvInfer.h"
#include <memory>

namespace mlir {
namespace MIC {
namespace TensorRT {

class TensorRTBuilder {
private:
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  TensorRTBuilder();
  ~TensorRTBuilder();

  nvinfer1::INetworkDefinition *getNetwork();
  nvinfer1::IBuilder *getBuilder();
  nvinfer1::IBuilderConfig *getConfig();

  std::unique_ptr<nvinfer1::IHostMemory> build();

  void setFP16Mode(bool enabled);
  void setINT8Mode(bool enabled);
  void setMaxBatchSize(int batchSize);
  void setMaxWorkspaceSize(size_t size);
};

} // namespace TensorRT
} // namespace MIC
} // namespace mlir
