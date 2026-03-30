---
hide:
  - tags
tags:
  - ONNX
  - Python
  - PyTorch
icon: simple/onnx
---

# ONNX 支持

## 概览

ONNX 模型可通过 IREE 工具链部署：

1. `iree-import-onnx` 把 `.onnx` 转成 MLIR。
2. `iree-compile` 编译为目标工件（`.vmfb`）。
3. `iree-run-module` 或 API 执行。

## 依赖安装

```bash
python -m pip install iree-base-compiler[onnx] iree-base-runtime
```

## 快速开始

### 1) ONNX -> MLIR

```bash
iree-import-onnx model.onnx --opset-version 17 -o model.mlir
```

### 2) MLIR -> VMFB（CPU 示例）

```bash
iree-compile model.mlir \
  --iree-hal-target-device=local \
  --iree-hal-local-target-device-backends=llvm-cpu \
  --iree-llvmcpu-target-cpu=host \
  -o model_cpu.vmfb
```

### 3) 运行

```bash
iree-run-module \
  --module=model_cpu.vmfb \
  --device=local-task \
  --function=... \
  --input=...
```

## 常见问题

### `torch.operator` 非法化失败

若出现类似：

- `failed to legalize operation 'torch.operator'`

通常是：

1. 该 ONNX op 还未实现或缺某些 case。
2. 你的 ONNX opset 太旧，尝试升级到较新版本（例如 17）。

可先执行 ONNX version convert，再重新 import + compile。

