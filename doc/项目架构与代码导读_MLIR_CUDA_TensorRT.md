# MIC-for-learning 项目架构与代码导读（MLIR / CUDA / TensorRT）

> 目标读者：想快速理解本仓库整体设计、关键代码入口、当前实现边界、以及后续可演进方向的开发者。

---

## 1. 项目定位与核心目标

MIC-for-learning 是一个教学型 AI 推理编译器原型，目标是把“模型表示”逐步降到“可执行推理引擎（TensorRT Engine）”。

核心目标：

1. 打通端到端链路：`模型 -> 中间表示 -> 后端引擎`。
2. 使用 MLIR 方言 + Pass 组织中端优化流程。  
3. 与 CUDA/TensorRT 后端衔接，形成可运行闭环。
4. 保留工程可读性，方便教学和二次开发。

---

## 2. 代码目录速览

```text
include/
  MIC/
    Dialect/         # NN dialect 定义与 TableGen 产物
    Passes/          # Pass 对外构造函数声明
    TensorRT/        # TensorRTBuilder / NetworkConverter 接口
    CUDA/            # CUDA Kernel / Plugin 头文件

lib/
  Dialect/NN/        # NN dialect 实现
  Passes/            # ONNX->NN->Linalg->TensorRT calls 的 pass 实现
  TensorRT/          # Builder 封装 + MLIR call 到 TRT API 映射
  CUDA/              # CUDA kernel + LayerNorm TensorRT plugin 示例

tools/mlir2trt/
  mlir2trt.cpp       # 工具主入口，ONNX/MLIR 导入与构建引擎

scripts/
  utils/             # pipeline 测试、onnx->mlir 转换脚本、稳定运行脚本
  benchmark/         # PyTorch vs TensorRT 性能对比

doc/
  *.md               # 架构文档（本文件）
```

---

## 3. 两条主工作流（非常关键）

当前 `mlir2trt` 支持两条导入路径：

1. ONNX 直连 TensorRT（稳定主路径）
2. ONNX 先转 MLIR，再走 MLIR pipeline（实验/教学路径）

### 3.1 ONNX 直连 TensorRT 路径

```text
model.onnx
  -> TensorRT ONNX Parser
  -> addDefaultDynamicProfile(如有动态维)
  -> TensorRTBuilder::build()
  -> model.engine
```

对应代码：

- `tools/mlir2trt/mlir2trt.cpp`
  - `importOnnxToNetwork(...)`
  - `addDefaultDynamicProfile(...)`

### 3.2 MLIR pipeline 路径

```text
model.mlir / (onnx->mlir结果)
  -> parse MLIR module
  -> LowerONNXToNN
  -> Canonicalizer
  -> LowerNNToLinalg
  -> Linalg Fuse / Tile / Vectorize
  -> LowerLinalgToTensorRTCalls (func.call @mic_trt_*)
  -> NetworkConverter(把 mic_trt_* 映射到 TensorRT API)
  -> TensorRTBuilder::build()
  -> model.engine
```

对应代码：

- `tools/mlir2trt/mlir2trt.cpp`：`importMlirToNetwork(...)`
- `lib/Passes/PipelineLoweringPasses.cpp`
- `lib/TensorRT/NetworkConverter.cpp`

---

## 4. 关键模块深度导读

## 4.1 Dialect 层（NN 方言）

核心文件：

- `include/MIC/Dialect/NNOps.td`
- `include/MIC/Dialect/NNDialect.h`
- `lib/Dialect/NN/NN.cpp`
- `lib/Dialect/NN/NNOps.cpp`

职责：

1. 定义 `nn.linear / nn.conv2d / nn.relu / nn.matmul ...` 等抽象算子。
2. 作为 ONNX 到后端之间的语义中间层。
3. 为 pass 提供稳定可重写目标。

当前实现特点：

- 允许 unknown operations（用于教学场景下增量映射和过渡 op 承载）。
- TableGen 生成声明 + 手写实现共同组成。

---

## 4.2 Pass 层（中端 pipeline）

核心文件：

- `include/MIC/Passes/Passes.h`
- `lib/Passes/PipelineLoweringPasses.cpp`
- `lib/Passes/NNToRuntimeLoweringPass.cpp`

### 4.2.1 Pass 阶段与当前行为

1. `LowerONNXToNN`
- 将 `onnx.*` 改写到 `nn.*`（例如 `onnx.Gemm -> nn.linear`）。

2. `LowerNNToLinalg`
- 当前重点实现：`nn.add / nn.mul / nn.relu / nn.matmul / nn.linear`
- 生成 `linalg.generic / linalg.matmul / linalg.fill / linalg.init_tensor`。

3. `LinalgFusePass`
- 调用 MLIR 官方 linalg elementwise fusion pass。

4. `LinalgTilePass`
- 调用 MLIR 官方 linalg tiling pass。

5. `LinalgVectorizePass`
- 调用 MLIR 官方 strategy vectorize pass。

6. `LinalgToTensorRTPass`
- 将 `linalg.matmul` 和标注 `mic.backend.op` 的 `linalg.generic`
  改写为 `func.call @mic_trt_*`。

> 注：这条链路属于“可运行原型 + 渐进完善”状态，不是完整工业级 lowering。

---

## 4.3 TensorRT Backend 层

核心文件：

- `include/MIC/TensorRT/TensorRTBuilder.h`
- `lib/TensorRT/TensorRTBuilder.cpp`
- `include/MIC/TensorRT/NetworkConverter.h`
- `lib/TensorRT/NetworkConverter.cpp`

### 4.3.1 TensorRTBuilder

职责：

1. 封装 `IBuilder / INetworkDefinition / IBuilderConfig` 生命周期。
2. 提供 `buildSerializedNetwork` 输出 engine bytes。

现状：

- 已可构建；`setFP16Mode/setINT8Mode` 等接口仍是空实现（预留）。

### 4.3.2 NetworkConverter

职责：

1. 识别 `func.call @mic_trt_*`。
2. 分发到 TensorRT API：
   - `mic_trt_matmul -> addMatrixMultiply`
   - `mic_trt_relu -> addActivation(kRELU)`
   - `mic_trt_add/mul -> addElementWise`
   - `mic_trt_linear -> matmul + bias add`
3. 维护映射：
   - `MLIR Value -> ITensor*`
   - `ITensor* -> MLIR Value`
4. 处理输入输出边界：
   - entry block 参数 => `network->addInput`
   - `func.return` => `network->markOutput`

---

## 4.4 CUDA 与 Plugin 层

核心文件：

- `lib/CUDA/CUDAKernel.cu`
- `lib/CUDA/LayerNormPlugin.cpp`

职责：

1. 提供 CUDA kernel 示例（LayerNorm / FusedLinearGELU）。
2. 提供 TensorRT plugin 示例（LayerNormPlugin）。

注意点：

- 该层当前更偏教学示例，部分实现（如维度获取、健壮性）需要进一步工程化。

---

## 4.5 工具与脚本层

核心文件：

- `tools/mlir2trt/mlir2trt.cpp`
- `scripts/utils/test_pipeline.py`
- `scripts/benchmark/benchmark.py`
- `scripts/utils/onnx_to_mlir.sh`
- `scripts/utils/run_mlir2trt_safe.sh`

### 4.5.1 `mlir2trt` 参数（当前）

```text
mlir2trt [--verbose-pipeline]
         [--onnx-import=auto|trt|mlir]
         [--onnx-mlir-converter=<cmd>]
         [--keep-temp-mlir]
         <model.onnx|model.mlir>
```

### 4.5.2 ONNX 导入模式

1. `--onnx-import=trt`
- ONNX parser 直连 TensorRT。

2. `--onnx-import=mlir`
- 先调用外部转换器把 ONNX 变 MLIR，再跑 pipeline。

3. `--onnx-import=auto`
- 当前默认走 TRT 分支。

### 4.5.3 稳定运行脚本

`run_mlir2trt_safe.sh` 会在 `exit 139 (SIGSEGV)` 时自动重试，适合作为实验阶段防抖手段。

---

## 5. 关键数据流与控制流（按代码执行顺序）

### 5.1 工具入口执行顺序

1. 解析参数（包含 onnx-import 模式）。
2. 创建 `TensorRTBuilder`。
3. 分支导入：
   - ONNX->TRT 或 ONNX->MLIR->pipeline 或 MLIR->pipeline。
4. `builder.build()` 生成序列化 engine。
5. 写入 `.engine` 文件。

### 5.2 MLIR pipeline 执行回显

`--verbose-pipeline` 下会显示：

- stage 序号/名称/耗时
- 每阶段前后 IR 摘要
  - `total_ops`
  - `nn/linalg/arith/tensor`
  - `func.call`
  - `mic_trt_call`

用途：快速观察“IR 是否按预期往后端调用收敛”。

---

## 6. 当前覆盖模型建议

仓库内新增示例：

- `models/coverage_all_passes.mlir`

作用：

1. 覆盖 ONNX 风格输入 `onnx.Gemm/onnx.Conv` 到 NN 的映射。
2. 覆盖 NN 到 Linalg 的主链路。
3. 覆盖 linalg fuse/tile/vectorize pass 执行。
4. 覆盖 `mic_trt_*` 到 TensorRT API 映射并最终构建 engine。

---

## 7. 你需要重点关注的“实现边界”

1. ONNX->MLIR 目前依赖外部转换器命令约定。
2. NN/Linalg lowering 覆盖面有限（主打关键算子路径）。
3. TensorRTBuilder 的精度/workspace 接口有预留但未完整实现。
4. NetworkConverter 目前是“按 `mic_trt_*` call contract”映射，不是通用 MLIR 全算子转换器。
5. ONNX 直连 TRT 在某些环境下可能出现随机崩溃，建议用 safe-run 脚本防抖并持续定位根因。

---

## 8. 开发与调试建议

1. 先保证最小可运行闭环
- 用小模型快速验证 `build -> run -> benchmark`。

2. 再扩算子与 pass
- 每新增算子都补：
  - Dialect 表达
  - Lowering
  - Backend 映射
  - 回归模型

3. 日志先行
- 保持 `--verbose-pipeline` 常开（至少开发阶段）。

4. 明确 ABI 契约
- 固化 `mic_trt_*` 调用签名（输入输出 shape/attr/broadcast 规则）。

5. 分层验证
- ONNX->MLIR
- MLIR pass
- call->TRT layer
- engine 执行

---

## 9. AI 编译器常见问题（提问 + 解答，含底层原理）

### Q1: 为什么需要中间层 Dialect，而不是 ONNX 直接到后端？
A: 底层原因是“语义解耦”。ONNX 是交换格式，后端 API（如 TensorRT）是执行格式，两者抽象层级不同。直接 ONNX->后端会把优化规则绑定在具体前端和具体后端上，规则难复用。引入 NN/Linalg 等中间层后，优化可在“统一语义空间”进行，然后分别降到不同后端，这就是编译器里经典的“前端-中端-后端”分层思想。

### Q2: 什么时候该做图级融合，什么时候保留算子边界？
A: 融合本质是在“算子调度开销”和“中间张量读写”之间做权衡。若两个算子连续执行且中间结果只被下游消费，融合通常能减少显存往返和 kernel launch 次数；但融合后可能带来寄存器压力增大、并行度下降、数值路径变化。工程上应以 profile 数据决定：先看 memory-bound 还是 compute-bound，再决定融合收益是否为正。

### Q3: 动态 shape 为什么容易出问题？
A: 动态 shape 会同时影响三层系统：编译期 shape 推导、后端 tactic 选择、运行时内存规划。以 TensorRT 为例，动态维必须有 optimization profile（min/opt/max），否则 builder 无法确定可选 kernel 范围。再进一步，shape 传播若不精确，会导致后续 pass 无法匹配或生成非法索引映射，因此动态 shape 是编译正确性和性能稳定性的共同难点。

### Q4: 为何 pass 顺序这么重要？
A: pass 是“有前提条件的重写系统”。例如某个 pass 只匹配 `nn.*`，那它必须在 `onnx.* -> nn.*` 之后执行；另一个 pass 只对 `linalg.*` 生效，就必须在 NN->Linalg 之后执行。顺序错误会导致“可优化图形态”尚未出现，从而错失优化机会，甚至触发非法 IR。可把 pass pipeline 理解为状态机：每个阶段都把 IR 推向下一阶段的可接受域。

### Q5: 为什么 vectorize 后性能不一定提升？
A: 向量化提升的是指令级并行，但不自动保证全链路更快。性能最终由 roofline 约束：若瓶颈是内存带宽，vectorize 可能只增加 load/store 压力而无收益；若瓶颈是计算，才可能提升。此外向量宽度、数据对齐、尾处理、寄存器占用和 occupancy 都会影响结果。所以向量化是“候选优化”，需要结合硬件计数器和端到端测试验证。

### Q6: TensorRT 不支持某个算子怎么办？
A: 底层策略有三层：  
1) 语义等价重写：在中端把该算子拆成 TensorRT 支持的算子组合；  
2) 自定义 plugin：把算子实现成后端扩展层；  
3) 分段执行：不支持子图回退到其他运行时。  
优先级通常是 1 > 2 > 3，因为 1 维护成本低、部署复杂度低；2 性能可控但工程维护重；3 最灵活但跨运行时的数据搬运代价高。

### Q7: plugin 开发最大风险是什么？
A: 风险不在“写出 kernel”，而在“契约一致性”。plugin 需要在构建期和运行期维持一致的 shape/format/dtype 协议，还要保证序列化字节布局跨版本可读。如果协议漂移（比如字段增减、对齐变化），engine 反序列化会崩溃或 silent wrong result。高质量 plugin 通常必须有版本号、严格校验和反序列化兼容策略。

### Q8: 为什么要维护 Value <-> ITensor 双向映射？
A: 前向映射（Value->ITensor）是构图必需；反向映射（ITensor->Value）是调试与诊断必需。没有反向映射时，出现后端报错只能看到 TensorRT 层名，很难追溯到 MLIR 源位置和 pass 阶段。双向映射相当于保留了“源 IR 与后端图”的可追踪关系，是可观测性和可维护性的基础设施。

### Q9: 如何判断“转换正确”而不是“刚好能跑”？
A: 需要三类等价性：  
1) 语义等价：同输入下输出数值在误差容忍范围内；  
2) 结构等价：shape、dtype、layout 约束一致；  
3) 范围等价：在多组输入分布（边界值、动态 shape、随机值）上都成立。  
“能跑”只说明程序未崩溃，不说明语义正确。编译器验证应包含 golden reference、误差统计（max/mean/percentile）和随机回归。

### Q10: 性能评估为何要多 batch？
A: batch 改变了算子并行粒度和内存复用模式。小 batch 常受 launch/sync 开销影响，大 batch 常受带宽或 workspace 限制影响。只测单 batch 会得到偏差结论。多 batch 曲线可以帮助判断系统处于哪一类瓶颈区间，也能指导 profile（min/opt/max）设置是否合理。

### Q11: 为什么有时 PyTorch 比 TensorRT 快？
A: 常见原因有三类：  
1) 模型过小，TensorRT 构图/调度收益无法覆盖运行开销；  
2) 输入准备和 H2D/D2H 拷贝主导总时延；  
3) 后端未命中最优 tactic（shape/profile/精度设置不理想）。  
另外，如果 benchmark 包含同步点不一致（如一个路径隐式同步更多），也会造成“假慢/假快”。

### Q12: 工程上如何避免“改一个 pass 崩全链路”？
A: 做“分层可验证”设计：每层都有独立断言和回归。典型做法是：  
1) pass 级单测（输入 IR -> 期望 IR）；  
2) 后端映射单测（特定 call -> 特定 layer）；  
3) 端到端 smoke（小模型快速回归）。  
同时保持 pass 失败信息可定位（阶段名、源位置、操作名），把失败前移到最接近根因的层级。

### Q13: ONNX->MLIR 工具链为什么常成为瓶颈？
A: ONNX 生态版本变化快：opset 语义、shape inference 细节、可选属性默认值都可能变化。前端转换器若跟不上，会出现“模型能导出但不能稳定降级”。本质上这是“规范演进速度”与“编译器实现速度”的矛盾，因此需要版本矩阵管理（ONNX 版本、opset、转换器版本、后端版本）和兼容测试。

### Q14: 为什么需要保留 ONNX 直连 TRT 路径？
A: 这是工程上的“兜底执行面”。当中端 pipeline 覆盖不足或新 pass 处于迭代期时，直连路径能保证交付不中断。可以把它理解为双轨制：MLIR 路径用于优化探索和可解释性，TRT 直连用于生产稳定性。双轨并行能显著降低研发风险。

### Q15: AI 编译器项目最容易忽略的点是什么？
A: 最容易忽略的是“长期稳定性与可观测性”。很多项目早期只关注峰值性能，但缺少：错误分层、版本兼容、输入分布覆盖、性能回归阈值、日志与追踪链路。真正可用的编译器不是一次性跑通，而是在模型、版本、硬件变化时仍可诊断、可回退、可持续迭代。

---

## 10. 推荐阅读顺序（新同学上手）

1. `tools/mlir2trt/mlir2trt.cpp`（总控流程）
2. `lib/Passes/PipelineLoweringPasses.cpp`（IR 变换主逻辑）
3. `lib/TensorRT/NetworkConverter.cpp`（后端映射核心）
4. `include/MIC/Dialect/NNOps.td` + `lib/Dialect/NN/*`（语义层）
5. `scripts/utils/test_pipeline.py` 与 `scripts/benchmark/benchmark.py`（验证与性能）
6. `lib/CUDA/*`（插件与 kernel 示例）

---

## 10.1 面试/答辩模板版（30秒版 + 3分钟版）

### T1: 为什么要 ONNX -> NN/Linalg -> TensorRT，而不是 ONNX 直连？

`30秒版`  
ONNX 直连适合快速落地，但优化规则会和前后端强耦合。引入 NN/Linalg 中间层后，优化能在统一 IR 上复用，再分别降到 TensorRT/CUDA，工程可扩展性更好。

`3分钟版`  
可以把 ONNX 看成交换格式、TensorRT 看成执行格式。两者抽象层不同，直接连会把“模型语义”和“后端细节”写在一起，导致新增算子或换后端时代码重写。  
中间层（NN/Linalg）提供了可分析、可重写、可验证的语义空间：  
1) 前端把异构输入统一成内部语义；  
2) 中端做融合、布局、向量化等与后端弱耦合优化；  
3) 后端只做最后映射。  
这个分层让项目同时保留 ONNX 直连兜底路径和 MLIR 优化路径，形成“稳定交付 + 持续演进”的双轨体系。

### T2: 动态 shape 为什么难？你怎么解决？

`30秒版`  
动态 shape 同时影响编译期推导和后端 kernel 选择。TensorRT 必须有 optimization profile（min/opt/max），否则无法稳定构建和执行。

`3分钟版`  
难点是三件事要一致：  
1) IR 里的 shape 推导；  
2) 后端可用 tactic 集；  
3) 运行时输入范围。  
只要有一个不一致就会报错或性能异常。工程上做法是：  
先在 ONNX 路径自动补 profile；  
MLIR 路径明确边界契约并补 profile 配置；  
回归时覆盖边界 shape（最小、常用、最大）验证构建和数值稳定性。

### T3: pass 顺序如何设计？如何证明顺序合理？

`30秒版`  
pass 顺序本质是前提依赖：先把 IR 变成后续 pass 可匹配形态，再做对应优化。顺序错了会“匹配不到”或生成非法 IR。

`3分钟版`  
我会把 pipeline 当状态机设计：每个阶段把 IR 推向下一阶段可接受域。  
例如先 `onnx->nn`，再 `nn->linalg`，再做 linalg 优化，最后降到 `mic_trt_*` 调用。  
验证顺序合理性的方法是：  
1) 每阶段前后打印 IR 摘要（项目已有 `--verbose-pipeline`）；  
2) 为关键 pass 写输入/输出期望测试；  
3) 对失败阶段做定位（阶段名、op 名、源位置）。

### T4: TensorRT 不支持算子时你的决策路径是什么？

`30秒版`  
优先语义等价改写，其次 plugin，最后分段执行。优先级按维护成本和部署复杂度排序。

`3分钟版`  
三层策略：  
1) 图改写：在 MLIR 中把不支持算子拆成受支持组合，优先使用；  
2) plugin：当算子语义必须保留且性能关键时开发 plugin；  
3) 分段执行：最后兜底，但会有跨运行时搬运开销。  
决策标准看三个维度：性能、维护成本、上线风险。通常先做 1，必要时做 2，尽量避免长期依赖 3。

### T5: 你如何说明“转换是正确的”而不只是“能跑”？

`30秒版`  
至少验证语义等价、shape 等价、输入范围等价三层，不通过任何一层都不能算正确。

`3分钟版`  
正确性验证分三级：  
1) 数值：与参考实现对比误差（max/mean/p99）；  
2) 结构：shape、dtype、layout 一致；  
3) 范围：随机输入 + 边界输入 + 动态 shape 全覆盖。  
同时把验证放进自动化回归，防止后续 pass 改动引入 silent wrong result。

### T6: 为什么有时 TensorRT 反而比 PyTorch 慢？

`30秒版`  
小模型和小 batch 下，运行时开销可能大于优化收益；另外 shape/profile 不合适时也会错过最佳 tactic。

`3分钟版`  
慢的常见根因：  
1) 模型太小，kernel launch/sync 占主导；  
2) 数据拷贝成本掩盖算子执行收益；  
3) profile 设置不佳，后端没选到最优策略；  
4) benchmark 同步方式不一致造成测量偏差。  
我会通过多 batch 曲线、同步策略统一、剖析工具和 profile 调参来定位。

### T7: 为什么要维护 Value <-> ITensor 双向映射？

`30秒版`  
前向映射用于构图，反向映射用于定位问题。没有反向映射，后端报错很难追到源 IR。

`3分钟版`  
前向映射解决“怎么把 SSA 值接成 TRT 图”；反向映射解决“出错时是谁生成了这个 tensor”。  
在复杂 pipeline 里，调试效率高度依赖可追踪性。双向映射让你能从 TensorRT 层回溯到 MLIR op 和 pass 阶段，极大降低定位成本。

### T8: AI 编译器项目最容易失败在什么地方？

`30秒版`  
不是性能不够，而是稳定性和可观测性缺失：版本漂移、输入分布变化后无法诊断。

`3分钟版`  
常见失败点是“只追峰值，不建体系”：  
1) 缺版本矩阵和兼容策略；  
2) 缺分层测试和错误分层；  
3) 缺线上可观测（日志、指标、回退路径）。  
真正可用的编译器要在模型/框架/驱动变化时还能定位、回退、修复，而不是一次性 demo 跑通。

---

## 11. 一组建议的验证命令

### 11.1 编译工具

```bash
cmake --build build --target mlir2trt -j$(nproc)
```

### 11.2 跑覆盖模型（MLIR 路径）

```bash
build/tools/mlir2trt/mlir2trt --verbose-pipeline models/coverage_all_passes.mlir
```

### 11.3 ONNX 直连 TRT

```bash
build/tools/mlir2trt/mlir2trt --onnx-import=trt test_model.onnx
```

### 11.4 ONNX -> MLIR -> TRT

```bash
build/tools/mlir2trt/mlir2trt \
  --onnx-import=mlir \
  --onnx-mlir-converter=/mlir-tutorial/MIC-for-learning/scripts/utils/onnx_to_mlir.sh \
  --verbose-pipeline \
  test_model.onnx
```

### 11.5 ONNX 直连不稳定时的防抖

```bash
scripts/utils/run_mlir2trt_safe.sh --max-retries 8 -- --onnx-import=trt test_model.onnx
```

---

## 12. 下一阶段建议（从“能跑”到“更像编译器”）

1. 扩展 NN->Linalg 覆盖（Conv/Softmax/LayerNorm/Reshape/Transpose）。
2. 为动态 shape 的 MLIR 路径补完整 profile 构建。
3. 在 NetworkConverter 中建立更严格的 shape/type 校验与错误信息。
4. 引入 lit/单测，拆分 pass 的可验证边界。
5. 收敛 `mic_trt_*` ABI 文档，避免调用协议漂移。
6. 增加 CI：固定依赖版本 + smoke + benchmark 回归阈值。
