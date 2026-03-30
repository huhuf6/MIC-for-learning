---
hide:
  - tags
tags:
  - Python
  - TensorFlow
icon: simple/tensorflow
---

# TensorFlow 集成

## :octicons-book-16: 概览

IREE 支持编译与运行 TensorFlow 程序，输入可以是：

- `tf.Module` 类
- `SavedModel` 格式

典型流程：`iree-import-tf` 导入为 MLIR（StableHLO）-> `iree-compile` 编译 -> Runtime 执行。

## :octicons-download-16: 前置依赖

1. 安装 TensorFlow：

```shell
python -m pip install tensorflow
```

2. 安装 IREE 组件（源码构建或 pip）：

=== "Stable releases"

    ```shell
    python -m pip install \
      iree-base-compiler \
      iree-base-runtime \
      iree-tools-tf
    ```

=== ":material-alert: Nightly releases"

    ```shell
    python -m pip install \
      --find-links https://iree.dev/pip-release-links.html \
      --upgrade \
      iree-base-compiler \
      iree-base-runtime \
      iree-tools-tf
    ```

## :octicons-package-dependents-16: 导入模型

### From SavedModel on TensorFlow Hub

可直接从本地 SavedModel 或 TF Hub 模型导入。

#### Using the command-line tool

```shell
# 1) 导入 SavedModel 到 MLIR
iree-import-tf \
  --tf-import-type=savedmodel_v2 \
  --tf-savedmodel-exported-names=serving_default \
  /path/to/saved_model \
  -o model.mlir

# 2) 编译为 vmfb
iree-compile model.mlir \
  --iree-hal-target-backends=llvm-cpu \
  -o model.vmfb

# 3) 运行
iree-run-module \
  --module=model.vmfb \
  --function=serving_default
```

你也可以使用 Python API 完成导入与编译，适合把流程集成到训练/部署脚本里。

## :octicons-code-16: 示例

- IREE `samples/` 中的 TensorFlow 相关目录
- `integrations/tensorflow` 中的导入与测试样例

## :octicons-question-16: 故障排查

### Missing serving signature in SavedModel

如果报错提示缺少 `serving_default`：

- 检查导出 SavedModel 时是否保存了签名
- 使用 `saved_model_cli show --all --dir /path/to/saved_model` 查看可用签名
- 在 `--tf-savedmodel-exported-names` 中填入实际签名名
