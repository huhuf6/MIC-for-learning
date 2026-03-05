#include "MIC/CUDA/LayerNormPlugin.h"
#include "MIC/CUDA/CUDAKernel.h"
#include "NvInfer.h"
#include "NvInferPluginUtils.h"
#include <cuda_runtime.h>

using namespace mlir;
using namespace MIC::CUDA;

class LayerNormPlugin::Impl {
private:
  float epsilon;
  nvinfer1::Weights scaleWeights;
  nvinfer1::Weights biasWeights;

public:
  Impl(float epsilon) : epsilon(epsilon) {
    scaleWeights.type = nvinfer1::DataType::kFLOAT;
    scaleWeights.values = nullptr;
    scaleWeights.count = 0;
    
    biasWeights.type = nvinfer1::DataType::kFLOAT;
    biasWeights.values = nullptr;
    biasWeights.count = 0;
  }

  Impl(const void *data, size_t length) {
    // Deserialize plugin
    const char *ptr = static_cast<const char *>(data);
    epsilon = *reinterpret_cast<const float *>(ptr);
    ptr += sizeof(float);

    // Deserialize scale weights
    scaleWeights.count = *reinterpret_cast<const size_t *>(ptr);
    ptr += sizeof(size_t);
    scaleWeights.type = nvinfer1::DataType::kFLOAT;
    scaleWeights.values = ptr;
    ptr += scaleWeights.count * sizeof(float);

    // Deserialize bias weights
    biasWeights.count = *reinterpret_cast<const size_t *>(ptr);
    ptr += sizeof(size_t);
    biasWeights.type = nvinfer1::DataType::kFLOAT;
    biasWeights.values = ptr;
  }

  size_t getSerializationSize() const {
    return sizeof(float) + sizeof(size_t) + scaleWeights.count * sizeof(float) +
           sizeof(size_t) + biasWeights.count * sizeof(float);
  }

  void serialize(void *buffer) const {
    char *ptr = static_cast<char *>(buffer);
    *reinterpret_cast<float *>(ptr) = epsilon;
    ptr += sizeof(float);

    *reinterpret_cast<size_t *>(ptr) = scaleWeights.count;
    ptr += sizeof(size_t);
    memcpy(ptr, scaleWeights.values, scaleWeights.count * sizeof(float));
    ptr += scaleWeights.count * sizeof(float);

    *reinterpret_cast<size_t *>(ptr) = biasWeights.count;
    ptr += sizeof(size_t);
    memcpy(ptr, biasWeights.values, biasWeights.count * sizeof(float));
  }

  void setWeights(nvinfer1::Weights scale, nvinfer1::Weights bias) {
    scaleWeights = scale;
    biasWeights = bias;
  }

  float getEpsilon() const {
    return epsilon;
  }

  const nvinfer1::Weights &getScaleWeights() const {
    return scaleWeights;
  }

  const nvinfer1::Weights &getBiasWeights() const {
    return biasWeights;
  }
};

LayerNormPlugin::LayerNormPlugin(float epsilon) : impl(std::make_unique<Impl>(epsilon)) {}

LayerNormPlugin::LayerNormPlugin(const void *data, size_t length) : impl(std::make_unique<Impl>(data, length)) {}

LayerNormPlugin::~LayerNormPlugin() = default;

const char *LayerNormPlugin::getPluginType() const noexcept {
  return "LayerNormPlugin";
}

const char *LayerNormPlugin::getPluginVersion() const noexcept {
  return "1.0";
}

int LayerNormPlugin::getNbOutputs() const noexcept {
  return 1;
}

nvinfer1::Dims LayerNormPlugin::getOutputDimensions(int index, const nvinfer1::Dims *inputs, int nbInputDims) noexcept {
  return inputs[0];
}

int LayerNormPlugin::initialize() noexcept {
  return 0;
}

void LayerNormPlugin::terminate() noexcept {
}

size_t LayerNormPlugin::getWorkspaceSize(int maxBatchSize) const noexcept {
  return 0;
}

int LayerNormPlugin::enqueue(int batchSize, const void *const *inputs, void *const *outputs, void *workspace, cudaStream_t stream) noexcept {
  auto inputDims = getOutputDimensions(0, nullptr, 0);
  int hiddenSize = inputDims.d[1];
  
  // Launch CUDA kernel for LayerNorm
  launchLayerNormKernel(
      static_cast<const float *>(inputs[0]),
      static_cast<const float *>(impl->getScaleWeights().values),
      static_cast<const float *>(impl->getBiasWeights().values),
      static_cast<float *>(outputs[0]),
      batchSize,
      hiddenSize,
      impl->getEpsilon(),
      stream
  );
  return 0;
}

size_t LayerNormPlugin::getSerializationSize() const noexcept {
  return impl->getSerializationSize();
}

void LayerNormPlugin::serialize(void *buffer) const noexcept {
  impl->serialize(buffer);
}

void LayerNormPlugin::configureWithFormat(const nvinfer1::Dims *inputs, int nbInputs, const nvinfer1::Dims *outputs, int nbOutputs, nvinfer1::DataType type, nvinfer1::PluginFormat format, int maxBatchSize) noexcept {
}

bool LayerNormPlugin::supportsFormat(nvinfer1::DataType type, nvinfer1::PluginFormat format) const noexcept {
  return type == nvinfer1::DataType::kFLOAT && format == nvinfer1::PluginFormat::kLINEAR;
}

void LayerNormPlugin::setWeights(nvinfer1::Weights scale, nvinfer1::Weights bias) {
  impl->setWeights(scale, bias);
}

nvinfer1::IPluginV2 *LayerNormPlugin::clone() const noexcept {
  auto plugin = new LayerNormPlugin(impl->getEpsilon());
  plugin->setWeights(impl->getScaleWeights(), impl->getBiasWeights());
  return plugin;
}

void LayerNormPlugin::setPluginNamespace(const char *pluginNamespace) noexcept {
}

const char *LayerNormPlugin::getPluginNamespace() const noexcept {
  return "MIC";
}

// Plugin creator
class LayerNormPluginCreator : public nvinfer1::IPluginCreator {
private:
  static const char *PluginName;
  static const char *PluginVersion;
  static const nvinfer1::PluginFieldCollection FieldCollection;
  static std::vector<nvinfer1::PluginField> Fields;

public:
  LayerNormPluginCreator() {}

  const char *getPluginName() const noexcept override {
    return PluginName;
  }

  const char *getPluginVersion() const noexcept override {
    return PluginVersion;
  }

  const nvinfer1::PluginFieldCollection *getFieldNames() noexcept override {
    return &FieldCollection;
  }

  nvinfer1::IPluginV2 *createPlugin(const char *name, const nvinfer1::PluginFieldCollection *fc) noexcept override {
    float epsilon = 1e-5f;
    for (int i = 0; i < fc->nbFields; ++i) {
      const char *fieldName = fc->fields[i].name;
      if (strcmp(fieldName, "epsilon") == 0) {
        epsilon = *static_cast<const float *>(fc->fields[i].data);
      }
    }
    return new LayerNormPlugin(epsilon);
  }

  nvinfer1::IPluginV2 *deserializePlugin(const char *name, const void *serialData, size_t serialLength) noexcept override {
    return new LayerNormPlugin(serialData, serialLength);
  }

  void setPluginNamespace(const char *pluginNamespace) noexcept override {
  }

  const char *getPluginNamespace() const noexcept override {
    return "MIC";
  }
};

const char *LayerNormPluginCreator::PluginName = "LayerNormPlugin";
const char *LayerNormPluginCreator::PluginVersion = "1.0";
std::vector<nvinfer1::PluginField> LayerNormPluginCreator::Fields = {
  nvinfer1::PluginField("epsilon", nullptr, nvinfer1::PluginFieldType::kFLOAT32, 1)
};
const nvinfer1::PluginFieldCollection LayerNormPluginCreator::FieldCollection = {
  static_cast<int>(LayerNormPluginCreator::Fields.size()),
  LayerNormPluginCreator::Fields.data()
};

// Register plugin
REGISTER_TENSORRT_PLUGIN(LayerNormPluginCreator);
