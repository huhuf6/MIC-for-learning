# op和operation的抽象

operation是具体存储operation的参数 返回值 attributes operation name的结构体
operands（输入）
results（输出）
attributes（属性）
regions（子区域）
location（调试信息）

op含有一个指向operation的指针,dyn_cast<constantop>(xxxop)的底层是 op->getName().getIdentifier() == ConstantOp::getOperationName() 这里是指针比较
为什么不让op继承operation?
解耦operation的数据结构和语义,如果是继承,那么
致命问题一：无法支持动态扩展（dialect）

MLIR 的核心目标是：

用户可以定义自己的 dialect + op（甚至运行时加载）

比如你写：

%0 = mydialect.foo ...

👉 这个 op：
在编译 MLIR 时根本不存在
可能来自插件 / 动态库
🚨 如果用 C++ 继承：
每个 op 都必须是一个 C++ class
那就意味着：
❌ 必须提前编译进程序
❌ 动态加载困难（typeid / vtable 不安全）
❌ 插件之间 ABI 不一致
👉 直接破坏 MLIR 的 extensibility
❌ 3. 致命问题二：RTTI / typeid 跨动态库不可靠
如果你写：
typeid(*op) == typeid(ConstantOp)
在多动态库场景：
不同 .so 里的 ConstantOp 的 typeid 可能不同
👉 结果：
❌ dyn_cast 失效
❌ interface dispatch 出问题
❌ UB 风险
❌ 4. 致命问题三：内存 & 性能问题
如果每个 op 都是一个 C++ 类：
每个 Operation 都有：
→ vtable
→ 不同 size
→ 不同 layout
👉 会导致：
❌ 内存不连续（cache 不友好）
❌ allocator 复杂
❌ IR 遍历变慢
✅ 5. MLIR 的解决方案（核心设计）
MLIR 选择：
Operation（统一数据结构）
+ Op wrapper（语义层）
+ classof（类型判断）
🔹 Operation（统一内存布局）
class Operation {
  OperationName name;
  OperandStorage operands;
  ResultStorage results;
  ...
};
同一个结构！连续存储！cache 友好！
🔹 Op wrapper（语义层）
class ConstantOp : public Op<ConstantOp, ...> { ... };
👉 只是：
Operation* 的“视图 + API”
✔ 不增加内存
✔ 不影响布局
🔹 classof（动态类型判断）
static bool classof(Operation *op) {
  return op->getName() == "arith.constant";
}
👉 实际是：
Identifier 指针比较（O(1)）

# interface机制
interface dialect interface /operation interface
1.dialect interface适用于整个dialect的op,影响跨op
2.opeartion interface适用于dialect的某个op,只影响该op本身
| 维度   | OpInterface             | DialectInterface      |
| ---- | ----------------------- | --------------------- |
| 作用对象 | 单个 op                   | 整个 dialect            |
| 谁实现  | op 类                    | dialect 类             |
| 调用方式 | dyn_cast(op)            | dialect->getInterface |
| 粒度   | 局部行为                    | 全局策略                  |
| 生命周期 | 编译期生成                   | 运行时注册                 |
| 典型例子 | BufferizableOpInterface | InlinerInterface      |

# lowering
partial lowering = IR 会同时存在“高层语义（tensor）”和“低层语义（memref）”
→ 必须设计 bridge（桥接机制）
mlir::applyPartialConversion(op, target, patterns)
只要求：所有“被标记为 illegal 的 op”必须被转换
允许：IR 中存在仍然合法的“高层 op”

full conversion
所有 op 都必须合法
→ IR 完全变成目标 dialect

# mlir 和 llvm ir的本质不同
MLIR
“语义驱动的编译器框架”
🔷 LLVM
“执行驱动的底层 IR”
🎯 10. 一句话总结（建议背）
MLIR 负责“保留语义并逐步降低”，
LLVM 负责“执行这些语义”
🚀 11. 再给你一个更本质的理解（高手视角）
MLIR = 编译器的“中间世界”（优化空间）
LLVM = 编译器的“现实世界”（执行空间）

# populateXXXPatterns 
 “往 pattern 集合里批量注册一组 lowering 规则”
 patterns.add
patterns.add<MyPattern>();
👉 你自己写的规则
🔥 populateXXX
populateXXXPatterns(patterns, ...)
👉 MLIR 官方提供的一整套规则

# Structured Ops 的本质：
用 IR 显式表达“循环结构 + 数据访问 + 计算语义”，
mlir可以不一定是结构化的,但结构化用于保存语义,便于优化
从而让编译器可以做高层优化
Structured Op = 明确描述：
- iteration space（循环空间）
- indexing map（访问方式）
- computation（计算逻辑）
举个例子：fusion
C = matmul(A, B)
D = relu(C)
在 Linalg：
✔ 知道是 elementwise + matmul
✔ 可以 fuse
在 LLVM IR：
❌ 只是 load/store/add
❌ 根本不知道可以 fusion
MLIR 把：
“for 循环 + 标量操作”

抽象成：
“高维向量 + 逐元素操作”
🎯 一句话总结（建议记住）
Uniform Elementwise Extension 的本质是：
让一个标量算子天然具备 N 维并行语义，
并延迟展开为具体循环或指令

为什么“保留结构”这么重要？
vector.reduction
🔥 如果一开始就变 loop
❌ 编译器不知道这是 reduction
👉 那就无法：
❌ 用 SIMD reduction 指令
❌ 做 tree reduction
❌ 做并行 reduction
✅ 保留为 vector.reduction
✔ 延迟决定实现方式
✔ 可以 target-specific lowering

# linalg.generic = “计算 + 调度解耦”

👉 计算：
region 里定义
👉 调度：
tiling / fusion / parallel

🔷 vector.contract
✔ 操作在寄存器（SSA value）
✔ 不可变（functional）
✔ 计算是固定的（mul+add）
🔷 linalg.generic
✔ 操作在内存（memref）
✔ 有“读 + 写”
✔ 计算由 region 定义（任意）

# tensor memref vector 为什么“设计成相同结构”？
tensor 满足SSA ,只产生新tensor memref不行(别名,指向同一块内存,含内存size effect)
文中说：

“specifically designed this way”
linalg.generic(tensor)
≈
linalg.generic(memref)
≈
vector.contract（思想）
👉 这样可以：
✔ 同一套优化逻辑复用
✔ 只换“数据表示”
👉 这就是：
“结构不变，表示变化”
🧠 9. 一个非常关键的理解（高手视角）
tensor = “延迟决定内存布局”
👉 为什么？
没有 alloc / dealloc
没有 pointer
没有 layout 约束
👉 编译器可以：
✔ 决定什么时候分配
✔ 决定 layout
✔ 决定是否 in-place

# tiling and fusion
🔥 tiling 在 MLIR 的本质
1. 利用“隐式 iteration space”
2. 切分为多个 tile
3. 用 loop 控制 tile 执行
4. 保持语义不变
🎯 一句话总结（建议记住）
MLIR 中的 tiling 本质是：
用“slice + loop”显式化原本隐式的计算结构，从而控制数据局部性和并行性
🔥 最关键的一句话（面试杀手）
因为 linalg.generic 是无副作用 + 无顺序语义，
所以 tiling 不需要 dependence analysis
这段 IR 的本质就是：把“先算整张表再用”，变成“按块算 + 当场用”，从而节省内存并提高性能
fusion 的本质 = 把 “producer 的 slice” 替换为 “producer 的局部计算”
✔ 1. 消除中间 tensor
不再 materialize C
✔ 2. cache locality 极好
tile 内计算 → 直接消费
✔ 3. 支持复杂算子
matmul + elemwise + activation + bias

# transform dialect
指导mlir优化的ir
precisely target transformations at specific operations
你可以指定：只优化某一个 op
🔥 举个例子
❌ 传统方式
所有 matmul 都 tile
✅ Transform Dialect
只 tile 这个 matmul（比如 shape=512x512 的）
👉 精细控制：
✔ 哪个 op
✔ 用什么策略
✔ 顺序如何
为什么 IREE 用它？
✔ 不同硬件（CPU / GPU / Vulkan）
✔ 不同调度策略
✔ 可调优（auto-tuning）

👉 Transform Dialect 正好解决：

“调度策略可编程” 类比
| 系统     | 调度方式              |
| ------ | ----------------- |
| Halide | schedule 语言       |
| TVM    | schedule API      |
| MLIR   | Transform Dialect |
✔ loop tiling
✔ loop fusion
✔ loop reordering
✔ mapping to GPU

# mlir lowering
有，而且你已经抓到核心了。MLIR lowering 常见设计哲学可以概括为这几条：

渐进降级（Progressive Lowering）
不追求一步到位，分层逐步把高语义变成低语义。

语义保留到最后一刻（Preserve Semantics Late）
在能优化的阶段尽量保留结构信息（如 linalg 的索引映射、并行/归约语义），避免过早摊平成“丑 IR”。

关注点分离（Separation of Concerns）
类型、控制流、数据布局、算术、设备执行分别交给不同 dialect/pass 处理。

合法化驱动（Legality-Driven）
每个 pass 都有“目标 dialect 合法集合”，通过 conversion pattern 把非法 op 逐步清空。

可组合、可插拔（Composable/Extensible）
pipeline 可按前端/后端拼装，插件可在 Input 等阶段注入。

先规范化再优化（Canonicalize then Optimize）
大量 canonicalize/CSE 穿插在 lowering 中，保证模式匹配稳定、后续优化收益更高。

多层 IR 为多类优化服务（Right Level for Right Optimization）
算子融合/分块常在 linalg，控制流细化在 scf，更底层指令化在 llvm/gpu 等，不强行在单层做完所有事。

一句话：
MLIR 的 lowering 不是“降成越低越好”，而是“在每一层保留恰好足够的语义来做那一层最有效的优化”。

# Constraint 
mlir内部的约束系统
Single-entity	单个 operand/attr/result
Multi-entity	多个实体关系
Trait	op 整体性质
Multi-entity可以看成 PredOp Trait,最终会生成 verifier
Constraint Primitive
MLIR 提供很多 primitive。
例如：

type equality
AllTypesMatch
shape equality
AllShapesMatch
rank constraint
HasRank<2>
custom predicate
CPred<"...">

def MyAddOp : MyDialect_Op<"add", [
    SameOperandsAndResultType,
    NoMemoryEffect
]> {

  let arguments = (ins
    TensorOf<[F32]>:$lhs,
    TensorOf<[F32]>:$rhs
  );

  let results = (outs
    TensorOf<[F32]>:$result
  );
}

Single-entity constraint
TensorOf<[F32]>

保证：

必须是 f32 tensor
Multi-entity constraint
SameOperandsAndResultType

保证：

lhs/rhs/result type 一致
Trait
NoMemoryEffect

保证：

无 side effect

# region ssacfg和graph
| 维度                  | 控制流图（CFG, Control Flow Graph）    | 依赖图（Dependency Graph / Dataflow Graph） |
| ------------------- | -------------------------------- | -------------------------------------- |
| 核心语义                | 程序如何执行                           | 数据/任务谁依赖谁                              |
| 本质驱动                | control-driven                   | dependency-driven                      |
| 图节点                 | basic block / instruction        | operation / task / tensor op           |
| 图边含义                | 执行路径转移                           | 数据或依赖关系                                |
| 边表示                 | “下一步可能执行谁”                       | “谁的结果被谁使用”                             |
| 是否表达执行顺序            | 是，核心语义                           | 通常不是                                   |
| textual order 是否重要  | 通常重要                             | 通常不重要                                  |
| 是否存在程序计数器语义         | 有                                | 没有                                     |
| 是否表达 branch/if/loop | 是                                | 通常弱化或不存在                               |
| 是否表达“执行路径”          | 是                                | 否                                      |
| 是否表达“可达性”           | 是                                | 不强调                                    |
| 是否表达“依赖关系”          | 间接                               | 核心                                     |
| operation 是否像“指令”   | 是                                | 更像“计算节点”                               |
| SSA value 含义        | 程序变量                             | 图边(edge)                               |
| dominance 是否核心      | 是                                | 通常弱化                                   |
| side effect 顺序      | 默认存在                             | 通常需显式 dependency                       |
| 是否天然支持并行            | 不强                               | 强                                      |
| graph 中无边代表什么       | 不一定能并行                           | 可以独立执行                                 |
| graph 中环的含义         | 控制流循环(backedge)                  | 反馈依赖(feedback loop)                    |
| cycle 的本质           | 重复执行                             | 循环依赖                                   |
| 调度模型                | sequential execution             | partial order execution                |
| 是否要求 total order    | 倾向是                              | 不要求                                    |
| 能否自由 reorder op     | 通常不能                             | 通常可以                                   |
| 内存状态                | 隐式全局状态                           | 常需显式依赖                                 |
| IR 更像               | 程序                               | 数学关系图                                  |
| 数学本质                | 状态机(state transition system)     | 偏序关系(partial order)                    |
| 编译器分析重点             | reachability / dominance / loops | dependency / scheduling / fusion       |
| 优化核心                | 控制流优化                            | 图调度/融合                                 |
| 典型优化                | LICM、branch folding              | fusion、topological scheduling          |
| 更接近 CPU 模型          | 是                                | 否                                      |
| 更接近 AI 计算图          | 否                                | 是                                      |
| 更接近 von Neumann     | 是                                | 否                                      |
| 更接近函数式/dataflow     | 否                                | 是                                      |
| 典型 IR               | LLVM IR、SCF、Machine IR           | ONNX、StableHLO、FX、Relay                |
| MLIR RegionKind     | SSACFG                           | Graph                                  |
| lowering 方向         | 接近机器执行                           | 接近数学计算                                 |
| 最终通常会不会变 CFG        | 已经是                              | 通常会                                    |
| 编译器最终目标             | 执行程序                             | 找到合法执行调度                               |

# operation
operation             ::= op-result-list? (generic-operation | custom-operation)
                          trailing-location?
generic-operation     ::= string-literal `(` value-use-list? `)`  successor-list?
                          dictionary-properties? region-list? dictionary-attribute?
                          `:` function-type
custom-operation      ::= bare-id custom-operation-format
op-result-list        ::= op-result (`,` op-result)* `=`
op-result             ::= value-id (`:` integer-literal)?
successor-list        ::= `[` successor (`,` successor)* `]`
successor             ::= caret-id (`:` block-arg-list)?
dictionary-properties ::= `<` dictionary-attribute `>`
region-list           ::= `(` region (`,` region)* `)`
dictionary-attribute  ::= `{` (attribute-entry (`,` attribute-entry)*)? `}`
trailing-location     ::= `loc` `(` location `)`



为什么 region 里的计算更容易 fusion
在linalg中,computation structure 被一等公民化了
因为它是：
pure scalar function
例如：
relu(add(a,b))
组合函数即可。

而 LLVM：
已经变成：
memory program
store tmp
load tmp
branch
这时候 fusion 必须：
重建 dependence
消除 memory
修改 CFG
复杂度暴增。
Linalg fusion 本质上是在组合数学函数，
LLVM fusion 本质上是在重写底层程序。

Linalg Named Ops（命名形式）

Linalg 提供很多 shorthand：

matmul
convolution
dot
pooling

等。
%matmul = linalg.matmul
    ins(%lhs, %rhs :
        tensor<8x10xf32>,
        tensor<10x16xf32>)
    outs(%init :
        tensor<8x16xf32>)
    -> tensor<8x16xf32>

它本质上等价于：
对应的 linalg.generic。

不需要手动写：
indexing_maps
iterator_types
region body
编译器内部仍然知道：

它是 structured op。

# transoform dialect

基础构成
top-level operation: transform.named_sequence @__transform_main 
__transform_main是入口主函数
Interpreter Pass 必须保证入口函数名和第一个参数
直接使用
applyTransforms/applyNamedSequence时，
- 使用任意 sequence 名字
- 不使用 __transform_main
- 不依赖固定参数形式
- 手动绑定 payload handles
handle: arg指针 SSA 左边的对象,一个handle可以指向payload ir的多个entity
Transform handle
本质是 payload operation 的引用。
tile/fuse 等 transformation
通常会 erase/recreate payload op。
原 handle 会失效（consumed）。
transform dialect 要求每个 transform op
显式声明：
- 哪些 handles 是 readonly
- 哪些 handles 会被 consumed
从而避免 dangling handles。

做tile-fuse 时
“从最后一个 op 开始切 tile，然后向前融合 producer”
consumer决定需要的slice,produer根据slice自动tile
dataflow-driven tiling
tile+fuse 的本质：
把 producer linalg op
移动/克隆
到 consumer tile loop 内(不是consumer linalg内)
也不是把多个 linalg region
合并成一个 region
“跨 op 的 producer-consumer fusion”
主要发生在：   scf loop nesting 层
merge_handles将多个handle合并成一个
%all = transform.merge_handles %add, %arg1
fuse_into_containing_op 会自动做拓扑排序,决定fuse顺序

transform dialect 的layout

定义transform op,类似dialect op,但要在自己的
// In extenison.cpp (don't forget a declaration in TransformDialect.h);
extenison ::mlir::transform::TransformDialectExtension中的init里  
registerTransformOps<
    // TODO: list the operation classes.
  >();

// In TransformDialect.cpp (don't forget a declaration in TransformDialect.h);

void registerMyExtension(::mlir::DialectRegistry &registry) {
  registry.addExtensions<MyExtension>();
}
ODS的定义中不能缺少的两个interface
[DeclareOpInterfaceMethods<TransformOpInterface>,
    DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> 

type/attribute 同理
DeclareTypeInterfaceMethods<TransformHandleTypeInterface>
void MyExtension::init() {
  // ...

  registerTypes<
#define GET_TYPEDEF_LIST
#include "MyExtensionTypes.cpp.inc"
  >();
}


op : 负责指向transform本身的过程
handle ：transform 的数据流,op的传入参数,返回结果,指向一类operation,类似set<xxOp>
type 是 handle的类型/约束 : 这个 handle 能关联哪些 payload op,规范指向的op类型

transform match operation
以脚本的形式匹配某类op获取handle,替代外部interpreter/pass 辅助输入参数 handle 
transform.collect_matching
transform.match.operation_name


Matching Chains
匹配一组特定的 operation 链,如 matmul elementwise elementwise
// It starts matching from the last operation in the use-def chain
// and goes back because each operand (use) has exactly one definition.

它从 use-def 链中的最后一个 operation 开始匹配，
Transform dialect matcher
正在做：
use-def graph pattern matching

match operation定义
[DeclareOpInterfaceMethods<TransformOpInterface>,
    DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> 
    MatchOpInterface(目前只是个tag,无需要实现的方法)

for
    match then action 匹配替换模式,但dialect实现
transform.foreach_match in %root
@match_matmul_elemwise -> @print_matmul_elemwise
: (!transform.any_op) -> !transform.any_op

property inference 基于结构语义推导的特征分析匹配
真正 robust 的 matcher

必须匹配：
“它是不是一个 matmul-like computation”
而不是：“它名字是不是 matmul”
语义推断,而不是单纯字符串名称匹配
如匹配matmul
条件1:Total rank of 3. iterator space i,j,k
对应 iterator_types = [
  "parallel",
  "parallel",
  "reduction"
]
条件2:两个输入参数是投影permutation
lhs i,k rhs k,j
条件3:一个输出同样是投影permutation
out i,j
条件4:迭代维度必须可以划分为子
iteration dimensions can be subdivided into:
lhs parallel
rhs parallel
reduction
i -> lhs parallel
j -> rhs parallel
k -> reduction
contraction
条件5:body部分必须有乘法和加法
arith.mulf
arith.addf
linalg.yield %add

只通过推导而不是trait,通过
分析indexing map
iterator type
region body 


可以在 apply() 里写复杂分析逻辑
bool isMatmulLike(Operation *op) {
    auto generic = dyn_cast<linalg::GenericOp>(op);
    checkIndexingMaps(...);
    checkIteratorTypes(...);
    checkReduction(...);
    checkBodyPattern(...);
}

transform matcher：
transform.structured.match.matmul_like
匹配 generic matmul
Transform matcher op：
本质是 C++ analysis + handle system
做 compiler-analysis-level 的匹配
MLIR 已经提供了：
结构化线代 matcher 库
contraction matcher
reduction matcher
transpose matcher
broadcast matcher

和其他匹配的区别
系统	核心目标
DRR	declarative peephole rewrite
PDL	declarative IR pattern matching
RewritePattern	SSA DAG rewrite
Transform Dialect	transformation orchestration + structural analysis

# Reproducing Halide Schedule
计算和调度分离 参考官网,通过transform dialect 将halide schedule写到 mlir

# bufferization
bufferization不是一个 pass而是一整套 passes/framework

对比tensor buffer
无 alias
immutable
SSA graph
producer-consumer 明确

buf 是否 alias
是否会提前 overwrite
是否需要 copy
是否 escape

One-Shot Bufferize
它和传统 bufferization 最大区别：
不是：
先分配buffer
再做alias analysis
而是：
直接在 tensor SSA 图上推导
哪些 tensor 可以共享同一个 buffer
1. designed for IR in destination-passing style
Destination-Passing Style (DPS)
op显式提供：输出 buffer/destination
%0 = linalg.matmul
  ins(%A, %B)
  outs(%C) 
destination --outs(%C) 即输入和输出绑定同一块tensor
为后续bufferization 提供inplace的可能
op举例:
linalg.* op
tensor.insert_slice
tensor.parallel_insert_slice

aggressive in-place bufferization
尽量避免 copy
2. Monolithic
analysis + rewrite 由一个pass 完成

3. Extensible via op interface
只要实现：
BufferizableOpInterface 可接入One-Shot Bufferize

4. whole-function at a time analysis
整个 function SSA 图分析,inplace合法性依赖def-use chain

5. 2-Phase
一阶段:analysis 哪些 inplace 哪些 copy
alias set equivalence set
先做SSA分析,而不是先buffer再做alisa分析(别名分析成本高)
二阶段: rewrite
6. Greedy
analyzed one-by-one
不求全局最优求解,greedily 决定,基于heuristic
7. Modular
analysis可替换，通过AnalysisState
查询：
isInPlace
alias info
equivalence info
可以
自定义 analysis
自定义 inplace policy
自定义 copy policy
甚至：
AlwaysCopyAnalysisState
永远copy
8. does not deallocate buffers
不负责free
通过Ownership-based Buffer Deallocation

## bufferization的目标
1.尽可能少用内存,少alloca
2.尽可能少copy，尽可能就地
类似Register Allocation
resource reuse problem

Register Allocation
尽量少寄存器
尽量少spill
尽量复用寄存器
都有:
1. lifetime analysis
2. interference
3. aliasing
4. reuse legality
5. graph coloring 类问题
其他问题:
recomputation vs copy tradeoff
If the contents of a buffer are expensive to compute
可能compute once + copy更划算
哪种更好取决于(构建一个参数化的成本模型?)：
FLOPs
bandwidth
cache
tensor size
hardware
即不仅仅是memory optimization有可能是compute-memory tradeoff
这和：
activation checkpointing
rematerialization
是同类问题。
LLVM register allocation：
也有：
spill
vs
recompute  的tradeoff。

某些硬件不允许动态alloc
some architectures cannot allocate runtime buffers
DSP
MCU
embedded accelerator
SRAM-only NPU
realtime systems

所有 memory 静态规划编译时必须知道：
buffer 数量
buffer 大小
buffer lifetime
reuse plan
因此bufferization在这些系统里非常接近 static memory scheduling
bufferization->address assignment

DPS 让 bufferization 变得“可解”
bufferization本质NP-hard memory reuse problem,如何安全高效的找到可以inplace的buffer

One-Shot Bufferize was designed to take advantage of destination-passing style,对这种op的inplace有效
例:
%r = tensor.insert %f into %t[%idx] 中
%t是 destination operand,result %r基于其构建,那么%t是其可以复用的buffer中的选择之一,不然常规思维,需要遍历全部的buffer
possible “anchor” for the bufferization algorithm
This allows the user to shape the input in a form that guarantees close to optimal bufferization
即IR 主动暴露优化结构,“这里可能适合 inplace”少copy，少alloc，做alias
%r = linalg.generic
     ins(%A)
     outs(%B)
result %r  对应 destination %B
然后尝试 result buffer alias %B buffer
必须满足：没有冲突 use,如果旧值t被使用,那么需要新alloc

One-Shot Bufferize 是“局部贪心”的
故意限制buffer reuse 搜索空间
DPS op：
%r = linalg.generic outs(%d)
One-Shot Bufferize：
会优先考虑：
复用 destination 的buffer
buffer(%r) = buffer(%d)
这是One-Shot Bufferize最核心的inplace candidate
而这里作者说：
There may be other buffers in the same function
除了buffer(%d)以外。函数里可能还有：
buffer(%x)
buffer(%y)
buffer(%z)
这些其实也可能已经 dead。理论上也能复用给
buffer(%r)
例如：
%0 = ...
%1 = ...
%2 = linalg.generic outs(%1)
虽然destination是%1但也许%0已经dead，而：buffer(%0)大小shapelayout都兼容。
那么理论上完全可以buffer(%r) = buffer(%0)
这就像register allocation里的 任意空闲寄存器复用
但目前只考虑destination buffer,保证简洁性
如果复用,会引入全局内存调度,加大复杂性
lifetime analysis
interference graph
alias legality
dominance
escape analysis
shape/layout compatibility
memory space compatibility
cost model
未来可能添加：
arbitrary buffer reuse
global memory pools
static memory assignment
heap coloring
rematerialization tradeoff
async overlap
One-Shot Bufferize：

将tensor.x op 打包改写成dps的linalg,
在做extract_slice/update/等操作时,看似的 immutable SSA COPY,都变成了memref subview inplace alisa
%t = tensor.extract_slice %s [%idx] [%sz] [1] : tensor<?xf32> to tensor<?xf32>
%0 = linalg.generic ... outs(%t) { ... } -> tensor<?xf32>
%1 = tensor.insert_slice %0 into %s [%idx] [%sz] [1]
    : tensor<?xf32> into tensor<?xf32>

%t = tensor.extract_slice %s -> %sub = memref.subview %buffer_of_s
subview不是新内存,而是原buffer的一段view,alias 原内存
linalg.generic对alias 原内存进行操作,能否直接修改,避免RAW,做
alias legality analysis
lifetime analysis
RaW conflict analysis
最后,tensor.insert_slice %0 可以不需要了,

初始状态
runtime 给你 memref
中间
转成 tensor
做高层 tensor optimization
最后
materialize 回已有 buffer

外部 runtime buffer
↔ tensor compiler world
↔ bufferized memref world

## external model
op延迟绑定interface,不造成循环依赖
linalg.generic
本身定义时并没有实现 BufferizableOpInterface，而是在：
Linalg/Transforms/
里额外注册：
struct GenericOpInterface
  : BufferizableOpInterface::ExternalModel<...>

这样 One-Shot Bufferize 才知道
result 是否 alias outs operand
是否允许 inplace
遇到写操作是否需要 copy
如何从 tensor IR rewrite 到 memref IR

但如果开启：
-allow-unknown-ops
则 One-Shot 会把 unknown op 当成黑盒处理，在边界插：
bufferization.to_buffer
bufferization.to_tensor
即：
tensor world
  ↓
memref black box
  ↓
tensor world
这样 unknown op 不参与 One-Shot analysis，只是被隔离起来。

bufferization::bufferizeOp
这个 API 和 One-Shot 最大区别是：
它不做 analysis。
One-Shot 的核心价值其实是：
alias analysis + inplace decision
而 bufferizeOp 不分析，直接采用最保守策略：
写之前先 copy
相当于：AlwaysCopyAnalysisState
因此简单，但性能很差。

One-Shot Bufferize can be configured to bufferize only ops from a set of dialects with dialect-filter
pass只对dialect-filter里的dialect的op进行bufferization

函数间的bufferization暂时没有实现,复杂度太高

## Memory  layouts
%0 = "my_dialect.unbufferizable_op"(%t)
     : (tensor<?x?xf32>) -> tensor<?x?xf32>

%1 = tensor.extract %0[%i, %j]

这里 tensor.extract 是可以 bufferize 的，但 my_dialect.unbufferizable_op 不会 bufferize，因为它没有实现 BufferizableOpInterface。如果开启了 allow-unknown-ops，One-Shot 不会直接报错，而是会在边界插一个：
%0_m = bufferization.to_buffer %0

问题就来了：%0_m 的 memref type 应该是什么？

tensor type 只有 shape 和 element type
memref type 还包含 layout 信息，例如 stride、offset、是否 contiguous
但 One-Shot 现在根本不知道 unbufferizable_op 未来会如何 lower。它以后可能会变成：
contiguous buffer
subview
transpose
tiled layout
带动态 stride 的 view
因此如果现在贸然假设 identity layout，未来真正 bufferize 这个 op 时，可能会发现 layout 根本不匹配。
所以默认策略是最保守的：
memref<?x?xf32, strided<[?, ?], offset: ?>>
即 fully dynamic layout
另一个选项：

identity-layout-map

会强行生成：
memref<?x?xf32>
这种传统连续布局。
这对很多老 backend 更友好，因为它们不支持复杂 memref layout。

但缺点是：如果未来真实 layout 不是 identity，那么可能需要插额外的 memref.cast 甚至 buffer copy。

identity layout 指的是：

逻辑索引
直接按默认连续内存布局
映射到物理地址
没有：
transpose、padding、tiling、subview、非连续 stride 等额外映射。
有些bufferizable的op可能也无法buffer,如
tensor.cast tensor<*xf32>
         to tensor<?x?xf32>
因为无法知道stride/offset/contigious,不能保证真实layout,采用 
fully dynamic layout
就是说,在无法bufferization时,可以采用
fully dynamic layout或identity layout 

## 支持 One-Shot Bufferize
Users must at least implement the following interface methods
bufferizesToMemoryRead: Return true if the buffer of the given tensor OpOperand is read.
bufferizesToMemoryWrite: Return true if the buffer of the given tensor OpOperand is written (if bufferizing in-place).
getAliasingOpResult: Return the OpResults that may share the same buffer as the given OpOperand. This interface method describes to OpOperand-to-OpResult mapping wrt. destination-passing style.
bufferRelation: Return BufferRelation::Equivalent if the given OpResult is the exact same memref as the aliasing OpOperand after bufferization (in case of in-place bufferization). Otherwise, (e.g., they overlap but are not necessarily the exact same memrefs), BufferRelation::Unknown should be returned. Additional buffer relations will be added in the future, but BufferRelation::Unknown is always safe.
描述opresult和alias operand的alias程度,完全相同,返回BufferRelation::Equivalent，如果只是overlap,如slice/view/subview,返回Unknown“它们可能共享内存，但关系不够精确”
bufferize: Rewrite the op with the given rewriter. Ops should be replaced with bufferization::replaceOpWithBufferizedValues
当已经有DestinationStyleOpInterface时,可以从DstBufferizableOpInterfaceExternalModel直接继承,只需要实现bufferize方法。

func.func @test(%arg0: f32, %arg1: f32, %arg2: index, %arg3: index) -> (f32, tensor<3xf32>) {
  // Create a new tensor with [%arg0, %arg0, %arg0].
  %0 = tensor.from_elements %arg0, %arg0, %arg0 : tensor<3xf32>

  // Insert something into the new tensor.
  %1 = tensor.insert %arg1 into %0[%arg2] : tensor<3xf32>

  // Read from the old tensor.
  %r = tensor.extract %0[%arg3] : tensor<3xf32>

  // Return the extracted value and the result of the insertion.
  func.return %r, %1 : f32, tensor<3xf32>
}

func.func @test(%arg0: f32, %arg1: f32, %arg2: index, %arg3: index) -> (f32, tensor<3xf32>) {
  %from_elements = tensor.from_elements %arg0, %arg0, %arg0 {"C_0[DEF: result 0]"} : tensor<3xf32>
  %inserted = tensor.insert %arg1 into %from_elements[%arg2] {"C_0[CONFL-WRITE: 1]", __inplace_operands_attr__ = ["none", "false", "none"]} : tensor<3xf32>
  %extracted = tensor.extract %from_elements[%arg3] {"C_0[READ: 0]", __inplace_operands_attr__ = ["true", "none"]} : tensor<3xf32>
  return {__inplace_operands_attr__ = ["none", "true"]} %extracted, %inserted : f32, tensor<3xf32>
}

Every operation with tensor semantics has a __inplace_operands_attr__ attribute with one value per operand. If an operand is not a tensor, the respective value is none. Otherwise, if the operand was decided to be bufferized in-place, the value is true. A value of false indicates a buffer copy. In the above example, a buffer copy would be inserted for tensor.insert, so that it does not overwrite buffer(%from_elements), which is still needed for tensor.extract
如果一个op的operand不是tensor,那么它__inplace_operands_attr__对应的下标处为none,如果是in-place,为true,不是的话,为false

For each RaW (there is only one in the example), three C_i attributes were added:

C_0[DEF: result 0]: A tensor is defined: 0-th result of tensor.from_elements.
C_0[CONFL-WRITE: 1]: An operation (if bufferized in-place) would write into the future buffer of the defined tensor: 1-st operand of tensor.insert.
C_0[READ: 0]: An operation reads the tensor definition: 0-th operand of tensor.extract.

func.func @test(%arg0: f32, %arg1: f32, %arg2: index, %arg3: index) -> (f32, memref<3xf32>) {
  %c2 = arith.constant 2 : index
  %c1 = arith.constant 1 : index
  %c0 = arith.constant 0 : index
  %alloc = memref.alloc() {alignment = 64 : i64} : memref<3xf32>
  memref.store %arg0, %alloc[%c0] : memref<3xf32>
  memref.store %arg0, %alloc[%c1] : memref<3xf32>
  memref.store %arg0, %alloc[%c2] : memref<3xf32>
  %alloc_0 = memref.alloc() {alignment = 64 : i64} : memref<3xf32>
  memref.copy %alloc, %alloc_0 : memref<3xf32> to memref<3xf32>
  memref.store %arg1, %alloc_0[%arg2] : memref<3xf32>
  %0 = memref.load %alloc[%arg3] : memref<3xf32>
  return %0, %alloc_0 : f32, memref<3xf32>
}

# Data Layout Modeling
mlir中,data layout是以attribute的形式添加到op上的,一般是module op
Data Layout 的核心作用是回答：
“某个 type 在内存里到底长什么样”
因为 IR 里的 type：
i32
vector<4xf32>
memref<...>
tensor<...>
是抽象类型,而具体的
size
alignment
offset
stride
vector packing
pointer width
必须知道它们才能落实到真实内存里
LLVM 的 type system 基本封闭：

i32
struct
pointer
vector
但在mlir中,因为支持type的拓展interface-driven extensible system
1. attribute interfaces
#dlti.dl_spec<...>里面描述：
alignment
pointer size
ABI info
2. type interfaces
在type中描述
alignment
pointer size
ABI info
3. operation interfaces 
在op中描述
alignment
pointer size
ABI info
data layout scope module op
4. dialect interfaces
为整个dialect给出layout描述

## scope
对于含有region的OP,它会给region中其他的op赋一个默认的layout描述,前提要实现
DataLayoutOpInterface or ModuleOp。
在region或嵌套的op中,可以自己拓展/覆盖/修改默认的layout,但是必须inner必须兼容outer
DataLayout 查询很贵，
所以 MLIR 做了 cache
找当前 scope
往外层 scope 查 layout spec
merge 多层 layout
查 dialect interface
查 type interface
解析 alignment/layout rule
计算 vector/container layout

# pattern rewriter
restriction
1. benefit 根据benefit选择匹配对象
2. restriction,所有的ir mutation,必须通过patternRewriter
3. root operation 要么修改/删除/就地更新

递归处理,一个pattern 有可能rewriter成自己还可以match的情况,检测到这种情况,不允许跑,除非
pattern显式声明setHasBoundedRewriteRecursion();
RewritePattern
    ↓
描述 rewrite 规则

RewritePatternSet
    ↓
收集 patterns

PatternApplicator
    ↓
pattern dispatch + cost model

PatternRewriter
    ↓
安全 IR mutation

PatternDriver
    ↓
控制 rewrite 流程/fixpoint/worklist