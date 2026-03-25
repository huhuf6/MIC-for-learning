# input torch
Pass 作用
registerTorchPasses
注册 Torch dialect 自身的规范化/分解/形状相关 pass（仍在 torch 语义层）。
registerTorchConversionPasses
注册 torch 到 torch_c（TorchConversion）等中间层的转换 pass。
registerConversionPasses
注册 torch-mlir 通用转换 pass（往 linalg/tensor/scf/arith/ml_program 等方向降）。
registerTorchOnnxToTorchPasses
支持 ONNX 输入先转成 Torch IR（onnx 路径前置步骤）。
registerTMTensorConversionPasses
IREE 自己的 Torch 输入转换 pipeline（torch-to-iree），含符号形状绑定、bitcast、TMTensor/LinalgExt 转换、函数 ABI 改写等。

Dialect 作用
torch：PyTorch 前端 IR 主语义。
torch_c：Torch backend contract/桥接层，便于继续向标准 MLIR 降。
tm_tensor：torch-mlir 的张量算子层（如 scan/sort/scatter/attention），后续会转 IREE LinalgExt。
ml_program：函数/全局等程序结构层，作为进入 IREE 通用输入层的重要中间表示。
iree_linalg_ext：IREE 扩展算子（比如更贴近后端的 attention/scatter 等）。
flow/hal/stream/tensor_ext/util：Torch 转 IREE 时会直接构造这些 IREE 方言的 op/type/attr，所以必须提前注册。

# input_stablehlo
Pass 作用
registerStableHLOConversionPasses
StableHLO/VHLO 到 IREE 可接受输入层的主转换 pass 集。
registerChloLegalizeToStablehloPass
先把 CHLO 规范化到 StableHLO，减少后续转换分支。
Dialect 作用
shape：shape 计算与推理支持。
chlo：client HLO 扩展算子，通常先降到 stablehlo。
stablehlo：OpenXLA 稳定算子集（核心输入之一）。
vhlo：versioned HLO，常先反序列化/转 stablehlo。
flow：StableHLO 转 IREE 过程中会进入 Flow 层。

# input_tosa
Pass 作用
registerTOSAConversionPasses
IREE/TOSA 输入主转换 pass 集。
registerTosaToArithPass
TOSA 中可直接表达为算术的部分先降到 arith。
registerTosaToLinalg
大部分 tensor 计算降到 linalg（后端更易优化/codegen）。
registerTosaToTensorPass
把部分 TOSA 语义落到 tensor 方言辅助表示。
Dialect 作用
tosa：TOSA 输入模型的算子方言本体；插件扫描到它就走 TOSA pipeline。



# 为什么不需要走stableHLO呢


因为 IREE 的设计是“多前端直达统一中后端”，不是“所有前端先汇到 StableHLO 再继续”。

input_torch 已经能把 Torch/ONNX 直接降到 IREE 能接受的统一输入层（linalg/tensor + IREE dialect），所以再绕一遍 StableHLO 没必要，主要原因：

避免额外中间转换成本
多一跳会增加转换复杂度、潜在语义丢失和调试难度。

Torch 语义有专门处理
input_torch 里有针对 Torch/ONNX 的特化 pass（形状绑定、bitcast、函数 ABI、TMTensor/LinalgExt 等），直接走更稳。

StableHLO 不是所有前端的天然“最优中间层”
它更像 OpenXLA/JAX 生态的核心 IR；Torch 路线本来就有 torch-mlir 体系。

IREE 后续中后端本来就统一
无论从 Torch 还是 StableHLO 进来，都会汇入 IREE 的 Flow/Stream/HAL/VM 主线，不需要先强制统一到 StableHLO。

# Dispatch 可调度单元
一段被封装的计算（通常是一簇 linalg/tensor 运算）
有明确输入输出
会在后续被做 tiling、vectorization、codegen，最终变成某个后端 executable 的 kernel/workgroup 任务

# 自研芯片落地 IREE 的最小实施路线图
阶段 1：先跑通（2-4 周）

新建 HAL target 插件骨架
路径可参考：compiler/plugins/target/VMVX、.../LLVMCPU
目标：让 --iree-hal-target-backends=<your_backend> 可识别。
定义设备与 executable target
实现 populateHALTargetDevices / populateHALTargetBackends。
打通最小编译产物
先用“占位可执行格式”或简单二进制封装，确保能从 iree-compile 产出并被 runtime 加载。
先选一个输入前端
建议先 StableHLO 或 ONNX->torch 路径，缩小变量。

阶段 2：可用性能（4-8 周）

建立你的 codegen pass pipeline
在 Codegen/Common + 你的后端目录加入：
tiling strategy
bufferization strategy
layout / vector lowering
把芯片约束编码进 IR
如 shared/local memory 限制、向量宽度、DMA 对齐、bank 冲突规则。
引入 profiling 回路
最少要有：
编译日志（pass 前后 IR dump）
runtime kernel 计时
基准模型回归脚本

阶段 3：规模化（持续）

做自动调参
按 op shape/target 配置自动选 tile size、pipeline depth、vector shape。
做模型覆盖矩阵
按模型族（LLM/CV/ASR）维护“可编译率 + 性能 + 精度”看板。
做稳定性工程
编译缓存
失败回退路径
CI 回归（功能 + 性能）

#   auto tuning
核心原理：把“编译/调度参数选择”当成搜索问题，用实测性能做反馈。

最常见流程：

定义搜索空间
例如 tile size、vector width、unroll、pipeline depth、memory layout、并行度。

生成候选实现
每组参数生成一个可运行版本（kernel 或子图代码）。

跑基准并测性能
在目标硬件上测 latency/throughput/功耗等指标。

选择或学习下一批参数

简单：网格搜索/随机搜索
进阶：贝叶斯优化、遗传算法、cost model、bandit
保存最优配置
按算子形状、dtype、硬件型号做 key，缓存最优参数，下次直接命中。

一句话：
Auto tuning = “自动试参数 + 实测反馈 + 缓存最优”。

# triton
把 Triton 当“自动调参器/内核生成器”
Triton 负责搜索参数和产出候选内核配置。
你自己的编译器/IREE 后端负责真正 codegen 到芯片 ISA。
这是最现实、最快落地的路线。
做 Triton 原生后端（难度高）
要实现 Triton 到你芯片后端的完整 lowering/codegen/runtime 支持。
工程量接近做一套新编译后端，不是简单“适配一下”。
给你可执行的建议（推荐路线 1）：

定义统一 key（shape/dtype/layout/chip）。
用 Triton 生成或搜索 tile/warp/stage 等参数。
把参数写回你 IREE codegen 配置（lowering config）。
实测性能后把结果存离线 DB。
编译时先查 DB，未命中再触发补调。
如果你坚持做“原生 Triton 后端”，最少要补：

目标描述（线程层级/内存层级/向量语义）
Triton lowering 到你的后端 IR
后端代码生成与汇编/二进制封装
运行时 launcher 与参数打包协议
correctness + 性能回归体系

# 目标代码到芯片执行需要哪些过程

大致要经过这 10 步：

前端导入
PyTorch/ONNX/StableHLO -> IREE IR

图级优化与切分
做融合/布局/shape 优化，并切成 dispatch。

后端代码生成（你芯片 target backend）
dispatch 降到你芯片可执行代码（ISA/微码/核函数）。

可执行封装
把代码、元数据（workgroup、常量、binding）打包成 HAL executable binary。

产物打包成 VMFB
IREE 把模块 + 可执行 + 调用接口打成 .vmfb（参数可内置或外置）。

运行时加载
IREE runtime 加载 VMFB，创建 VM/HAL 上下文，初始化你的设备 driver。

参数与输入准备
把权重/输入 tensor 放到设备可访问内存，建立 buffer/buffer_view。

命令构建与提交
runtime 按调用生成 command buffer（dispatch、copy、barrier），提交到你的设备队列。

芯片执行
你的 driver/固件把命令翻译成硬件调度动作，启动核函数执行。

同步与取回输出
等待 fence/signal，必要时把结果从设备内存拷回 host，返回给上层。

核心上你要补齐的“自研芯片关键环节”是：

编译侧：HAL target backend + codegen + executable format
运行侧：HAL driver + 内存管理 + 命令提交 + 同步机制

# codegen 
主要做的是把“高层算子 IR”变成“目标硬件可执行代码”。核心工作：

确定执行策略
选 tiling、并行映射（block/thread/core）、vectorization、pipeline 深度。

内存层级映射
决定数据放寄存器/片上 SRAM/全局内存，插入 copy/cache/prefetch，做 bufferization。

算子降级
把 linalg/tensor 等高层 op 逐步降到更低层 IR（scf/arith/vector/llvm 或目标方言）。

目标特化优化
用硬件特性做重写（tensor core、SIMD、DMA、bank-conflict 避免、指令选择）。

生成可执行实体
产出后端二进制或目标代码片段，并附带 launch 元数据（workgroup、bindings、常量布局）。

最佳位置：放在 codegen 的“配置阶段（configuration）”，也就是决定 tiling/vectorization/bufferization 参数之前或当下。

推荐分层：

主放置点（必须）
Codegen configuration pass
作用：根据 op+shape+dtype+chip 选 lowering config
这里最适合接“离线DB命中 -> 未命中再tune -> 写回配置”
次放置点（可选）
dispatch 创建后、codegen前
适合做更粗粒度策略选择（比如选哪条 pipeline）
不要放的位置
太前（InputConversion/GlobalOptimization）：信息不够硬件化
太后（已经生成目标代码后）：改参数成本高，反馈闭环慢
一句话：
把 autotuning 放在“后端相关但还没最终降级”的那层，收益最高、成本最低。

进入 codegen 前
外层多是 flow / hal 结构（已经有 dispatch/executable 边界）
executable variant 里常是 linalg/tensor/scf/arith 这类
codegen 中段
做 tiling/vectorization/bufferization
IR 会出现更多 vector, memref, scf 等低层结构
codegen 末段（按后端）
CPU 路：趋向 llvm dialect
GPU/SPIR-V 路：趋向 gpu/spirv 或 LLVM GPU 相关 IR
VMVX 路：趋向 VMVX 对应低层表示

# codegen常见优化策略（按优先级）

Tiling 策略
目标：提高缓存命中、并行粒度匹配硬件
调参：tile_m/n/k、多级 tiling（block/warp/thread）
Workgroup/线程映射
目标：充分占用计算单元并减少调度空洞
调参：workgroup shape、线程分工、wave/warp 利用率
Vectorization
目标：吃满 SIMD/向量单元
调参：vector width、transfer shape、unroll 因子
Bufferization 与内存层级
目标：减少中间分配和带宽压力
手段：promote 到 shared/local、in-place/alias、copy 消除
Pipeline/Prefetch
目标：隐藏访存延迟
调参：software pipeline depth、prefetch 距离、双缓冲
Layout 优化
目标：避免 bank conflict、对齐硬件访存
手段：转置传播、packed layout、stride 调整
融合/去融合平衡
目标：减少 launch 和中间写回，同时避免寄存器爆炸
做法：图级先融合，codegen 层按资源再细调

# passes
注册阶段：把 pass “挂到系统里可用”
执行阶段：编译时按 pipeline 顺序真正运行
下面按“执行阶段”来讲它们各自作用：

input_plugin pass（Torch/StableHLO/TOSA 插件）
阶段：Input phase 最前面
作用：把前端方言（onnx/torch/stablehlo/tosa）降到 IREE 主线可接受 IR（linalg/tensor + IREE 输入层）
典型位置：compiler/plugins/input/*
GlobalOptimization pass（你说的 globalzation）
阶段：GlobalOptimization phase（在 Preprocessing 后、DispatchCreation 前）
作用：模块级高层优化，如常量求值、代数化简、transpose 传播、精度相关优化等
目标：在切 dispatch 之前把图尽量优化干净
MLIR passes（你说的 milr pass）
阶段：不固定，按需要穿插在各 phase
作用：这是上游 MLIR 通用工具箱（canonicalize/CSE/inliner/bufferization/scf/linalg 等）
说明：它不是一条独立 pipeline，而是被 IREE 各阶段调用的基础 pass 库
CodegenPasses
阶段：主要在 HAL/Executable* 相关后端 codegen 阶段
作用：对每个 dispatch 做后端特化：tiling、vectorization、bufferization、lowering 到 LLVM/SPIR-V/ROCDL/VMVX/WGSL 等，并生成目标二进制片段
典型位置：compiler/src/iree/compiler/Codegen/*
AllIreePasses
阶段：它本身不是执行阶段，是 IREE 自己 pass 的“总注册入口”
作用：把 ABI/InputConversion/Preprocessing/GlobalOptimization/DispatchCreation/Flow/HAL/Stream/VM 等 IREE pass 全部注册
真正执行时，还是由主 pipeline 按 phase 调度

图级/高层融合
常在 GlobalOptimization（以及部分 Preprocessing）做，目标是减少中间张量、暴露更大计算块。

面向 dispatch 的融合
在 DispatchCreation 阶段也会做“为了形成更优 dispatch”的融合与聚合。

后端低层融合
到 Codegen 还可能继续做局部融合（更偏 tile/loop/vector 层）。