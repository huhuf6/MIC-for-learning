---
hide:
  - tags
tags:
  - Python
  - JAX
icon: simple/python
---

# JAX 集成

> 当前 JAX 支持仍在持续开发中。

IREE 与 JAX 主要有两条路径：

1. **AOT 路径**：抽取并编译完整模型，脱离 JAX 执行。
   相关项目：`iree-org/iree-jax`。
2. **PJRT 路径**：把 IREE 作为 JAX 原生后端，用于在线/JIT 场景。
   相关目录：`integrations/pjrt`。

