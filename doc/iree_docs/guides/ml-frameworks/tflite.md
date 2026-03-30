---
hide:
  - tags
tags:
  - Python
  - TensorFlow
icon: simple/tensorflow
---

# TensorFlow Lite（LiteRT）集成

!!! warning

    TFLite 支持依赖 TOSA 方言。该链路在版本迁移期间可能不稳定。
    详情可见：<https://github.com/iree-org/iree/issues/19777>。

## :octicons-book-16: 概览

IREE 支持编译和运行 TensorFlow Lite FlatBuffer（`.tflite`）模型：

1. `iree-import-tflite` 导入为 MLIR（TOSA）
2. `iree-compile` 编译到目标后端
3. Runtime 加载执行

## :octicons-download-16: 前置依赖

1. 安装 TensorFlow（与文档兼容版本）：

```shell
python -m pip install "tensorflow<=2.18.0"
```

2. 安装 IREE 组件：

=== "Stable releases"

    ```shell
    python -m pip install \
      "iree-base-compiler<=3.1.0" \
      "iree-base-runtime<=3.1.0" \
      "iree-tools-tflite<=20250107.1133"
    ```

=== ":material-alert: Nightly releases"

    ```shell
    python -m pip install \
      --find-links https://iree.dev/pip-release-links.html \
      --upgrade iree-base-compiler iree-base-runtime iree-tools-tflite
    ```

## :octicons-package-dependents-16: 导入与编译

### Using Command Line Tools

```shell
# 获取一个 tflite 模型（示例）
# 例如来自 Kaggle: https://www.kaggle.com/models/tensorflow/posenet-mobilenet

# 1) 导入为 MLIR（TOSA）
iree-import-tflite model.tflite -o model_tosa.mlir

# 2) 编译 CPU 后端
iree-compile model_tosa.mlir \
  --iree-hal-target-backends=llvm-cpu \
  -o model.vmfb
```

### Using the Python API

```python
import numpy as np
import iree.compiler as ireec
import iree.runtime as ireert

# 下载/读取模型
tflite_bytes = open("model.tflite", "rb").read()

# 编译（可选保存中间 tflite/tosa 结果用于调试）
vmfb = ireec.compile_str(
    tflite_bytes,
    input_type="tosa",
    target_backends=["llvm-cpu"],
)

# 配置 runtime 并加载模块
config = ireert.Config("local-task")
vm_module = ireert.VmModule.from_flatbuffer(config.vm_instance, vmfb)

# 准备输入并执行（大多 TFLite 模型默认入口为 main）
# 这里只演示调用方式，输入 shape/dtype 以模型实际要求为准
```

## :octicons-code-16: 示例

- IREE `samples/` 与 `integrations/` 中的 TFLite 示例
- 官方文档中的端到端命令行示例

## :octicons-question-16: 故障排查

常见问题：

- 算子未覆盖：检查模型是否包含 IREE 尚未支持的 TOSA/TFLite 算子
- 版本不匹配：按文档固定 TensorFlow 与 iree-tools-tflite 版本
- 输入签名不一致：确认 `main` 的输入 shape/dtype 与喂入数据一致
