
## 新 session 接续上下文

仓库：`/mlir-tutorial/MIC-for-learning`

当前学习目标：
- 系统梳理 IREE 编译 pipeline
- 重点看 `DispatchCreation / Flow / Stream`
- 继续往 `HAL / codegen` 接

回答风格要求：
- 中文
- 尽量附前后真实 IR
- 以“代码行 -> IR 变化/语义”方式讲
- 偏源码对照，不只讲抽象概念

### 总体阶段线

`Input -> ABI -> Preprocessing -> GlobalOptimization -> DispatchCreation -> Flow -> Stream -> HAL -> VM`

### DispatchCreation

主要 pass：
- `createFusionPreprocessingPass`
- `FormDispatchRegionsPass`
- `ElementwiseOpFusionPass`
- `CloneProducersIntoDispatchRegionsPass`
- `CollapseDimensionsPass`
- 可选 encoding 链：
  - `AnnotateDataTilingHintsPass`
  - `SetEncodingPass`
  - `HoistEncodingOpsPass`
  - `FuseEncodingOpsIntoDispatchRegionsPass`
  - `ConvertEncodingToFlowPass`

关键理解：
- dispatch root 常见是：
  - 非 `fill` 的 linalg named op
  - 带 reduction 的 `linalg.generic`
  - 某些 `TilingInterface` op
  - `linalg.unpack`
- `createElementwiseOpFusionPass` 有两种模式：
  - dispatch 外：`intraDispatch=false`
  - dispatch 内：`intraDispatch=true`
- 不是所有 producer 都会被放进 dispatch；共享 producer、跨 block、会破坏 dominance 的 producer 往往会留外面

### Flow

主要 pass：
- `CaptureDynamicDimsPass`
- `OutlineDispatchRegionsPass`
- `InitializeEmptyTensorsPass`
- `AnnotateDispatchesPass`
- `OutlineConstantsPass`
- `DeduplicateExecutablesPass`
- `CleanupTensorShapesPass`

关键理解：
- `CaptureDynamicDimsPass` 不是重算 shape，而是把隐式动态维显式化成 SSA/block args/iter_args
- `flow.dispatch.workgroups` 的主 body 会 outline 成 `func.func`
- `count/workgroup_count` region 会搬到 `flow.executable.export ... workgroups(...)`
- `workload = 问题规模`
- `workgroups = block/grid 数量`
- `workgroup.size = block 内线程数`

### Stream 早期 tensor/resource 阶段

主线：
- `ConvertToStreamPass`
- `SpecializeEncodingsPass`
- `EncodeHostTensorsPass`
- `EncodeDeviceTensorsPass`
- `MaterializeEncodingsPass`

关键理解：
- `stream.tensor.*` 主要是 `ConvertToStreamPass` 之后出现的中间层
- 它不是一步直接变 `stream.async.*`
- `EncodeHostTensorsPass` 把 host 侧 `stream.tensor.*` 降成 `stream.async.*`
- `EncodeDeviceTensorsPass` 主要处理 executable 边界上的：
  - `stream.binding.subspan`
  - `dispatch.tensor.load/store`
- `stream.executable` 内部仍可保留：
  - `tensor.empty`
  - `linalg.fill`
  - `linalg.generic`
  这些会留到后面的 HAL/codegen/bufferization

### MaterializeEncodingsPass

作用：
- 把抽象的 `stream.tensor.encode`
  物化成：
  - `stream.executable`
  - `stream.executable.export`
  - `stream.async.dispatch`

关键理解：
- 生成的是“load -> unset/set encoding -> store”的专用 executable
- 会按 `(sourceEncoding, resultEncoding)` 缓存 executable
- `source` = 输入侧
- `result` = 输出侧
- `workgroups(...)` 是 export 的 workgroup-count region 签名，不是线程数

### MaterializeBuiltinsPass

当前主要处理：
- `stream.async.splat`
- `stream.async.fill`

关键理解：
- 类型不原生支持时，会改写成：
  - builtin `stream.executable`
  - `stream.async.dispatch @__builtin_*`
- 它放在 scheduling 之后，是为了先保留 builtin 的高层语义给 partitioning/placement，再转成 opaque dispatch

### ScheduleExecutionPass

关键理解：
- `partitionSet` 里的 partition = 一个 `stream.async.execute` 边界
- 不是“partition 内所有 op 都并行”
- 内部并发留给后面的 wave / `stream.async.concurrent`

主线：
- `partitionStreamableOpsReference(...)` 按 block 切 execute partition
- 每个 partition 变成一个 `stream.async.execute`
- 旧 escaping value 改接到：
  - `execute result`
  - `stream.timepoint.await`
- 原 streamable op 放进 `deadOps`
- 删旧 op 前，如果 nested region 还在用 `preferCloneToConsumers()` 的 op，就先在 nested region 里 rematerialize 一份
- 最后再递归进入 `scf.for / scf.if / cf` 等控制流 region

子 execute 位置：
- 不会嵌到父 execute 里
- 会出现在控制流 op 的子 region / 子 block 里

### partitionStreamableOpsReference

这是：
- 单 block
- 逆序扫描
- 基于 use-def / hazard / affinity / nested capture / sync op / cloneable producer 的保守启发式分区器

关键概念：
- `opInfo.hazards`
- `nestedRegionHazards`
- `consumers`
- `usableBuilders`
- `candidates`
- `syncOps`
- `clonedOps`
- `clonedEscapingOps`

关键理解：
- `nestedRegionHazards` 不是普通依赖，而是“这个 op 绝对不能进入哪些 partition”的禁入集合
- 非 streamable 且不可 trivially dead 的 op 会：
  - `usableBuilders.reset()`
  - 充当 partition 屏障
- `AsyncBarrierOp / AsyncTransferOp` 会先挂到 `syncOps[producer]`
- cloneable op 和 non-cloneable op 的 hazard 口径不同
- non-cloneable 基本按“单份共享 producer”模型分析

### ScheduleConcurrencyPass / wave

关键理解：
- 处理单位是单个 `stream.async.execute` 的 body
- `partitionRegionConcurrency(...)` 算的是 waveSet
- 只有 `ops.size() > 1` 的 wave 才会物化成 `stream.async.concurrent`

wave 概念：
- wave = execute 内的一层并发批次
- wave 内并行
- 多个 wave 通常按依赖关系前后推进
- 不是所有 wave 彼此都并行

### ResourceHazardAnalysis

作用：
- 判断两个访问 `!stream.resource` 的 op 能不能并发

大致规则：
- 同一 resource
- range 可能重叠
- 且至少一边写
=> 有 hazard

read-read 重叠例外，不算 hazard

### timepoint

关键理解：
- `stream.timepoint` 不是某一个 pass 一次性创造出来的
- 可能来源于：
  - `stream.timepoint.import`
  - `stream.async.execute` result timepoint
  - `resource.alloca/dealloca`
  - `cmd.execute`
  - `timepoint.join/immediate`

`stream.timepoint.await %tp => %res` 语义：
- `%res` 是异步工作产出的资源
- `%tp` 表示何时完成
- await 后得到 ready 版本的资源

`deferredTimepoint`：
- 如果下游 timeline-aware consumer 本身能提供更合适的等待时刻
- 就复用它，避免过早同步

### PropagateTimepointsPass

作用：
- 把 resource readiness 显式沿整个程序传播
- 让 `!stream.resource` 的流动旁边跟着 `!stream.timepoint`
- 尽量把等待往真正的 consumer 附近下沉

它会改：
- globals
- func signatures
- call/return
- block args / branch operands
- execute captures

### buildStreamCmdPassPipeline

总体作用：
- 把已调度好的 `stream.async.*` IR
  落实成显式分配、显式 subrange、显式生命周期管理
  最后收口到 `stream.cmd.*`

主要 pass：
- `createScheduleAllocationPass`
- `createEmplaceTransientsPass`
- `createMaterializeTransientSizeQueriesPass`
- `createPackConstantsPass`
- `createLayoutSlicesPass`
- `createPropagateSubrangesPass`
- `createAutomaticReferenceCountingPass`
- `createAnnotateConstantTransientSizePass`
- `createVerifyLoweringToCmdPass`

术语：
- `transient` = 临时资源/临时内存
- `backing storage` = 承载逻辑 resource 的底层真实存储

### HAL / ABI

`!hal.buffer_view`：
- 是类型，不是 op
- 主要由 `WrapEntryPointsPass` 在 ABI 包装时引入
- 高层 tensor ABI 会被包装成 `!hal.buffer_view`
- body 里再由：
  - `hal.tensor.import/export`
  - 后续变成 `stream.tensor.import/export`

`ExecutionModel::HostOnly`：
- 仅 host 本地代码，不需要执行调度
- 会跳过：
  - DispatchCreation
  - Flow
  - Stream
  - HAL

### linalg.generic / indexing_maps / iterator_types

关键理解：
- `iterator_types` 定义公共迭代空间的维，以及每维是：
  - `parallel`
  - `reduction`
- `indexing_maps` 定义每个 tensor 如何从公共迭代维取下标
- `d0..dN` 可以近似看成有序迭代维，通常可按外到内循环顺序理解

已确认例子：
- conv：
  - parallel = `oc/oh/ow`
  - reduction = `ic/kh/kw`
- matmul：
  - 若 `(d0,d1,d2)=(m,c,n)`，则
    - A: `(d0,d2)`
    - B: `(d2,d1)`
    - C: `(d0,d1)`

### 当前已生成的阶段 IR

- DispatchCreation：
  - `testmodels/resnet18_like_before_dispatch_creation.mlir`
- Flow：
  - `testmodels/resnet18_like_before_flow.mlir`
- Stream/HAL 前：
  - `testmodels/resnet18_like_before_hal_stream.mlir`

注意：
- 原来的 `before_hal_stream` 已恢复，不要覆盖
- `before_flow` / `before_dispatch_creation` 是新增文件

### 新 session 最适合继续的话题

1. 继续细讲 `PropagateTimepointsPass`
2. 用真实 IR 继续讲 `buildStreamCmdPassPipeline`
3. 对照：
   - `before_dispatch_creation`
   - `before_flow`
   - `before_hal_stream`
   看同一个 dispatch 的演化
4. 接 HAL：
   - device assignment
   - materialize interfaces
   - configure executables
   - translate executables
   - convert-to-HAL
   - serialize
