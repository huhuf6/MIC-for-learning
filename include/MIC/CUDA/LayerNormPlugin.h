#pragma once

#include "NvInfer.h"
#include <memory>

namespace mlir {
namespace MIC {
namespace CUDA {

class LayerNormPlugin : public nvinfer1::IPluginV2 {
private:
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  LayerNormPlugin(float epsilon);
  LayerNormPlugin(const void *data, size_t length);
  ~LayerNormPlugin();

  // Inherited from IPluginV2
  const char *getPluginType() const noexcept override;
  const char *getPluginVersion() const noexcept override;
  int getNbOutputs() const noexcept override;
  nvinfer1::Dims getOutputDimensions(int index, const nvinfer1::Dims *inputs, int nbInputDims) noexcept override;
  int initialize() noexcept override;
  void terminate() noexcept override;
  size_t getWorkspaceSize(int maxBatchSize) const noexcept override;
  int enqueue(int batchSize, const void *const *inputs, void **outputs, void *workspace, cudaStream_t stream) noexcept override;
  size_t getSerializationSize() const noexcept override;
  void serialize(void *buffer) const noexcept override;
  void configureWithFormat(const nvinfer1::Dims *inputs, int nbInputs, const nvinfer1::Dims *outputs, int nbOutputs, nvinfer1::DataType type, nvinfer1::PluginFormat format, int maxBatchSize) noexcept override;
  bool supportsFormat(nvinfer1::DataType type, nvinfer1::PluginFormat format) const noexcept override;
  nvinfer1::IPluginV2 *clone() const noexcept override;
  void setPluginNamespace(const char *pluginNamespace) noexcept override;
  const char *getPluginNamespace() const noexcept override;

  // Custom methods
  void setWeights(nvinfer1::Weights scale, nvinfer1::Weights bias);
};

} // namespace CUDA
} // namespace MIC
} // namespace mlir
