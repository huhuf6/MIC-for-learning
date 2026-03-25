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
一句话总结：