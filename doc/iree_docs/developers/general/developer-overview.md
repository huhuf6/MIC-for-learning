---
icon: octicons/book-16
---

# 开发者总览（Developer Overview）

本文介绍 IREE 的项目结构和常用开发工具，重点偏编译侧。

## 项目代码结构

- `/compiler/`：MLIR dialect、LLVM/MLIR pass、模块翻译等编译核心。
  - `bindings/`：编译器语言绑定（Python 等）。
- `/runtime/`：运行时核心（VM、HAL 驱动等）。
  - `bindings/`：运行时语言绑定。
- `/integrations/`：与外部框架的集成（如 TF）。
- `/tests/`：端到端测试。
- `/tools/`：开发工具（`iree-compile`、`iree-run-module` 等）。
- `/samples/`：示例。

## IREE 编译器目录重点

- `API/`：公开 C API。
- `Codegen/`：kernel/codegen 相关。
- `Dialect/`：IREE 自定义方言（Flow/HAL/Stream/VM 等）。
- `InputConversion/`：输入方言转换与预处理。

## 编译开发常用工具

### iree-opt

用于单独运行 pass 或 pass pipeline（类似 `mlir-opt`）。适合调试 pass 行为。

```bash
../iree-build/tools/iree-opt \
  --split-input-file \
  --mlir-print-ir-before-all \
  --iree-util-drop-compiler-hints \
  compiler/src/iree/compiler/Dialect/Util/Transforms/test/drop_compiler_hints.mlir
```

### iree-compile

主编译驱动：输入 MLIR，输出部署工件（通常 `.vmfb`）。

```bash
../iree-build/tools/iree-compile \
  --iree-hal-target-device=local \
  --iree-hal-local-target-device-backends=vmvx \
  samples/models/simple_abs.mlir \
  -o /tmp/simple_abs_vmvx.vmfb
```

### iree-run-module

加载并执行已编译模块，适合验证输入输出与快速回归。

```bash
../iree-build/tools/iree-run-module \
  --module=/tmp/simple_abs_vmvx.vmfb \
  --device=local-task \
  --function=abs \
  --input=f32=-2
```

### 其他工具

- `iree-check-module`：以测试方式执行模块。
- `iree-run-mlir`：直接对 `.mlir` 做“编译+执行”（调试用）。
- `iree-dump-module`：查看 `.vmfb` 内容。

## 编译调试建议

- 先用 `iree-opt` 缩小问题范围（定位某个 pass）。
- 再用 `iree-compile --mlir-timing` 看耗时热点。
- 必要时配合 `--compile-from/--compile-to` 分段调试 pipeline。

