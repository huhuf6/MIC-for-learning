ScheduleAllocation.cpp 做的不是“看到一个结果就立刻插一个 alloca”，而是更像：

先做 storage planning，再把 stream.async.* 里的逻辑 resource 值，改写成显式 backing storage + offset/length 的 stream.cmd.*。

你可以把它拆成 5 步看。

1. 入口：对每个 stream.async.execute 做一次 region allocation

在 2187-2217 (line 2187)：

先跑 AffinityAnalysis
然后遍历 callable 里的 op
碰到 stream.async.execute
-> 调 allocateExecutionRegion(op, affinityAnalysis)
所以真正的 alloc 主函数是
allocateExecutionRegion (line 1641)

2. 先不分配，先建“逻辑值 -> 物理区间”的映射

这一步靠两块：

alias 分析：40-133 (line 40)
liveness 区间：154-317 (line 154)
核心理解：

tied result 和它的 operand 视为 alias
111-123 (line 111)
yield 到外面的 tied 结果，也会并进 alias 集
87-99 (line 87)
liveness 会给每个 resource value 一个 [start, end]
alias 集里的值会把区间并起来
267-303 (line 267)
然后用 AllocationScope 记：

某个逻辑 resource value
对应到底层哪个 base resource
总 size 是多少
自己的 offset/length 是多少
见 361-500 (line 361)

也就是它内部维护的是：

logical_value -> ResourceRange(base, base_size, offset, length)
3. 常量先抽出去，不在 execute 里“现做现用”

allocateExecutionRegion 一上来先调 extractConstants(...)
见 1673-1751 (line 1673)

它会把 region 里的：

%0 = stream.async.constant ...
抽成 execute 外面的：

%res, %tp = stream.resource.constants ...
对应实现：

常量 batch 分配：1293-1340 (line 1293)
按 affinity / lifetime 分桶：1347-1458 (line 1347)
真实 IR：

schedule_allocation.mlir 里 @extractConstants：

前：

%results:4, %tp = stream.async.execute ... {
  %0 = stream.async.constant ...
  %1 = stream.async.constant ...
  %2 = stream.async.constant ...
  %3 = stream.async.fill ...
  stream.yield %0, %1, %2, %3
}
后：

%cst:2, %cst_tp = stream.resource.constants ...
%var, %var_tp = stream.resource.constants ...
%exec_tp = stream.cmd.execute await(...) => with(...)
这一步的意思是：

常量不再作为 execute 内部临时值，而是先变成外部已上传资源，再 capture 回 execute。

4. 对 escaping results 分配 backing storage

这是最关键的一步，在 1773-1922 (line 1773)

逻辑是：

如果 result 已经 tied 到 operand
-> 不新分配，直接复用 operand storage
1786-1831 (line 1786)

如果 result 其实只是 passthrough operand
-> 也不分配
1834-1853 (line 1834)

否则
-> 这是一个真的“新结果”
-> 先用 deriveResultAffinity(...) 算放哪儿
1219-1263 (line 1219)
-> 再按 (affinity, lifetime) 分桶
1499-1529 (line 1499)
-> 最后调用 ResourceAllocaOp::createSuballocations(...) 一次分大 slab，再切 subview
1873-1920 (line 1873)

createSuballocations 本体在
StreamOps.cpp (line 1809)

它做的是：

stream.resource.pack
stream.resource.alloca 一个总 slab
对每个结果建 stream.resource.subview
真实 IR：

schedule_allocation.mlir 里 @producedResults：

前：

%r:2, %tp = stream.async.execute with() -> (!stream.resource<transient>{%size0}, !stream.resource<transient>{%size1}) {
  %0 = stream.async.splat ...
  %1 = stream.async.splat ...
  stream.yield %0, %1
}
后：

%pack:3 = stream.resource.pack slices({ ... })
%alloca, %alloca_tp = stream.resource.alloca uninitialized : !stream.resource<transient>{%pack#0}
%sub0 = stream.resource.subview %alloca[%pack#1] ...
%sub1 = stream.resource.subview %alloca[%pack#2] ...
%exec_tp = stream.cmd.execute await(%alloca_tp) => with(%sub0 as %cap0, %sub1 as %cap1) {
  stream.cmd.fill ..., %cap0
  stream.cmd.fill ..., %cap1
}
这就是它对“逃出 execute 的新结果”的 alloc 方式。

5. 对不逃出的 local transient 做 slab packing

这块在
allocateLocalTransients (line 1053)

关键逻辑：

只看 !stream.resource<transient>
且不是 live-in / live-out
1074-1077 (line 1074)
用 liveness interval + alias 信息算每个局部值生命周期
1064-1098 (line 1064)
stream.resource.pack 时把 lifetime interval 传进去
1104-1112 (line 1104)
再插一个外部 stream.resource.alloca
execute body 里 capture 这块 slab
每个局部值只是在 slab 中映射一个 offset/length
真实 IR：

schedule_allocation.mlir 里 @locals：

前：

%tp = stream.async.execute ... {
  %0 = stream.async.splat ... -> !stream.resource<transient>{%size0}
  %1 = stream.async.splat ... -> !stream.resource<transient>{%size1}
  stream.yield
}
后：

%slices:3 = stream.resource.pack on(...) slices({
  [0, 0] = %size0,
  [1, 1] = %size1
})
%alloca, %alloca_tp = stream.resource.alloca uninitialized on(...) await(%await_tp)
%join = stream.timepoint.join max(%await_tp, %alloca_tp)
%exec_tp = stream.cmd.execute on(...) await(%join) => with(%alloca as %cap) {
  stream.cmd.fill ..., %cap[%slices#1 for %size0]
  stream.cmd.fill ..., %cap[%slices#2 for %size1]
}
%dealloca_tp = stream.resource.dealloca on(...) await(%exec_tp) => %alloca
%ret_tp = stream.timepoint.join max(%dealloca_tp, %exec_tp)
也就是：

局部 transient 不各分各的，而是整个 execute 内打包成一个临时 slab。

6. 最后把 async op 全改成基于已分配 storage 的 cmd op

这一步在
applyAsyncAllocations (line 961)

它会反向 walk region，把：

async.splat -> cmd.fill
529-537 (line 529)
async.clone/slice/update/copy -> cmd.copy
540-610 (line 540)
async.fill -> cmd.fill
568-578 (line 568)
async.dispatch -> cmd.dispatch
723-790 (line 723)
async.transfer -> cmd.invalidate/copy/flush
675-721 (line 675)
这里它不再用“逻辑 resource 值”，而是查 scope.lookupResourceRange(value)，把每个值展开成：

base resource
base size
offset
length
所以 allocation 的真正落地，不只是插入 alloca，而是：

把所有 region 内部 op 都改成“对已分配 backing storage 的区间读写”。

一句话总结

ScheduleAllocation.cpp 的 alloc 逻辑是：

先做 alias + liveness planning
把常量先抽出成 resource.constants
对 escaping results 按 affinity/lifetime 分桶，pack + alloca + subview
对 local transients 按生命周期打包成一个 slab
重建 stream.cmd.execute，把 async body 改写成对这些显式 storage range 的 stream.cmd.* 操作
最后补 dealloca 和 timepoint join
如果你愿意，下一条我可以直接只盯一个 case 讲，比如：

@producedResults：专看“结果分配”
@locals：专看“局部 transient slab”
@tiedOperandSubviews：专看“已有 storage 如何复用，不重新 alloc”


23:21
iree在hal 阶段做了什么,是怎么做codegen的,以llvm ir为例
23:42



已处理 9m 11s
可以，我们把它分成两层来看：

HAL 阶段本身做什么
LLVMCPU 目标下，hal.executable.variant 里的代码怎么一路变成 LLVM IR / object / hal.executable.binary
其实最关键的一句是：

HAL 阶段一半在做“设备/接口/宿主侧调用约定”的事情，另一半在调目标后端把 executable 真正翻译掉。

一、HAL 阶段总共做什么

主线在 Passes.cpp (line 202) 的几个 pipeline builder 里：

buildHALDeviceAssignmentPassPipeline
buildHALConfigurationPassPipeline
buildHALTransformPassPipeline
按顺序看，大概是：

先决定有哪些 device/target
把 stream.executable 变成 hal.executable + hal.executable.variant
给每个 variant 选后端 lowering/codegen 策略
把 variant 内部代码翻译到目标 IR
对 LLVMCPU 来说就是先到 MLIR LLVM dialect
把宿主侧 stream.* 改成 hal.*
把 variant 序列化成真正二进制
对 LLVMCPU 来说就是 .o/.so/.elf，最后塞进 hal.executable.binary
二、HAL 前半段：设备和 executable 接口是怎么立起来的

1. device assignment / materialize target devices
源码在：

Passes.cpp (line 202)
MaterializeTargetDevices.cpp (line 1)
这里做的事情很“程序外壳化”：

把模块级 hal.device.targets 变成真正的 device global
生成默认 device
给没有明确 affinity 的地方补上 stream.affinity.default
也就是说，HAL 从这一步开始让程序知道：

“我要在哪些设备上跑，默认走哪个设备。”

2. MaterializeInterfacesPass：把 stream.executable 展开成 hal.executable.variant
这是 HAL 里最关键的“接口成型”步骤，源码在：

MaterializeInterfaces.cpp (line 645)
makePipelineLayoutAttr 在 262 (line 262)
cloneFuncWithInterface 在 297 (line 297)
declareEntryPointOps 在 351 (line 351)
它做三件核心事：

a) 一个 stream.executable 按 target 展开成多个 hal.executable.variant
前：

stream.executable private @ex {
  stream.executable.export public @entry ...
  builtin.module {
    func.func @entry(%arg0: !stream.binding, ...)
  }
}
后：

hal.executable private @ex {
  hal.executable.variant public @arm_64 target(...)
  hal.executable.variant public @x86_64 target(...)
}
真实对照看 materialize_interfaces.mlir (line 1)。

b) 给每个 export 生成 HAL ABI 描述
也就是生成：

hal.executable.export
ordinal
layout(#hal.pipeline.layout<...>)
count(...) workgroup count region
比如测试里会变成：

hal.executable.export public @entry ordinal(0) layout(#pipeline_layout) ...
这一步的语义是：

“dispatch 的绑定布局、常量个数、workgroup count 计算，都变成 HAL 明确认识的 ABI/interface 信息。”

c) 把 entry 函数从 !stream.binding 参数改写成 hal.interface.* 访问
cloneFuncWithInterface 干的就是这个。

原来 executable 内函数可能是：

func.func @entry(%arg0: i32,
                 %arg1: !stream.binding,
                 %arg2: !stream.binding,
                 %arg3: !stream.binding) {
  ...
}
改完以后函数签名会被清空，函数体里改成：

hal.interface.constant.load
hal.interface.binding.subspan
hal.interface.workgroup.id/count/size
也就是：

不再把 binding/constant 当普通 SSA 参数传进来，而是改成“通过 HAL ABI 约定去取”。

这件事非常关键，因为后面的 LLVM lowering 就是围着这些 hal.interface.* 来展开 ABI struct 访问的。

三、HAL 中段：给每个 variant 选 codegen 策略

这一层是 ConfigureExecutables.cpp (line 1) 和 target backend 的 buildConfigurationPassPipeline(...) 配合完成的。

在总 pipeline 里对应：

createConfigureExecutablesPass(...)
在 Passes.cpp (line 377) 左右。

对 LLVMCPU backend，真正落到：

LLVMCPUTarget.cpp (line 1)
buildConfigurationPassPipeline(...)
进一步进 Codegen/LLVMCPU/Passes.cpp (line 638)
这里做的不是最后 codegen，而是配置：

createSpecializeExportsPass
createMaterializeTuningSpecsPass
createMaterializeUserConfigsPass
createMaterializeDeviceEncodingPass
createCPUPropagateDataLayoutPass
createLLVMCPUSelectLoweringStrategyPass
语义上是：

先决定这个 dispatch 用哪条 lowering pipeline、tile/vector size 是什么、编码/layout 怎么定。

所以 HAL 的 configure 阶段，本质上是在给后面的 codegen 做“目标特定配置”。

四、HAL 真正开始做 codegen：TranslateAllExecutablesPass

源码：

TranslateExecutables.cpp (line 1)
总 pipeline 在 Passes.cpp (line 430)
这里的逻辑非常直接：

遍历 hal.executable
对每个 target 的 hal.executable.variant
调该 backend 的 buildTranslationPassPipeline(targetAttr, pm)
对 LLVMCPU，落到：

LLVMCPUTarget.cpp (line 1)
buildTranslationPassPipeline(...)
然后调用 Codegen/LLVMCPU/Passes.cpp (line 638) 的 buildLLVMCPUCodegenPassPipeline(...)
五、以 LLVMCPU 为例：hal.executable.variant 内部怎么一路变成 LLVM dialect

这里你可以把它分成两段：

A. 先把 linalg/tensor/dispatch 内核降成接近 LLVM 的结构
入口在：

LLVMCPULowerExecutableTarget.cpp (line 1)
LLVMCPULowerExecutableTargetPass::runOnOperation() 做的核心是：

看 TranslationInfoAttr
看 target triple / cpu_features
组一条具体 CPU lowering pipeline
再 runPipeline(passManager, funcOp)
也就是说：

每个 entry func 会根据目标 CPU 和选定策略，跑一套 tile/vectorize/bufferize/lower 的 pipeline。

典型 pass 在 Passes.cpp (line 1) 里能看到：

createLLVMCPUTileAndFuseProducerConsumerPass
createLLVMCPUSplitReductionPass
createGenericVectorizationPass
addCPUBufferizePasses
buildLLVMCPUVectorLoweringPipeline
这一段主要干的是：

linalg/tensor
-> tile/fuse
-> vectorize
-> bufferize
-> vector lowering
B. 再从这些高层/中层 dialect 转成 MLIR LLVM dialect
这一步在 Passes.cpp (line 479) 的 addLowerToLLVMPasses(...)，最后调用：

createConvertToLLVMPass(...)
这个 pass 是 IREE 自己的 ConvertToLLVM.cpp (line 1)

这里会做：

LinalgExt -> loops
Linalg -> loops
SCF -> CF
arith/math/memref/vector/... -> LLVM
最后 ConvertToLLVMPass
六、这里最关键的 IR 变化：HAL interface 怎么变成 LLVM ABI 访问

这一步最值得直接看真实 IR。

1. entry 函数签名怎么变
看 convert_to_llvm.mlir (line 1)

前：

func.func @entry_point() {
  return
}
后：

llvm.func @entry_point(
  %arg0: !llvm.ptr,
  %arg1: !llvm.ptr,
  %arg2: !llvm.ptr
) -> i32
对应源码在 ConvertToLLVM.cpp (line 76) 的 ConvertHALEntryPointFuncOp。

代码行 -> 语义：

HALDispatchABI::getInputTypes(...)
-> 给 entry 补上 HAL ABI 三个指针参数
public func.func () -> ()
-> 改成 llvm.func(...)->i32
return
-> 改成 llvm.return 0 : i32
也就是：

HAL executable entry 最后不是普通函数，而是 runtime 约定好的 dispatch ABI 函数。

2. hal.interface.binding.subspan 怎么变
看 hal_interface_bindings.mlir (line 1)

前：

%memref = hal.interface.binding.subspan layout(#pipeline_layout) binding(1)
  offset(%c72) : memref<?x2xf32, strided<[2, 1], offset: 18>>{%c128}
%value = memref.load %memref[%c5, %c1]
后面 LLVM dialect 里会看到：

%state = llvm.load %arg1
%binding_ptrs = llvm.extractvalue %state[10]
%array_ptr = llvm.getelementptr %binding_ptrs[1]
%base_ptr = llvm.load %array_ptr
...
%offset_ptr = llvm.getelementptr ...
%value = llvm.load %offset_ptr
对应源码还是 ConvertToLLVM.cpp (line 1) 里的一组 HAL interface lowering pattern。

语义很直接：

HAL binding 不是抽象 memref 了，而是从 dispatch state 结构里取 binding base pointer，再自己算 offset/stride，最后发 LLVM load/store。

3. hal.interface.workgroup.* 怎么变
看 hal_interface_workgroup_info.mlir (line 1)

前：

%workgroup_id_z = hal.interface.workgroup.id[2] : index
后：

%state = llvm.load %arg2
%z16 = llvm.extractvalue %state[2]
%z64 = llvm.zext %z16 : i16 to i64
也就是：

workgroup id/count/size 最终都是从 ABI struct 里按字段读出来。

4. hal.executable.constant.load 怎么变
看 hal_executable_constants.mlir (line 1)

前：

%v0 = hal.executable.constant.load "foo" : i32
后：

%foo_ordinal_ptr = llvm.mlir.addressof @__constant_ordinal_foo
%foo_ordinal = llvm.load %foo_ordinal_ptr
%foo_ptr = llvm.getelementptr ...
%foo = llvm.load %foo_ptr
语义是：

先通过 constant key 找 ordinal，再从 executable constant table 取值。

七、注意：到这里还只是 MLIR LLVM dialect，不是最终 native LLVM IR

这点非常重要。

createConvertToLLVMPass 之后，variant 里是这种 IR：

llvm.func
llvm.load
llvm.getelementptr
llvm.mlir.global
这还是 MLIR 的 LLVM dialect，还没到 llvm::Module。

真正变成 native LLVM IR 的地方在 LLVMCPU target backend 的 serializeExecutable(...)：

LLVMCPUTarget.cpp (line 1)
关键代码线是：

auto llvmModule =
    mlir::translateModuleToLLVMIR(variantOp.getInnerModule(), context, libraryName);
这一步语义是：

builtin.module 里的 MLIR LLVM dialect -> 真正的 llvm::Module

然后它继续做：

runLLVMIRPasses(...)
见 LLVMIRPasses.cpp (line 1)
跑 LLVM pass pipeline

runEmitObjFilePasses(...)
生成：

object file
assembly
再走链接：

embedded ELF
system dylib / so / dll / wasm
static library
八、最后 serialization 后 IR 会怎么变

HAL 侧的 SerializeAllExecutablesPass 在：

SerializeExecutables.cpp (line 1)
它会对每个 variant 调 backend 的 serializeExecutable(...)，然后：

删掉 hal.executable.variant
插入 hal.executable.binary
也就是大致从：

hal.executable @ex {
  hal.executable.variant @embedded_elf_x86_64 {
    builtin.module {
      llvm.func @dispatch(...)
    }
  }
}
变成：

hal.executable @ex {
  hal.executable.binary @embedded_elf_x86_64
    format("embedded-elf-x86_64")
    {mime_type = "application/x-elf", data = dense<...> : vector<Nxi8>}
}
语义上就是：

HAL 阶段最后把“还能看懂的目标 IR”封成运行时可加载的 binary blob。

九、别忘了：HAL 还要把宿主侧 stream.* 改成真正的 HAL runtime 调用

这一步是 ConvertToHAL.cpp (line 1)

对应真实 IR 可以直接看 convert_to_hal.mlir (line 1)

你会看到：

前：

stream.cmd.dispatch @ex::@embedded_elf_aarch64::@dispatch,
                    @ex::@embedded_elf_x86_64::@dispatch
后：

%exec = hal.executable.lookup ...
%ordinal = hal.executable.export.ordinal ...
hal.command_buffer.dispatch ...
同时：

stream.tensor.import/export
-> hal.buffer_view / hal.buffer 相关操作
stream.cmd.execute
-> hal.command_buffer.create/finalize
stream.timepoint.await
-> hal.fence.await
资源分配
-> hal.allocator.allocate
所以 HAL 阶段不是只管 executable，它还把宿主侧运行时调用也正式落成 HAL runtime API。

十、一句话把整条线串起来

以 LLVMCPU 为例，HAL 阶段可以压成这条链：

stream.executable / stream.cmd.dispatch
-> materialize interfaces
-> hal.executable + hal.executable.variant + hal.executable.export
-> LLVMCPU configure pipeline 选 lowering 策略
-> LLVMCPU translation pipeline
   (linalg/tensor -> vector/buffer -> MLIR LLVM dialect)
-> translateModuleToLLVMIR
-> LLVM IR opt
-> emit obj / link elf|so
-> hal.executable.binary
同时 host 侧:
stream.* -> hal.command_buffer / hal.buffer / hal.executable.lookup / hal.fence
如果你愿意，下一条我可以继续顺着这个回答，专门挑一条再细讲：

只讲 MaterializeInterfacesPass：stream.executable -> hal.executable.variant
只讲 LLVMCPU codegen：func.func @entry -> llvm.func
只讲 ConvertToHALPass：host 侧 stream.cmd.dispatch -> hal.command_buffer.dispatch
我建议下一步讲第 2 条，这样最贴你刚问的“以 LLVM IR 为例”。