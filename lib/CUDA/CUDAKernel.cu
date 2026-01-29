#include "MIC/CUDA/CUDAKernel.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

using namespace mlir;
using namespace MIC::CUDA;

__global__ void layerNormKernel(const float *input, const float *scale, const float *bias, float *output, int batchSize, int hiddenSize, float epsilon) {
  int tid = blockIdx.x * blockDim.x + threadIdx.x;
  if (tid >= batchSize * hiddenSize) {
    return;
  }

  int batchIdx = tid / hiddenSize;
  int hiddenIdx = tid % hiddenSize;

  // Calculate mean
  float mean = 0.0f;
  for (int i = 0; i < hiddenSize; ++i) {
    mean += input[batchIdx * hiddenSize + i];
  }
  mean /= hiddenSize;

  // Calculate variance
  float var = 0.0f;
  for (int i = 0; i < hiddenSize; ++i) {
    float diff = input[batchIdx * hiddenSize + i] - mean;
    var += diff * diff;
  }
  var /= hiddenSize;

  // Normalize
  float normalized = (input[tid] - mean) / sqrtf(var + epsilon);

  // Apply scale and bias
  if (scale && bias) {
    output[tid] = normalized * scale[hiddenIdx] + bias[hiddenIdx];
  } else if (scale) {
    output[tid] = normalized * scale[hiddenIdx];
  } else if (bias) {
    output[tid] = normalized + bias[hiddenIdx];
  } else {
    output[tid] = normalized;
  }
}

__global__ void fusedLinearGELUKernel(const float *input, const float *weight, const float *bias, float *output, int batchSize, int inFeatures, int outFeatures) {
  int tid = blockIdx.x * blockDim.x + threadIdx.x;
  if (tid >= batchSize * outFeatures) {
    return;
  }

  int batchIdx = tid / outFeatures;
  int outIdx = tid % outFeatures;

  // Linear transformation
  float linearOut = 0.0f;
  for (int i = 0; i < inFeatures; ++i) {
    linearOut += input[batchIdx * inFeatures + i] * weight[outIdx * inFeatures + i];
  }
  if (bias) {
    linearOut += bias[outIdx];
  }

  // GELU activation
  // Approximation: 0.5 * x * (1 + tanh(sqrt(2/pi) * (x + 0.044715 * x^3)))
  float x = linearOut;
  float geluOut = 0.5f * x * (1.0f + tanhf(sqrtf(2.0f / M_PI) * (x + 0.044715f * x * x * x)));

  output[tid] = geluOut;
}

void MIC::CUDA::launchLayerNormKernel(const float *input, const float *scale, const float *bias, float *output, int batchSize, int hiddenSize, float epsilon, cudaStream_t stream) {
  int numThreads = 256;
  int numBlocks = (batchSize * hiddenSize + numThreads - 1) / numThreads;

  layerNormKernel<<<numBlocks, numThreads, 0, stream>>>(input, scale, bias, output, batchSize, hiddenSize, epsilon);
}

void MIC::CUDA::launchFusedLinearGELUKernel(const float *input, const float *weight, const float *bias, float *output, int batchSize, int inFeatures, int outFeatures, cudaStream_t stream) {
  int numThreads = 256;
  int numBlocks = (batchSize * outFeatures + numThreads - 1) / numThreads;

  fusedLinearGELUKernel<<<numBlocks, numThreads, 0, stream>>>(input, weight, bias, output, batchSize, inFeatures, outFeatures);
}
