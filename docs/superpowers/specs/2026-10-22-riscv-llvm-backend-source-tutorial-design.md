# RISC-V LLVM 后端源码阅读教程设计

## 意图

基于 `rvv/RISC-V编译入门_一周学习计划.pdf` 中第 3-6 页的 LLVM 后端与 RVV 内容，整理一份面向已有 C/C++ 基础、刚开始接触编译器后端的中文教程。教程首先帮助读者建立“一个操作如何穿过 LLVM RISC-V 后端”的源码主线，再扩展到调用约定、栈帧和 RVV。

源码基准为本地 `llvm-project` 的 `main` 分支提交 `043acd707`。教程优先引用稳定的文件名、类名和函数名；行号只作为辅助，避免 LLVM 更新后失效。

## 目标

- 能画出 LLVM IR 到 RISC-V 汇编之间的主要后端阶段。
- 能从一个 LLVM IR 操作定位到 SelectionDAG lowering、TableGen pattern、MachineInstr 和最终汇编。
- 能定位 RISC-V ABI、参数传递、返回值和 callee-saved 寄存器的实现。
- 能读懂栈帧建立与恢复的主要入口。
- 能理解 RVV pseudo 指令与 `RISCVInsertVSETVLI` 的职责边界。
- 每章都有明确的阅读顺序、关键问题和小练习，读者可以在本地源码中复现定位过程。

## 非目标

- 不逐行解释 `llvm/lib/Target/RISCV` 的全部文件。
- 不从零讲解 C++、LLVM IR 或完整 RISC-V ISA。
- 不要求先完整编译当前 LLVM 源码；命令实验作为源码阅读的辅助。
- 不深入 GlobalISel 的全部实现。主线采用当前 RISC-V 后端更适合入门的 SelectionDAG 路径，并在架构图中说明 GlobalISel 的位置。
- 不做硬件性能调优或完整微架构调度模型分析。

## 已选方法

采用“案例追踪”方式，以小例子沿数据流进入源码；在第一次遇到组件时补充其架构职责。

考虑过的其他方法：

1. 架构总览优先：全局完整，但初学者容易只记住名词，难以建立源码与输出之间的联系。
2. 文件手册：查阅方便，但阅读顺序弱，容易陷入大量 `.td` 和辅助 Pass。
3. 案例追踪：以输出问题驱动源码定位，最符合学习计划第 4 天“从 CodeGen 测试入手”的安排，因此采用。

## 教程结构

### 0. 阅读地图与准备

- 解释前端、中端、后端边界，以及 LLVM IR、SelectionDAG、MachineInstr、MCInst、ELF 的关系。
- 给出 RISC-V 后端目录地图。
- 说明 `-march` 选择 ISA、`-mabi` 选择 ABI，并把它们分别映射到 Subtarget feature 与 calling convention。
- 给出源码搜索方法：从测试的 `RUN`/`CHECK`、符号搜索、TableGen include 链开始。

### 案例一：整数加法如何变成 `add`

- 从 `llvm/test/CodeGen/RISCV/alu64.ll` 选择最小 IR。
- 进入 `RISCVTargetMachine` 和 RISC-V CodeGen pass pipeline。
- 解释 `RISCVISelLowering` 的合法化职责，以及哪些简单操作可直接用通用 DAG pattern。
- 阅读 `RISCVInstrInfo.td`、基础指令格式文件和相关 pattern。
- 追踪到 `RISCVISelDAGToDAG`、MachineInstr、`RISCVAsmPrinter` 与 MC 层。
- 产出一张 `IR -> DAG -> MachineInstr -> MCInst -> assembly` 流程图。

### 案例二：函数调用与栈帧

- 从带 `noinline` 调用和局部变量的示例出发。
- 阅读 `RISCVCallingConv.td/.cpp`，区分 ABI 规则、参数分配和 callee-saved 集合。
- 定位 `RISCVISelLowering::LowerFormalArguments`、`LowerCall`、`LowerReturn`。
- 阅读 `RISCVFrameLowering::emitPrologue/emitEpilogue`。
- 关联 `RISCVRegisterInfo::eliminateFrameIndex`，解释抽象 frame index 如何变成 `sp/fp + offset`。
- 给出“为什么叶子函数可能没有栈帧”的检查清单。

### 案例三：RVV 数组加法与 `vsetvli`

- 从 scalable vector IR 或现有 RVV CodeGen 测试开始，先区分自动向量化和后端代码生成。
- 阅读 `RISCVInstrInfoV.td`、`RISCVInstrInfoVPseudos.td` 以及向量 pattern 文件。
- 解释 RVV pseudo 为什么携带 AVL、SEW、LMUL、mask/tail policy 信息。
- 阅读 `RISCVInsertVSETVLI` 的三阶段数据流：块入口状态、块出口状态、插入与局部优化。
- 说明 `vsetvli` 插入发生在 MachineInstr 层，不能与 Loop Vectorizer 混为一谈。

### 收尾：独立阅读新指令

- 给出从 CodeGen 测试反查实现的固定步骤。
- 用一个 Zbb 标量指令或 RVV 指令作为练习。
- 提供源码阅读记录模板：输入、目标特性、关键 IR、关键 pseudo、最终指令、涉及文件、仍未确认的问题。

## 教学表达

- 每章统一采用“先看现象 -> 提问题 -> 按顺序读源码 -> 回到输出验证 -> 小练习”。
- 代码只截取理解所需的短片段，同时给出本地相对路径和稳定符号名。
- 对容易混淆的层次使用明确提示框，例如“向量化发生在中端；`vsetvli` 插入发生在后端”。
- 图示包括一张总代码生成流水线、一张函数调用/栈帧职责图、一张 RVV 配置状态传播图。
- 术语首次出现时给出中文解释和英文原名，后续使用 LLVM 源码中的英文名。

## 产物

- 可编辑教程：`rvv/RISC-V_LLVM后端源码阅读教程.md`
- 排版版教程：`output/pdf/RISC-V_LLVM后端源码阅读教程.pdf`
- PDF 采用 A4，Noto Sans CJK 正文，等宽字体显示源码；包含目录、页码、页眉和可辨识的代码块。
- 中间渲染文件放在 `tmp/pdfs/`，最终确认后清理。

## 验证

- 对教程引用的每个文件和关键符号用当前源码实际搜索确认。
- Markdown 中所有本地路径均指向现有文件。
- 生成 PDF 后逐页渲染成 PNG，检查中文字体、代码溢出、表格、图示、页码和章节换页。
- 使用 `pdftotext` 抽取最终 PDF，确认标题、章节和代码文本可搜索且没有明显乱码。
- 对比原学习计划，确认 LLVM 后端、ABI、栈帧、RVV 与回归测试均被覆盖。

## 留给编写阶段的判断

- 具体选择哪个 RVV CodeGen 测试，以当前源码中最短且能体现 AVL/SEW/LMUL 的测试为准。
- 源码片段长度、图表分页和提示框数量由最终可读性决定。
- 若当前系统 LLVM 版本与源码主分支差异过大，命令只作为观察辅助，并明确标注版本差异。
