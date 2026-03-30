---
hide:
  - tags
tags:
  - Python
  - PyTorch
icon: simple/pytorch
---

# PyTorch + IREE = :octicons-heart-16:

## :octicons-book-16: 概览

[iree-turbine](https://github.com/iree-org/iree-turbine) 提供了 IREE、
[torch-mlir](https://github.com/llvm/torch-mlir) 与
[PyTorch](https://pytorch.org/) 之间的紧密集成。

- [x] 与标准 PyTorch 工作流无缝衔接
- [x] 支持在云端和边缘设备部署 PyTorch 模型
- [x] 提供通用模型编译与执行工具

支持两类路径：JIT（即时执行）与 AOT（预编译导出）。

!!! info

    iree-turbine 文档：
    <https://iree-turbine.readthedocs.io/>。

## :octicons-download-16: 前置依赖

1. 安装较新的 PyTorch（按官方说明）：

    === ":fontawesome-brands-linux: Linux"

        ```shell
        python -m pip install torch --index-url https://download.pytorch.org/whl/cpu
        ```

    === ":fontawesome-brands-apple: macOS"

        ```shell
        python -m pip install torch
        ```

    === ":fontawesome-brands-windows: Windows"

        ```shell
        python -m pip install torch
        ```

2. 安装 IREE Turbine：

    ```shell
    python -m pip install iree-turbine
    ```

3. （可选）安装 nightly 组件（若你需要最新特性）：

    ```shell
    python -m pip install \
      --find-links https://iree.dev/pip-release-links.html \
      --upgrade iree-base-compiler iree-base-runtime iree-turbine
    ```

## :octicons-flame-16: JIT（即时）执行

JIT 路径适合在 Python 环境中快速验证：保留 PyTorch 编程体验，
并让 Turbine 在运行时对计算图做编译与调用。

### :octicons-rocket-16: 快速开始

```python
import torch
from iree.turbine import aot

# 定义要运行的 nn.Module 或 Python 函数
class MyModule(torch.nn.Module):
    def forward(self, x):
        return torch.sin(x) + 1.0

m = MyModule()
x = torch.randn(4, 4)

# 使用 turbine 后端编译/执行
# 注意：实际 API 可能随版本变化，请以 iree-turbine 文档为准
y = m(x)
print(y)
```

### :octicons-code-16: 示例

- `iree-org/iree-turbine` 仓库中的 examples
- IREE `samples/` 中的 PyTorch 相关示例

## :octicons-package-dependents-16: AOT（预编译）导出

AOT 路径用于把 PyTorch 程序导出为可部署工件（如 `.vmfb`），
适合生产部署与离线构建。

### :octicons-plug-16: 简单 API

```python
import torch
import iree.turbine.aot as aot

# 定义要导出的 nn.Module
class LinearModule(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.linear = torch.nn.Linear(16, 32)

    def forward(self, x):
        return self.linear(x)

module = LinearModule().eval()
example_arg = (torch.randn(1, 16),)

# 导出为 MLIR
exported = aot.export(module, *example_arg)
mlir_module = exported.mlir_module

# 编译为部署工件
compiled = aot.compile_to_vmfb(exported)

# 通过 IREE Runtime API 进行验证（按你的运行时环境配置设备）
print(type(compiled))
```

#### :octicons-code-16: 示例

- `iree-turbine` AOT quickstart
- 参数外置（external parameters）示例

### :octicons-tools-16: 高级 API

高级 API 适合需要精细控制导出行为的场景，例如：

- 函数级导出控制（指定导出哪些入口）
- 全局变量处理
- 外部参数文件（如 safetensors）
- IR 后处理与多目标编译

```python
import torch
import iree.turbine.aot as aot

# 最小程序示例
class Basic(torch.nn.Module):
    def forward(self, x):
        return x * 2

prog = Basic().eval()
exp = aot.export(prog, torch.randn(4))
print(exp.mlir_module)
```

#### :material-function: 导出函数

可按函数粒度控制导出入口，便于：

- 拆分推理阶段
- 暴露多个服务接口
- 对不同入口单独调优

#### :material-variable: 全局变量

支持在导出流程中处理模型权重等全局变量；
可与参数外置结合以减小 IR 内嵌常量。

#### :octicons-file-symlink-file-16: 使用外部参数

常见流程：

1. 准备参数字典，键名与模块属性一致。
2. 将权重外置，IR 中仅保留符号引用。
3. 编译导出模块生成二进制。
4. 通过命令行或 Runtime API 在加载时注入参数。

这样做的好处：

- 减少编译产物体积
- 支持参数热更新
- 便于多模型共享同一执行图

#### :octicons-code-16: 示例

可参考官方文档中的 external parameter 示例，典型会包含：

- 保存参数为 `safetensors`
- 生成输入 `.npy`
- 使用 `iree-run-module` 进行端到端验证

```shell
iree-run-module \
  --module=model.vmfb \
  --parameters=model.safetensors \
  --function=main \
  --input="1x16xf32=@input.npy"
```
