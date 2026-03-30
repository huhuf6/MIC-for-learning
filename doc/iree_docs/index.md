---
hide:
  - navigation
---

# IREE（中文）

IREE（**I**ntermediate **R**epresentation **E**xecution **E**nvironment）是一个基于 [MLIR](https://mlir.llvm.org/) 的端到端编译器与运行时系统。它可以把机器学习模型 lowering 到统一 IR，并同时覆盖数据中心场景与移动/边缘场景。

## 关键特性

- **AOT（预编译）**
  调度逻辑与执行逻辑一起编译，减少运行时开销。
- **支持复杂模型能力**
  包括动态 shape、控制流、流式执行等。
- **面向多种硬件**
  CPU / GPU / 其他加速器均可作为目标。
- **低开销管线化执行**
  编译阶段会引入执行所需的调度信息，减少系统调用与调度损耗。

## 项目架构（编译视角）

IREE 编译流程大体是：

1. 从上游框架导入（PyTorch/ONNX/TF/JAX 等）。
2. 在 MLIR 上执行多阶段优化与转换（Input/ABI/Flow/Stream/HAL/VM）。
3. 为目标后端生成可执行产物（如 `.vmfb`）。
4. 由 IREE Runtime 在目标设备上加载并执行。

## 从 ML 框架导入

常见路径：

- PyTorch：通过 `iree-turbine` / `torch-mlir` 导出并编译。
- ONNX：`iree-import-onnx` -> `iree-compile`。
- TensorFlow / TFLite：`iree-import-tf` / `iree-import-tflite` -> `iree-compile`。
- JAX：AOT 导出与 PJRT 插件路径（仍在持续演进）。

## 选择部署配置

IREE 支持不同部署组合（编译目标 + 运行时设备），例如：

- CPU（`llvm-cpu` + `local-sync/local-task`）
- Vulkan（`vulkan-spirv` + `vulkan`）
- CUDA（`cuda` + `cuda`）
- ROCm（`rocm` + `hip`）
- Metal（`metal-spirv` + `metal`）

## 一个最小编译示例

```bash
iree-compile model.mlir \
  --iree-hal-target-device=local \
  --iree-hal-local-target-device-backends=llvm-cpu \
  -o model.vmfb
```

## 工作流总览

1. 模型导出/导入成 MLIR。
2. `iree-compile` 生成 `.vmfb`。
3. `iree-run-module` 或应用侧 API 加载并推理。

