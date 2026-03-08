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
    // TensorRT 10+ 已默认 explicit batch，旧 flag 在新版本中已废弃。
    // 使用 0 避免跨版本行为差异导致的不稳定问题。
    network = builder->createNetworkV2(0U);
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
