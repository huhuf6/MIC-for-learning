---
icon: octicons/file-symlink-file-16
---
# IREE Lowering Config（降级配置）

## Overview

lowering config 是 dispatch 内部从 tensor 级向 vector 级 lowering 时使用的
属性集合。其取值通常由以下因素共同决定：

1. 计算类型（matmul、reduction、convolution 等）
2. 硬件属性（subgroup 大小、带宽、计算单元）
3. 调优器给出的性能修正

IREE 会按后端与计算类型选择不同 lowering config 变体。

---

## LLVMGPU [Vector Distribute](https://www.youtube.com/watch?v=ueYi9NnK4Pw) Pipeline

### Reduction

该配置采用了内存带宽受限场景常见的 reduction 策略，可参考
[Optimizing Parallel Reduction in CUDA](https://developer.download.nvidia.com/assets/cuda/files/reduction.pdf)
的高层思想。

#### Relevant lowering config attributes

- `workgroup` tile sizes
- `thread` tile sizes
- `partial_reduction` tile sizes
- `lane_basis`（subgroup 内线程分布）
- `subgroup_basis`（workgroup 内 subgroup 分布）
- `expand_dims`（维度重关联）

#### Summary

| Attribute           | 语义 |
| ------------------- | ---- |
| `workgroup`         | 各维度 workgroup 级 tile 大小 |
| `thread`            | 各维度 thread 级 tile 大小（如单线程加载宽度） |
| `partial_reduction` | 由 workgroup 处理的 reduction 维度分块大小 |
| `lane_basis`        | subgroup 内线程到迭代空间的映射 |
| `subgroup_basis`    | workgroup 内 subgroup 到迭代空间的映射 |
| `expand_dims`       | 对 reduction 维做拆分以便更细粒度累加 |

#### Tile sizes

tile size 是按迭代空间维度给出的整数数组。
某维为 `0` 表示该层级不对该维做 tiling。

该 pipeline 里重点关注三个层级：

- `workgroup`
- `thread`
- `partial_reduction`

示例：

```mlir
workgroup = [16, 0]
```

含义：在 `d0` 维每个 workgroup 生成 16 个输出元素。

##### `partial_reduction` tile sizes

仅作用于 reduction 维。可把 reduction 维 `r` 拆分为：

- `r_outer`
- `r_partial`

通常做法是对 `r_outer` 做串行循环，每次处理 `r_partial` 个部分累加值，
最后再做合并归约。

#### `Basis` attributes

`lane_basis` / `subgroup_basis` 用来定义“谁算哪一块”的映射规则，
本质是把硬件并行层级映射到迭代空间。

##### The `counts` Array

`counts` 描述每一层 basis 在各维度上的分配规模。

##### The `mapping` Array

`mapping` 描述 basis 元素如何映射到目标维度。

##### Computing thread position based on lane_basis（步骤）

1. 根据 lane id 获取 subgroup 内线程编号。
2. 按 `counts` 做多维展开/分解。
3. 按 `mapping` 投影到目标迭代维。
4. 与 tile 原点叠加得到线程实际处理坐标。

##### Concrete Example: Thread 42 with `[[16, 4], [1, 0]]`

在该示例中，可将线程 42 的线性编号拆分后映射到目标二维坐标：

- 第一组 basis 影响第 1 个目标维
- 第二组 basis 影响第 0 个目标维

最终得到该线程在一个 tile 内负责的元素/向量片段位置。

#### Dimension Expansion (`expand_dims`)

`expand_dims` 通过重关联把原维度拆分为多个子维度，
可提升并行映射灵活性并改善局部累加策略。

常见收益：

- 更细粒度并行
- 更高寄存器复用
- 减少跨线程归约开销

## Example

实际工程里，建议把 lowering config 与调优工具联动：

1. 先用默认配置跑基线。
2. 对热点 dispatch 调整 `workgroup/thread/partial_reduction`。
3. 再细化 `lane_basis/subgroup_basis/expand_dims`。
4. 用 benchmark 验证延迟、吞吐与稳定性。
