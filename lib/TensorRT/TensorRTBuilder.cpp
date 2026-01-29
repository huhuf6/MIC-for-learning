#include "MIC/TensorRT/TensorRTBuilder.h"
#include "NvInfer.h"
#include "NvInferBuilder.h"

using namespace mlir;
using namespace MIC::TensorRT;

class TensorRTBuilder::Impl {
private:
  nvinfer1::IBuilder *builder;
  nvinfer1::INetworkDefinition *network;
  nvinfer1::IBuilderConfig *config;

public:
  Impl() {
    builder = nvinfer1::createInferBuilder(nvinfer1::ILogger::Severity::kINFO);
    network = builder->createNetworkV2(1U << static_cast<uint32_t>(nvinfer1::NetworkDefinitionCreationFlag::kEXPLICIT_BATCH));
    config = builder->createBuilderConfig();
  }

  ~Impl() {
    if (config) config->destroy();
    if (network) network->destroy();
    if (builder) builder->destroy();
  }

  nvinfer1::INetworkDefinition *getNetwork() {
    return network;
  }

  nvinfer1::IBuilder *getBuilder() {
    return builder;
  }

  nvinfer1::IBuilderConfig *getConfig() {
    return config;
  }

  std::unique_ptr<nvinfer1::IHostMemory> build() {
    return std::unique_ptr<nvinfer1::IHostMemory>(
        builder->buildSerializedNetwork(*network, *config));
  }
};

TensorRTBuilder::TensorRTBuilder() : impl(std::make_unique<Impl>()) {}

TensorRTBuilder::~TensorRTBuilder() = default;

nvinfer1::INetworkDefinition *TensorRTBuilder::getNetwork() {
  return impl->getNetwork();
}

nvinfer1::IBuilder *TensorRTBuilder::getBuilder() {
  return impl->getBuilder();
}

nvinfer1::IBuilderConfig *TensorRTBuilder::getConfig() {
  return impl->getConfig();
}

std::unique_ptr<nvinfer1::IHostMemory> TensorRTBuilder::build() {
  return impl->build();
}

void TensorRTBuilder::setFP16Mode(bool enabled) {
  if (enabled) {
    impl->getConfig()->setFlag(nvinfer1::BuilderFlag::kFP16);
  }
}

void TensorRTBuilder::setINT8Mode(bool enabled) {
  if (enabled) {
    impl->getConfig()->setFlag(nvinfer1::BuilderFlag::kINT8);
  }
}

void TensorRTBuilder::setMaxBatchSize(int batchSize) {
  impl->getBuilder()->setMaxBatchSize(batchSize);
}

void TensorRTBuilder::setMaxWorkspaceSize(size_t size) {
  impl->getConfig()->setMaxWorkspaceSize(size);
}
