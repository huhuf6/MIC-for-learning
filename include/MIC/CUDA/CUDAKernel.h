//===----------------------------------------------------------------------===//
// CUDA Kernel头文件
// 功能：声明CUDA Kernel函数接口
//===----------------------------------------------------------------------===//

#ifndef MIC_CUDA_CUDAKERNEL_H
#define MIC_CUDA_CUDAKERNEL_H

#include <cuda_runtime.h>

namespace mlir {
namespace MIC {
namespace CUDA {

// 启动LayerNorm Kernel
// 参数：
//   input: 输入张量
//   scale: 缩放因子张量（可选）
//   bias: 偏置张量（可选）
//   output: 输出张量
//   batchSize: 批次大小
//   hiddenSize: 隐藏层大小
//   epsilon: 防止除零的小值
//   stream: CUDA流（可选）
void launchLayerNormKernel(const float *input, const float *scale, const float *bias, float *output, int batchSize, int hiddenSize, float epsilon, cudaStream_t stream);

// 启动FusedLinearGELU Kernel
// 参数：
//   input: 输入张量
//   weight: 权重张量
//   bias: 偏置张量（可选）
//   output: 输出张量
//   batchSize: 批次大小
//   inFeatures: 输入特征维度
//   outFeatures: 输出特征维度
//   stream: CUDA流（可选）
void launchFusedLinearGELUKernel(const float *input, const float *weight, const float *bias, float *output, int batchSize, int inFeatures, int outFeatures, cudaStream_t stream);

} // namespace CUDA
} // namespace MIC
} // namespace mlir

#endif // MIC_CUDA_CUDAKERNEL_H
