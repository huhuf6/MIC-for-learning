# 什么是算子?
通俗来说是在加速器或GPU上执行计算的代码,在前端只有计算语义,叫算子,通常说的写算子,其实是写kernel,即计算如何在硬件设备上高速执行

# 什么是计算图调度
调度是在不改变计算语义的前提下,选择计算的具体实现方式。TVM 的“调度”需要分为两个层次:

```text
Relax IR: 决定哪些高层算子组合、融合和 Lowering
    ↓
TIR PrimFunc: 决定循环、内存、并行和硬件指令如何实现
```

## Relax 图级变换

Relax 的操作对象是算子图和函数,它没有像 `tir.Schedule` 那样的循环调度原语,主要通过 Pass 完成图级改写:

- `FuseOps`: 将多个算子划分到一个融合区域。
- `FuseTIR`: 将融合后的 Relax 函数变成 TIR `PrimFunc`。
- `MergeCompositeFunctions`: 按模式将算子组合成复合函数,常用于外部后端分区。
- `LegalizeOps`: 将 Relax 算子降低为对应的 TIR 实现。
- `ConvertLayout`: 改变算子和张量的数据布局。
- `FoldConstant`: 在编译期计算常量表达式。
- `DeadCodeElimination`: 删除无用计算。

例如:

```text
matmul -> add -> relu
       FuseOps
           ↓
fused(matmul, add, relu)
           ↓ LegalizeOps/FuseTIR
       TIR PrimFunc
```

Relax 主要回答“哪些算子放在一起,使用什么布局和实现”。

## TIR 调度原语

TIR 的操作对象是循环、Block 和 Buffer,用来确定算子在 CPU/GPU 上的具体实现。

```text
Loop Transformation
├── split                 # 拆分循环,可用于 tiling
├── fuse                  # 合并多个循环
├── reorder               # 重排循环顺序
└── add_unit_loop         # 添加长度为 1 的循环

Parallelization
├── parallel              # CPU 多线程并行
├── vectorize             # SIMD 向量化
├── unroll                # 循环展开
└── bind                  # 绑定 GPU block/thread 等硬件线程轴

Producer/Consumer
├── compute_at             # 将生产者移到消费者的某层循环内
├── reverse_compute_at     # 将消费者移到生产者的某层循环内
├── compute_inline         # 内联生产者
└── reverse_compute_inline # 反向内联消费者

Memory
├── cache_read             # 创建读缓存
├── cache_write            # 创建写缓存
├── set_scope              # 设置 global/shared/local 等存储空间
└── storage_align          # 调整 Buffer 存储对齐

Layout
├── transform_layout       # 改变 Buffer 的索引和数据布局
└── transform_block_layout # 改变 Block 迭代空间的布局

Reduction
├── decompose_reduction    # 将归约拆成初始化和更新
└── rfactor                # 拆分为部分归约,便于并行化

Tensorization
├── blockize               # 将循环区域转成可张量化的 Block
└── tensorize              # 映射到 Tensor Core/AVX/VNNI 等硬件 intrinsic
```

`tile` 通常不是新 TIR Schedule 中的独立原语,而是通过 `split + reorder` 组合得到。`tile-and-fuse` 通常先对消费者循环做 `split/reorder`,再用 `compute_at` 把生产者移入 tile 内,因此主要发生在 TIR。

## 自动调优与旧接口

MetaSchedule 通过 `sample_perfect_tile`、`sample_categorical`、`sample_compute_location` 等采样决策构造和搜索 TIR 调度空间,再使用 `MetaScheduleTuneIRMod`/`MetaScheduleApplyDatabase` 调优并应用最佳记录。

旧版 TVM 使用 TE Schedule + AutoTVM,例如 `s[C].split()`、`s[C].fuse()`、`s[C].compute_at()`;新版主要使用 TIR Schedule + MetaSchedule。两者都属于底层实现调度,只是接口和搜索体系不同。

简单判断:

```text
操作对象是算子图/函数      -> Relax
操作对象是循环/Block/Buffer -> TIR
操作对象是调度参数搜索      -> MetaSchedule
```

# 算子融合
计算图高层将多个算子融合成一个算子,减少中间内存访问,但是也会增加局部资源的消耗

# tvm 张量化
通过张量指令优化低阶算子性能,并通过调整前端计算图,以取得更好的计算效果,tir上完成分析和转换后,会转成更低阶的ir,如llvm ir

# tvm 自动调优
tvm可以为深度学习系统的复杂算子编译高性能底层代码实现
第一步:定义搜索空间
第二步:运行搜索算法，探索算子实现空间

搜索空间可以认为是对调度的参数化,类似transform dialect的工作,
有哪些参数?
layout/type/loop order/tile大小/loop拆分次数等
通过可调度调度模板,从预定义候选参数集中选择在不同硬件目标上的最优参数组合,
不同输入形状 和 不同目标硬件 下 高性能底层代码的实现
定义并使用不同原语,应用到调度器,得到搜索空间
再通过调优器,找到最优调度 
1.搜索空间小,用随机/顺序遍历
2.搜索空间大GAtuner遗传算法,或者XGBoost模拟退火算法

# 采用代价模型的硬件后端搜索
黑盒模型/基于机器学习的代价模型/预定义代价模型

# XLA JIT 图优化
grapper/graphOptmizer/

优化器:
MARKforcompilationPass 聚类,将tf计算图转换xla计算图,并添加_XlaCluster属性
EncapsulateSubgraphPass 将具有_XlaCluster属性的所有节点封装到函数中,并将原图中包含聚类节点的子图替换成对该函数的调用,同时用_XlacompiledKernel标记,传递给buildXla
BuildXlaOpsPass 将_XlacompiledKernel属性值为真的节点替换为_Xla_compile和_xla_Run,前者编译缓存,后者执行

# XLA JIT 代码生成
BuildXlaOpsPass 中 _XlaCompile OpKernel负责代码生成


# IR设计
高阶IR处理张量
低阶处理变量,提供计算调优和内存访问,挖掘硬件潜力
低阶IR:
1.基于halide的ir
2.基于多面体的ir polyhedral model
3.其他

# AI 编译器前端优化?
AI 编译器前端可以对计算图做不同类型和层次的图优化。 例如， 通过算子融合，可以将多个
小操作融合在一起， 减少数据传输；通过常量折叠，可以减少执行开销； 通过静态内存规划pass,
可以为中间张量预先分配内存。 这些优化方法都是图的局部块级优化。 2.1. J 节介绍了涉及多个优
化过程的全局图优化方法。 例如， 数据布局转换是全局图优化的重要环节， 其目的是优化计算图中
张最的数据布局，因为相同操作在不同数据布局上执行性能可能不同， 而且，不同硬件的最优数据
布局也各不相同。 通过数据布局转换， 将内部数据布局转换为对后端友好的布局形式， 可更好地在
目标硬件执行计算。 例如， 在GPU 上， NCHW格式的操作通常运行速度更快， 因而其他格式的张
量可以先转换为NCHW格式。

# AI 编译器后端
Al 编译器中采用的硬件相关优化方法主要包括： 硬件intrinsic 映射、内存延迟隐藏、循环优化
和并行化。 硬件intrinsic 映射是一种将低阶IR 中的特定操作模式映射为优化内核的机制。 而内存延
迟隐藏的基本思想是通过重叠内存计算操作， 使内存利用率和计算效率最大化。 2.1.2节介绍了在
TVM 中使用硬件intrinsic 映射的方法和示例代码， TVM 中的内存延迟隐藏实现方法将在第4 章中介
绍。 循环优化和并行化是提高深度学习模型中计算密集操作效率的关键。 循环优化中采用的关键技
术包括循环融合、滑动窗口、分片、 循环重新排序和循环展开。 LLVM 巳经集成了循环优化技术，
可以在AI 编译器后端直接使用。 Halide使用并行调度原语来并行化计算任务， 为线程级并行指定
循环的并行化维度。 其中的每个并行任务可以进一步递归地细分为子任务， 以便充分利用目标体系
结构上的多级线程层次结构。 Halide可以用向最语句替换循环，然后通过硬件intrinsic 映射将向批
语句映射为硬件相关的SIMD操作码。 Glow依赖于厂商提供的优化算子库，而且 Glow将向量化处
理放到LLVM 中完成，因为只要有张量尺寸和循环轮次信息， LLVM 自动向量化功能就完全可以正
常工作。 上述编译器后端的各种设计技术利用软硬件设计特性可以实现更好的数据局部性和并行
化，最终将AI模型的计算图转换为不同硬件上的高效机器码实现。

# TVM RELAY/RELAX IR
Relay Dataflow
-> 以共享表达式节点和数据边为主

Relay ANF
-> 嵌套 Let，每个变量单次绑定

Relax
-> SeqExpr + BindingBlock + VarBinding
-> 扁平 ANF，具有 functional SSA 性质

MLIR/LLVM
-> BasicBlock + block argument/phi
-> CFG SSA

可以进行
DCE
liveness
constant propagation
substitution
fusion
memory planning

# TVM的数据表示
TVM 的数据表示包括 张量数据表示、形状表示、数据布局 和 边界推断

## 数据布局和转换pass
ConvertLayout pass  希望能以最少的数据布局转换次数改变整个图的数据布局．
并根据算子类型分别指定不同的ITVMConvertOpLayout和 FlnferCorrectLayout 属性值

# TVM 方言
tvm方言是在图IR级之上的IR,在relay时还使用,在relax时已经内置在relax中

# TVM 定制化后端需要补充接口
TargetKind
-> 支持哪些 target 属性

TIR lowering
-> TIR 如何变成后端能够接受的形式

CodeGen
-> 如何翻译 For、BufferLoad、Call、线程绑定、intrinsic

Runtime Module
-> 如何保存代码、查找函数、启动 kernel

DeviceAPI
-> 如何分配内存、拷贝数据、同步设备
BuildXXX 是完整 TIR 后端必须提供的最终入口；它内部调用哪些方法没有固定要求，但如果目标是新硬件，通常还需要配套的 TIR lowering、代码生成、Runtime Module，必要时还要实现 DeviceAPI

# SelectionDAG

| 依赖 | SDValue 类型 | 作用 |
|---|---|---|
| 数据依赖 | `i32/f32/v4f32/...` | 传递真实计算值 |
| Chain 依赖 | `MVT::Other` | 约束有副作用操作的先后顺序 |
LLVM SDep 明确定义四种调度依赖：
enum Kind {
  Data,    // true dependence
  Anti,    // WAR
  Output,  // WAW
  Order    // 其他顺序依赖
};
call、volatile、barrier
| Glue 依赖 | `MVT::Glue` | 将机器相关节点紧密绑定、连续调度 |

真依赖：后一个操作需要前一个操作产生的值，不能靠重命名消除。

假依赖：没有数值传递，只因复用寄存器名或存储位置产生，寄存器重命名通常可以消除。

# 指令选择

## selDAG
通过vm字节码替换大量类似switch case 的指令选择,自动生成matchtable
在selectioncode中调用,td文件中写pat/patfrag等规则
关键数据结构
class SDnode
{
    int32_t NodeType;        // opcode
    SDUse *OperandList;      // 输入
    const EVT *ValueList;    // 各个输出的类型
    SDUse *UseList;          // 哪些节点使用了本节点
    unsigned NumOperands;
    unsigned NumValues;
}


class SDValue {
  SDNode *Node;
  unsigned ResNo; 某个node的结果
};

class SDUse {
  SDValue Val;       // 使用哪个节点的哪个结果
  SDNode *User;      // 谁在使用它
  SDUse *Next;       // use-list
};

具体 DAG 示例
LLVM IR：
define i32 @foo(ptr %p, i32 %x) {
  %a = load i32, ptr %p
  %b = add i32 %a, %x
  store i32 %b, ptr %p
  ret i32 %b
}
简化后的 SelectionDAG：
t0: ch       = EntryToken

t1: i64, ch  = CopyFromReg t0, %p
t2: i32, ch  = CopyFromReg t0, %x

t3: i32, ch  = load t0, t1
t4: i32      = add t3:0, t2:0
t5: ch       = store t3:1, t4, t1

t6: ch       = CopyToReg t5, $eax, t4
t7: ch       = RET_FLAG t6, $eax

Root = t7 root是SDValue

SDNode EntryNode;          // DAG 起始 chain
SDValue Root;              // DAG 最终 chain
ilist<SDNode> AllNodes;    // 所有节点
FoldingSet<SDNode> CSEMap; // 节点公共子表达式消除

## globalSel
IRtranslator 将llvm ir -> gmir
legalizer 将selDAG的合法化合并,提升或者降级/或者组合成多条指令
regbankselect
instructionSelect
llvm的各个后端需要独立实现这4个pass

## 候选指令匹配规则
让具体模式优先于通用模式
G_ADD x, 5 → 特殊立即数指令
G_ADD x, y → 普通寄存器加法

# 指令调度
在块内或块间进行局部/全局调度
最常见是拓扑排序,对于循环,要遍历到循环不动点,不动点就是所有活跃信息都已在控制流图中传播完毕，再计算一次也不会得到任何新信息的状态。

## 关键路径优先调度算法
关键路径是数据依赖图中节点的最长的路径，从就绪列表中选择指令的标准是最小化关键路径上的指令序列执行时间，减少流水线停顿的发生频率 在实际实现中，可以增加其他优先级策略，如寄存器压力 随着就绪列表中的指令不断被调度，数据依赖图中的其他节点相继被加入就绪列表，并重复上述过程 ，直到所有节点都被调度，数据依赖图为空时，调度过程结束。

全局指令调度可以在基本块间重排指令，需要考虑的情况更为复杂，因而大部分编译器只实现局部指令调度

# LLVM的调度器
调度方向: 自底向上/自顶向下/双向

## 指令选择阶段的调度器
所有调度器的实现类都继承自 ScheduleDAG ScheduleDAG 类的两个子类分别是 ScheduleDAGSDNodes 类和 ScheduleDAGlnstrs,
ScheduleDAGSDNodes是给指令选择阶段做调度的基类,调度对象是SDnode
ScheduleDAGlnstrs是寄存器分配前后的调度器实现基类,对象是Machininstr实例

enum Preference {
  None,        // No preference
  Source,      // Follow source order.
  RegPressure, // Scheduling for lowest register pressure.
  Hybrid,      // Scheduling for both latency and register pressure.
  ILP,         // Scheduling for ILP in low register pressure mode.
  VLIW,        // Scheduling for VLIW targets.
  Fast,        // Fast suboptimal list scheduling
  Linearize    // Linearize DAG, no scheduling
};

ScheduleDAGRRList 类继承自ScheduleDAGSDNodes
burrListDAGScheduler 自底向上表示从 DAG 的使用者/根节点开始选择，然后反向构造最终指令顺序
sourceListDAGScheduler  按照 DAG 节点记录的原始顺序进行调度
hybridListDAGScheduler
如果某条长延迟指令位于关键路径，会尽量提前发射
如果提前发射会显著增加寄存器压力，则可能推迟
优先选择既能推进关键路径又不会制造太多活跃值的节点
ILPListDAGScheduler
暴露更多指令级并行
          ↕
避免寄存器压力过高
这些调度器一般只在当前 SelectionDAG 对应的基本块或调度区域内重排，不做跨整个函数的任意移动。

基于优先队列的块内调度
BURegReductionPrioritqueue
### ScheduleDAGRRList 类  Schedule()
创建依赖图,根据SelectionDAG对象构建的SUnit图,但不包括调度无关节点
SUnit 图中的每个 SUnit对象表示粘合在一起的 DAGNode 节点
SUnit图构建的三个过程
1.聚类
2.构建sunit节点 普通情况 一个SDNode → 一个SUnit
但通过 Glue 连起来的节点会被合并成一个 SUnit
BuildSchedUnits() 会把所有 Glue 链合并成一个 SUnit，不论 Glue 是否由 ClusterNodes() 创建。
3.添加调度依赖
AddSchedEdges()
数据依赖/Chain/内存依赖/物理寄存器依赖/延迟信息

### RegReductionPriorityQueue
每一种 ScheduleDAGRRList 类都有 对应的RegReductionPriorityQueue(SchedulingPriorityQueue),但是排序不同,不同调度器优先级不同
using BURegReductionPriorityQueue =
    RegReductionPriorityQueue<bu_ls_rr_sort>;

using SrcRegReductionPriorityQueue =
    RegReductionPriorityQueue<src_ls_rr_sort>;

using HybridBURRPriorityQueue =
    RegReductionPriorityQueue<hybrid_ls_rr_sort>;

using ILPBURRPriorityQueue =
    RegReductionPriorityQueue<ilp_ls_rr_sort>;
| 调度器 | Sethi–Ullman 的地位 |
|---|---|
| `burrList` | 核心优先级 |
| `sourceList` | 原始顺序相同时的后备规则 |
| `hybridList` | 压力和延迟无法区分时的后备规则 |
| `ILPList` | ILP、压力、关键路径无法区分时的后备规则 |
使用sethi-ullman算法最小化寄存器压力

四种不同的优先队列决定了当候选指令不止一条时,指令的排序,BURR 是在 SelectionDAG 的反向拓扑排序过程中，对当前拓扑上同时就绪、互不依赖的候选 SUnit，结合寄存器压力、延迟和硬件约束进行优先级选择。
AvailableQueue = {A, B}
        ↓
BURRSort(A, B)

Source
优先保持原始LLVM IR顺序
无法区分时使用BURRSort

Hybrid
寄存器压力
+ 指令延迟
+ 流水线Stall
+ BURRSort兜底
ILP
寄存器压力变化
+ Live Uses
+ 流水线Stall
+ 关键路径
+ 指令级并行
+ BURRSort兜底

# PRE-RA调度
SelectionDAG 指令选择与调度
        ↓
EmitSchedule：生成 MachineInstr
        ↓
FinalizeISel
        ↓
Machine SSA 优化 EarlyTailDuplicate/OptimizePHIs/StackColoring/LocalStackSlotAllocation/DeadMachineInstructionElim/addILPOpts/MachineCSE/MachineSinking
        ↓
PHI 消除(完成后machinIR退出SSA)/DetectDeadLanes/ProcessImplicitDefs/UnreachableMachineBlockElim/LiveVariables(块级活跃性、kill/dead 信息)/LiveIntervals(基于指令位置的精确活跃区间)
        ↓
TwoAddress 转换
        ↓
寄存器合并,RegisterCoalescer 主要消除 COPY
        ↓
独立子寄存器重命名
        ↓
Pre-RA MachineScheduler
考虑延迟、资源和寄存器压力重新排序
指令选择完成后,是已经调度过的machineinstr,再重新构成调度DAG,构建sunit,构建调度区域,从底向上分析

## LiveVariables
SlotIndexes 给指令编号 SlotIndexes下分4个slot SlotIndex
| Slot | 含义 |
|---|---|
| `B` | Block 边界位置 |
| `e` | Early-clobber 位置 |
| `r` | 普通寄存器读取/定义位置 |
| `d` | Dead definition 结束位置 |
核心数据结构
class VNInfo {
    unsigned id;     // 值编号,同一个寄存器值可能有不同编号,表示其定义的唯一性
    SlotIndex def;   // 这个值的定义位置
};

struct Segment {
    SlotIndex start;
    SlotIndex end;
    VNInfo *valno;   // 该区间保存的是哪个值,且segment有唯一性
};

LiveRange = 一组 Segment + VNInfo

## 调度边界(自底向上)
为什么要区分边界,边界决定调度的最小允许范围
scheduleRegions()
  -> getSchedRegions()              只划分 region
  -> Scheduler.enterRegion()        只记录 RegionBegin/RegionEnd
  -> ScheduleDAGMILive::schedule()
     -> buildDAGWithRegPressure()
        -> buildSchedGraph()
           -> clearDAG()
           -> initSUnits()
           -> 构建 SDep 依赖边
  -> pickNode()/scheduleMI()         使用已经建好的 SUnit 调度

PreRA:
  使用虚拟寄存器和 LiveIntervals
  主要关注延迟、资源和寄存器压力
  FixKillFlags = false

PostRA:
  已经分配物理寄存器
  调度后需要修复 kill 标记
  FixKillFlags = true

Scheduler.schedule()
    PreRA 实际调用：
    ScheduleDAGMILive::schedule()
    主要步骤：
    buildDAGWithRegPressure();
    postProcessDAG();
    findRootsAndBiasEdges();
    SchedImpl->initialize(this);
    initQueues();

    while (SUnit *SU = SchedImpl->pickNode(IsTopNode)) {
    scheduleMI(SU, IsTopNode);
    SchedImpl->schedNode(SU, IsTopNode);
    updateQueues(SU, IsTopNode);
    }
以region为单位构建sunitDAG,region 为单位构建 SUnit DAG，region 之间禁止跨界调度

pre-RA中sunit的依赖进行了重建
显式/隐式寄存器 operand -> Data/Anti/Output
MachineMemOperand + AA   -> Order
call/unmodeled side effect -> Barrier
terminator/特殊状态指令   -> scheduling boundary
Glue 语义如何延续
例如 x86 比较和条件跳转：
SelectionDAG：
CMP -> glue -> JCC
生成 MIR：
CMP32rr %0, %1, implicit-def $eflags
JCC_1 %bb.1, implicit $eflags
PreRA 不再看到 Glue，但能根据 $eflags 重建：

RegPressure 保存整个当前 region 的：
struct RegisterPressure {
  std::vector<unsigned> MaxSetPressure;
  SmallVector<RegisterMaskPair> LiveInRegs;
  SmallVector<RegisterMaskPair> LiveOutRegs;
};

指令边界级状态
RegPressureTracker 内部：
MachineBasicBlock::const_iterator CurrPos;
LiveRegSet LiveRegs;
std::vector<unsigned> CurrSetPressure;

单条 SUnit 的压力变化
每条指令对应一个 SUnit，它有自己的：
SUPressureDiffs[SU->NodeNum]

PressureSet
-> 一组虚拟寄存器共同消耗哪些有限物理寄存器资源
例如简化硬件：
物理寄存器：R0 R1 R2 R3
LOW_GPR  = {R0, R1}
ALL_GPR  = {R0, R1, R2, R3}

scheduleMI(SU, IsTopNode);          // 移动指令、更新压力
SchedImpl->schedNode(SU, IsTopNode);// 更新 cycle 和硬件资源
updateQueues(SU, IsTopNode);        // 更新 DAG，释放新节点

picknode是从候选选择调度节点的主要决定算法
只有一个候选
SU = Bot.pickOnlyChoice();
pickOnlyChoice() 会：
1. 检查 Pending 中是否有节点已经可以发射
2. 将当前产生 hazard 的 Available 节点移到 Pending
3. 如果 Available 为空，推进 cycle
4. 如果最终只有一个 Available 节点，直接返回
5. 如果有多个候选，返回 nullptr，进入启发式比较
多个候选
initCandidate()
为候选计算动态寄存器压力：
RPTracker.getUpwardPressureDelta(
    SU->getInstr(),
    DAG->getPressureDiff(SU),
    Cand.RPDelta,
    ...);
得到：
RPDelta.Excess
RPDelta.CriticalMax
RPDelta.CurrentMax
tryCandidate()
它不是计算总分，而是按层次逐项比较：
1. 缩短物理寄存器 live range
2. 避免超过寄存器数量限制 RegExcess
3. 避免增加关键寄存器压力 RegCritical
4. 避免 latency stall
5. 保持 clustered 节点相邻
6. 处理 weak edge
7. 避免增加 region 最大压力 RegMax
8. 平衡处理器资源
9. 避免延长关键路径
10. 原始 NodeOrder 兜底
前面的启发式一旦区分出优劣，后面的就不再参与。
双向调度
1. Bottom 只有一个候选 -> 直接选 Bottom
2. Top 只有一个候选 -> 直接选 Top
3. 分别选出最佳 BotCand 和 TopCand
4. 比较两个方向的最佳候选
5. 返回最终节点，并设置 IsTopNode

因此整个 while 的本质是：
pickNode
-> 从 ready 节点中做性能选择

scheduleMI
-> 把选择落实到 MIR，并更新寄存器压力

schedNode
-> 更新目标处理器 cycle/资源状态

updateQueues
-> 消耗依赖边，产生下一批 ready 节点
