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
