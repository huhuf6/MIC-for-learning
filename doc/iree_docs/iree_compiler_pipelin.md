# IREE 编译 Pipeline 调用路线图（结合代码）

本文对应 `iree-compile --compile-mode=std` 的主路径，重点是：

1. 顶层调用链（工具层 -> C API -> Pipeline builder）。
2. `buildIREEVMTransformPassPipeline` 的 phase 顺序。
3. 每个 phase 的二级 `build*PassPipeline`/关键 pass 入口。

---

## 1) 顶层调用链（从命令行到 Pipeline）

1. `iree-compile` 解析 `--compile-from/--compile-to`，调用：
   `ireeCompilerInvocationPipeline(inv, IREE_COMPILER_PIPELINE_STD)`  
   代码：`iree/compiler/src/iree/compiler/Tools/iree_compile_lib.cc`
2. C API 进入 `Invocation::runPipeline(...)` 的 `STD` 分支。  
   代码：`iree/compiler/src/iree/compiler/API/Internal/CompilerDriver.cpp`
3. 在 `STD` 分支中调用：
   `buildIREEVMTransformPassPipeline(...)`  
   代码：`iree/compiler/src/iree/compiler/Pipelines/Pipelines.cpp`

你现在关注的 `buildIREEVMTransformPassPipeline` 就是总调度器。

---

## 2) Phase 总图（STD 模式）

`IREEVMPipelinePhase` 枚举定义在：
`iree/compiler/src/iree/compiler/Pipelines/Pipelines.h`

执行顺序：

`Start`
→ `Input`
→ `ABI`
→ `Preprocessing`
→ `GlobalOptimization`
→ `DispatchCreation`
→ `Flow`
→ `Stream`
→ `ExecutableSources`（HAL 子阶段）
→ `ExecutableConfigurations`（HAL 子阶段）
→ `ExecutableTargets`（HAL 子阶段）
→ `HAL`
→ `VM`
→ `End`

> 说明：HAL 内部 3 个 executable 子阶段由 `getHALPipelinePhase` 映射到 VM phase。

---

## 3) 总调度函数如何串接

`buildIREEVMTransformPassPipeline(...)` 的骨架是：

1. 先调用 `buildIREEPrecompileTransformPassPipeline(...)`。
2. 若 `compileTo <= GlobalOptimization`，直接 early-exit。
3. 再依次构建：
   - `DispatchCreation`
   - `Flow`
   - `Stream`
   - `HAL`
   - `VM`
4. 每个阶段都支持：
   - `compileFrom < 当前阶段` 才执行（late-entry）
   - `compileTo == 当前阶段` 时提前返回（early-exit）

---

## 4) 各 Phase 的二级函数路线

### A. Input Phase

入口：`buildIREEPrecompileTransformPassPipeline(...)`

关键流程：

1. 根据 `inputOptions.parseInputTypeMnemonic()` 选择输入转换前置逻辑：
   - `none`
   - `auto_detect` -> `createAutoInputConversionPipelinePass(...)`
   - `plugin` -> `extendCustomInputConversionPassPipeline(...)`
2. 统一调用  
   `InputConversion::buildCommonInputConversionPassPipeline(...)`

二级函数定义：

- `iree/compiler/src/iree/compiler/InputConversion/Common/Passes.cpp`
  - `buildCommonInputConversionPassPipeline(...)`
  - 包含 `createIREEImportPublicPass / createImportMLProgramPass / sanitize + 类型升降精度 pass`

### B. ABI Phase

入口：`buildIREEPrecompileTransformPassPipeline(...)`

关键流程：

1. `bindingOptions.native` 时调用  
   `IREE::ABI::buildTransformPassPipeline(...)`
2. `bindingOptions.tflite` 时调用  
   `IREE::TFLite::buildTransformPassPipeline(...)`

二级函数定义：

- `iree/compiler/src/iree/compiler/Bindings/Native/Transforms/Passes.cpp`
- `iree/compiler/src/iree/compiler/Bindings/TFLite/Transforms/Passes.cpp`

### C. Preprocessing Phase

入口：`buildIREEPrecompileTransformPassPipeline(...)`

二级函数：

- `Preprocessing::buildPreprocessingPassPipeline(...)`  
  文件：`iree/compiler/src/iree/compiler/Preprocessing/Passes.cpp`

内部顺序：

1. CLI 显式配置的 pipeline（文本、transform spec、PDL spec）优先。
2. 插件扩展 `pipelineExtensions->extendPreprocessingPassPipeline(...)`。
3. 属性驱动的 `createAttrBasedPipelinePass`。

### D. GlobalOptimization Phase

入口：`buildIREEPrecompileTransformPassPipeline(...)`

二级函数：

- `GlobalOptimization::buildGlobalOptimizationPassPipeline(...)`  
  文件：`iree/compiler/src/iree/compiler/GlobalOptimization/Passes.cpp`

关键结构（按大块）：

1. 参数导入（可选）。
2. 函数级预处理与 canonicalize/CSE。
3. `expand tensor shapes` + transpose/reshape 等传播。
4. 可选 data tiling/encoding。
5. 全局折叠与 IPO。
6. 可选 const expr hoisting。
7. 可选 const eval（通过 hook 注入 JIT pass pipeline）。
8. 可选 numeric precision reduction。
9. 参数导出/生成 splat 参数（可选）。

### E. DispatchCreation Phase

入口：`buildIREEVMTransformPassPipeline(...)`

二级函数：

- `DispatchCreation::buildDispatchCreationPassPipeline(...)`  
  文件：`iree/compiler/src/iree/compiler/DispatchCreation/Passes.cpp`

关键结构：

1. 早期 tracing + pad/fusion 预处理。
2. fixed-point IPO 子管线。
3. dispatch region 形成前预处理。
4. `addDispatchRegionCreationPasses(...)` 形成 dispatch。
5. 转成 workgroups + `tensor -> flow` + workgroup count materialize + cleanup。

### F. Flow Phase

入口：`buildIREEVMTransformPassPipeline(...)`

二级函数：

- `IREE::Flow::buildFlowTransformPassPipeline(...)`  
  文件：`iree/compiler/src/iree/compiler/Dialect/Flow/Transforms/Passes.cpp`

关键结构：

1. 输入合法性与初始化顺序校验。
2. outline dispatch externs/regions。
3. annotate/deduplicate executables。
4. 可选 benchmark/tracing/debug 注入。
5. cleanup + fixed-point IPO。

### G. Stream Phase

入口：`buildIREEVMTransformPassPipeline(...)`

二级函数：

- `IREE::Stream::buildStreamTransformPassPipeline(...)`  
  文件：`iree/compiler/src/iree/compiler/Dialect/Stream/Transforms/Passes.cpp`

内部由四个子 pipeline 串接：

1. `buildStreamTensorPassPipeline(...)`
2. `buildStreamAsyncPassPipeline(...)`
3. `buildStreamCmdPassPipeline(...)`
4. `buildStreamOptimizationPassPipeline(...)`

最后做 cleanup + `SymbolDCE`。

### H. HAL Phase（含 executable 子阶段）

入口：`buildIREEVMTransformPassPipeline(...)`

根据 `executionModel` 分叉：

1. `AsyncInternal/AsyncExternal`（默认主路径）：
   `IREE::HAL::buildHALTransformPassPipeline(...)`
2. `InlineStatic`：
   `IREE::HAL::Inline::buildHALInlineStaticTransformPassPipeline(...)`
3. `InlineDynamic`：
   `IREE::HAL::Loader::buildHALInlineDynamicTransformPassPipeline(...)`
4. `HostOnly`：跳过 HAL。

#### H1. 主 HAL 路径（Async）

文件：`iree/compiler/src/iree/compiler/Dialect/HAL/Transforms/Passes.cpp`

子阶段顺序：

1. `ExecutableSources`：device assignment + configuration + optional preprocess-executables
2. `ExecutableConfigurations`：configure executable variants
3. `ExecutableTargets`：translate all executables
4. Host program convert to HAL
5. 可选 link executables
6. resolve ordinals / materialize caches / memoize queries
7. initialize devices + affine/scf lowering cleanup
8. 可选 serialize executables
9. fixed-point IPO 收尾

#### H2. InlineStatic / InlineDynamic

文件：

- `iree/compiler/src/iree/compiler/Modules/HAL/Inline/Transforms/Passes.cpp`
- `iree/compiler/src/iree/compiler/Modules/HAL/Loader/Transforms/Passes.cpp`

两者都包含：

1. device assignment + configuration
2. configure + translate executables

区别：

- InlineStatic：inline executable functions，再做 `HAL::Inline` conversion。
- InlineDynamic：走 `HAL::Loader` conversion + link/serialize/materialize executables。

### I. VM Phase

入口：`buildIREEVMTransformPassPipeline(...)`

二级函数：

- `IREE::VM::buildVMTransformPassPipeline(...)`  
  文件：`iree/compiler/src/iree/compiler/Dialect/VM/Transforms/Passes.cpp`

关键结构：

1. inliner + symbol DCE
2. init-order verify + combine initializers
3. 函数内 loop/scf/affine 规范化与 lowering
4. `std/util/... -> VM` conversion
5. rodata 重写与去重
6. 再次 inliner + DCE
7. global init/deinit 物化 + 收尾优化

---

## 5) 三个最关键“控制开关”

1. `--compile-from/--compile-to`
   - 控制从哪个 phase 恢复、到哪个 phase 截断。
2. `--iree-execution-model`（通过 `SchedulingOptions`）
   - 决定 HAL 路径（Async / InlineStatic / InlineDynamic / HostOnly）。
3. `IREEVMPipelineHooks`
   - `beforePhase/afterPhase`（如 phase dump）
   - `buildConstEvalPassPipelineCallback`（const-eval 递归编译所需 hook）
   - `pipelineExtensions`（插件扩展 input/preprocess 等）

---

## 6) 快速定位索引（建议从这里点进去读）

- 总入口调度：`iree/compiler/src/iree/compiler/Pipelines/Pipelines.cpp`
- phase 枚举：`iree/compiler/src/iree/compiler/Pipelines/Pipelines.h`
- 工具层入口：`iree/compiler/src/iree/compiler/Tools/iree_compile_lib.cc`
- Invocation 调度：`iree/compiler/src/iree/compiler/API/Internal/CompilerDriver.cpp`

调用顺序（从 iree-compile 到 buildIREEVMTransformPassPipeline）

iree-compile 解析参数，设置 --compile-from/--compile-to，并调用 C API 的 ireeCompilerInvocationPipeline(..., IREE_COMPILER_PIPELINE_STD)。
iree_compile_lib.cc (line 114)
iree_compile_lib.cc (line 252)
iree_compile_lib.cc (line 275)

进入 Invocation::runPipeline，STD 分支会解析 phase 名称并调用 buildIREEVMTransformPassPipeline(...)。
CompilerDriver.cpp (line 998)
CompilerDriver.cpp (line 934)
CompilerDriver.cpp (line 1032)

buildIREEVMTransformPassPipeline 先调用 buildIREEPrecompileTransformPassPipeline，然后继续后半段。
Pipelines.cpp (line 305)
