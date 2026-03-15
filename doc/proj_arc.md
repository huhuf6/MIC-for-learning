# MIC-for-learning 项目架构与 Pipeline

## 1. 项目目标（优化版）
MIC-for-learning 是一个面向 AI 推理部署的教学型编译器原型，核心目标是：

1. 打通从模型表示到可执行 TensorRT Engine 的端到端链路。  
2. 用 MLIR 方言/Pass 组织中间表示与优化流程，形成可扩展的编译框架。  
3. 提供可复现实验脚本，对比 PyTorch 与 TensorRT 推理性能。  
4. 在保持代码可读性的前提下，逐步从“原型可运行”演进到“结构化可扩展”。  

简言之：**当前先保证链路跑通，再逐步完善 IR 降低与优化质量。**

---

## 2. 总体分层
目标分层（设计视图）：

```text
ONNX
  ↓
NN Dialect (MIC 自定义方言)
  ↓
Linalg / 标准 MLIR 方言
  ↓
Backend (TensorRT / CUDA)
```

实现分层（当前代码视图）：

```text
Frontend:
  - ONNX 输入（当前已跑通）
  - MLIR/NN Dialect 输入（框架已具备）

Middle-end:
  - NN Dialect + Pass 框架（常量折叠/融合/布局变换骨架）

Backend:
  - TensorRT Builder 封装
  - ONNX Parser -> TensorRT Engine（当前主路径）
  - CUDA Kernel 示例与基准脚本
```

---

## 3. Pipeline（目标态 vs 当前态）

### 3.1 目标态 Pipeline（规划）
```text
ONNX
  -> LowerONNXToNN
  -> NN Dialect Canonicalize / Fusion
  -> Lower NN to Linalg/Tensor
  -> linalg-fuse / tile / vectorize
  -> Lower to backend (TensorRT/CUDA)
  -> Engine / Kernel
```

### 3.2 当前可运行 Pipeline（已实现）
```text
PyTorch Model
  -> Export ONNX
  -> mlir2trt (ONNX 分支，TensorRT ONNX Parser)
  -> TensorRT Engine (.engine)
  -> Benchmark(PyTorch vs TensorRT)
```

说明：
1. `mlir2trt` 现已支持直接 `.onnx` 输入并构建 engine。  
2. NN Dialect 与 Pass 体系可编译，但“ONNX->NN->Linalg->Backend”的完整降低链路仍在建设中。  

---

## 4. 主要模块与职责

### 4.1 Dialect 层（IR 定义）
路径：
`include/MIC/Dialect/*`, `lib/Dialect/NN/*`

职责：
1. 定义 NN 语义操作（如 `linear/conv2d/attention/gelu/layer_norm`）。  
2. 通过 TableGen 生成 Op 声明/定义与 Dialect 注册代码。  
3. 作为后续图优化与降级的统一 IR 承载层。  

### 4.2 Pass 层（中端优化框架）
路径：
`lib/Passes/*`

职责：
1. 提供常量折叠、融合、布局变换的 pass 框架。  
2. 对 NN 方言操作进行模式化遍历与改写。  
3. 为后续引入真实优化策略（pattern rewrite/cost model）预留入口。  

### 4.3 TensorRT Backend 层
路径：
`include/MIC/TensorRT/*`, `lib/TensorRT/*`

职责：
1. `TensorRTBuilder` 封装 TensorRT `IBuilder/INetworkDefinition/IBuilderConfig` 生命周期。  
2. `NetworkConverter` 作为 MLIR Op -> TRT Layer 的转换入口（当前仍是骨架实现）。  
3. 在 `mlir2trt` 中支持 ONNX 解析 + profile 配置 + engine 序列化。  

### 4.4 Tool 与脚本层
路径：
`tools/mlir2trt/*`, `scripts/*`, `run_all.sh`

职责：
1. `mlir2trt`: 转换入口工具。  
2. `test_pipeline.py`: 自动执行导出、转换、基准。  
3. `benchmark.py`: 多 batch 下 PyTorch/TensorRT 延迟吞吐对比。  
4. `run_all.sh`: 一键 `env + build + test`。  

---

## 5. 核心实现原理（简述）

### 5.1 为什么要引入 NN Dialect
1. ONNX 与后端 API 都较“具体”，直接耦合会让优化逻辑分散。  
2. NN Dialect 把模型语义抽象成统一 IR，便于做图级优化和跨后端复用。  
3. 通过 MLIR 的 TableGen/Pass infra，降低新增算子与优化的工程成本。  

### 5.2 为什么当前优先 ONNX->TensorRT 直连
1. 先建立可验证闭环（能生成 engine、能跑 benchmark）。  
2. 为后续 NN/Linalg 降低链路提供回归基线。  
3. 当中端未完全实现时，保证项目仍可交付“可运行结果”。  

### 5.3 动态 shape 处理原则
1. ONNX 输入如包含动态维度，构建 TensorRT engine 必须提供 optimization profile。  
2. 工具侧会为动态输入设置默认 `min/opt/max`，避免 build 阶段失败。  

---

## 6. 当前功能边界（务实说明）
已具备：
1. 项目可编译（MLIR 15 + TensorRT 10 适配后）。  
2. `mlir2trt` 支持 `.onnx` 输入并输出 `.engine`。  
3. 自动化 pipeline 与 benchmark 可运行。  

仍在完善：
1. ONNX -> NN Dialect 的系统化 Lowering。  
2. NN -> Linalg -> Backend 的完整 lowering pipeline。  
3. `NetworkConverter` 内部算子映射与鲁棒性（当前偏骨架）。  

---

## 7. 可复现执行方式

### 一键执行（推荐）
```bash
./run_all.sh
```

### 分步执行
```bash
# 1) 配置
cmake -S . -B build \
  -DCMAKE_CUDA_COMPILER=/usr/bin/nvcc \
  -DCUDA_TOOLKIT_ROOT_DIR=/usr \
  -DLLVM_DIR=/usr/lib/llvm-15/lib/cmake/llvm \
  -DMLIR_DIR=/usr/lib/llvm-15/lib/cmake/mlir \
  -DTENSORRT_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvinfer.so \
  -DTENSORRT_PLUGIN_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvinfer_plugin.so \
  -DTENSORRT_ONNX_PARSER_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvonnxparser.so \
  -DTENSORRT_PARSERS_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvonnxparser.so

# 2) 构建
cmake --build build --target mlir2trt -j$(nproc)

# 3) 测试
PATH="$(pwd)/build/tools/mlir2trt:$PATH" python3 scripts/utils/test_pipeline.py
```

---

## 8. 里程碑建议（下一阶段）
1. 完成 ONNX->NN Dialect lowering 与 round-trip 测试。  
2. 为 NN Pass 增加模式重写与单元测试（而非仅遍历骨架）。  
3. 在 `NetworkConverter` 落实关键算子（Linear/Conv/GELU/LayerNorm）映射。  
4. 增加 CI：固定环境 + 快速 smoke test + benchmark 回归阈值。  
