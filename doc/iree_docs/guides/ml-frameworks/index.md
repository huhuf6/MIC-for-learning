# ML 框架集成（ML Frameworks）

IREE 以统一编译基础设施支持多种主流 ML 框架。

## 总体流程

1. 在框架侧导出模型图。
2. 导入为 MLIR。
3. 交给 IREE 编译。
4. 在 IREE Runtime 执行。

## 已支持的框架入口

- [JAX](./jax.md)
- [ONNX](./onnx.md)
- [PyTorch](./pytorch.md)
- [TensorFlow](./tensorflow.md)
- [TensorFlow Lite](./tflite.md)

## Export / Import 一般阶段

1. 捕获/冻结模型图。
2. 写出交换格式（SavedModel/TorchScript/ONNX 等）。
3. 用导入器转成 MLIR。
4. 做输入合法化（只保留 IREE 可接受算子）。
5. 产出可独立编译的 MLIR。

## 编译与执行

- 编译：针对目标后端（CPU/GPU 等）生成优化后的模块。
- 执行：由运行时选择设备，加载模块并调用入口函数。

