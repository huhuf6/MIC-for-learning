#pragma once

#include "mlir/IR/Value.h"
#include "NvInfer.h"
#include <memory>

namespace mlir {
namespace MIC {
namespace TensorRT {

/// TensorRT Builder/Network/Config 的轻量 RAII 封装。
///
/// 典型用法：
/// 1) 通过 getter 获取 network/builder/config；
/// 2) 导入模型图；
/// 3) 调用 build() 生成序列化 engine 字节流。
class TensorRTBuilder {
private:
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  TensorRTBuilder();
  ~TensorRTBuilder();

  /// 获取可写的 TensorRT Network，用于添加层。
  nvinfer1::INetworkDefinition *getNetwork();
  /// 获取原生 TensorRT Builder，用于设置 profile 等构建细节。
  nvinfer1::IBuilder *getBuilder();
  /// 获取 BuilderConfig（精度开关/工作空间/profile 等）。
  nvinfer1::IBuilderConfig *getConfig();

  /// 构建并序列化当前 network。
  std::unique_ptr<nvinfer1::IHostMemory> build();

  /// 开关 FP16 精度模式。
  void setFP16Mode(bool enabled);
  /// 开关 INT8 精度模式。
  void setINT8Mode(bool enabled);
  /// 兼容旧版最大 batch 接口（显式 batch 网络可能忽略）。
  void setMaxBatchSize(int batchSize);
  /// 设置 tactic 选择可用的 workspace 内存预算。
  void setMaxWorkspaceSize(size_t size);
};

} // namespace TensorRT
} // namespace MIC
} // namespace mlir
