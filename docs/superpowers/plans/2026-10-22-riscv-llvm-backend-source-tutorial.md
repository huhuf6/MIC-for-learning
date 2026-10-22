# RISC-V LLVM 后端源码阅读教程实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 产出一份以三个案例追踪 LLVM RISC-V 后端源码的中文 Markdown 教程和经过逐页检查的 A4 PDF。

**Architecture:** Markdown 是内容的唯一事实来源，按“总览、整数加法、调用与栈帧、RVV、独立阅读方法”组织。临时 Python 构建脚本读取 Markdown 的受控子集，用 ReportLab 生成 PDF；源码证据检查和 PDF 视觉检查分别验证技术准确性与排版质量。

**Tech Stack:** LLVM `main` 源码提交 `043acd707`、Markdown、Python 3、ReportLab、Poppler (`pdftoppm`, `pdftotext`, `pdfinfo`)、Noto Sans CJK。

**Spec:** `docs/superpowers/specs/2026-10-22-riscv-llvm-backend-source-tutorial-design.md`

## Global Constraints

- 源码基准为 `/root/MIC-for-learning/llvm-project` 的 `main` 提交 `043acd707`。
- 以稳定的文件名、类名和函数名为主要定位方式；行号只能作为辅助。
- 主线采用 SelectionDAG，GlobalISel 只说明位置和当前边界。
- 教程不要求读者先完整构建当前 LLVM。
- Markdown 路径为 `rvv/RISC-V_LLVM后端源码阅读教程.md`。
- PDF 路径为 `output/pdf/RISC-V_LLVM后端源码阅读教程.pdf`，A4、中文字体正确、源码使用等宽字体。
- 只使用 ASCII `-`，不使用不可见的不换行连字符。
- 不提交 Git commit，除非用户另行要求。

## Review Focus

- **LLVM 版本漂移：** 每个核心符号必须能在提交 `043acd707` 中搜到；Task 1 的符号检查覆盖。
- **前端/中端/后端混淆：** 自动向量化与 `RISCVInsertVSETVLI` 必须明确分层；Task 2 的术语检查覆盖。
- **路径不可复现：** 所有引用的本地源码和测试路径必须存在；Task 1 的路径检查覆盖。
- **中文或代码排版损坏：** PDF 不得出现方框字、代码截断、表格越界；Task 4 的逐页渲染覆盖。
- **Markdown/PDF 内容漂移：** PDF 标题、五个主章节和关键符号必须可由 `pdftotext` 检出；Task 4 的文本检查覆盖。

---

### Task 1: 建立源码证据表与教程骨架

**Files:**
- Create: `tmp/pdfs/riscv-llvm-source-evidence.txt`
- Create: `rvv/RISC-V_LLVM后端源码阅读教程.md`

**Interfaces:**
- Consumes: 设计文档、原学习计划 PDF、`llvm-project` 提交 `043acd707`。
- Produces: 已核验的文件/符号清单，以及含完整章节结构的 Markdown 骨架。

- [ ] **Step 1: 记录源码版本与所有教程入口文件**

在证据表中记录 `git rev-parse --short HEAD`，并列出以下存在性检查：

`llvm/test/CodeGen/RISCV/alu64.ll`、`RISCVTargetMachine.cpp`、`RISCVCodeGenPassBuilder.cpp`、`RISCVSubtarget.cpp`、`RISCVISelLowering.cpp`、`RISCVISelDAGToDAG.cpp`、`RISCVInstrInfo.td`、`RISCVCallingConv.td`、`RISCVFrameLowering.cpp`、`RISCVRegisterInfo.cpp`、`RISCVAsmPrinter.cpp`、`MCTargetDesc/RISCVMCCodeEmitter.cpp`、`RISCVInstrInfoV.td`、`RISCVInstrInfoVPseudos.td`、`RISCVInsertVSETVLI.cpp`。

- [ ] **Step 2: 核验教程使用的关键符号**

使用 `rg -n` 确认 `LowerFormalArguments`、`LowerCall`、`LowerReturn`、`emitPrologue`、`emitEpilogue`、`eliminateFrameIndex`、`runOnMachineFunction`、`addInstSelector` 等符号实际位置，把输出追加到证据表。

- [ ] **Step 3: 创建 Markdown 完整骨架**

写入标题、适用读者、阅读方法、五个主章节、三个案例、练习与参考路径；章节标题固定为：

1. `先建立后端地图`
2. `案例一：整数加法如何变成 add`
3. `案例二：函数调用与栈帧`
4. `案例三：RVV 与 vsetvli`
5. `形成自己的源码阅读方法`

- [ ] **Step 4: 验证证据与骨架**

Run:

```bash
test "$(git -C llvm-project rev-parse --short HEAD)" = "043acd707"
test -s tmp/pdfs/riscv-llvm-source-evidence.txt
rg -n '^## (先建立后端地图|案例一：整数加法如何变成 add|案例二：函数调用与栈帧|案例三：RVV 与 vsetvli|形成自己的源码阅读方法)$' rvv/RISC-V_LLVM后端源码阅读教程.md
```

Expected: 三条命令退出码均为 0，最后一条输出 5 个章节。

### Task 2: 完成案例驱动的 Markdown 教程

**Files:**
- Modify: `rvv/RISC-V_LLVM后端源码阅读教程.md`

**Interfaces:**
- Consumes: Task 1 的证据表和章节骨架。
- Produces: 可独立阅读的完整教程，供 Task 3 直接转为 PDF。

- [ ] **Step 1: 编写后端总览**

用一张文本流程图说明 `LLVM IR -> SelectionDAG -> MachineInstr -> MCInst -> assembly/object`，解释 `TargetMachine`、`Subtarget`、pass pipeline、TableGen 和 MC 层，并说明 `-march`/`-mabi` 分别影响什么。

- [ ] **Step 2: 编写整数加法案例**

从 `alu64.ll` 的最小操作出发，依次阅读测试、合法化/Lowering、TableGen pattern、DAG instruction selection、AsmPrinter 和 MCCodeEmitter。加入“观察问题”和一个改变 IR/feature 的练习。

- [ ] **Step 3: 编写函数调用与栈帧案例**

解释 ABI 规则和实现的边界，覆盖 `RISCVCallingConv.td/.cpp`、`LowerFormalArguments/LowerCall/LowerReturn`、`emitPrologue/emitEpilogue`、`eliminateFrameIndex`，并加入叶子函数检查清单。

- [ ] **Step 4: 编写 RVV 案例**

先明确 Loop Vectorizer 属于中端；再覆盖 scalable vector、VP pseudo、AVL/SEW/LMUL、mask/tail policy 和 `RISCVInsertVSETVLI` 的三个阶段。加入配置状态传播图和一个从 RVV CodeGen 测试反查实现的练习。

- [ ] **Step 5: 编写收尾方法与术语表**

给出“从测试反查实现”的固定七步法、源码阅读记录模板、建议阅读顺序和核心术语表。

- [ ] **Step 6: 验证完整性和易混点**

Run:

```bash
rg -n 'SelectionDAG|MachineInstr|MCInst|LowerCall|emitPrologue|eliminateFrameIndex|RISCVInsertVSETVLI|Loop Vectorizer|AVL|SEW|LMUL' rvv/RISC-V_LLVM后端源码阅读教程.md
rg -n '向量化.*中端|vsetvli.*后端' rvv/RISC-V_LLVM后端源码阅读教程.md
```

Expected: 所有关键术语均有命中，并明确出现中端/后端分层说明。

### Task 3: 用 ReportLab 生成教程 PDF

**Files:**
- Create: `tmp/pdfs/build_riscv_llvm_backend_tutorial.py`
- Create: `output/pdf/RISC-V_LLVM后端源码阅读教程.pdf`

**Interfaces:**
- Consumes: Task 2 的 Markdown。
- Produces: A4 PDF；构建脚本支持标题、二三级标题、段落、列表、代码块、引用提示框、简单表格和分页。

- [ ] **Step 1: 实现受控 Markdown 到 ReportLab Flowable 的转换**

脚本入口固定为：

```python
def build_pdf(markdown_path: str, output_path: str) -> None:
```

注册 Noto Sans CJK 正文/粗体与 DejaVu Sans Mono 代码字体；定义 A4 页边距、页眉、页脚和页码。

- [ ] **Step 2: 为代码与表格增加分页保护**

长代码按行拆分，使用等宽字体和浅色背景；表格列宽限制在正文宽度内，单元格使用可换行 `Paragraph`。

- [ ] **Step 3: 生成 PDF**

Run:

```bash
python3 tmp/pdfs/build_riscv_llvm_backend_tutorial.py \
  rvv/RISC-V_LLVM后端源码阅读教程.md \
  output/pdf/RISC-V_LLVM后端源码阅读教程.pdf
```

Expected: 退出码 0，PDF 文件存在且大于 100 KB。

- [ ] **Step 4: 进行结构冒烟检查**

Run:

```bash
pdfinfo output/pdf/RISC-V_LLVM后端源码阅读教程.pdf
pdftotext output/pdf/RISC-V_LLVM后端源码阅读教程.pdf tmp/pdfs/riscv-backend-final.txt
rg -n '案例一|案例二|案例三|RISCVInsertVSETVLI' tmp/pdfs/riscv-backend-final.txt
```

Expected: PDF 为 A4、未加密，四个关键词全部出现。

### Task 4: 逐页视觉检查与最终验收

**Files:**
- Modify: `rvv/RISC-V_LLVM后端源码阅读教程.md`（仅在发现内容问题时）
- Modify: `tmp/pdfs/build_riscv_llvm_backend_tutorial.py`（仅在发现排版问题时）
- Regenerate: `output/pdf/RISC-V_LLVM后端源码阅读教程.pdf`
- Delete after validation: `tmp/pdfs/riscv-backend-page-*.png`、联系表和临时抽取文本；保留构建脚本与源码证据表便于复现。

**Interfaces:**
- Consumes: Task 3 的 PDF。
- Produces: 视觉无缺陷、文本可搜索、与 Markdown 内容一致的最终 PDF。

- [ ] **Step 1: 渲染全部页面**

Run:

```bash
pdftoppm -png -r 144 \
  output/pdf/RISC-V_LLVM后端源码阅读教程.pdf \
  tmp/pdfs/riscv-backend-page
```

Expected: 每页生成一张 PNG，无 Poppler 错误。

- [ ] **Step 2: 创建联系表并逐页查看**

检查封面、目录、章节换页、代码块、表格、流程图、中文字体、页眉和页码；任何裁切、重叠、黑方块或孤立标题都必须修复并重新生成/渲染。

- [ ] **Step 3: 验证 Markdown/PDF 关键内容一致**

Run:

```bash
pdftotext output/pdf/RISC-V_LLVM后端源码阅读教程.pdf tmp/pdfs/riscv-backend-final.txt
for term in '先建立后端地图' '整数加法如何变成 add' '函数调用与栈帧' 'RVV 与 vsetvli' '形成自己的源码阅读方法'; do
  grep -F "$term" tmp/pdfs/riscv-backend-final.txt
done
```

Expected: 五个主章节全部命中。

- [ ] **Step 4: 最终文件和源码状态检查**

Run:

```bash
test -s rvv/RISC-V_LLVM后端源码阅读教程.md
test -s output/pdf/RISC-V_LLVM后端源码阅读教程.pdf
git -C llvm-project status --porcelain=v1
```

Expected: 两个教程产物存在，LLVM 源码仓库保持干净。

- [ ] **Step 5: 清理一次性渲染文件**

删除 Task 4 生成的逐页 PNG、联系表和最终文本抽取文件；保留原学习计划渲染、证据表和 PDF 构建脚本。
