---
icon: simple/llvm
---

# MLIR Dialect 与 Pass

这些页面包含 IREE 仓库中定义的 MLIR dialect 的自动生成文档。
IREE 同时使用了大量上游 MLIR dialect/pass，可参考：

- <https://mlir.llvm.org/docs/Dialects/>
- <https://mlir.llvm.org/docs/Passes/>

## IREE 内部 Dialect

这些 dialect 属于 IREE 编译器内部实现细节，但插件与高级集成场景也会用到。
多数源码位于：
[`iree/compiler/Dialect/`](https://github.com/iree-org/iree/tree/main/compiler/src/iree/compiler/Dialect)

Dialect | 说明
--- | ---
[Check](./Check.md) | 为 IREE 测试定义断言
[Encoding](./Encoding.md) | Tensor 编码属性及相关操作
[Flow](./Flow.md) | 建模执行数据流与分区
[HAL](./HAL.md) | 针对 IREE HAL[^1] 的操作表示
[HAL/Inline](./HALInline.md) | 内联 HAL 互操作运行时模块方言
[HAL/Loader](./HALLoader.md) | HAL 内联可执行加载器运行时模块方言
[IO/Parameters](./IOParameters.md) | 外部参数资源管理 API
[IREECodegen](./IREECodegen.md) | IREE 代码生成通用功能
[IREECPU](./IREECPU.md) | 面向 CPU/VMVX 的代码生成通用功能
[IREEGPU](./IREEGPU.md) | 面向 GPU 的代码生成通用功能
[IREEVectorExt](./IREEVectorExt.md) | Vector 方言扩展
[LinalgExt](./LinalgExt.md) | Linalg 方言扩展
[PCF](./PCF.md) | 用于建模并行控制流的方言
[Stream](./Stream.md) | 建模执行分区与调度
[TensorExt](./TensorExt.md) | Tensor 方言扩展
[Util](./Util.md) | IREE 子方言共享的类型与操作
[VM](./VM.md) | 抽象虚拟机操作表示
[VMVX](./VMVX.md) | 虚拟机向量扩展

[^1]: Hardware Abstraction Layer（硬件抽象层）
