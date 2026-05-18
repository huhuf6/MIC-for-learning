# LLVM `-O1/-O2/-O3` 优化 Pass 与优化内容整理

## 1. 适用范围与结论口径

- 本文基于本机实测：
  - `opt --version`：LLVM 14.0.0
  - `clang --version`：clang 14.0.0
- 结论口径：
  - `-O1/-O2/-O3` 的区分标准首先是**优化目标与代价权衡**（编译时间、代码体积、运行速度）。
  - 具体 pass 由 LLVM 的默认 pipeline 决定，会随 LLVM 版本变化；因此本文结果是**LLVM 14 实测版**。

## 2. 区分标准（官方语义，简化版）

- `O1`：快速优化，尽量保留可调试性，不做高成本复杂变换。
- `O2`：默认高性价比优化，要求优化“基本值得其编译开销/体积开销”。
- `O3`：在 `O2` 上更激进，允许更多编译时间和代码体积来换取潜在性能。

## 3. `O1/O2/O3` 最核心优化（含中文翻译）

> 说明：这里是“最核心、最常被问”的代表性 pass，不是完整列表。完整清单见后文。

## `O1` 核心优化

- `mem2reg`：内存转寄存器（将局部变量提升为 SSA 寄存器形式）
- `sroa`：标量替换聚合体（把 struct/array 拆成标量）
- `instcombine`：指令合并与代数化简（等价变换、常量折叠）
- `simplifycfg`：控制流图简化（合并基本块、删除冗余分支）
- `licm`：循环不变代码外提（把循环内不变计算搬到循环外）
- `inline`：函数内联（减少调用开销并暴露跨函数优化机会）

## `O2` 核心优化（在 O1 基础上）

- `gvn`：全局值编号（消除重复计算）
- `dse`：死存储删除（删除最终不会被读取的写操作）
- `jump-threading`：跳转穿线（利用条件已知性重接分支路径）
- `slp-vectorizer`：SLP 向量化（基本块内标量打包成 SIMD）
- `loop-vectorize`：循环向量化（把循环标量操作改为 SIMD 并行）
- `loop-unroll<O2>`：循环展开（减少分支/索引开销，提升指令级并行）

## `O3` 核心优化（在 O2 基础上）

- `aggressive-instcombine`：激进指令合并（更高成本、更强代数化简）
- `argpromotion`：参数提升（把可提升的内存参数改为标量参数）
- `callsite-splitting`：调用点拆分（按上下文拆分路径，利于后续传播）
- `simple-loop-unswitch<nontrivial;trivial>`：更激进循环 unswitch（把循环内条件外提并复制循环体）
- `loop-unroll<O3>`：更高等级循环展开（更愿意以体积换速度）

## 4. 分级差异总览（最关键）

## `O1 -> O2` 新增/增强 Pass

- `speculative-execution`
- `jump-threading`
- `correlated-propagation`
- `tailcallelim`
- `mldst-motion<no-split-footer-bb>`
- `gvn<>`
- `dse`
- `slp-vectorizer`
- `openmp-opt-cgscc`
- `simplifycfg<...;hoist-common-insts;sink-common-insts>`（更激进 CFG 简化形态）
- `loop-vectorize<...;no-vectorize-forced-only;>`（从“仅强制向量化”转为常规可向量化）
- `loop-unroll<O1> -> loop-unroll<O2>`

## `O2 -> O3` 新增/增强 Pass

- `callsite-splitting`
- `argpromotion`
- `aggressive-instcombine`
- `simple-loop-unswitch<no-nontrivial;trivial> -> simple-loop-unswitch<nontrivial;trivial>`
- `loop-unroll<O2> -> loop-unroll<O3>`

## 5. 优化内容（按主题）

## A. 过程间/调用图优化（IPO/CGSCC）

- `inline` / `inline<only-mandatory>`：函数内联，减少调用开销，暴露更多后续优化机会。
- `devirt<4>`：去虚调用，提升间接调用可优化性。
- `argpromotion`（O3）：把可提升参数改为标量传递，减少内存访问和别名障碍。
- `callsite-splitting`（O3）：按调用点上下文拆分路径，为后续常量传播/简化创造条件。
- `function-attrs` / `forceattrs` / `inferattrs`：推断或附加属性，帮助别名分析与优化决策。

## B. 标量与代数简化

- `instcombine` / `aggressive-instcombine`（后者 O3）：指令级等价变换、折叠、强度削弱。
- `reassociate`：表达式重结合，利于常量折叠和公共子表达式优化。
- `gvn<>`（O2+）：全局值编号，消除冗余计算。
- `sccp` / `ipsccp`：稀疏条件常量传播（函数内/跨过程）。
- `adce` / `bdce` / `globaldce`：删除死代码、死位、死全局。

## C. 控制流与分支优化

- `simplifycfg<...>`：合并基本块、删除冗余分支、switch/icmp 转换等。
- `jump-threading`（O2+）：穿线优化，基于条件已知性简化分支流向。
- `correlated-propagation`（O2+）：利用条件相关信息做传播和裁剪路径。
- `tailcallelim`（O2+）：尾调用消除，降低栈开销。

## D. 内存与别名相关优化

- `sroa`：聚合对象标量化，提升寄存器化机会。
- `mem2reg`：内存变量 SSA 化（Promote Memory to Register）。
- `memcpyopt`：`memcpy/memmove` 相关优化。
- `dse`（O2+）：死存储删除。
- `mldst-motion`（O2+）：内存 load/store 运动，减少冗余访存。

## E. 循环与向量化优化

- `licm`：循环不变代码外提。
- `indvars`：归纳变量标准化。
- `loop-rotate` / `loop-simplifycfg` / `loop-deletion`：循环结构规范化与删除无效循环。
- `simple-loop-unswitch`：循环外提条件分支。O3 支持 non-trivial 级别，激进度更高。
- `loop-vectorize`：
  - O1：`vectorize-forced-only`（仅强制场景）
  - O2/O3：常规可向量化场景默认可做
- `slp-vectorizer`（O2+）：基本块内超字级并行向量化。
- `loop-unroll<O1/O2/O3>`：展开阈值与策略随等级上升而更激进。

## F. 协程/OpenMP 与其他

- `coro-early` / `coro-elide` / `coro-split` / `coro-cleanup`：协程转换与清理。
- `openmp-opt` / `openmp-opt-cgscc`（后者 O2+）：OpenMP 相关优化。
- `vector-combine`：向量指令组合与简化。

## 6. 优化原理（为什么这些 Pass 能提速/减体积）

## 6.1 通用底层原理

- SSA 形式让“定义-使用链”清晰，常量传播、死代码删除、冗余消除更容易做。
- 数据流分析和稀疏传播（如 SCCP）把“值可能是什么”变成可计算问题，再触发级联简化。
- 别名分析（AA）缩小“可能互相影响的内存访问集合”，给 LICM/GVN/DSE 等提供安全前提。
- 规范化先行：很多优化先做 canonicalization（如 loop-simplify、indvars），把 IR 变成统一形态，再做高收益变换。
- 迭代收敛：`instcombine/simplifycfg` 往往多轮穿插运行，每轮暴露下一轮机会，直到收益变小。

## 6.2 关键优化机制

- 内联（`inline`）：
  - 原理：消除调用边界，把调用者/被调者合成一个更大优化单元。
  - 收益：减少 call 开销，暴露跨函数常量传播、死代码删除机会。
  - 代价：代码膨胀，I-cache 压力上升，编译时间增长。

- GVN/CSE（`gvn`, `early-cse`）：
  - 原理：基于表达式等价性和支配关系复用已有计算结果。
  - 收益：减少重复算术和访存，降低指令数。
  - 约束：依赖别名分析与内存模型，避免错误合并。

- DCE/DSE（`adce/bdce/globaldce/dse`）：
  - 原理：删除对程序可观测行为无贡献的计算或存储。
  - 收益：减少动态指令和内存带宽占用。
  - 关键：必须保留有副作用/异常语义/可见性的操作。

- CFG 简化与分支传播（`simplifycfg/jump-threading/correlated-propagation`）：
  - 原理：利用已知条件和路径相关信息，合并块、去冗余分支、重写控制流。
  - 收益：降低分支数量和预测压力，给向量化/调度创造更直的热路径。

- 循环优化（`licm/indvars/unswitch/unroll`）：
  - 原理：把循环外可计算内容外提，规范归纳变量，复制循环体减少分支与索引开销。
  - 收益：提升 ILP、降低每次迭代固定开销，利于后端调度。
  - 代价：展开和 unswitch 会增大代码体积；O3 更愿意承担这个代价。

- 向量化（`loop-vectorize/slp-vectorizer`）：
  - 原理：把多条标量独立操作打包成 SIMD 指令并行执行。
  - 收益：吞吐显著提升，尤其对数值循环和规则数据访问。
  - 约束：受数据依赖、对齐、步长、目标 ISA 宽度、成本模型影响。

- 内存标量化与访存重排（`sroa/mem2reg/mldst-motion/memcpyopt`）：
  - 原理：把聚合内存对象拆成标量寄存器值，减少不必要 load/store 和内存拷贝。
  - 收益：降低内存层级开销，暴露更多算术与常量优化。

## 6.3 为什么 O1/O2/O3 会呈现这些差异

- `O1`：优先“低编译开销 + 保留调试友好”，因此通常不开重型向量化/激进分支变换。
- `O2`：开启多数“能自证收益”的优化（如 GVN、DSE、SLP、jump-threading），是默认平衡点。
- `O3`：在 O2 基础上放宽代码体积与编译时间约束，引入更激进变换（如 `aggressive-instcombine`、non-trivial unswitch、更高展开级别）。

## 7. LLVM 14 实测：各级完整 Pass 集合（去重后）

> 说明：以下是通过 `opt -passes='default<Ox>' --print-pipeline-passes` 提取后，按分隔符拆分、去重、排序得到。用于“有哪些 pass”速查。

## `O1`（70）

```text
adce
alignment-from-assumptions
annotation-remarks
annotation2metadata
bdce
called-value-propagation
cg-profile
cgscc
constmerge
coro-cleanup
coro-early
coro-elide
coro-split
deadargelim
devirt<4>
div-rem-pairs
early-cse<>
early-cse<memssa>
elim-avail-extern
float2int
forceattrs
function
function-attrs
function<eager-inv>
globaldce
globalopt
indvars
inferattrs
inject-tli-mappings
inline
inline<only-mandatory>
instcombine
instsimplify
invalidate<aa>
ipsccp
libcalls-shrinkwrap
licm
loop
loop-deletion
loop-distribute
loop-idiom
loop-instsimplify
loop-load-elim
loop-mssa
loop-rotate
loop-simplifycfg
loop-sink
loop-unroll-full
loop-unroll<O1>
loop-vectorize<no-interleave-forced-only;vectorize-forced-only;>
lower-constant-intrinsics
lower-expect
mem2reg
memcpyopt
openmp-opt
reassociate
rel-lookup-table-converter
require<globals-aa>
require<opt-remark-emit>
require<profile-summary>
rpo-function-attrs
sccp
simple-loop-unswitch<no-nontrivial;trivial>
simplifycfg<bonus-inst-threshold=1;forward-switch-cond;switch-range-to-icmp;switch-to-lookup;no-keep-loops;hoist-common-insts;sink-common-insts>
simplifycfg<bonus-inst-threshold=1;no-forward-switch-cond;no-switch-range-to-icmp;no-switch-to-lookup;keep-loops;no-hoist-common-insts;no-sink-common-insts>
simplifycfg<bonus-inst-threshold=1;no-forward-switch-cond;switch-range-to-icmp;no-switch-to-lookup;keep-loops;no-hoist-common-insts;no-sink-common-insts>
sroa
transform-warning
vector-combine
verify
```

## `O2`（80）

```text
adce
alignment-from-assumptions
annotation-remarks
annotation2metadata
bdce
called-value-propagation
cg-profile
cgscc
constmerge
coro-cleanup
coro-early
coro-elide
coro-split
correlated-propagation
deadargelim
devirt<4>
div-rem-pairs
dse
early-cse<>
early-cse<memssa>
elim-avail-extern
float2int
forceattrs
function
function-attrs
function<eager-inv>
globaldce
globalopt
gvn<>
indvars
inferattrs
inject-tli-mappings
inline
inline<only-mandatory>
instcombine
instsimplify
invalidate<aa>
ipsccp
jump-threading
libcalls-shrinkwrap
licm
loop
loop-deletion
loop-distribute
loop-idiom
loop-instsimplify
loop-load-elim
loop-mssa
loop-rotate
loop-simplifycfg
loop-sink
loop-unroll-full
loop-unroll<O2>
loop-vectorize<no-interleave-forced-only;no-vectorize-forced-only;>
lower-constant-intrinsics
lower-expect
mem2reg
memcpyopt
mldst-motion<no-split-footer-bb>
openmp-opt
openmp-opt-cgscc
reassociate
rel-lookup-table-converter
require<globals-aa>
require<opt-remark-emit>
require<profile-summary>
rpo-function-attrs
sccp
simple-loop-unswitch<no-nontrivial;trivial>
simplifycfg<bonus-inst-threshold=1;forward-switch-cond;switch-range-to-icmp;switch-to-lookup;no-keep-loops;hoist-common-insts;sink-common-insts>
simplifycfg<bonus-inst-threshold=1;no-forward-switch-cond;no-switch-range-to-icmp;no-switch-to-lookup;keep-loops;no-hoist-common-insts;no-sink-common-insts>
simplifycfg<bonus-inst-threshold=1;no-forward-switch-cond;switch-range-to-icmp;no-switch-to-lookup;keep-loops;hoist-common-insts;sink-common-insts>
simplifycfg<bonus-inst-threshold=1;no-forward-switch-cond;switch-range-to-icmp;no-switch-to-lookup;keep-loops;no-hoist-common-insts;no-sink-common-insts>
slp-vectorizer
speculative-execution
sroa
tailcallelim
transform-warning
vector-combine
verify
```

## `O3`（83）

```text
adce
aggressive-instcombine
alignment-from-assumptions
annotation-remarks
annotation2metadata
argpromotion
bdce
called-value-propagation
callsite-splitting
cg-profile
cgscc
constmerge
coro-cleanup
coro-early
coro-elide
coro-split
correlated-propagation
deadargelim
devirt<4>
div-rem-pairs
dse
early-cse<>
early-cse<memssa>
elim-avail-extern
float2int
forceattrs
function
function-attrs
function<eager-inv>
globaldce
globalopt
gvn<>
indvars
inferattrs
inject-tli-mappings
inline
inline<only-mandatory>
instcombine
instsimplify
invalidate<aa>
ipsccp
jump-threading
libcalls-shrinkwrap
licm
loop
loop-deletion
loop-distribute
loop-idiom
loop-instsimplify
loop-load-elim
loop-mssa
loop-rotate
loop-simplifycfg
loop-sink
loop-unroll-full
loop-unroll<O3>
loop-vectorize<no-interleave-forced-only;no-vectorize-forced-only;>
lower-constant-intrinsics
lower-expect
mem2reg
memcpyopt
mldst-motion<no-split-footer-bb>
openmp-opt
openmp-opt-cgscc
reassociate
rel-lookup-table-converter
require<globals-aa>
require<opt-remark-emit>
require<profile-summary>
rpo-function-attrs
sccp
simple-loop-unswitch<nontrivial;trivial>
simplifycfg<bonus-inst-threshold=1;forward-switch-cond;switch-range-to-icmp;switch-to-lookup;no-keep-loops;hoist-common-insts;sink-common-insts>
simplifycfg<bonus-inst-threshold=1;no-forward-switch-cond;no-switch-range-to-icmp;no-switch-to-lookup;keep-loops;no-hoist-common-insts;no-sink-common-insts>
simplifycfg<bonus-inst-threshold=1;no-forward-switch-cond;switch-range-to-icmp;no-switch-to-lookup;keep-loops;hoist-common-insts;sink-common-insts>
simplifycfg<bonus-inst-threshold=1;no-forward-switch-cond;switch-range-to-icmp;no-switch-to-lookup;keep-loops;no-hoist-common-insts;no-sink-common-insts>
slp-vectorizer
speculative-execution
sroa
tailcallelim
transform-warning
vector-combine
verify
```

## 8. 复现命令（你可在任意 LLVM 版本重跑）

```bash
# 输出默认 pipeline
opt -passes='default<O1>' -disable-output --print-pipeline-passes > /tmp/o1.pipeline
opt -passes='default<O2>' -disable-output --print-pipeline-passes > /tmp/o2.pipeline
opt -passes='default<O3>' -disable-output --print-pipeline-passes > /tmp/o3.pipeline

# 拆分并去重（得到 pass 速查清单）
for f in /tmp/o1.pipeline /tmp/o2.pipeline /tmp/o3.pipeline; do
  tr ',()' '\n\n\n' < "$f" | sed 's/^ *//;s/ *$//' | rg -v '^$' | sort -u > "${f}.tokens"
done

# 看分级差异（新增项）
comm -13 /tmp/o1.pipeline.tokens /tmp/o2.pipeline.tokens
comm -13 /tmp/o2.pipeline.tokens /tmp/o3.pipeline.tokens
```
核心痛点

编译时间 vs 运行性能 vs 代码体积三者冲突。
同一优化在不同程序/输入/CPU 上收益差异很大。
优化之间会相互影响，顺序不当会“抢机会”或相互抵消。
调试体验与高优化天然冲突（变量消失、源码行错位）。
编译器成本模型不总是贴合你的真实负载。

主要难点

热点识别难：优化该投到哪里（真热点）不容易。
别名分析与跨函数信息不足会限制很多优化（向量化、LICM、GVN）。
分支不可预测、内存访问不规则时，O3 也可能无效甚至变慢。
代码膨胀导致 I-cache 压力，O3 常见副作用。
缺乏真实 profile 时，很多“激进优化”是盲打。

实用优化策略（工程上最有效）

默认从 -O2 起步，把它当基线。
用基准测试 + perf/火焰图先定位热点，再决定是否上 -O3。
对“热点文件/函数”局部用更激进级别，避免全局 O3 膨胀。
引入 PGO（-fprofile-generate/use）和 LTO（-flto），提升优化决策质量。
做“可优化代码形态”：减少不必要别名、简化控制流、规则化循环与内存访问。
对体积敏感场景优先 -Os/-Oz，不要盲目 O3。
生产发布和调试分离：开发 -O0/-O1 + -g，发布 -O2/-O3。
用自动化回归（性能+正确性）防止“优化升级”带来退化。
一句话建议：O2 作为默认，O3 只给已证实的热点，配合 PGO/LTO 和基准验证。


在 MLIR 的优化阶段，fusion 和 tiling 的核心难点在于：如何在数据复用、并行度、内存层次和硬件映射之间做全局权衡，同时还要处理依赖分析、shape 动态性以及 bufferization 带来的约束。

在 MLIR 中，fusion 和 tiling 的核心难点在于多目标优化问题。
fusion 需要在减少内存访问和控制资源使用（如寄存器、shared memory）之间权衡，同时要解决依赖分析、动态 shape 和与 bufferization 的交互问题；
tiling 的难点在于 tile size 的选择、多级 cache 层次映射以及并行化策略，同时还要处理边界条件；
更复杂的是 fusion 和 tiling 是强耦合的，不同顺序会显著影响性能，因此现代 AI 编译器通常采用 cost model 或 autotuning 来做联合优化决策。


1. 动态形状（Dynamic Shape）

痛点：很多 pass 依赖静态 shape；动态维度会导致 fusion/tiling/vectorization 保守。
策略：shape 约束前移（尽量静态化）、符号 shape 分析、按 shape 分桶编译、多版本内核 + 运行时派发。
2. 融合（Fusion）收益不稳定

痛点：融合过度会增大寄存器压力、降低 occupancy、反而变慢。
策略：代价模型驱动融合（而不是“能融就融”）、限制融合深度、以带宽瓶颈算子优先融合、融合后做快速性能回归。
3. Tiling/并行映射难调

痛点：tile size、线程块映射、向量宽度高度依赖硬件。
策略：参数化 tile + auto-tuning、分层 tiling（L2/shared/register）、目标后端特化 pipeline（CPU/GPU/NPU 分开）。
4. Bufferization（Tensor->MemRef）引发拷贝膨胀

痛点：in-place 判定保守会产生大量 alloc/copy。
策略：强化 alias + liveness 分析、one-shot bufferize 配合 in-place hints、内存池/复用、copy elimination pass 后置清理。
5. Layout 变换与数据重排成本高

痛点：NCHW/NHWC、blocked layout 转换插入过多 transpose/copy。
策略：统一中间布局、layout propagation、延迟 materialize、把 layout 选择纳入全图代价模型。
6. 量化（Quantization）精度/性能平衡难

痛点：校准误差、逐层 scale 传播不稳，导致精度掉点。
策略：QAT 优先于纯 PTQ（关键模型）、混合精度白名单、量化敏感层回退 FP16/FP32、量化后自动精度回归。
7. 多 dialect 降级链路长，可维护性差

痛点：pass 顺序强耦合，改一个 pass 可能全链路波动。
策略：分层 pipeline（高层图优化/中层并行化/低层 codegen）、每层定义 IR 契约、用 pass regression tests 锁行为。
8. 编译时间过长

痛点：大模型 + 多轮 canonicalize/CSE + autotune，编译时不可接受。
策略：分阶段编译缓存（IR hash cache）、热点子图单独重编、AOT+JIT 混合、仅对热子图启用重优化。
一条实战总策略

先做“稳定收益”的基础优化（shape、bufferize、layout、fusion），再把硬件相关的 tiling/vectorization 交给代价模型 + autotune；最后用端到端基准闭环验证（吞吐、时延、峰值内存、精度）。