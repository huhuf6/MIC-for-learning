# 在ai编译器后端中,跟llvm后端相同或不相同的地方在哪

相同点（本质一样）

都要做 lowering
把高层 IR 逐步降到目标可执行表示。
都要做 target-specific codegen
不同硬件走不同指令/二进制格式。
都要处理寄存器、内存、调度、指令选择
只是 AI 编译器经常把这些放在更高层（tensor/dispatch）先做一轮。
都要遵守 ABI / runtime 接口
参数传递、调用约定、对象格式、链接/加载都不能错。
都需要性能闭环
profile -> 调参 -> 回归测试。
不相同点（AI 后端特有重点）

优化粒度更“算子/子图/dispatch”
LLVM 经典后端偏基本块/指令级；AI 后端先在 tensor 层做 fusion/tiling/vectorization。
目标从“单条指令快”变成“整核函数吞吐高”
更关心带宽、并行映射、kernel launch、图切分成本。
强依赖硬件并行模型
GPU/NPU 的 warp/wave/workgroup、shared memory、tensor core 约束是核心。
shape/dtype/layout 是一等公民
动态 shape、量化、混合精度、layout 变换直接决定 codegen 路径。
运行时耦合更深
内存池、异步流、通信库（NCCL等）、多设备执行计划常和后端一起设计。
autotuning 更常见
tile size、vector width、pipeline depth 往往靠实测搜索，不只靠静态启发式。
多后端 IR 路径并存
LLVM/NVVM/ROCDL/SPIR-V/WGSL/专有 ISA，通常不是单一路径到底。

# back-end pipeline
作者：Frank Wang
链接：https://zhuanlan.zhihu.com/p/52724656
来源：知乎
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

1 首先，SelectionDAGBuilder遍历LLVM IR中的每一个function以及function中的每一个basic block，将其中的指令转成SDNode，整个function或basic block转成SelectionDAG。这时DAG中每个node的内容仍是LLVM IR 指令。

2 SelectionDAG经过legalization和其它optimizations，DAG节点被映射到目标指令。这个映射过程是指令选择。这时的DAG中的LLVM IR节点转换成了目标架构节点，也就是将LLVM IR 指令转换成了机器 指令. 所以这时候的DAG又称为machineDAG。

3 在machineDAG已经是机器 指令，可以用来执行basic block中的运算。所以可以在machineDAG上做instruction scheduling确定basic block中指令的执行顺序。指令调度分为寄存器分配前的指令调度，和寄存器分配后的指令调度。寄存器分配前的指令调度器实际有2个，作用于SelectionDAG，发射线性序列指令。主要考虑指令级的平行性。经过这个scheduling后的指令转换成了MachineInstr三地址表示。指令调度器有三种类型：list scheduling algo, fast algo, vliew.

4 寄存器分配为virtual Register分配physical Register并优化 Register分配过成使溢出最小化。virtual Register到physical Register的映射有2中方式：直接映射和间接映射。直接映射利用TargetRegisterInfo和MachineOperand类获取load/store指令插入位置，以及从内容去除和存入的值。间接映射利用VirtRegMap类处理load/store指令。寄存器分配算法有4种：Basic Register Allocator、Fast Register Allocator、PBQP Register Allocato、Greedy Register Allocator。

5 寄存器分配后的指令调度器作用于机器指令，也就是MachineInstr。这时能得到physical寄存器信息，可以结合physical Register的安全性和执行效率，对指令顺序做调整。

6 Code emission阶段将机器 指令转成MCInstr，并发射汇编或二进制代码。

# selctionDAG
SelectionDAG类用一个DAG表示一个basic block。SelectionDAG的创建是个基本的窥孔算法。LLVM IR经过SelectionDAGBuilder的处理后转换成SelectionDAG。下图是c代码实现除法，只有一个function，一个basic block。

DAG中的每个节点SDNode会维护一个记录，其中记录了本节点对其它节点的各种依赖关系，这些依赖关系可能是数据依赖（本节点使用了被其它节点定义的值），也可能是控制流依赖（本节点的指令必须在其它节点的指令执行后才能执行，或称为chain）。这种依赖关系通过SDValue对象表示，对象中封装了指向关联节点的指针和被影响结果的序列号。也可以说，DAG中的操作顺序通过DAG边的use-def关系确定。如果图中的sdiv节点有一个输出的边连到add节点，这意味着add节点定义define了一个值，这个值会被sdiv节点使用。因此，add操作必须在sdiv节点之前执行。

每个节点的类型，可以是实际的数据类型，如i32,i64等，也可以是chain类型，表示chain values，或者是glue类型，表示glue。 SelectionDAG对象有一个特殊的EntryToken来标记basic block的入口。EntryToken的类型是ch，允许被链接的节点以这个第一个token作为起始。

CopyFromReg：copy当前basic block外的register，用在当前环境，这里用于copy函数参数。CopyToReg：copy一个值给特定寄存器而不提供任何实际值给其它节点消费。然而，这个节点产生一个chain value被其它节点链接，这些其它节点不产生实际值。比如为了使用写到EAX的值，ret_flag节点使用EAX寄存器提供的i32结果，并消费CopyToReg节点产生的chain，这样保证EAX随着CopyToReg更新，因为chain会使得CopyToReg在ret_flag之前被调度。


# legalization
SDNode的合法化涉及类型和操作的合法化。
## 操作合法化
x86上没有 条件赋值（conditional moves） 指令，PowerPC也不支持从一个16-bit的内存上以符号扩展的方式读取整数。因此， 合法化 阶段要将这些不支持的指令按三种方式转换成平台支持的操作： 扩展（Expansion） ，用一组操作来模拟一条操作； 提升（promotion） 将数据转换成更大的类型来支持操作； 定制（Custom） ，通过目标平台相关的Hook实现合法化。  

下图以x86除法指令为例说明操作的合法化。LLVM IR的sdiv只计算商，而x86除法指令计算得到商和余数，分别保存在两个寄存器中。因为指令选择会区分SDIVREM和sdiv，因此当目标平台不支持SDIV时需要在合法化阶段将sdiv扩展到sdivrem指令。

目标平台相关的信息通过 TargetLowering 接口传递给SelectionDAG。目标设备会实现这个接口以描述如何将LLVM IR指令用合法的SelectionDAG操作实现。在x86的TargetLowering构造函数中会通过“expanded”这个label来标识。当SelectionDAGLegalize::LegalizeOp看到SDIV节点的Expand标志会用SDIVREM替换。类似的，目标平台相关的合并方法会识别一组节点组合的模式，并决定是否合并某些节点组合以提高指令选择质量。

## 类型合法化
如果平台的td文件的寄存器类定义没有相应的数据类型，那对平台来说就是非法数据类型。非法的类型必须被删除或做相应处理。根据非法数据类型不同，处理方式分为两种情况。第一种是标量。标量可以被promoted（将较小的类型转成较大的类型，比如平台只支持i32，那么i1/i8/i16都要提升到i32）, expanded（将较大的类型拆分成多个小的类型，如果目标只支持i32，加法的i64操作数就是非法的。这种情况下，类型合法化通过integer expansion将一个i64操作数分解成2个i32操作数，并产生相应的节点）；

第二种是矢量。LLVM IR中有目标平台无法支持的矢量，LLVM也会有两种转换方案， 加宽（widening） ，即将大vector拆分成多个可以被平台支持的小vector，不足一个vector的部分补齐成一个vector；以及 标量化（scalarizing） ，即在不支持SIMD指令的平台上，将矢量拆成多个标量进行运算。  

目标平台也可以实现自己的类型合法化方法。类型合法化方法会运行两次，一次是在第一次DAG combine之后，另一次是在矢量合法化之后。无论怎样，最终都要保证转换后的指令与原始的IR在行为上完全一致。

如果类型和操作都 illegal，会分阶段处理，通常先把类型搞合法，再处理操作合法性。
# 如果类型和操作都illgal呢
先做类型合法化（Type Legalization）
把结果/操作数类型变成目标支持的类型（promote / expand / split 等）。
再做操作合法化（Operation Legalization）
在“已合法类型”上检查该 opcode 是否支持；不支持再 expand/custom/libcall。
可能反复迭代
一次改写会引入新节点，节点仍可能 illegal，继续直到都 legal。
一个直观例子：
原始节点：SDIV i128（目标只支持 i32 add/sub，且无硬件除法）

第一步（类型合法化）：i128 -> 4 x i32（或其它拆分形式）
第二步（操作合法化）：SDIV 仍不支持 -> 变成 runtime libcall（如 __divti3）或软件算法序列
最终得到全是目标可接受的节点/调用

## 不需要枚举全部的类型/操作不符
通常只配置“与默认行为不同的关键点”。

可以这么理解：

LLVM 有默认合法化策略
许多 op+type 组合会走通用默认路径（比如 Expand、libcall 等）。
你只需要覆盖那些默认效果不理想或你有硬件特性的组合。
重点配置高频路径
先保证后端能编过（最小可用）。
再针对热点类型/操作（如 i32 add/load/store/branch、常用浮点）精调 setOperationAction。
常见做法是“少量显式 + 其余默认”
显式声明 Legal 的主力组合
对硬件不支持但你想自定义的，设 Custom 并在 LowerOperation 处理
其他不关心的交给 LLVM 通用合法化
实战节奏
第一步：最小 ISA 跑通（少量 action）
第二步：根据报错/性能热点增量补 setOperationAction
第三步：配合 DAGCombine、调用约定、寄存器类继续完善
一句话：不是“穷举表”，而是“以默认为底、按目标特性和热点增量覆盖”


## Combine pass
DAG Combine pass是将一组节点用更简单结构的节点代替。比如一组节点表示(add (Register X), (constant 0)) 将寄存器X中的值和常数0相加，这种情况可以简化成(Register X)，和常数0相加无效，被优化掉了。

Combine pass在legalization后执行，可以最小化SelectionDAG的冗余节点

# Instruction Lowering
SelectionDAGBuilder遍历LLVM IR中的每一个basic block，生成SelectionDAG。在这个过程中，某些指令，如call和ret已经需要目标相关信息。比如如何传递调用参数，如何从函数返回。  

为了解决这个问题，TargetLowering类中的算法会在这里被首次使用。不同目标的backend要实现TargetLowering抽象接口。只有一小部分节点会在这里转成目标相关节点，大部分节点都在instruction selection中在pattern match后被替换成机器指令


# Instruction selection
指令选择通过节点模式匹配完成DAG到DAG的转换，将SelectionDAG节点转换成表示目标指令的节点。这一阶段的输入是经过合法化的SelectionDAG。

## tablegen
tabgen有两方面的作用,一是为前端定义前向声明,让前端认识 ir中的Intrinsics，
比如 llvm.x86.*、llvm.aarch64.* 这类内建函数名、参数类型、返回类型、属性
二是映射到目标端具体的指令用于指令替换
llvm/IR/*.td：定义 IR 层 intrinsic/属性等（接口与语义描述）
llvm/lib/Target/*/*.td：定义 后端机器指令/寄存器/pattern（真正选指令与编码）

在Select方法最后会调用Selectcode方法，如下图所示。这个方法是tablegen为目标平台生成的。主要的作用就是将ISD和平台ISD映射到机器指令节点。这种映射通过Matcher table实现。

# Instruction scheduling

指令选择完成后的MachineDAG内容虽然是机器指令，但仍然是以DAG形式存在，CPU/GPU不能执行DAG，只能执行指令的线性序列。寄存器分配前的指令调度的目的就是通过给DAG节点指定执行顺序将DAG线性化。最简单的办法就是将DAG按拓扑结构排序，但LLVM backend用更智能的方法调度指令使其运行效率更高

MachineInstr(MI)

# Code Emission

<target>AsmPrinter 在 MachineFunction 层工作
先输出函数头（label、对齐、section 等）。
遍历每个 MachineBasicBlock、每条 MachineInstr。
调后端  重载的 EmitInstruction(const MachineInstr*)。

在 EmitInstruction 里先做 MI -> MCInst 降级
MachineInstr 是“代码生成阶段”的复杂表示（含寄存器分配/伪指令等信息）。
MCInst 是更接近最终汇编/机器码的轻量表示。
这一步由目标后端的 MCInstLowering（或同等 lower 函数）完成。

然后交给 MCStreamer 决定输出形态
AsmPrinter::EmitToStreamer() 把 MCInst 送进 streamer。
具体 streamer 有两条路：
MCAsmStreamer：产出文本汇编（.s）
MCObjectStreamer：产出目标文件机器码（.o）

文本汇编路径
MCAsmStreamer::emitInstruction 被调用。
目标后端的 MCInstPrinter::printInst 把 MCInst 格式化成目标汇编语法并写文件。

二进制目标文件路径
MCObjectStreamer::emitInstruction 被调用。
目标后端的 MCCodeEmitter::encodeInstruction 把 MCInst 编码成字节序列。
再由 MC 层写入 .o 的 section/relocation 等结构。