# MIC-for-learning 源码精读（不含 CMake/测试）

## 1. 阅读范围与目标

本文只覆盖项目的核心源码：
- `tools/mlir2trt/mlir2trt.cpp`
- `include/MIC/**` 与 `lib/**`（Dialect / Passes / TensorRT / CUDA）

显式不包含：
- `CMakeLists.txt`、`lib/**/CMakeLists.txt`、`tools/**/CMakeLists.txt`
- `test_*`、`scripts/utils/test_pipeline.py` 等测试与脚本验证代码

目标：
- 解释端到端流程（输入 ONNX/MLIR 到输出 TensorRT engine）
- 解释各模块职责与接口边界
- 穿插说明关键 C++ 用法（模板、RAII、PImpl、lambda、RTTI 风格转换等）

---

## 2. 项目主链路（先建立整体心智模型）

核心入口在 `mlir2trt`：
1. 解析命令行参数（导入路径、是否保留中间 MLIR、是否输出 pipeline 摘要）。
2. 根据输入后缀选择：
   - `ONNX -> TensorRT Parser` 直通
   - `ONNX -> 外部工具转 MLIR -> MLIR Pass Pipeline -> TensorRT`
   - 或直接 `MLIR -> Pass Pipeline -> TensorRT`
3. 使用 `TensorRTBuilder` 管理 Builder/Network/Config。
4. 对 MLIR 路径，调用 `NetworkConverter` 把 runtime call（`mic_trt_*`）映射为 TensorRT layer。
5. 构建序列化 engine 并写盘。

这条主链路本质上是“**双前端 + 单后端**”：
- 前端 A：ONNX Parser 直连 TensorRT
- 前端 B：MLIR 降低链
- 后端：统一落到 TensorRT 网络并 build

参考：`tools/mlir2trt/mlir2trt.cpp:40-49,114-141,259-411`

---

## 3. 入口程序精读：`tools/mlir2trt/mlir2trt.cpp`

### 3.1 参数与模式选择

`ConvertOptions` 把 CLI 参数收束为结构体，避免函数参数膨胀：
- `onnxImportMode`（Auto/TRT/MLIR）
- `onnxMlirConverter`（外部转换器命令）
- `verbosePipeline`

`parseArgs` 的策略是“轻量手写解析”：
- 用 `rfind("--key=", 0) == 0` 检测前缀参数
- 单个位置参数作为输入路径

参考：`mlir2trt.cpp:56-100`

C++ 用法点：
- 局部 lambda (`parseOnnxImportMode`) 用于限定解析逻辑作用域；避免引入全局函数污染。

### 3.2 ONNX->MLIR 外部转换的安全细节

`quoteShellArg` 通过单引号转义避免路径中包含特殊字符时命令串损坏，随后 `std::system` 执行外部工具。

参考：`mlir2trt.cpp:102-141`

C++ 用法点：
- 字符串构造 + 明确转义（比直接拼接裸参数更稳）。
- 用 `std::ifstream` 进行结果文件存在性检查，减少 silent failure。

### 3.3 动态 shape profile 的默认策略

`addDefaultDynamicProfile` 遍历 TensorRT 输入：
- 维度 `<0` 视为动态维，设置 `min/opt/max = 1/8/32`
- 若出现动态输入，向 config 注册 profile

参考：`mlir2trt.cpp:207-257`

设计含义：
- 这是“可跑优先”的保守默认，便于教学和原型验证。
- 真实部署通常会按业务分布定制 profile 区间。

### 3.4 MLIR pipeline 的组织方式

`importMlirToNetwork` 内部将 pass 阶段写成：
```cpp
struct PipelineStage {
  const char *name;
  std::function<std::unique_ptr<mlir::Pass>()> makePass;
};
```
再用 `std::vector<PipelineStage>` 顺序执行，逐阶段计时与失败短路。

参考：`mlir2trt.cpp:300-359`

C++ 用法点：
- `std::function + lambda`：实现“可配置阶段表”，比硬编码多段 `pm.addPass(...)` 更可维护。
- `std::chrono::steady_clock`：用于稳定耗时统计。

### 3.5 末端收敛

`run` 函数负责“导入 -> build -> 序列化写盘”完整闭环；`main` 只做参数校验与转发。

参考：`mlir2trt.cpp:383-418`

这符合单一职责：
- `main`: CLI 壳层
- `run`: 业务主流程

---

## 4. NN Dialect：IR 语义层

### 4.1 方言与操作定义来源

- 方言声明：`include/MIC/Dialect/NNDialect.h`
- 方言初始化：`lib/Dialect/NN/NN.cpp`
- 操作 schema：`include/MIC/Dialect/NNOps.td`

`NNOps.td` 里定义了 `linear/conv2d/attention/gelu/layer_norm/relu/softmax/add/mul/matmul/reshape/transpose`。

参考：`NNOps.td:31-149`

### 4.2 `allowUnknownOperations()` 的取舍

`NNDialect::initialize` 中显式调用了 `allowUnknownOperations()`。

参考：`lib/Dialect/NN/NN.cpp:19-25`

含义：
- 优点：迭代早期对过渡 op 更包容，便于教学/快速试验。
- 代价：会降低“未知 op 立刻报错”的严格性，问题可能后移到下游 pass 才暴露。

### 4.3 TableGen 驱动的代码生成模式

`NNOps.td` 描述 IR，C++ 通过：
- `#define GET_OP_CLASSES`
- `#include "NN.h.inc"`
- `#include "NN.cpp.inc"`
接入生成代码。

参考：`include/MIC/Dialect/NNOps.h`, `lib/Dialect/NN/NNOps.cpp`

C++/MLIR 用法点：
- 这是 MLIR 方言开发标准模式：声明在 `.td`，实现骨架由 TableGen 生成，减少手写样板。

---

## 5. Pass 管线：从 ONNX 名字重写到 TensorRT 调用

核心文件：`lib/Passes/PipelineLoweringPasses.cpp`

### 5.1 `LowerONNXToNNPass`：名字级映射

该 pass 通过 `StringMap` 把 `onnx.*` 重命名为 `nn.*`，并给 `softmax` 补默认 `axis=-1`。

参考：`PipelineLoweringPasses.cpp:256-295`

关键点：
- `renameOp` 复制 operands/results/attrs 再替换 uses。
- 当前属于“轻语义重命名”，不是完整 ONNX 语义 lower（教学原型常见做法）。

### 5.2 `LowerNNToLinalgPass`：语义到结构化张量

目前落地了：
- `nn.add/nn.mul` -> `linalg.generic`
- `nn.relu` -> `linalg.generic + arith.max`
- `nn.matmul/nn.linear` -> `linalg.matmul (+ bias add)`

参考：`PipelineLoweringPasses.cpp:102-254,298-336`

亮点：
- 为生成的 `linalg.generic` 打上 `mic.backend.op` 属性，后续用来推断 runtime callee（`add/mul/relu/linear`）。

### 5.3 Linalg 优化 pass

封装调用了 MLIR 官方 pass：
- elementwise fusion
- tiling（`{16,16}`）
- strategy vectorize

参考：`PipelineLoweringPasses.cpp:338-389`

这部分体现“用现成 pass + 自定义桥接 pass”的组合策略。

### 5.4 `LinalgToTensorRTPass`：后端边界收敛

把 `linalg.matmul` / 带 `mic.backend.op` 的 `linalg.generic` 改写为：
- `func.call @mic_trt_matmul`
- `func.call @mic_trt_add/mul/relu/linear`

并自动注入私有函数声明（若不存在）。

参考：`PipelineLoweringPasses.cpp:391-459`

C++ 用法点：
- `ensureRuntimeDeclaration` 先查符号表再插入声明，避免重复定义。

---

## 6. 另一条降低路径：`NNToRuntimeLoweringPass`

文件：`lib/Passes/NNToRuntimeLoweringPass.cpp`

该 pass 直接把 `nn.*` 映射为 `mic_nn_*` runtime call，属于“绕过 linalg 层”的简化路径。

参考：`NNToRuntimeLoweringPass.cpp:26-97`

注意：
- 当前 `mlir2trt` 主 pipeline 使用的是 `LinalgToTensorRTPass`（`mic_trt_*` 约定），两者并行存在，定位不同。

---

## 7. TensorRTBuilder：后端资源管理层

文件：`include/MIC/TensorRT/TensorRTBuilder.h`, `lib/TensorRT/TensorRTBuilder.cpp`

职责：
- 管理 `IBuilder / INetworkDefinition / IBuilderConfig`
- 提供 `buildSerializedNetwork` 封装

参考：`TensorRTBuilder.cpp:16-58`

### 7.1 C++ 设计点：PImpl + RAII

`TensorRTBuilder` 只暴露前置声明与 `unique_ptr<Impl>`：
- 头文件不暴露实现细节，减编译依赖（PImpl）
- 析构时回收 TensorRT 对象（RAII）

参考：`TensorRTBuilder.h`, `TensorRTBuilder.cpp:16-41,61-79`

### 7.2 当前实现状态

`setFP16Mode/setINT8Mode/setMaxBatchSize/setMaxWorkspaceSize` 目前是空实现。

参考：`TensorRTBuilder.cpp:81-84`

影响：
- 上层虽然调用了这些接口，但并未真正修改 config。
- 文档与接口语义已先行，后续需补全实现。

---

## 8. NetworkConverter：MLIR 值到 TensorRT 张量的桥

文件：`include/MIC/TensorRT/NetworkConverter.h`, `lib/TensorRT/NetworkConverter.cpp`

核心做法：
1. 维护 `DenseMap<Value, ITensor*>` 双向映射。
2. 识别 `func.call` 的 `mic_trt_*` 约定名并降到具体 TensorRT API。
3. 在 `func.return` 时 `markOutput`。

参考：`NetworkConverter.cpp:49-319`

### 8.1 支持的调用

已支持：
- `mic_trt_matmul`
- `mic_trt_relu`
- `mic_trt_add`
- `mic_trt_mul`
- `mic_trt_linear`

参考：`NetworkConverter.cpp:115-133`

### 8.2 输入张量物化策略

`getOrCreateTensor` 目前仅能把**函数入口 block 参数**物化为 TensorRT input。中间值必须先由已转换层产出并登记映射。

参考：`NetworkConverter.cpp:251-298`

这使约束很清晰：
- 入口由函数签名决定
- 中间值靠 lowering 链保证可追踪

### 8.3 C++/MLIR 用法点

- `dyn_cast / dyn_cast_or_null`：LLVM 风格安全下转型。
- `LogicalResult + failure()/success()`：MLIR 统一错误传递风格。
- `StringRef`：轻量字符串视图，减少不必要拷贝。

---

## 9. CUDA 与 TensorRT Plugin 层

### 9.1 CUDA Kernel

文件：`lib/CUDA/CUDAKernel.cu`

实现了：
- `layerNormKernel`
- `fusedLinearGELUKernel`

参考：`CUDAKernel.cu:10-98`

这两者目前是教学风格的直观实现：
- 一个线程计算一个输出元素，内部含较多串行循环，便于理解但性能不是最优。

### 9.2 LayerNormPlugin

文件：`include/MIC/CUDA/LayerNormPlugin.h`, `lib/CUDA/LayerNormPlugin.cpp`

实现了 TensorRT `IPluginV2` 生命周期、序列化/反序列化与 creator 注册。

参考：`LayerNormPlugin.cpp:10-225`

C++ 用法点：
- 再次使用 PImpl（`LayerNormPlugin::Impl`）隐藏权重与序列化细节。
- `REGISTER_TENSORRT_PLUGIN` 通过静态注册接入 TensorRT 插件工厂。

---

## 10. C++ 关键用法讲解（结合本项目）

### 10.1 `std::unique_ptr` 与所有权

典型位置：`TensorRTBuilder`, `NetworkConverter`, `LayerNormPlugin`。

特点：
- 独占所有权，析构自动释放。
- 配合 `std::make_unique` 减少裸 `new`。

在本项目中的价值：
- TensorRT/Plugin 对象生命周期明确，减少内存泄漏风险。

### 10.2 PImpl（Pointer to Implementation）

模式：
- 头文件只保留 `class Impl; std::unique_ptr<Impl> impl;`
- 源文件定义 `Impl` 细节

在本项目中的好处：
- 降低头文件对 TensorRT 实现细节的暴露和耦合。
- 改实现细节时不必大面积触发重编译。

### 10.3 Lambda 与局部策略对象

典型位置：
- 参数解析局部 lambda
- Pass pipeline 的 `std::function` 构造器
- `module.walk([&](Operation *op){...})`

价值：
- 把“只在局部使用”的规则就地定义，减少全局符号与跳转成本。

### 10.4 LLVM/MLIR RTTI 风格转换

常用 API：
- `dyn_cast<T>`
- `dyn_cast_or_null<T>`

相比 C++ `dynamic_cast`：
- 更贴近 LLVM 的类型系统与性能习惯。

### 10.5 `SmallVector` / `StringRef` / `StringMap`

这些是 LLVM ADT：
- `SmallVector`：小对象栈内优化，减少堆分配
- `StringRef`：非 owning 字符串视图
- `StringMap`：字符串 key 的高效哈希结构

本项目 pass 代码大量使用这三者，属于标准 LLVM 编码风格。

### 10.6 Pass 模板模式

`class XPass : public PassWrapper<XPass, OperationPass<ModuleOp>>`

含义：
- 通过模板把“pass 作用域（ModuleOp/FuncOp）”在类型层固定。
- 框架可据此自动处理 pass 注册与调度。

---

## 11. 代码现状观察（读源码时应注意）

以下是“现状说明”，不是测试结论：

1. `TensorRTBuilder` 的配置接口目前为空实现。
- 位置：`lib/TensorRT/TensorRTBuilder.cpp:81-84`

2. `FusionPass/ConstantFoldPass/LayoutTransformPass` 更像教学占位或草案。
- 文件：`lib/Passes/FusionPass.cpp`, `lib/Passes/ConstantFoldPass.cpp`, `lib/Passes/LayoutTransformPass.cpp`
- 主 pipeline 未直接使用它们。

3. `LayerNormPlugin::enqueue` 中通过 `getOutputDimensions(0, nullptr, 0)` 取维度，属于风险点（传空指针）。
- 位置：`lib/CUDA/LayerNormPlugin.cpp:118-121`

4. `NNDialect` 开启 `allowUnknownOperations`，利于迭代但降低前期约束强度。
- 位置：`lib/Dialect/NN/NN.cpp:20-22`

---

## 12. 建议的阅读顺序（第一次读这项目）

1. 先读入口：`tools/mlir2trt/mlir2trt.cpp`
2. 再读主 pass：`lib/Passes/PipelineLoweringPasses.cpp`
3. 读后端桥接：`lib/TensorRT/NetworkConverter.cpp`
4. 回看 IR 定义：`include/MIC/Dialect/NNOps.td` + `lib/Dialect/NN/NN.cpp`
5. 最后读 CUDA/Plugin：`lib/CUDA/CUDAKernel.cu` + `lib/CUDA/LayerNormPlugin.cpp`

这样可以先掌握“数据怎么流动”，再看“每层怎么实现”。

---

## 13. 一句话总结

这个项目的核心价值是把 MLIR 教学链路完整串到 TensorRT：
- 前端可走 ONNX 直通或 MLIR pipeline
- 中间通过 NN/Linalg 分层降低
- 后端通过 `mic_trt_*` 调用约定收敛到 TensorRT API

代码整体呈现“教学原型 + 可扩展骨架”形态：主路径可读性强，部分配置与插件细节仍待工程化补齐。
