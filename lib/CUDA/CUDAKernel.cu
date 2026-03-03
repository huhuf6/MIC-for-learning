//===----------------------------------------------------------------------===//
// CUDA Kernel实现
// 功能：实现神经网络中的LayerNorm和FusedLinearGELU操作的GPU加速
//===----------------------------------------------------------------------===//

#include "MIC/CUDA/CUDAKernel.h"
#include <cuda_runtime.h>
#include <math.h>

// LayerNorm CUDA Kernel
__global__ void layerNormKernel(const float *input, const float *scale, const float *bias, float *output, int batchSize, int hiddenSize, float epsilon) {
  int tid = blockIdx.x * blockDim.x + threadIdx.x;
  if (tid >= batchSize * hiddenSize) {
    return;
  }

  int batchIdx = tid / hiddenSize;

  // 计算当前样本的起始位置
  const float *sampleInput = input + batchIdx * hiddenSize;
  float *sampleOutput = output + batchIdx * hiddenSize;

  // 计算均值
  float mean = 0.0f;
  for (int i = 0; i < hiddenSize; ++i) {
    mean += sampleInput[i];
  }
  mean /= hiddenSize;

  // 计算方差
  float variance = 0.0f;
  for (int i = 0; i < hiddenSize; ++i) {
    float diff = sampleInput[i] - mean;
    variance += diff * diff;
  }
  variance /= hiddenSize;

  // 计算LayerNorm
  float invStd = 1.0f / sqrtf(variance + epsilon);
  for (int i = 0; i < hiddenSize; ++i) {
    float normalized = (sampleInput[i] - mean) * invStd;
    if (scale) {
      normalized *= scale[i];
    }
    if (bias) {
      normalized += bias[i];
    }
    sampleOutput[i] = normalized;
  }
}

// FusedLinearGELU CUDA Kernel
__global__ void fusedLinearGELUKernel(const float *input, const float *weight, const float *bias, float *output, int batchSize, int inFeatures, int outFeatures, float epsilon) {
  int tid = blockIdx.x * blockDim.x + threadIdx.x;
  if (tid >= batchSize * outFeatures) {
    return;
  }

  int batchIdx = tid / outFeatures;
  int outIdx = tid % outFeatures;

  // 计算线性变换
  float linearResult = 0.0f;
  for (int i = 0; i < inFeatures; ++i) {
    linearResult += input[batchIdx * inFeatures + i] * weight[outIdx * inFeatures + i];
  }

  // 添加偏置
  if (bias) {
    linearResult += bias[outIdx];
  }

  // 应用GELU激活函数
  // GELU(x) = 0.5 * x * (1 + erf(x / sqrt(2)))
  float x = linearResult;
  float geluResult = 0.5f * x * (1.0f + erff(x / 1.41421356237f));

  // 输出结果
  output[tid] = geluResult;
}

// 启动LayerNorm Kernel
void launchLayerNormKernel(const float *input, const float *scale, const float *bias, float *output, int batchSize, int hiddenSize, float epsilon, cudaStream_t stream) {
  int totalThreads = batchSize * hiddenSize;
  int blockSize = 256;
  int gridSize = (totalThreads + blockSize - 1) / blockSize;

  layerNormKernel<<<gridSize, blockSize, 0, stream>>>(input, scale, bias, output, batchSize, hiddenSize, epsilon);
}

// 启动FusedLinearGELU Kernel
void launchFusedLinearGELUKernel(const float *input, const float *weight, const float *bias, float *output, int batchSize, int inFeatures, int outFeatures, cudaStream_t stream) {
  int totalThreads = batchSize * outFeatures;
  int blockSize = 256;
  int gridSize = (totalThreads + blockSize - 1) / blockSize;

  fusedLinearGELUKernel<<<gridSize, blockSize, 0, stream>>>(input, weight, bias, output, batchSize, inFeatures, outFeatures, 1e-5f);
}
