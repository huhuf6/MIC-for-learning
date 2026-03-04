#include "MIC/TensorRT/NetworkConverter.h"
#include "MIC/Dialect/NNOps.h"
#include "NvInfer.h"

using namespace mlir;
using namespace mlir::MIC::TensorRT;

class NetworkConverter::Impl {
private:
  TensorRTBuilder &builder;
  nvinfer1::INetworkDefinition *network;
  DenseMap<Value, nvinfer1::ITensor *> valueMap;

public:
  Impl(TensorRTBuilder &builder) : builder(builder) {
    network = builder.getNetwork();
  }

  LogicalResult convertOperation(Operation *op) {
    return success();
  }

  const DenseMap<Value, nvinfer1::ITensor *> &getValueMap() const {
    return valueMap;
  }
};

NetworkConverter::NetworkConverter(TensorRTBuilder &builder) : impl(std::make_unique<Impl>(builder)) {}

NetworkConverter::~NetworkConverter() = default;

LogicalResult NetworkConverter::convertOperation(Operation *op) {
  return impl->convertOperation(op);
}

const DenseMap<Value, nvinfer1::ITensor *> &NetworkConverter::getValueMap() const {
  return impl->getValueMap();
}
