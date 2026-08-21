# 什么是算子?
通俗来说是在加速器或GPU上执行计算的代码,在前端只有计算语义,叫算子,通常说的写算子,其实是写kernel,即计算如何在硬件设备上高速执行

# 什么是计算图调度
调度是计算的实现方式,TVM 调度原语(计算的优化方式),获得同一个计算结果,使用不同的计算方式
Loop Transformation
├── split
├── fuse
├── reorder
└── tile

Parallelization
├── parallel
├── vectorize
├── unroll
└── bind

Memory
├── cache_read
├── cache_write
├── compute_at
├── reverse_compute_at
└── storage_align

Tensorization
└── tensorize

Reduction
├── decompose_reduction
└── rfactor

Layout
└── transform_layout
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
