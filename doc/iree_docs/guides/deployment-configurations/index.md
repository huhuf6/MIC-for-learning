# 部署配置（Deployment Configurations）

IREE 提供灵活的部署方式：从完整运行时环境到极简裸机环境。

## 稳定配置

- CPU
- CPU Bare-Metal
- GPU Vulkan
- GPU ROCm
- GPU CUDA
- GPU Metal

## 编译目标后端（Compiler Target Backends）

编译时可选择后端：

- `llvm-cpu`
- `vmvx`
- `vulkan-spirv`
- `rocm`
- `cuda`
- `metal-spirv`
- `webgpu-spirv`（实验）

常用指定方式：

- CLI：`--iree-hal-target-backends=`
- Python：`target_backends=[...]`

## 运行时 HAL 驱动与设备

运行时设备示例：

- `local-sync`
- `local-task`
- `cuda`
- `hip`
- `vulkan`
- `metal`
- `webgpu`（实验）

## 查询可用后端/设备

```bash
iree-compile --iree-hal-list-target-backends
iree-run-module --list_drivers
```

