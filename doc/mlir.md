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
