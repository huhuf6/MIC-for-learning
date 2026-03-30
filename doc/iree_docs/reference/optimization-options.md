---
icon: octicons/rocket-16
---

# 优化选项（Optimization options）

本文档说明 IREE 支持的常见优化参数，包括开关名与默认行为。

这些参数可用于：

- `iree-compile` 命令行
- Python `iree.compiler.tools` 的 `extra_args=["--flag"]`
- 进程内 Python 编译 API：`iree.compiler.transforms.iree-compile.CompilerOptions(...)`
- C API：`ireeCompilerOptionsSetFlags()`

## Optimization level

与 clang/gcc 类似，IREE 提供总开关 `--iree-opt-level=`：

- `O0`：最少优化（默认）
- `O1`：基础优化
- `O2`：更激进优化（可能有后端兼容边界）
- `O3`：最激进优化（更高性能，编译时间更长）

一般原则：

- 低等级更利于调试与稳定性
- 高等级更利于吞吐/延迟

!!! note

    并非所有性能相关开关都被 `iree-opt-level` 覆盖。
    仍可对子开关显式覆盖。

示例：

```bash
# 使用 O2 默认优化，同时保留断言
iree-compile --iree-opt-level=O2 --iree-strip-assertions=false

# 使用 O0，但开启激进融合
iree-compile --iree-opt-level=O0 --iree-dispatch-creation-enable-aggressive-fusion=true
```

### Pipeline-level control

除总开关外，IREE 还提供 pipeline 级别控制：

#### Dispatch Creation（`iree-dispatch-creation-opt-level`）

- `iree-dispatch-creation-enable-aggressive-fusion`（通常在 `O2` 开启）

用于开启更激进的融合策略（某些后端可能尚未完全支持）。

#### Global Optimization（`iree-global-optimization-opt-level`）

- `iree-opt-strip-assertions`（通常在 `O1` 开启）

移除输入程序中的 `std.assert`，可提升优化空间；
若你需要保留面向用户的断言信息，应显式关闭该移除行为。

## High level program optimizations

### Data-Tiling（`--iree-opt-data-tiling`，默认关）

适用于可利用分块提升局部性和并行度的场景。

可能收益：

- 更好的 cache 命中
- 降低带宽压力
- 改善向量化/并行映射

### Constant evaluation（`--iree-opt-const-eval`，默认开）

在编译期对常量表达式求值，减少运行时计算开销。

### Constant expression hoisting（`--iree-opt-const-expr-hoisting`，默认开）

将可提升的常量表达式外提，避免重复计算。

### Numeric precision reduction（`--iree-opt-numeric-precision-reduction`，默认关）

在允许精度损失的前提下降低数值精度以换取性能。

启用前建议：

- 做端到端精度回归
- 重点验证对损失敏感的模型（检测、生成、长序列任务）
