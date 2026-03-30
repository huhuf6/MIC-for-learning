---
hide:
  - tags
tags:
  - performance
icon: octicons/meter-16
status: new
---

# :octicons-meter-16: 调优（Tuning）

调优是压榨硬件性能的关键步骤。IREE 在编译时会为 workload 选择一组
执行参数（knobs），例如 GPU 的线程数、tile 大小等。默认参数通常对通用
负载表现稳健，但针对特定模型仍有可观优化空间。

所谓调优，就是在参数空间中迭代搜索，使目标指标（延迟、吞吐、功耗等）更优。

## :octicons-dependabot-16: SHARK Tuner

### :octicons-book-16: 概览

手工调优可行，但更推荐使用
[AMDSHARK Tuner](https://github.com/nod-ai/amd-shark-ai/tree/main/amdsharktuner)
自动搜索 dispatch 级别的参数组合。

dispatch 是 IREE 编译流程中把输入程序切分后形成的并发执行块。
对 dispatch 调优，通常能显著改善整体性能。

参考：

- SHARK Tuner 源码与文档
- `model_tuner` 示例

### :octicons-question-16: 什么是 dispatch？

可以把 dispatch 理解为：

- 原始计算图被切分后的可原子执行单元
- 便于映射到 GPU/并行硬件调度
- 是调优参数生效的核心边界

一个 workload 会包含多个 dispatch；热点 dispatch 往往是主要优化目标。

### :octicons-gear-16: Dispatch 里的 knobs

常见可调参数包括：

- workgroup size
- subgroup size
- tile sizes
- unroll/vectorization 相关配置
- pipeline 特定开关

不同后端可用参数不同，且约束不同（寄存器、共享内存、波前/warp 结构等）。

### :octicons-tools-16: 设置 knobs 与 tuning spec

调优通常通过 tuning spec 文件完成。流程：

1. 为目标模型编译并导出 dispatch 信息。
2. 为候选 dispatch 设定 knobs 搜索空间。
3. 批量 benchmark，记录指标。
4. 选出最佳参数并固化到 spec。

#### Usage in IREE

在 IREE 中，你可以通过编译参数加载 tuning spec，并在目标后端应用。
具体 CLI 参数名可能因版本变化，请以当前 `iree-compile --help` 为准。

### :octicons-search-16: tuning spec 解剖

#### Example

一个 tuning spec 通常包含：

- 匹配规则（匹配某类 dispatch）
- 参数候选集合（如 tile/workgroup 候选值）
- 目标指标与选择策略

#### Explanation

实践建议：

- 先锁定前 1-3 个热点 dispatch
- 控制搜索空间，避免组合爆炸
- 分离“编译失败”与“性能不佳”两类失败
- 固化后进行跨 batch/shape 回归，验证泛化性
