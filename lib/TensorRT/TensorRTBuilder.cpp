#include "MIC/TensorRT/TensorRTBuilder.h"
#include "NvInfer.h"

using namespace mlir;
using namespace mlir::MIC::TensorRT;

class SimpleLogger : public nvinfer1::ILogger {
  void log(Severity severity, const char* msg) noexcept override {
    if (severity <= Severity::kWARNING) {
    }
  }
};

static SimpleLogger gLogger;

class TensorRTBuilder::Impl {
private:
  nvinfer1::IBuilder *builder;
  nvinfer1::INetworkDefinition *network;
  nvinfer1::IBuilderConfig *config;

public:
  Impl() {
    builder = nvinfer1::createInferBuilder(gLogger);
    network = builder->createNetworkV2(1U << static_cast<uint32_t>(nvinfer1::NetworkDefinitionCreationFlag::kEXPLICIT_BATCH));
    config = builder->createBuilderConfig();
  }

  ~Impl() {
    if (config) {
      delete config;
    }
    if (network) {
      delete network;
    }
    if (builder) {
      delete builder;
    }
  }

  nvinfer1::IBuilder *getBuilder() {
    return builder;
  }

  nvinfer1::INetworkDefinition *getNetwork() {
    return network;
  }

  nvinfer1::IBuilderConfig *getConfig() {
    return config;
  }

  std::unique_ptr<nvinfer1::IHostMemory> build() {
    nvinfer1::IHostMemory *serializedModel = builder->buildSerializedNetwork(*network, *config);
    return std::unique_ptr<nvinfer1::IHostMemory>(serializedModel);
  }
};

TensorRTBuilder::TensorRTBuilder() : impl(std::make_unique<Impl>()) {}

TensorRTBuilder::~TensorRTBuilder() = default;

nvinfer1::IBuilder *TensorRTBuilder::getBuilder() {
  return impl->getBuilder();
}

nvinfer1::INetworkDefinition *TensorRTBuilder::getNetwork() {
  return impl->getNetwork();
}

nvinfer1::IBuilderConfig *TensorRTBuilder::getConfig() {
  return impl->getConfig();
}

std::unique_ptr<nvinfer1::IHostMemory> TensorRTBuilder::build() {
  return impl->build();
}

void TensorRTBuilder::setFP16Mode(bool enabled) {}
void TensorRTBuilder::setINT8Mode(bool enabled) {}
void TensorRTBuilder::setMaxBatchSize(int batchSize) {}
void TensorRTBuilder::setMaxWorkspaceSize(size_t size) {}
