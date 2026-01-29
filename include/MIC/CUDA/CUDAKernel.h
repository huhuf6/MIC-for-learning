#pragma once

#include "mlir/IR/Value.h"
#include <cuda_runtime.h>

namespace mlir {
namespace MIC {
namespace CUDA {

void launchLayerNormKernel(const float *input, const float *scale, const float *bias, float *output, int batchSize, int hiddenSize, float epsilon, cudaStream_t stream);

void launchFusedLinearGELUKernel(const float *input, const float *weight, const float *bias, float *output, int batchSize, int inFeatures, int outFeatures, cudaStream_t stream);

} // namespace CUDA
} // namespace MIC
} // namespace mlir
