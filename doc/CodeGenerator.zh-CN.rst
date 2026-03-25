==========================================
LLVM 目标无关的代码生成器
==========================================

.. role:: raw-html(raw)
   :format: html

.. raw:: html

  <style>
    .unknown { background-color: #C0C0C0; text-align: center; }
    .unknown:before { content: "?" }
    .no { background-color: #C11B17 }
    .no:before { content: "N" }
    .partial { background-color: #F88017 }
    .yes { background-color: #0F0; }
    .yes:before { content: "Y" }
    .na { background-color: #6666FF; }
    .na:before { content: "N/A" }
  </style>

.. contents::
   :local:

.. warning::
  This is a work in progress.

介绍
============

LLVM 与目标无关的代码生成器是一个提供套件的框架
用于将 LLVM 内部表示转换为
指定目标的机器代码——以汇编形式（适用于
静态编译器）或二进制机器代码格式（可用于 JIT
编译器）。 LLVM 与目标无关的代码生成器由六个主要部分组成
成分：

1. 捕获重要属性的“抽象目标描述”接口
   about various aspects of the machine, independently of how they will be used.
   These interfaces are defined in ``include/llvm/Target/``.

2. 用于表示目标“正在生成的代码”的类。  这些
   classes are intended to be abstract enough to represent the machine code for
   *any* target machine.  These classes are defined in
   ``include/llvm/CodeGen/``. At this level, concepts like "constant pool
   entries" and "jump tables" are explicitly exposed.

3. 用于表示目标文件级别代码的类和算法，
   `MC Layer`_.  These classes represent assembly level constructs like labels,
   sections, and instructions.  At this level, concepts like "constant pool
   entries" and "jump tables" don't exist.

4. “与目标无关的算法”用于实现本机的各个阶段
   code generation (register allocation, scheduling, stack frame representation,
   etc).  This code lives in ``lib/CodeGen/``.

5. `抽象目标描述接口的实现`_
   particular targets.  These machine descriptions make use of the components
   provided by LLVM, and can optionally provide custom target-specific passes,
   to build complete code generators for a specific target.  Target descriptions
   live in ``lib/Target/``.

6. 与目标无关的 JIT 组件。  LLVM JIT 完全是目标
   independent (it uses the ``TargetJITInfo`` structure to interface for
   target-specific issues.  The code for the target-independent JIT lives in
   ``lib/ExecutionEngine/JIT``.

根据您对代码生成器的哪一部分感兴趣，
其中的不同部分将对您有用。  无论如何，你应该
熟悉`目标描述`_和`机器代码表示`_
类。  如果您想为新目标添加后端，您将需要
为你的新目标实现目标描述类并理解
:doc:`LLVM 代码表示 <LangRef>`。  如果您有兴趣
实现一个新的“代码生成算法”，它应该只依赖于
目标描述和机器代码表示类，确保它是
便携的。

代码生成器中所需的组件
-----------------------------------------

LLVM 代码生成器的两个部分是
代码生成器和可用于构建的可重用组件集
特定于目标的后端。  两个最重要的接口 (:raw-html:`<tt>`
`TargetMachine`_ :raw-html:`</tt>` 和 :raw-html:`<tt>` `DataLayout`_
:raw-html:`</tt>`) 是唯一需要为
后端以适应 LLVM 系统，但如果需要，则必须定义其他后端
将使用可重用的代码生成器组件。

这个设计有两个重要的含义。  首先是LLVM可以支持
完全非传统的代码生成目标。  例如，C 后端
不需要寄存器分配、指令选择或任何其他
系统提供的标准组件。  因此，它只实现了这些
两个接口，并做自己的事情。请注意，C 后端已从
自 LLVM 3.1 发布以来的主干。像这样的代码生成器的另一个例子是
（纯粹假设）将 LLVM 转换为 GCC RTL 形式并使用的后端
GCC 为目标发出机器代码。

这种设计还意味着可以从根本上设计和实现
LLVM 系统中的不同代码生成器不使用任何
内置组件。  根本不建议这样做，但可能需要这样做
对于不适合 LLVM 机器的完全不同的目标
描述模型：例如FPGA。

.. _high-level design of the code generator:

代码生成器的高层设计
-------------------------------------------

LLVM 与目标无关的代码生成器旨在支持高效且
为基于标准寄存器的微处理器生成高质量代码。  代码
该模型的生成分为以下几个阶段：

1. `指令选择`_ --- 这个阶段确定了一种有效的方法
   express the input LLVM code in the target instruction set.  This stage
   produces the initial code for the program in the target instruction set, then
   makes use of virtual registers in SSA form and physical registers that
   represent any required register assignments due to target constraints or
   calling conventions.  This step turns the LLVM code into a DAG of target
   instructions.

2. `Scheduling and Formation`_ --- 这个阶段取目标的DAG
   instructions produced by the instruction selection phase, determines an
   ordering of the instructions, then emits the instructions as :raw-html:`<tt>`
   `MachineInstr`_\s :raw-html:`</tt>` with that ordering.  Note that we
   describe this in the `instruction selection section`_ because it operates on
   a `SelectionDAG`_.

3. `基于 SSA 的机器代码优化`_ --- 这个可选阶段由
   series of machine-code optimizations that operate on the SSA-form produced by
   the instruction selector.  Optimizations like modulo-scheduling or peephole
   optimization work here.

4. `寄存器分配`_ --- 目标代码由无限转化而来
   virtual register file in SSA form to the concrete register file used by the
   target.  This phase introduces spill code and eliminates all virtual register
   references from the program.

5. `Prolog/Epilog 代码插入`_ --- 生成机器代码后
   for the function and the amount of stack space required is known (used for
   LLVM alloca's and spill slots), the prolog and epilog code for the function
   can be inserted and "abstract stack location references" can be eliminated.
   This stage is responsible for implementing optimizations like frame-pointer
   elimination and stack packing.

6. `Late Machine Code Optimizations`_ --- 在“最终”上运行的优化
   machine code can go here, such as spill code scheduling and peephole
   optimizations.

7. `Code Emission`_ --- 最后阶段实际上会输出代码
   current function, either in the target assembler format or in machine
   code.

代码生成器基于这样的假设：指令选择器将
使用最佳模式匹配选择器来创建高质量的序列
本机指令。  基于模式的替代代码生成器设计
扩展和积极的迭代窥孔优化要慢得多。  这
设计允许高效编译（对于 JIT 环境很重要）并且
积极优化（离线生成代码时使用），允许
用于任何步骤的不同复杂程度的组件
汇编。

除了这些阶段之外，目标实现还可以插入任意
将特定目标传递到流程中。  例如，X86 目标使用
处理 80x87 浮点堆栈架构的特殊通道。  其他
可以根据需要通过自定义通道支持具有特殊要求的目标。

使用 TableGen 进行目标描述
-------------------------------------

目标描述类需要目标的详细描述
建筑学。  这些目标描述往往有大量的共同点
information (e.g., an ``add`` instruction is almost identical to a ``sub``
操作说明）。  为了允许最大程度的通用性
分解出来，LLVM 代码生成器使用
:doc:`TableGen/index` 工具来描述大块
目标机器，允许使用特定于域和特定于目标的
抽象以减少重复量。

随着 LLVM 的不断发展和完善，我们计划将越来越多的
目标描述为``.td`` 形式。  这样做给我们带来了一些
优点。  最重要的是它使 LLVM 的移植变得更容易，因为
它减少了必须编写的 C++ 代码量以及表面积
在有人能够获得之前需要理解的代码生成器
有效的东西。  其次，它使改变事情变得更容易。尤其，
如果表和其他东西都是由“tblgen”发出的，我们只需要改变
在一个地方（``tblgen``）将所有目标更新到新界面。

.. _Abstract target description:
.. _target description:

目标描述类
==========================

LLVM 目标描述类（位于“include/llvm/Target”
目录）提供独立于目标机器的抽象描述
任何特定的客户。  这些类旨在捕获*抽象*
properties of the target (such as the instructions and registers it has), and do
not incorporate any particular pieces of code generation algorithms.

所有目标描述类（除了 :raw-html:`<tt>` `DataLayout`_
:raw-html:`</tt>` 类）被设计为由具体目标进行子类化
实现，并实现虚拟方法。  为了达到这些
实现， :raw-html:`<tt>` `TargetMachine`_ :raw-html:`</tt>` 类
提供应由目标实现的访问器。

.. _TargetMachine:

“TargetMachine”类
---------------------------

“TargetMachine”类提供了用于访问的虚拟方法
各种目标描述类的特定于目标的实现
``get*Info`` 方法（``getInstrInfo``, ``getRegisterInfo``,
``getFrameInfo`` 等）。  该类旨在专门化具体的
目标实现（例如“X86TargetMachine”），它实现了各种
虚拟方法。  唯一需要的目标描述类是
:raw-html:`<tt>` `DataLayout`_ :raw-html:`</tt>` 类，但是如果代码
要使用生成器组件，应实现其他接口
以及。

.. _DataLayout:

“DataLayout”类
------------------------

“DataLayout”类是唯一需要的目标描述类，它
是唯一不可扩展的类（您不能从
它）。  ``DataLayout`` 指定有关目标如何布局内存的信息
对于结构，各种数据类型的对齐要求，大小
目标中的指针，以及目标是小尾数还是
大尾数。

.. _TargetLowering:

“TargetLowering”类
----------------------------

“TargetLowering” 类由基于 SelectionDAG 的指令选择器使用
主要是为了描述如何将 LLVM 代码降级为 SelectionDAG
运营。  除此之外，该类还表明：

* 用于各种“ValueType”的初始寄存器类，

* 目标机器本身支持哪些操作，

* “setcc”操作的返回类型，

* 用于转移金额的类型，以及

* 各种高级特征，例如转向是否有利可图
  division by a constant into a multiplication sequence.

.. _TargetRegisterInfo:

“TargetRegisterInfo” 类
--------------------------------

``TargetRegisterInfo`` 类用于描述寄存器文件
目标和寄存器之间的任何交互。

寄存器在代码生成器中由无符号整数表示。  身体的
寄存器（目标描述中实际存在的寄存器）是唯一的
small numbers, and virtual registers are generally large.  注意
寄存器“#0”被保留作为标志值。

Each register in the processor description has an associated
``TargetRegisterDesc`` 条目，为寄存器提供文本名称
（用于程序集输出和调试转储）和一组别名（用于
指示一个寄存器是否与另一个寄存器重叠）。

除了每个寄存器的描述之外，“TargetRegisterInfo”类
exposes a set of processor specific register classes (instances of the
``TargetRegisterClass`` 类）。  每个寄存器类包含寄存器组
具有相同属性（例如，它们都是 32 位整数
寄存器）。  指令选择器创建的每个 SSA 虚拟寄存器都有
an associated register class.  When the register allocator runs, it replaces
虚拟寄存器与物理寄存器在集合中。

这些类的特定于目标的实现是从
:doc:`TableGen/index` 寄存器文件的描述。

.. _TargetInstrInfo:

“TargetInstrInfo”类
-----------------------------

“TargetInstrInfo”类用于描述机器指令
得到目标的支持。  描述定义诸如助记符之类的东西
the opcode, the number of operands, the list of implicit register uses and defs,
该指令是否具有某些与目标无关的属性（访问
内存，可交换等），并保存任何特定于目标的标志。

“TargetFrameLowering”类
---------------------------------

“TargetFrameLowering”类用于提供有关堆栈的信息
目标的框架布局。它控制着堆栈增长的方向，即已知的
每个函数入口处的堆栈对齐以及本地区域的偏移量。
到局部区域的偏移量是函数上堆栈指针的偏移量
进入函数数据（局部变量、溢出）的第一个位置
位置）可以存储。

“TargetSubtarget” 类
-----------------------------

The ``TargetSubtarget`` class is used to provide information about the specific
成为目标的芯片组。  子目标通知代码生成
支持指令、指令延迟和指令执行
行程;即使用哪些处理单元、按什么顺序以及如何使用
长的。

“TargetJITInfo” 类
---------------------------

``TargetJITInfo`` 类公开了一个抽象接口，供
即时代码生成器，用于执行特定于目标的活动，例如
emitting stubs.  If a ``TargetMachine`` supports JIT code generation, it should
通过 getJITInfo 方法提供这些对象之一。

.. _code being generated:
.. _machine code representation:

机器码描述类
================================

在高层，LLVM 代码被转换为机器特定的表示
由 :raw-html:`<tt>` `MachineFunction`_ :raw-html:`</tt>` 组成，
:raw-html:`<tt>` `MachineBasicBlock`_ :raw-html:`</tt>` 和 :raw-html:`<tt>`
`MachineInstr`_ :raw-html:`</tt>` 实例（定义于
``include/llvm/CodeGen``）。  这种表示完全与目标无关，
以最抽象的形式表示指令：操作码和一系列
操作数。  此表示形式旨在支持 SSA 表示形式
用于机器代码，以及寄存器分配的非 SSA 形式。

.. _MachineInstr:

“MachineInstr”类
--------------------------

目标机器指令表示为“MachineInstr”的实例
班级。  这个类是表示机器的极其抽象的方式
指示。  特别是，它只跟踪操作码号和一组
操作数。

操作码编号是一个简单的无符号整数，仅对
具体后端。  目标的所有指令都应在
目标的``*InstrInfo.td`` 文件。操作码枚举值是自动生成的
从这个描述。  “MachineInstr”类没有任何信息
关于如何解释指令（即，指令的语义是什么）
指令是）；为此，您必须参考 :raw-html:`<tt>`
`TargetInstrInfo`_ :raw-html:`</tt>` 类。

机器指令的操作数可以有几种不同的类型：
寄存器引用、常量整数、基本块引用等。
此外，机器操作数应标记为定义或值的使用
（尽管只允许定义寄存器）。

By convention, the LLVM code generator orders instruction operands so that all
寄存器定义出现在寄存器使用之前，即使在以下架构上也是如此
通常以其他顺序打印。  例如，SPARC添加指令：
“``add %i1, %i2, %i3``” 添加“%i1”，“%i2”注册并存储
结果存入“%i3”寄存器。  在 LLVM 代码生成器中，操作数应该
be stored as "``%i3, %i1, %i2``": with the destination first.

将目标（定义）操作数保留在操作数列表的开头
有几个优点。  特别是，调试打印机将打印
像这样的指令：

.. code-block:: llvm

  %r3 = add %i1, %i2

Also if the first operand is a def, it is easier to `create instructions`_ whose
只有 def 是第一个操作数。

.. _create instructions:

使用“MachineInstrBuilder.h”函数
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

机器指令是使用“BuildMI”函数创建的，位于
“include/llvm/CodeGen/MachineInstrBuilder.h” 文件。  ``BuildMI``
函数可以轻松构建任意机器指令。  的使用
“BuildMI” 函数如下所示：

.. code-block:: c++

  // Create a 'DestReg = mov 42' (rendered in X86 assembly as 'mov DestReg, 42')
  // instruction and insert it at the end of the given MachineBasicBlock.
  const TargetInstrInfo &TII = ...
  MachineBasicBlock &MBB = ...
  DebugLoc DL;
  MachineInstr *MI = BuildMI(MBB, DL, TII.get(X86::MOV32ri), DestReg).addImm(42);

  // Create the same instr, but insert it before a specified iterator point.
  MachineBasicBlock::iterator MBBI = ...
  BuildMI(MBB, MBBI, DL, TII.get(X86::MOV32ri), DestReg).addImm(42);

  // Create a 'cmp Reg, 0' instruction, no destination reg.
  MI = BuildMI(MBB, DL, TII.get(X86::CMP32ri8)).addReg(Reg).addImm(42);

  // Create an 'sahf' instruction which takes no operands and stores nothing.
  MI = BuildMI(MBB, DL, TII.get(X86::SAHF));

  // Create a self looping branch instruction.
  BuildMI(MBB, DL, TII.get(X86::JNE)).addMBB(&MBB);

如果您需要添加定义操作数（可选目标除外）
注册），您必须明确地将其标记为：

.. code-block:: c++

  MI.addReg(Reg, RegState::Define);

固定（预先分配）寄存器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

代码生成器需要注意的一个重要问题是存在
固定寄存器。  特别是说明书中经常有这样的地方
寄存器分配器*必须*安排特定值的流
在特定的寄存器中。  由于指令的限制，可能会发生这种情况
设置（例如，X86 只能使用 ``EAX``/``EDX`` 进行 32 位除法
寄存器），或外部因素，如调用约定。  无论如何，
指令选择器应该发出将虚拟寄存器复制到或复制出的代码
需要时使用物理寄存器。

例如，考虑这个简单的 LLVM 示例：

.. code-block:: llvm

  define i32 @test(i32 %X, i32 %Y) {
    %Z = sdiv i32 %X, %Y
    ret i32 %Z
  }

X86 指令选择器可能会为“div”生成此机器代码，并且
``ret``：

.. code-block:: text

  ;; Start of div
  %EAX = mov %reg1024           ;; Copy X (in reg1024) into EAX
  %reg1027 = sar %reg1024, 31
  %EDX = mov %reg1027           ;; Sign extend X into EDX
  idiv %reg1025                 ;; Divide by Y (in reg1025)
  %reg1026 = mov %EAX           ;; Read the result (Z) out of EAX

  ;; Start of ret
  %EAX = mov %reg1026           ;; 32-bit return value goes in EAX
  ret

在代码生成结束时，寄存器分配器将合并
注册并删除由此产生的身份移动，产生以下结果
代码：

.. code-block:: text

  ;; X is in EAX, Y is in ECX
  mov %EAX, %EDX
  sar %EDX, 31
  idiv %ECX
  ret

这种方式极其通用（如果能处理X86架构的话，就可以了）
可以处理任何事情！）并允许有关目标的所有特定知识
在指令选择器中要隔离的指令流。  注意
为了良好的代码生成，物理寄存器应该具有较短的生命周期，并且
所有物理寄存器在进入和退出基本块时均被假定为死区
（在寄存器分配之前）。  因此，如果您需要一个跨基本值存在的值
块边界，它“必须”位于虚拟寄存器中。

被调用破坏的寄存器
^^^^^^^^^^^^^^^^^^^^^^^^

一些机器指令（例如调用）会破坏大量物理指令
寄存器。  不是为所有这些添加“<def,dead>”操作数，而是
可以使用“MO_RegisterMask”操作数来代替。  寄存器掩码
操作数保存保留寄存器的位掩码，其他一切都是
被认为是被指令破坏的。

SSA 形式的机器码
^^^^^^^^^^^^^^^^^^^^^^^^

“MachineInstr”最初以 SSA 形式选择，并以
SSA 形式直到寄存器分配发生。  在大多数情况下，这是
非常简单，因为 LLVM 已经采用 SSA 形式； LLVM PHI 节点变为
机器代码PHI节点，虚拟寄存器只允许有一个
定义。

寄存器分配后，机器代码不再是 SSA 形式，因为
are no virtual registers left in the code.

.. _MachineBasicBlock:

“MachineBasicBlock”类
-------------------------------

“MachineBasicBlock”类包含机器指令列表
（:raw-html:`<tt>` `MachineInstr`_ :raw-html:`</tt>` 实例）。  大致就是
对应于输入到指令选择器的LLVM代码，但是可以有
一对多映射（即一个 LLVM 基本块可以映射到多台机器
基本块）。 “MachineBasicBlock”类有一个““getBasicBlock”方法，
它返回它来自的 LLVM 基本块。

.. _MachineFunction:

“MachineFunction”类
-----------------------------

“MachineFunction”类包含机器基本块的列表
（:raw-html:`<tt>` `MachineBasicBlock`_ :raw-html:`</tt>` 实例）。  它
与指令选择器的 LLVM 函数输入一一对应。
除了基本块列表之外，“MachineFunction”还包含一个
“MachineConstantPool”、“MachineFrameInfo”、“MachineFunctionInfo”，以及
一个``MachineRegisterInfo``。  请参阅“include/llvm/CodeGen/MachineFunction.h”
更多信息。

``MachineInstr Bundles``
------------------------

LLVM 代码生成器可以将指令序列建模为 MachineInstr
捆绑。 MI 包可以对 VLIW 组/包进行建模，其中包含任意
并行指令的数量。它还可以用于对顺序列表进行建模
不合法的指令（可能具有数据依赖性）
分开（例如 ARM Thumb2 IT 块）。

从概念上讲，MI 捆绑包是一个 MI，其中嵌套了许多其他 MI：

::

  --------------
  |   Bundle   | ---------
  --------------          \
         |           ----------------
         |           |      MI      |
         |           ----------------
         |                   |
         |           ----------------
         |           |      MI      |
         |           ----------------
         |                   |
         |           ----------------
         |           |      MI      |
         |           ----------------
         |
  --------------
  |   Bundle   | --------
  --------------         \
         |           ----------------
         |           |      MI      |
         |           ----------------
         |                   |
         |           ----------------
         |           |      MI      |
         |           ----------------
         |                   |
         |                  ...
         |
  --------------
  |   Bundle   | --------
  --------------         \
         |
        ...

MI 捆绑包支持不会改变物理表示
MachineBasicBlock 和 MachineInstr。所有 MI（包括顶级和嵌套
）被存储为 MI 的顺序列表。 “捆绑”MI 标记为
“InsideBundle”标志。使用具有特殊 BUNDLE 操作码的顶级 MI
代表捆绑的开始。将 BUNDLE MI 与个人混合使用是合法的
不在捆绑包内也不代表捆绑包的 MI。

MachineInstr 通道应作为单个单元在 MI 包上运行。成员
已经教授了正确处理捆绑包和捆绑包内的 MI 的方法。
MachineBasicBlock 迭代器已被修改为跳过捆绑的 MI
强制执行捆绑为单个单元的概念。替代迭代器
instr_iterator 已添加到 MachineBasicBlock 以允许迭代
涵盖 MachineBasicBlock 中的所有 MI，包括嵌套的 MI
里面的捆绑。顶层 BUNDLE 指令必须具有正确的一组
寄存器 MachineOperand 代表累积输入和输出
捆绑的 MI。

VLIW 架构的 MachineInstr 打包/捆绑应该
通常作为寄存器分配超级通道的一部分来完成。更多的
具体来说，决定应该捆绑哪些 MI 的 pass
应该在代码生成器退出 SSA 表单后一起完成
（即在两个地址传递、PHI 消除和复制合并之后）。
此类捆绑包应最终确定（即添加 BUNDLE MI 以及输入和
虚拟寄存器后输出寄存器MachineOperands）
重写到物理寄存器中。这消除了添加的需要
虚拟寄存器操作数到 BUNDLE 指令，这将
有效地将虚拟寄存器定义和使用列表加倍。捆绑可能
使用虚拟寄存器并以SSA形式形成，但可能不是
适用于所有用例。

.. _MC Layer:

“MC”层
==============

MC层用于在原始机器代码上表示和处理代码
级别，缺乏“高级”信息，例如“常量池”，“跳转表”，
“全局变量”或类似的东西。  在这个级别，LLVM 处理事情
例如标签名称、机器指令和目标文件中的部分。  这
这一层的代码有很多重要的用途：
代码生成器使用它来编写 .s 或 .o 文件，并且它也被
llvm-mc 工具用于实现独立的机器代码汇编器和反汇编器。

本节描述一些重要的类。  还有一些
在这一层交互的重要子系统，它们将在后面描述
本手册。

.. _MCStreamer:

“MCStreamer” API
----------------------

MCStreamer 最好被视为汇编器 API。  它是一个抽象的API
以不同的方式“实现”（例如，输出 .s 文件、输出 ELF .o
文件等），但其 API 直接对应于您在 .s 文件中看到的内容。
MCStreamer 每个指令都有一个方法，例如 EmitLabel、EmitSymbolAttribute、
switchSection、emitValue（对于.byte、.word）等，直接对应
汇编级指令。  它还有一个 EmitInstruction 方法，用于
将 MCInst 输出到流媒体。

该 API 对于两个客户端来说最重要：llvm-mc 独立汇编器是
实际上是解析一行的解析器，然后调用 MCStreamer 上的方法。在
代码生成器，代码生成器的“Code Emission”阶段降低
更高级别的 LLVM IR 和 Machine* 构建到 MC 层，发射
通过 MCStreamer 发出指令。

MCStreamer的实现方面，主要有两个实现：
一种用于写出 .s 文件 (MCAsmStreamer)，一种用于写出 .o
文件（MCObjectStreamer）。  MCAsmStreamer 是一个简单的实现
打印出每个方法的指令（例如``EmitValue -> .byte``），但是
MCObjectStreamer 实现了完整的汇编程序。

对于目标特定指令，MCStreamer 有一个 MCTargetStreamer 实例。
每个需要它的目标都定义一个继承自它的类，并且有很多
就像 MCStreamer 本身一样：每个指令有一个方法和两个类
从它继承一个目标对象流送器和一个目标asm流送器。目标
asm Streamer 只是打印它（``emitFnStart -> .fnstart``），并且对象
Streamer 为其实现汇编逻辑。

要使 llvm 使用这些类，目标初始化必须调用
TargetRegistry::RegisterAsmStreamer 和 TargetRegistry::RegisterMCObjectStreamer
传递分配相应目标流并传递它的回调
createAsmStreamer 或适当的对象流构造函数。

``MCContext`` 类
-----------------------

MCContext 类是各种独特数据结构的所有者
MC层，包括符号、部分等。因此，这是您需要的类
交互以创建符号和部分。  这个类不能被子类化。

``MCSymbol`` 类
----------------------

MCSymbol 类表示程序集文件中的符号（也称为标签）。  那里
有两种有趣的符号：汇编程序临时符号和普通符号
符号。  汇编器临时符号由汇编器使用和处理
但在生成目标文件时被丢弃。  区别通常是
通过在标签上添加前缀来表示，例如“L”标签是
MachO 中的汇编程序临时标签。

MCSymbols 由 MCContext 创建并且在那里是唯一的。  这意味着 MCSymbols
可以比较指针等价性，以确定它们是否是相同的符号。
请注意，指针不等式并不能保证标签最终会出现在
虽然地址不同。  输出这样的东西是完全合法的
到 .s 文件：

::

  foo:
  bar:
    .byte 4

在这种情况下，foo 和 bar 符号将具有相同的地址。

``MCSection`` 类
-----------------------

``MCSection`` 类代表目标文件特定的部分。这是
由目标文件特定实现子类化（例如“MCSectionMachO”，
``MCSectionCOFF``, ``MCSectionELF``) and these are created and uniqued by
MCContext。  MCStreamer 有当前部分的概念，可以是
使用 SwitchToSection 方法进行更改（对应于“.section”
.s 文件中的指令）。

.. _MCInst:

``MCInst`` 类
--------------------

“MCInst”类是指令的独立于目标的表示。
这是一个简单的类（比 `MachineInstr`_ 更重要），包含一个
特定于目标的操作码和 MCOpands 向量。  反过来，MCOperand 是一个
三种情况的简单可判并：1）简单立即数，2）目标
寄存器 ID，3) 符号表达式（例如“``Lfoo-Lbar+42``”）作为 MCExpr。

MCInst 是用于表示 MC 上的机器指令的通用货币
层。  是指令编码器、指令打印机使用的类型，
以及汇编解析器和反汇编器生成的类型。

.. _ObjectFormats:

目标文件格式
------------------

MC层的对象编写器支持多种对象格式。由于
对象格式的目标特定方面每个目标仅支持一个子集
MC层支持的格式。大多数目标支持发射 ELF
对象。其他特定于供应商的对象通常仅在目标上受支持
该供应商支持的（即 MachO 仅在目标上受支持）
Darwin 支持，XCOFF 仅在支持 AIX 的目标上受支持）。
此外，一些目标有自己的对象格式（即 DirectX、SPIR-V
和 WebAssembly）。

下表简要介绍了 LLVM 中的目标文件支持：

  .. table:: Object File Formats

     ================== ========================================================
     Format              Supported Targets
     ================== ========================================================
     ``COFF``            AArch64, ARM, X86
     ``DXContainer``     DirectX
     ``ELF``             AArch64, AMDGPU, ARM, AVR, BPF, CSKY, Hexagon, Lanai, LoongArch, M86k, MSP430, MIPS, PowerPC, RISCV, SPARC, SystemZ, VE, X86
     ``GCOFF``           SystemZ
     ``MachO``           AArch64, ARM, X86
     ``SPIR-V``          SPIRV
     ``WASM``            WebAssembly
     ``XCOFF``           PowerPC
     ================== ========================================================

.. _Target-independent algorithms:
.. _code generation algorithm:

与目标无关的代码生成算法
=============================================

本节记录了“高级设计”中描述的阶段
代码生成器`_.  它解释了它们的工作原理以及背后的一些基本原理
他们的设计。

.. _Instruction Selection:
.. _instruction selection section:

指令选择
---------------------

指令选择是将 LLVM 代码翻译成
代码生成器生成特定于目标的机器指令。  有几个
文献中众所周知的方法可以做到这一点。  LLVM 使用基于 SelectionDAG
指令选择器。

DAG 指令选择器的部分是从目标生成的
描述（``*.td``）文件。  我们的目标是整个指令选择器
从这些“.td”文件生成，尽管目前仍然有
需要自定义 C++ 代码的东西。

`GlobalISel <https://llvm.org/docs/GlobalISel/index.html>`_ 是另一个
指令选择框架。

.. _SelectionDAG:

SelectionDAG 简介
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

SelectionDAG 提供了代码表示的抽象：
适合使用自动技术进行指令选择
（例如基于动态编程的最佳模式匹配选择器）。这也是
非常适合代码生成的其他阶段；特别是指令
调度（SelectionDAG 与选择后调度 DAG 非常接近）。
此外，SelectionDAG 提供了一个主机表示，其中大量
各种非常低级（但与目标无关）的“优化”可能是
执行；需要有关说明的大量信息的
得到目标的有效支持。

SelectionDAG 是一个有向无环图，其节点是
“SDNode”类。  “SDNode”的主要有效负载是它的操作代码
（操作码）指示节点执行什么操作以及操作数
手术。  各种操作节点类型在顶部描述
“include/llvm/CodeGen/ISDOpcodes.h” 文件。

尽管大多数操作定义单个值，但图中的每个节点都可以
定义多个值。  例如，组合的 div/rem 操作将定义
股息和余额。许多其他情况需要多个
价值观也是如此。  每个节点还有一定数量的操作数，这些操作数是
定义使用值的节点。  因为节点可能定义多个值，
边由“SDValue”类的实例表示，它是一个
``<SDNode, unsigned>`` 对，指示正在使用的节点和结果值，
分别。  “SDNode” 生成的每个值都有一个关联的“MVT”
（机器值类型）指示值的类型是什么。

SelectionDAG 包含两种不同类型的值： 代表数据的值
流和表示控制流依赖性的那些。  数据值很简单
具有整数或浮点值类型的边。  控制边是
表示为“MVT::Other”类型的“链”边。  这些边缘
提供具有副作用的节点之间的排序（例如加载、存储、
来电、回电等）。  所有有副作用的节点都应该获取令牌
链作为输入并产生一个新的作为输出。  按照惯例，代币链
输入始终是操作数#0，链结果始终是最后一个值
由操作产生。然而，在选择指令后，
机器节点的链位于指令的操作数之后，并且
后面可能是粘合节点。

SelectionDAG 已指定“条目”和“根”节点。  入口节点是
始终是操作码为“ISD::EntryToken”的标记节点。  根节点是
代币链中的最后一个副作用节点。例如，在单个基本
块函数它将是返回节点。

SelectionDAG 的一个重要概念是“合法”与“合法”的概念。
“非法”DAG。  目标的合法 DAG 是仅使用受支持的 DAG
操作和支持的类型。  例如，在 32 位 PowerPC 上，一个 DAG
i1、i8、i16 或 i64 类型的值是非法的，使用
SREM 或 UREM 操作。  “类型合法化”和“操作合法化”阶段
负责将非法 DAG 转变为合法 DAG。

.. _SelectionDAG-Process:

SelectionDAG指令选择过程
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

基于SelectionDAG的指令选择包括以下步骤：

#. `构建初始 DAG`_ --- 此阶段执行从
   input LLVM code to an illegal SelectionDAG.

#. `Optimize SelectionDAG`_ --- 该阶段对
   SelectionDAG to simplify it, and recognize meta instructions (like rotates
   and ``div``/``rem`` pairs) for targets that support these meta operations.
   This makes the resultant code more efficient and the `select instructions
   from DAG`_ phase (below) simpler.

#. `Legalize SelectionDAG Types`_ --- 此阶段转换 SelectionDAG 节点
   to eliminate any types that are unsupported on the target.

#. `Optimize SelectionDAG`_ --- 运行 SelectionDAG 优化器进行清理
   redundancies exposed by type legalization.

#. `Legalize SelectionDAG Ops`_ --- 此阶段将 SelectionDAG 节点转换为
   eliminate any operations that are unsupported on the target.

#. `Optimize SelectionDAG`_ --- 运行 SelectionDAG 优化器以消除
   inefficiencies introduced by operation legalization.

#. `Select instructions from DAG`_ --- 最后是目标指令选择器
   matches the DAG operations to target instructions.  This process translates
   the target-independent input DAG into another DAG of target instructions.

#. `SelectionDAG调度和形成`_ --- 最后阶段分配一个线性
   order to the instructions in the target-instruction DAG and emits them into
   the MachineFunction being compiled.  This step uses traditional prepass
   scheduling techniques.

所有这些步骤完成后，SelectionDAG 将被销毁，并且
运行其余的代码生成过程。

可视化这里发生的情况的一种好方法是利用一些
LLC 命令行选项。  以下选项会弹出一个窗口，显示
SelectionDAG 在特定时间（如果您只在控制台上打印错误）
使用此功能时，您可能需要配置您的
系统 <ProgrammersManual.html#viewing-graphs-while-debugging-code>`_ 添加对其的支持）。

* ``-view-dag-combine1-dags`` 显示构建后、构建之前的 DAG
  first optimization pass.

* ``-view-legalize-dags`` 显示合法化之前的 DAG。

* ``-view-dag-combine2-dags`` 显示第二次优化之前的 DAG
  pass.

* ``-view-isel-dags`` 显示选择阶段之前的 DAG。

* ``-view-sched-dags`` 显示调度前的 DAG。

“-view-sunit-dags” 显示调度程序的依赖关系图。  这张图
基于最终的SelectionDAG，节点必须一起调度
捆绑到单个调度单元节点中，并带有立即操作数和
与调度无关的其他节点被省略。

选项“-filter-view-dags”允许选择基本块的名称
您有兴趣可视化并过滤所有以前的内容
``view-*-dags`` 选项。

.. _Build initial DAG:

初步选择DAG构建
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

初始的 SelectionDAG 是 na\ :raw-html:`§`\ 非常窥视孔扩展自
由“SelectionDAGBuilder”类输入的 LLVM。  本次通行证的目的
是向 SelectionDAG 公开尽可能多的低级、特定于目标的详细信息
可能的。  这个过程主要是硬编码的（例如 LLVM ``add`` 变成
``SDNode add`` 而``getelementptr`` 则扩展为明显的
算术）。此过程需要特定于目标的挂钩来减少调用、返回、
可变参数等。对于这些功能，:raw-html:`<tt>` `TargetLowering`_
使用 :raw-html:`</tt>` 接口。

.. _legalize types:
.. _Legalize SelectionDAG Types:
.. _Legalize SelectionDAG Ops:

SelectionDAG 合法化类型阶段
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Legalize 阶段负责将 DAG 转换为仅使用以下类型：
目标本身就支持。

有两种主要方法可以将不支持的标量类型的值转换为
支持类型的值：将小类型转换为较大类型（“升级”），
并将大整数类型分解为较小的整数类型（“扩展”）。  为了
例如，目标可能要求将所有 f32 值提升为 f64，并且
所有 i1/i8/i16 值都提升为 i32。  相同的目标可能需要
所有 i64 值都扩展为 i32 值对。  这些更改可以插入
根据需要进行符号和零扩展，以确保最终代码具有相同的
行为作为输入。

有两种主要方法可以将不支持的向量类型的值转换为
支持类型的值：分割向量类型，必要时多次分割，
直到找到合法类型，并通过添加元素来扩展向量类型
最后将它们四舍五入为合法类型（“扩大”）。  如果向量被分割
一直到单元素部分，不支持向量类型
found, the elements are converted to scalars ("scalarizing").

目标实现告诉合法化者支持哪些类型（以及哪些类型）
通过调用“addRegisterClass”方法来注册它们使用的类）
它的“TargetLowering”构造函数。

.. _legalize operations:
.. _Legalizer:

SelectionDAG 合法化阶段
^^^^^^^^^^^^^^^^^^^^^^^^^^^

合法化阶段负责将 DAG 转换为仅使用操作
目标本身支持的。

目标通常有奇怪的限制，例如不支持每个操作
每个支持的数据类型（例如 X86 不支持字节条件移动和
PowerPC 不支持从 16 位内存位置进行符号扩展加载）。
Legalize 通过开放编码另一个操作序列来解决这个问题
通过将一种类型提升为更大的类型来模拟操作（“扩展”）
支持操作（“促销”），或者通过使用特定于目标的钩子来
实施合法化（“习俗”）。

目标实现告诉合法化者哪些操作不受支持
（以及采取上述三个操作中的哪一个）通过调用
其“TargetLowering”构造函数中的“setOperationAction”方法。

如果目标具有合法的向量类型，则有望产生高效的机器
使用这些类型的 shufflevector IR 指令的常见形式的代码。
这可能需要对 SelectionDAG 向量操作进行自定义合法化
是从 shufflevector IR 创建的。 shufflevector 的形式应该是
处理的内容包括：

* 向量选择——向量的每个元素选自
  corresponding elements of the 2 input vectors. This operation may also be
  known as a "blend" or "bitwise select" in target assembly. This type of shuffle
  maps directly to the ``shuffle_vector`` SelectionDAG node.

* 插入子向量---将向量放入更长的向量类型开始
  at index 0. This type of shuffle maps directly to the ``insert_subvector``
  SelectionDAG node with the ``index`` operand set to 0.

* 提取子向量 --- 从较长的向量类型开始提取向量
  at index 0. This type of shuffle maps directly to the ``extract_subvector``
  SelectionDAG node with the ``index`` operand set to 0.

* Splat --- 向量的所有元素都具有相同的标量元素。这
  operation may also be known as a "broadcast" or "duplicate" in target assembly.
  The shufflevector IR instruction may change the vector length, so this operation
  may map to multiple SelectionDAG nodes including ``shuffle_vector``,
  ``concat_vectors``, ``insert_subvector``, and ``extract_subvector``.

在合法化通行证存在之前，我们要求每个目标
`selector`_ 支持并处理每个运算符和类型，即使它们不是
原生支持。  合法化阶段的引入允许所有
标准化模式可以在目标之间共享，并且使得很容易
优化规范化代码，因为它仍然是 DAG 的形式。

.. _optimizations:
.. _Optimize SelectionDAG:
.. _selector:

SelectionDAG 优化阶段：DAG 组合器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

SelectionDAG 优化阶段运行多次以生成代码，
DAG 构建后立即以及每次合法化后一次。  第一个
该过程的运行允许清理初始代码（例如执行
依赖于知道运算符具有受限类型的优化
输入）。  该过程的后续运行会清理由
Legalize 通行证，这使得 Legalize 变得非常简单（它可以专注于制作
编写合法的代码，而不是专注于生成*好的*和合法的代码）。

所执行的一类重要的优化是优化插入的符号和
零扩展指令。  我们目前使用临时技术，但可以移动
未来将采取更严格的技术。  这里有一些关于
主题：

"`Widening integer arithmetic <http://www.eecs.harvard.edu/~nr/pubs/widen-abstract.html>`_" :raw-html:`<br>`
凯文·雷德温和诺曼·拉姆齐 :raw-html:`<br>`
编译器构造国际会议 (CC) 2004

"`有效的符号扩展消除 <http://portal.acm.org/itation.cfm?doid=512529.512552>`_" :raw-html:`<br>`
河人元弘、小松秀明和中谷敏夫 :raw-html:`<br>`
ACM SIGPLAN 2002 编程语言设计会议论文集
和实施。

.. _Select instructions from DAG:

SelectionDAG 选择阶段
^^^^^^^^^^^^^^^^^^^^^^^^^

选择阶段是指令的大部分特定于目标的代码
选择。  该阶段采用合法的SelectionDAG作为输入，模式匹配
目标支持的指令发送到此 DAG，并生成一个新的 DAG
目标代码。  例如，考虑以下 LLVM 片段：

.. code-block:: llvm

  %t1 = fadd float %W, %X
  %t2 = fmul float %t1, %Y
  %t3 = fadd float %t2, %Z

此 LLVM 代码对应于 SelectionDAG，基本上如下所示：

.. code-block:: text

  (fadd:f32 (fmul:f32 (fadd:f32 W, X), Y), Z)

如果目标支持浮点乘加 (FMA) 运算，则以下之一
加法可以与乘法合并。  以 PowerPC 为例，
指令选择器的输出可能类似于以下 DAG：

::

  (FMADDS (FADDS W, X), Y, Z)

“FMADDS”指令是一个三进制指令，它将其第一个指令相乘
两个操作数并将第三个操作数相加（作为单精度浮点数）。
“FADDS”指令是一个简单的二进制单精度加法指令。
为了执行此模式匹配，PowerPC 后端包括以下内容
指令定义：

.. code-block:: text
  :emphasize-lines: 4-5,9

  def FMADDS : AForm_1<59, 29,
                      (ops F4RC:$FRT, F4RC:$FRA, F4RC:$FRC, F4RC:$FRB),
                      "fmadds $FRT, $FRA, $FRC, $FRB",
                      [(set F4RC:$FRT, (fadd (fmul F4RC:$FRA, F4RC:$FRC),
                                             F4RC:$FRB))]>;
  def FADDS : AForm_2<59, 21,
                      (ops F4RC:$FRT, F4RC:$FRA, F4RC:$FRB),
                      "fadds $FRT, $FRA, $FRB",
                      [(set F4RC:$FRT, (fadd F4RC:$FRA, F4RC:$FRB))]>;

指令定义的突出显示部分表示模式
用于匹配指令。 DAG 运算符（如“fmul”/“fadd”）
在“include/llvm/Target/TargetSelectionDAG.td”文件中定义。
“``F4RC``”是输入值和结果值的寄存器类别。

TableGen DAG 指令选择器生成器读取指令模式
在“.td”文件中并自动构建部分模式匹配代码
为了你的目标。  它具有以下优点：

* 在编译器编译时，它会分析您的指令模式并告诉您
  if your patterns make sense or not.

* 它可以处理模式匹配操作数的任意约束。  在
  particular, it is straight-forward to say things like "match any immediate
  that is a 13-bit sign-extended value".  For examples, see the ``immSExt16``
  and related ``tblgen`` classes in the PowerPC backend.

* 它知道所定义模式的几个重要特性。  例如，
  it knows that addition is commutative, so it allows the ``FMADDS`` pattern
  above to match "``(fadd X, (fmul Y, Z))``" as well as "``(fadd (fmul X, Y),
  Z)``", without the target author having to specially handle this case.

* 它具有功能齐全的类型推断系统。  特别是，您应该
  rarely have to explicitly tell the system what type parts of your patterns
  are.  In the ``FMADDS`` case above, we didn't have to tell ``tblgen`` that all
  of the nodes in the pattern are of type 'f32'.  It was able to infer and
  propagate this knowledge from the fact that ``F4RC`` has type 'f32'.

* 目标可以定义自己的（并依赖于内置的）“模式片段”。
  Pattern fragments are chunks of reusable patterns that get inlined into your
  patterns during compiler-compile time.  For example, the integer "``(not
  x)``" operation is actually defined as a pattern fragment that expands as
  "``(xor x, -1)``", since the SelectionDAG does not have a native '``not``'
  operation.  Targets can define their own short-hand fragments as they see fit.
  See the definition of '``not``' and '``ineg``' for examples.

* 除了指令之外，目标还可以指定映射的任意模式
  to one or more instructions using the 'Pat' class.  For example, the PowerPC
  has no way to load an arbitrary integer immediate into a register in one
  instruction. To tell tblgen how to do this, it defines:

  ::

    // Arbitrary immediate support.  Implement in terms of LIS/ORI.
    def : Pat<(i32 imm:$imm),
              (ORI (LIS (HI16 imm:$imm)), (LO16 imm:$imm))>;

  If none of the single-instruction patterns for loading an immediate into a
  register match, this will be used.  This rule says "match an arbitrary i32
  immediate, turning it into an ``ORI`` ('or a 16-bit immediate') and an ``LIS``
  ('load 16-bit immediate, where the immediate is shifted to the left 16 bits')
  instruction".  To make this work, the ``LO16``/``HI16`` node transformations
  are used to manipulate the input immediate (in this case, take the high or low
  16-bits of the immediate).

* 当使用“Pat”类将模式映射到具有一个的指令时
  or more complex operands (like e.g. `X86 addressing mode`_), the pattern may
  either specify the operand as a whole using a ``ComplexPattern``, or else it
  may specify the components of the complex operand separately.  The latter is
  done e.g. for pre-increment instructions by the PowerPC back end:

  ::

    def STWU  : DForm_1<37, (outs ptr_rc:$ea_res), (ins GPRC:$rS, memri:$dst),
                    "stwu $rS, $dst", LdStStoreUpd, []>,
                    RegConstraint<"$dst.reg = $ea_res">, NoEncode<"$ea_res">;

    def : Pat<(pre_store GPRC:$rS, ptr_rc:$ptrreg, iaddroff:$ptroff),
              (STWU GPRC:$rS, iaddroff:$ptroff, ptr_rc:$ptrreg)>;

  Here, the pair of ``ptroff`` and ``ptrreg`` operands is matched onto the
  complex operand ``dst`` of class ``memri`` in the ``STWU`` instruction.

* 虽然系统确实实现了很多自动化，但它仍然允许您编写自定义 C++
  code to match special cases if there is something that is hard to
  express.

虽然它有很多优点，但该系统目前存在一些局限性，
主要是因为它是一项正在进行的工作，尚未完成：

* 总体而言，无法定义或匹配 SelectionDAG 节点，这些节点定义
  multiple values (e.g. ``SMUL_LOHI``, ``LOAD``, ``CALL``, etc).  This is the
  biggest reason that you currently still *have to* write custom C++ code
  for your instruction selector.

* 目前还没有很好的方法来支持匹配复杂的寻址模式。  在
  the future, we will extend pattern fragments to allow them to define multiple
  values (e.g. the four operands of the `X86 addressing mode`_, which are
  currently matched with custom C++ code).  In addition, we'll extend fragments
  so that a fragment can match multiple different patterns.

* 我们还不会自动推断像“isStore”/“isLoad”这样的标志。

* 我们不会自动生成支持的寄存器和操作集
  for the `Legalizer`_ yet.

* 我们还没有办法绑定定制的合法节点。

尽管存在这些限制，指令选择器生成器仍然非常有用
对于典型指令中的大多数二进制和逻辑运算很有用
套。  如果您遇到任何问题或不知道如何做某事，
请让克里斯知道！

.. _Scheduling and Formation:
.. _SelectionDAG Scheduling and Formation:

SelectionDAG调度和形成阶段
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

调度阶段从选择中获取目标指令的DAG
阶段并分配顺序。  调度程序可以根据以下条件选择顺序
机器的各种限制（即最小套准压力的订单或
尝试覆盖指令延迟）。  一旦订单建立，DAG
转换为 :raw-html:`<tt>` `MachineInstr`_\s :raw-html:`</tt>` 列表以及
SelectionDAG 被销毁。

请注意，该阶段在逻辑上与指令选择阶段是分开的，
但在代码中与它密切相关，因为它在 SelectionDAG 上运行。

SelectionDAG 的未来方向
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#.可选的一次功能选择。

#.从“.td”文件自动生成整个选择器。

.. _SSA-based Machine Code Optimizations:

基于SSA的机器代码优化
------------------------------------

待写

直播间隔
--------------

实时间隔是变量“实时”的范围（间隔）。  他们是
由某些“寄存器分配器”传递来确定是否有两个或多个虚拟
需要相同物理寄存器的寄存器位于同一点
程序（即它们冲突）。  当这种情况发生时，一个虚拟
寄存器必须*溢出*。

Live Variable Analysis
^^^^^^^^^^^^^^^^^^^^^^

确定变量的生存区间的第一步是计算
指令执行后立即失效的一组寄存器（即
指令计算值，但从未使用过）和寄存器组
由指令使用，但在指令之后不再使用
（即，他们被杀）。计算实时变量信息
每个*虚拟*寄存器和*寄存器可分配*物理寄存器
在函数中。  这是以非常有效的方式完成的，因为它使用 SSA
稀疏计算虚拟寄存器的生命周期信息（位于 SSA 中）
形式）并且只需跟踪块内的物理寄存器。  注册前
分配时，LLVM 可以假设物理寄存器仅存在于
单个基本块。  这使得它能够进行单一的本地分析来解决
每个基本块内的物理寄存器寿命。如果物理寄存器是
不可寄存器分配（例如，堆栈指针或条件代码），它不是
被跟踪。

物理寄存器可以存在于函数中或函数之外。生活的价值观是
通常是寄存器中的参数。活出值通常是返回值
寄存器。实时值被如此标记，并被赋予虚拟“定义”
实时间隔分析期间的指导。如果a的最后一个基本块
函数是一个“return”，那么它被标记为使用中的所有实时输出值
功能。

``PHI`` nodes need to be handled specially, because the calculation of the live
来自函数 CFG 深度优先遍历的变量信息
不能保证定义了“PHI”节点使用的虚拟寄存器
before it's used. When a ``PHI`` node is encountered, only the definition is
已处理，因为使用将在其他基本块中处理。

对于当前基本块的每个“PHI”节点，我们模拟一个分配
当前基本块的末尾并遍历后继基本块。如果一个
successor basic block has a ``PHI`` node and one of the ``PHI`` node's operands
来自当前基本块，则该变量被标记为*alive*
在当前基本块及其所有前身基本块内，直到
遇到带有定义指令的基本块。

实时区间分析
^^^^^^^^^^^^^^^^^^^^^^^

我们现在拥有可用于执行实时间隔分析的信息
自己建立实时间隔。  我们首先对基本块进行编号
和机器指令。  然后我们处理“居住”值。  这些都在
物理寄存器，因此假设物理寄存器在结束时被杀死
的基本块。  虚拟寄存器的生存间隔是针对某些计算的
机器指令“[1, N]”的排序。  生存间隔是一个间隔
``[i, j)``，其中``1 >= i >= j > N``，变量处于活动状态。

.. note::
  More to come...

.. _Register Allocation:
.. _register allocator:

Register Allocation
-------------------

*寄存器分配问题*在于映射程序
:raw-html:`<b><tt>` P\ :sub:`v`\ :raw-html:`</tt></b>`，可以使用无界
number of virtual registers, to a program :raw-html:`<b><tt>` P\ :sub:`p`\
:raw-html:`</tt></b>` 包含有限（可能很小）数量的物理
registers. Each target architecture has a different number of physical
寄存器。如果物理寄存器的数量不足以容纳所有
虚拟寄存器，其中一些必须映射到内存中。这些
虚拟被称为*溢出虚拟*。

寄存器在 LLVM 中的表示方式
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

在 LLVM 中，物理寄存器由整数表示，通常范围为
从 1 到 1023。查看如何为特定的编号定义该编号
体系结构，您可以阅读“GenRegisterNames.inc”文件来了解它
architecture. For instance, by inspecting
``lib/Target/X86/X86GenRegisterInfo.inc`` 我们看到 32 位寄存器
“EAX”由 43 表示，MMX 寄存器“MM0”映射到 65。

Some architectures contain registers that share the same physical location. A
值得注意的例子是 X86 平台。例如，在 X86 架构中，
寄存器“EAX”、“AX”和“AL”共享前八位。这些物理
寄存器在 LLVM 中被标记为*别名*。给定一个特定的架构，你
可以通过检查其“RegisterInfo.td”来检查哪些寄存器是别名的
文件。此外，类“MCRegAliasIterator”枚举了所有物理
寄存器的别名。

LLVM 中的物理寄存器按*寄存器类*分组。  中的元素
相同的寄存器类在功能上是等效的，并且可以互换
用过的。每个虚拟寄存器只能映射到一个物理寄存器
特定的班级。例如，在X86架构中，一些虚拟只能
be allocated to 8 bit registers.  A register class is described by
“TargetRegisterClass” 对象。  发现虚拟寄存器是否
与给定的物理兼容，可以使用以下代码：

.. code-block:: c++

  bool RegMapping_Fer::compatible_class(MachineFunction &mf,
                                        unsigned v_reg,
                                        unsigned p_reg) {
    assert(TargetRegisterInfo::isPhysicalRegister(p_reg) &&
           "Target register must be physical");
    const TargetRegisterClass *trc = mf.getRegInfo().getRegClass(v_reg);
    return trc->contains(p_reg);
  }

有时，主要出于调试目的，更改数量是有用的
目标架构中可用的物理寄存器。这必须完成
静态地，在``TargetRegisterInfo.td`` 文件内。只需“grep”即可
``RegisterClass``，最后一个参数是寄存器列表。只是
注释掉一些是避免它们被使用的一种简单方法。更有礼貌
方法是从*分配顺序*中显式排除某些寄存器。请参阅
``GR8`` 寄存器类的定义
``lib/Target/X86/X86RegisterInfo.td`` 作为一个例子。

虚拟寄存器也用整数表示。与物理相反
寄存器，不同的虚拟寄存器永远不会共享相同的编号。然而
物理寄存器在“TargetRegisterInfo.td”文件中静态定义
and cannot be created by the application developer, that is not the case with
虚拟寄存器。为了创建新的虚拟寄存器，请使用以下方法
“MachineRegisterInfo::createVirtualRegister()”。该方法将返回一个新的
虚拟寄存器。使用“IndexedMap<Foo, VirtReg2IndexFunctor>”来保存
每个虚拟寄存器的信息。如果需要枚举所有虚拟
寄存器，使用函数“TargetRegisterInfo::index2VirtReg()”来查找
虚拟寄存器编号：

.. code-block:: c++

    for (unsigned i = 0, e = MRI->getNumVirtRegs(); i != e; ++i) {
      unsigned VirtReg = TargetRegisterInfo::index2VirtReg(i);
      stuff(VirtReg);
    }

在寄存器分配之前，指令的操作数大多是虚拟的
寄存器，尽管也可以使用物理寄存器。为了检查是否
给定的机器操作数是一个寄存器，使用布尔函数
“MachineOperand::isRegister()”。要获取寄存器的整数代码，请使用
“MachineOperand::getReg()”。指令可以定义或使用寄存器。为了
例如，``ADD reg:1026 := reg:1025 reg:1024`` 定义寄存器 1024，并且
使用寄存器 1025 和 1026。给定一个寄存器操作数，该方法
``MachineOperand::isUse()`` 通知该寄存器是否正在被使用
操作说明。方法“MachineOperand::isDef()”通知该寄存器是否是
被定义。

我们将在寄存器之前调用 LLVM 位码中存在的物理寄存器
分配*预着色寄存器*。预着色寄存器用于许多领域
不同的情况，例如传递函数调用的参数，以及
存储特定指令的结果。预着色有两种类型
寄存器：*隐式*定义的寄存器和*显式*定义的寄存器
定义的。显式定义的寄存器是普通操作数，可以访问
与“MachineInstr::getOperand(int)::getReg()”。  为了检查哪个
寄存器由指令隐式定义，使用
``TargetInstrInfo::get(opcode)::ImplicitDefs``，其中``opcode`` 是操作码
的目标指令。显式和显式之间的一个重要区别
隐式物理寄存器的特点是后者是为每个寄存器静态定义的
指令，而前者可能会因程序而异
编译。例如，表示函数调用的指令将
始终隐式定义或使用同一组物理寄存器。要阅读
指令隐式使用的寄存器，使用
``TargetInstrInfo::get(opcode)::ImplicitUses``。预着色寄存器强加
对任何寄存器分配算法的限制。寄存器分配器必须
确保它们都没有被虚拟寄存器的值覆盖
趁还活着。

将虚拟寄存器映射到物理寄存器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

有两种方法可以将虚拟寄存器映射到物理寄存器（或内存）
插槽）。第一种方式，我们称之为“直接映射”，基于使用
类“TargetRegisterInfo”和“MachineOperand”的方法。这
第二种方式，我们称之为“间接映射”，依赖于“VirtRegMap”
类，以便插入加载和存储，发送和获取值
记忆。

直接映射为寄存器开发人员提供了更大的灵活性
分配器；然而，它更容易出错，并且需要更多的实施
工作。  基本上，程序员必须指定加载和存储的位置
指令应按顺序插入到正在编译的目标函数中
在内存中获取和存储值。将物理寄存器分配给虚拟寄存器
给定操作数中存在的寄存器，请使用``MachineOperand::setReg(p_reg)``。到
插入存储指令，使用``TargetInstrInfo::storeRegToStackSlot(...)``，
要插入加载指令，请使用``TargetInstrInfo::loadRegFromStackSlot``。

间接映射使应用程序开发人员免受复杂性的影响
插入加载和存储指令。为了将虚拟寄存器映射到
物理的，使用“VirtRegMap::assignVirt2Phys(vreg, preg)”。  为了映射
某个虚拟寄存器到内存，使用
“VirtRegMap::assignVirt2StackSlot(vreg)”。该方法将返回堆栈
``vreg`` 的值所在的槽。  如果需要映射另一个
虚拟寄存器到同一个堆栈槽，使用
“VirtRegMap::assignVirt2StackSlot(vreg, stack_location)”。重要的一点
使用间接映射时要考虑的是，即使虚拟寄存器
映射到内存后，还需要映射到物理寄存器。这
物理寄存器是虚拟寄存器应该所在的位置
在存储之前或重新加载之后发现。

如果采用间接策略，则在所有虚拟寄存器都被分配完之后
映射到物理寄存器或堆栈槽，需要使用溢出器
对象在代码中放置加载和存储指令。每个虚拟机都有
映射到堆栈槽后将被存储到内存中，并且将
使用前先加载。实施溢出者尝试回收
load/store instructions, avoiding unnecessary instructions. For an example of
如何调用溢出器，请参阅“RegAllocLinearScan::runOnMachineFunction”
``lib/CodeGen/RegAllocLinearScan.cpp``。

处理两个地址指令
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

除了极少数例外（例如函数调用），LLVM 机器代码
指令是三地址指令。也就是说，每条指令都是
预计最多定义一个寄存器，并且最多使用两个寄存器。
然而，一些体系结构使用两个地址指令。在这种情况下，
定义的寄存器也是使用的寄存器之一。例如，一条指令
例如``ADD %EAX, %EBX``，在X86中实际上相当于``%EAX = %EAX +
%EBX``。

为了生成正确的代码，LLVM 必须转换三个地址指令
将两个地址指令表示为真正的两个地址指令。 LLVM
为此特定目的提供了“TwoAddressInstructionPass” 通行证。它
必须在寄存器分配发生之前运行。其执行后，
生成的代码可能不再是 SSA 形式。例如，这种情况发生在
situations where an instruction such as ``%a = ADD %b %c`` is converted to two
instructions such as:

::

  %a = MOVE %b
  %a = ADD %a %c

请注意，在内部，第二条指令表示为“ADD”
%a[定义/使用] %c``。即，寄存器操作数 ``%a`` 被使用和定义
指令。

SSA解构阶段
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

寄存器分配期间发生的一个重要转换称为
*SSA 解构阶段*。 SSA 表格简化了许多分析
在程序的控制流程图上执行。然而，传统
指令集不实现 PHI 指令。因此，为了生成
可执行代码，编译器必须用其他指令替换 PHI 指令
保留它们的语义。

有多种方法可以安全地从 PHI 指令中删除
目标代码。最传统的PHI解构算法替代PHI
instructions with copy instructions. That is the strategy adopted by LLVM. The
SSA解构算法的实现是
``lib/CodeGen/PHIElimination.cpp``。为了调用此过程，标识符
``PHIEliminationID`` 必须按照寄存器代码的要求进行标记
分配器。

说明书折叠
^^^^^^^^^^^^^^^^^^^

*指令折叠*是在寄存器分配期间执行的优化
删除不必要的复制指令。例如，一个序列
instructions such as:

::

  %EBX = LOAD %mem_address
  %EAX = COPY %EBX

可以安全地用单个指令替换：

::

  %EAX = LOAD %mem_address

说明书可以用折叠
``TargetRegisterInfo::foldMemoryOperand(...)`` 方法。当
折叠说明；折叠指令可能与折叠指令有很大不同
原始指令。请参阅“LiveIntervals::addIntervalsForSpills”
``lib/CodeGen/LiveIntervalAnalysis.cpp`` 了解其使用示例。

内置寄存器分配器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

LLVM 基础设施为应用程序开发人员提供了三种不同的
寄存器分配器：

* *快速* --- 该寄存器分配器是调试版本的默认分配器。它
  allocates registers on a basic block level, attempting to keep values in
  registers and reusing registers as appropriate.

* *基本* --- 这是寄存器分配的增量方法。居住
  ranges are assigned to registers one at a time in an order that is driven by
  heuristics. Since code can be rewritten on-the-fly during allocation, this
  framework allows interesting allocators to be developed as extensions. It is
  not itself a production register allocator but is a potentially useful
  stand-alone mode for triaging bugs and as a performance baseline.

* *贪婪* --- *默认分配器*。这是一个高度调整的实现
  the *Basic* allocator that incorporates global live range splitting. This
  allocator works hard to minimize the cost of spill code.

* *PBQP* --- 基于分区布尔二次规划 (PBQP) 的寄存器
  allocator. This allocator works by constructing a PBQP problem representing
  the register allocation problem under consideration, solving this using a PBQP
  solver, and mapping the solution back to a register assignment.

可以使用以下命令选择“llc”中使用的寄存器分配器的类型
行选项``-regalloc=...``：

.. code-block:: bash

  $ llc -regalloc=linearscan file.bc -o ln.s
  $ llc -regalloc=fast file.bc -o fa.s
  $ llc -regalloc=pbqp file.bc -o pbqp.s

.. _Prolog/Epilog Code Insertion:

Prolog/Epilog 代码插入
----------------------------

.. note::

  To Be Written

紧凑的放松
--------------

引发异常需要从函数中“展开”。有关信息
如何展开给定函数传统上用 DWARF unwind 表示
（又名框架）信息。但该格式最初是为调试器开发的
回溯，每个帧描述条目 (FDE) 需要约 20-30 个字节
功能。还有从函数中的地址映射到
运行时相应的 FDE。另一种展开编码称为 *compact
unwind* 并且每个函数仅需要 4 个字节。

紧凑展开编码是一个 32 位值，编码为
特定于架构的方式。它指定要恢复的寄存器以及从哪些寄存器恢复
在哪里以及如何退出该功能。当链接器创建最终的
链接图像，它将创建一个 ``__TEXT,__unwind_info`` 部分。本节是
运行时访问任何给定的展开信息的一种小而快速的方法
功能。如果我们为该函数发出紧凑展开信息，则该紧凑展开
信息将在``__TEXT,__unwind_info``部分进行编码。如果我们发射 DWARF
展开信息，``__TEXT,__unwind_info`` 部分将包含
最终链接图像中“__TEXT,__eh_frame”部分中的 FDE。

对于 X86，紧凑展开编码有三种模式：

*带有帧指针的函数（``EBP`` 或``RBP``）*
  ``EBP/RBP``-based frame, where ``EBP/RBP`` is pushed onto the stack
  immediately after the return address, then ``ESP/RSP`` is moved to
  ``EBP/RBP``. Thus to unwind, ``ESP/RSP`` is restored with the current
  ``EBP/RBP`` value, then ``EBP/RBP`` is restored by popping the stack, and the
  return is done by popping the stack once more into the PC. All non-volatile
  registers that need to be restored must have been saved in a small range on
  the stack that starts ``EBP-4`` to ``EBP-1020`` (``RBP-8`` to
  ``RBP-1020``). The offset (divided by 4 in 32-bit mode and 8 in 64-bit mode)
  is encoded in bits 16-23 (mask: ``0x00FF0000``).  The registers saved are
  encoded in bits 0-14 (mask: ``0x00007FFF``) as five 3-bit entries from the
  following table:

    ==============  =============  ===============
    Compact Number  i386 Register  x86-64 Register
    ==============  =============  ===============
    1               ``EBX``        ``RBX``
    2               ``ECX``        ``R12``
    3               ``EDX``        ``R13``
    4               ``EDI``        ``R14``
    5               ``ESI``        ``R15``
    6               ``EBP``        ``RBP``
    ==============  =============  ===============

*具有小恒定堆栈大小的无帧（“EBP”或“RBP”不用作帧指针）*
  To return, a constant (encoded in the compact unwind encoding) is added to the
  ``ESP/RSP``.  Then the return is done by popping the stack into the PC. All
  non-volatile registers that need to be restored must have been saved on the
  stack immediately after the return address. The stack size (divided by 4 in
  32-bit mode and 8 in 64-bit mode) is encoded in bits 16-23 (mask:
  ``0x00FF0000``). There is a maximum stack size of 1024 bytes in 32-bit mode
  and 2048 in 64-bit mode. The number of registers saved is encoded in bits 9-12
  (mask: ``0x00001C00``). Bits 0-9 (mask: ``0x000003FF``) contain which
  registers were saved and their order. (See the
  ``encodeCompactUnwindRegistersWithoutFrame()`` function in
  ``lib/Target/X86FrameLowering.cpp`` for the encoding algorithm.)

*具有大的恒定堆栈大小的无框架（“EBP”或“RBP”不用作帧指针）*
  This case is like the "Frameless with a Small Constant Stack Size" case, but
  the stack size is too large to encode in the compact unwind encoding. Instead
  it requires that the function contains "``subl $nnnnnn, %esp``" in its
  prolog. The compact encoding contains the offset to the ``$nnnnnn`` value in
  the function in bits 9-12 (mask: ``0x00001C00``).

.. _Late Machine Code Optimizations:

后期机器代码优化
-------------------------------

.. note::

  To Be Written

.. _Code Emission:

代码发射
-------------

代码生成的代码发射步骤负责从
代码生成器抽象（如 `MachineFunction`_、`MachineInstr`_ 等）
MC 层使用的抽象（`MCInst`_、`MCStreamer`_ 等）。  这
是通过几个不同类的组合完成的：（命名错误）
与目标无关的 AsmPrinter 类、特定于目标的 AsmPrinter 子类
（例如 SparcAsmPrinter）和 TargetLoweringObjectFile 类。

由于 MC 层在目标文件的抽象级别工作，因此它不
有函数、全局变量等的概念。相反，它考虑的是
标签、指令和说明。  此时使用的一个关键类是
MCStreamer 类。  这是一个以不同方式实现的抽象 API
（例如，输出 .s 文件、输出 ELF .o 文件等），这实际上是
“汇编器API”。  MCStreamer 每个指令都有一个方法，例如 EmitLabel，
EmitSymbolAttribute、switchSection等，直接对应Assembly
级别指令。

如果您有兴趣为目标实现代码生成器，可以使用
为了实现你的目标，你必须实施三件重要的事情：

#. First, you need a subclass of AsmPrinter for your target.  This class
   implements the general lowering process converting MachineFunction's into MC
   label constructs.  The AsmPrinter base class provides a number of useful
   methods and routines, and also allows you to override the lowering process in
   some important ways.  You should get much of the lowering for free if you are
   implementing an ELF, COFF, or MachO target, because the
   TargetLoweringObjectFile class implements much of the common logic.

#.其次，您需要为您的目标实现一个指令打印机。  这
   instruction printer takes an `MCInst`_ and renders it to a raw_ostream as
   text.  Most of this is automatically generated from the .td file (when you
   specify something like "``add $dst, $src1, $src2``" in the instructions), but
   you need to implement routines to print operands.

#.第三，您需要实现将 `MachineInstr`_ 降低为 MCInst 的代码，
   usually implemented in "<target>MCInstLower.cpp".  This lowering process is
   often target specific, and is responsible for turning jump table entries,
   constant pool indices, global variable addresses, etc into MCLabels as
   appropriate.  This translation layer is also responsible for expanding pseudo
   ops used by the code generator into the actual machine instructions they
   correspond to. The MCInsts that are generated by this are fed into the
   instruction printer or the encoder.

最后，根据您的选择，您还可以实现 MCCodeEmitter 的子类
它将 MCInst 降低为机器代码字节和重定位。  这是
如果您想支持直接 .o 文件发射，或者想要
为您的目标实现一个汇编器。

发出函数堆栈大小信息
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A section containing metadata on function stack sizes will be emitted when
``TargetLoweringObjectFile::StackSizesSection`` 不为空，并且
设置“TargetOptions::EmitStackSizeSection”（-stack-size-section）。这
部分将包含函数符号值对的数组（指针大小）
和堆栈大小（无符号 LEB128）。堆栈大小值仅包括空间
在函数序言中分配。具有动态堆栈分配的函数是
不包括在内。

VLIW 打包器
---------------

In a Very Long Instruction Word (VLIW) architecture, the compiler is responsible
用于将指令映射到架构上可用的功能单元。到
为此，编译器创建称为“数据包”的指令组或
*捆绑*。 LLVM 中的 VLIW 打包器是一种独立于目标的机制
启用机器指令的打包。

从指令到功能单元的映射
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

VLIW 目标中的指令通常可以映射到多个功能
单位。在打包的过程中，编译器必须能够推理
关于是否可以将指令添加到数据包中。这个决定可以是
复杂，因为编译器必须检查所有可能的指令映射
到职能单位。因此，为了减轻编译时间的复杂性，
VLIW打包器解析目标的指令类并生成表
在编译器构建时。然后可以通过提供的查询这些表
独立于机器的 API 来确定指令是否可以容纳在
包。

打包表如何生成和使用
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

分包器从目标的行程中读取指令类并创建
表示数据包状态的确定性有限自动机 (DFA)。 DFA
由三个主要元素组成：输入、状态和转换。的集合
生成的 DFA 的输入表示添加到
包。这些状态代表功能单元的可能消耗
数据包中的说明。在 DFA 中，从一种状态转换到另一种状态
发生在向现有数据包添加指令时。如果有一个
legal mapping of functional units to instructions, then the DFA contains a
相应的过渡。缺乏过渡表明法律
映射不存在，并且该指令无法添加到数据包中。

要为 VLIW 目标生成表，请添加 *Target*\ GenDFAPacketizer.inc 作为
target 到目标目录中的 Makefile。导出的API提供了三个
函数：``DFAPacketizer::clearResources()``,
“DFAPacketizer::reserveResources(MachineInstr *MI)”，以及
“DFAPacketizer::canReserveResources(MachineInstr *MI)”。这些功能允许
目标分包器，用于向现有数据包添加指令并检查
是否可以将指令添加到数据包中。看
``llvm/CodeGen/DFAPacketizer.h`` 了解更多信息。

实现本机汇编器
===============================

Though you're probably reading this because you want to write or maintain a
编译器后端，LLVM 还完全支持构建本机汇编器。
我们努力从 .td 文件自动生成汇编程序
（特别是指令语法和编码），这意味着大量
部分手动和重复的数据输入可以分解并与
编译器。

指令解析
-------------------

.. note::

  To Be Written


指令别名处理
----------------------------

指令解析完成后，就进入MatchInstructionImpl函数。
MatchInstructionImpl 函数执行别名处理，然后执行实际操作
匹配。

Alias processing is the phase that canonicalizes different lexical forms of the
相同的指令归结为一种表示。  有几种不同的种类
可以实现的别名，它们按顺序在下面列出
它们已被处理（按从最简单/最弱到最强大的顺序排列）
复杂/强大）。  一般来说，您想使用第一个别名机制
满足您的教学需求，因为它可以让您的教学更加简洁
描述。

助记符别名
^^^^^^^^^^^^^^^^

别名处理的第一阶段是简单的指令助记符重新映射
允许使用两种不同助记符的指令类别。  这
阶段是从一个输入助记符到一个输入助记符的简单且无条件的重新映射
输出助记词。  这种形式的别名不可能查看
操作数，因此重新映射必须适用于给定助记符的所有形式。
Mnemonic aliases are defined simply, for example X86 has:

::

  def : MnemonicAlias<"cbw",     "cbtw">;
  def : MnemonicAlias<"smovq",   "movsq">;
  def : MnemonicAlias<"fldcww",  "fldcw">;
  def : MnemonicAlias<"fucompi", "fucomip">;
  def : MnemonicAlias<"ud2a",    "ud2">;

...以及许多其他人。  通过 MnemonicAlias 定义，助记符被重新映射
简单直接。  尽管 MnemonicAlias 无法查看 MnemonicAlias 的任何方面
指令（例如操作数）它们可以依赖于全局模式（相同
匹配器支持的），通过 Requires 子句：

::

  def : MnemonicAlias<"pushf", "pushfq">, Requires<[In64BitMode]>;
  def : MnemonicAlias<"pushf", "pushfl">, Requires<[In32BitMode]>;

在此示例中，助记符被映射到不同的助记符，具体取决于
当前指令集。

指令别名
^^^^^^^^^^^^^^^^^^^

别名处理的最一般阶段发生在匹配发生时：
它为匹配器提供了新的形式来匹配特定的指令
来生成。  指令别名由两部分组成：要匹配的字符串和
指令来生成。  例如：

::

  def : InstAlias<"movsx $src, $dst", (MOVSX16rr8W GR16:$dst, GR8  :$src)>;
  def : InstAlias<"movsx $src, $dst", (MOVSX16rm8W GR16:$dst, i8mem:$src)>;
  def : InstAlias<"movsx $src, $dst", (MOVSX32rr8  GR32:$dst, GR8  :$src)>;
  def : InstAlias<"movsx $src, $dst", (MOVSX32rr16 GR32:$dst, GR16 :$src)>;
  def : InstAlias<"movsx $src, $dst", (MOVSX64rr8  GR64:$dst, GR8  :$src)>;
  def : InstAlias<"movsx $src, $dst", (MOVSX64rr16 GR64:$dst, GR16 :$src)>;
  def : InstAlias<"movsx $src, $dst", (MOVSX64rr32 GR64:$dst, GR32 :$src)>;

This shows a powerful example of the instruction aliases, matching the same
根据存在的操作数以多种不同的方式助记
大会。  指令别名的结果可以包括操作数
与目标指令的顺序不同，并且可以使用输入倍数
次，例如：

::

  def : InstAlias<"clrb $reg", (XOR8rr  GR8 :$reg, GR8 :$reg)>;
  def : InstAlias<"clrw $reg", (XOR16rr GR16:$reg, GR16:$reg)>;
  def : InstAlias<"clrl $reg", (XOR32rr GR32:$reg, GR32:$reg)>;
  def : InstAlias<"clrq $reg", (XOR64rr GR64:$reg, GR64:$reg)>;

此示例还表明绑定操作数仅列出一次。  在X86中
在后端，XOR8rr 有两个输入 GR8 和一个输出 GR8（其中一个输入与
到输出）。  InstAliases 采用没有重复项的扁平操作数列表
for tied operands.  The result of an instruction alias can also use immediates
和固定的物理寄存器，它们作为简单的立即操作数添加到
结果，例如：

::

  // Fixed Immediate operand.
  def : InstAlias<"aad", (AAD8i8 10)>;

  // Fixed register operand.
  def : InstAlias<"fcomi", (COM_FIr ST1)>;

  // Simple alias.
  def : InstAlias<"fcomi $reg", (COM_FIr RST:$reg)>;

指令别名还可以有一个 Requires 子句来使它们成为子目标
specific.

如果后端支持，指令打印机可以自动发出
别名而不是被别名的内容。它通常会带来更好、更多
可读的代码。如果最好打印出别名的内容，则传递“0”
作为 InstAlias 定义的第三个参数。

指令匹配
--------------------

.. note::

  To Be Written

.. _Implementations of the abstract target description interfaces:
.. _implement the target description:

针对特定目标的实施说明
====================================

本文档的这一部分解释了以下功能或设计决策
特定于特定目标的代码生成器。

.. _tail call section:

尾部调用优化
----------------------

尾部调用优化，被调用者重用调用者的堆栈，目前正在
在 x86/x86-64、PowerPC、AArch64 和 WebAssembly 上受支持。它执行于
x86/x86-64、PowerPC 和 AArch64，如果：

* 调用者和被调用者具有调用约定``fastcc``，``cc 10``（GHC
  calling convention), ``cc 11`` (HiPE calling convention), ``tailcc``, or
  ``swifttailcc``.

* 该调用是尾部调用 - 位于尾部位置（ret 紧跟在调用之后，并且
  ret uses value of call or is void).

* 选项“-tailcallopt”已启用或调用约定为“tailcc”。

* 满足特定于平台的约束。

x86/x86-64 限制：

* 不使用变量参数列表。

* 在 x86-64 上，生成 GOT/PIC 代码时仅模块本地调用（可见性 =
  hidden or protected) are supported.

PowerPC 限制：

* 不使用变量参数列表。

* 不使用 byval 参数。

* 在 ppc32/64 GOT/PIC 上，仅限模块本地调用（可见性 = 隐藏或受保护）
  are supported.

WebAssembly 约束：

* 不使用变量参数列表

* 'tail-call' 目标属性已启用。

* 调用者和被调用者的返回类型必须匹配。呼叫者不能
  be void unless the callee is, too.

AArch64 约束：

* 不使用变量参数列表。

例子：

调用“llc -tailcallopt test.ll”。

.. code-block:: llvm

  declare fastcc i32 @tailcallee(i32 inreg %a1, i32 inreg %a2, i32 %a3, i32 %a4)

  define fastcc i32 @tailcaller(i32 %in1, i32 %in2) {
    %l1 = add i32 %in1, %in2
    %tmp = tail call fastcc i32 @tailcallee(i32 inreg %in1, i32 inreg %in2, i32 %in1, i32 %l1)
    ret i32 %tmp
  }

“-tailcallopt”的含义：

在被调用者有更多的情况下支持尾部调用优化
参数比调用者使用“被调用者弹出参数”约定。这
currently causes each ``fastcc`` call that is not tail call optimized (because
不满足上述一项或多项限制）则需重新调整
堆栈的。因此在这种情况下性能可能会更差。

兄弟通话优化
-------------------------

兄弟调用优化是尾调用优化的一种受限形式。
与上一节中描述的尾调用优化不同，它可以是
当没有“-tailcallopt”选项时，在任何尾部调用上自动执行
指定的。

当前在 x86/x86-64 上执行同级调用优化
满足以下约束：

* 调用者和被调用者具有相同的调用约定。它可以是“c”或
  ``fastcc``.

* 该调用是尾部调用 - 位于尾部位置（ret 紧跟在调用之后，并且
  ret uses value of call or is void).

* 调用者和被调用者具有匹配的返回类型，或者不使用被调用者结果。

* 如果任何被调用者参数在堆栈中传递，则它们必须是
  available in caller's own incoming argument stack and the frame offsets must
  be the same.

例子：

.. code-block:: llvm

  declare i32 @bar(i32, i32)

  define i32 @foo(i32 %a, i32 %b, i32 %c) {
  entry:
    %0 = tail call i32 @bar(i32 %a, i32 %b)
    ret i32 %0
  }

X86 后端
---------------

X86 代码生成器位于“lib/Target/X86”目录中。  这段代码
生成器能够针对各种 x86-32 和 x86-64 处理器，并且
包括对 ISA 扩展的支持，例如 MMX 和 SSE。

支持 X86 目标三元组
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

以下是 X86 支持的已知目标三元组
后端。  这不是一个详尽的列表，添加这些内容会很有用
人们测试的。

* **i686-pc-linux-gnu** --- Linux

* **i386-未知-freebsd5.3** --- FreeBSD 5.3

* **i686-pc-cygwin** --- Win32 上的 Cygwin

* **i686-pc-mingw32** --- Win32 上的 MingW

* **i386-pc-mingw32msvc** --- Linux 上的 MingW 交叉编译器

* **i686-apple-darwin*** --- X86 上的 Apple Darwin

* **x86_64-unknown-linux-gnu** --- Linux

支持 X86 调用约定
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

后端已知以下特定于目标的调用约定：

* **x86_StdCall** --- stdcall calling convention seen on Microsoft Windows
  platform (CC ID = 64).

* **x86_FastCall** --- Microsoft Windows 上的 fastcall 调用约定
  platform (CC ID = 65).

* **x86_ThisCall** --- 与 X86_StdCall 类似。在 ECX 中传递第一个参数，
  others via stack. Callee is responsible for stack cleaning. This convention is
  used by MSVC by default for methods in its ABI (CC ID = 70).

.. _X86 addressing mode:

在 MachineInstrs 中表示 X86 寻址模式
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

x86 具有非常灵活的内存访问方式。  它能够形成
直接在整数指令中的以下表达式的内存地址
（使用 ModR/M 寻址）：

::

  SegmentReg: Base + [1,2,4,8] * IndexReg + Disp32

为了表示这一点，LLVM 为每个内存跟踪不少于 5 个操作数
这种形式的操作数。  这意味着 '``mov``' 的“加载”形式具有
按照以下顺序执行“MachineOperand”：

::

  Index:        0     |    1        2       3           4          5
  Meaning:   DestReg, | BaseReg,  Scale, IndexReg, Displacement Segment
  OperandTy: VirtReg, | VirtReg, UnsImm, VirtReg,   SignExtImm  PhysReg

存储和所有其他指令将四个内存操作数视为相同的
way and in the same order.  If the segment register is unspecified (regno = 0),
那么就不会生成段覆盖。  “Lea”业务没有分部
寄存器指定，因此它们只有 4 个操作数用于内存引用。

支持 X86 地址空间
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

x86 有一个功能，可以执行加载和存储
通过 x86 段寄存器不同的地址空间。  段覆盖
指令上的前缀字节导致指令的内存访问转到
指定的段。  LLVM地址空间0是默认地址空间，即
包括堆栈以及程序中任何不合格的内存访问。  地址
空格 1-255 当前保留给用户定义的代码。  GS 段是
由地址空间256表示，FS段由地址空间表示
257，SS段由地址空间258表示。其他x86段
尚未分配的地址空间号。

虽然这些地址空间可能看起来类似于通过“thread_local”进行的 TLS
关键字，并且经常使用相同的底层硬件，有一些基本的
差异。

``thread_local`` 关键字适用于全局变量并指定它们
将在线程本地内存中分配。没有类型限定符
涉及到，并且这些变量可以用普通指针指向
通过正常负载和存储进行访问。  ``thread_local`` 关键字是
在 LLVM IR 级别与目标无关（尽管 LLVM 还没有
它的某些配置的实现）

相反，特殊地址空间适用于静态类型。每次加载和存储
在其地址操作数类型中具有特定的地址空间，这就是
确定访问哪个地址空间。  LLVM 忽略这些特殊地址
全局变量上的空格限定符，并且不提供直接的方法
在其中分配存储空间。  在 LLVM IR 级别，这些特殊的行为
地址空间部分取决于底层操作系统或运行时环境，并且
它们是特定于 x86 的（并且 LLVM 在某些方面尚未正确处理它们）
例）。

某些操作系统和运行时环境使用（或将来可能使用）
the FS/GS-segment registers for various low-level purposes, so care should be
考虑它们时采取的。

指令命名
^^^^^^^^^^^^^^^^^^

指令名称由基本名称、默认操作数大小和
每个操作数具有可选的特殊大小的字符。例如：

::

  ADD8rr      -> add, 8-bit register, 8-bit register
  IMUL16rmi   -> imul, 16-bit register, 16-bit memory, 16-bit immediate
  IMUL16rmi8  -> imul, 16-bit register, 16-bit memory, 8-bit immediate
  MOVSX32rm16 -> movsx, 32-bit register, 16-bit memory

The PowerPC backend
-------------------

PowerPC 代码生成器位于 lib/Target/PowerPC 目录中。  代码
生成可重定向到 PowerPC 的多个变体或“子目标”
ISA；包括 ppc32、ppc64 和 altivec。

LLVM PowerPC ABI
^^^^^^^^^^^^^^^^

LLVM 遵循 AIX PowerPC ABI，但有两个偏差。 LLVM 使用相对于 PC 的
（PIC）或用于访问全局值的静态寻址，因此没有 TOC（r2）
用过的。其次，r31用作帧指针以允许堆栈的动态增长
框架。  LLVM利用没有TOC的优势来提供空间来保存帧
调用者框架的 PowerPC 链接区域中的指针。  其他细节
PowerPC ABI 可以在“PowerPC ABI”中找到
<http://developer.apple.com/documentation/DeveloperTools/Conceptual/LowLevelABI/Articles/32bitPowerPC.html>`_\
。注意：此链接描述了 32 位 ABI。  64 位 ABI 类似，除了
GPR 的空间为 8 字节宽（不是 4 个字节），r13 保留供系统使用。

框架布局
^^^^^^^^^^^^

PowerPC 帧的大小通常在函数执行期间是固定的
调用。  由于框架是固定大小的，因此框架中的所有引用都可以
通过堆栈指针的固定偏移量进行访问。  例外情况是
when dynamic alloca or variable sized arrays are present, then a base pointer
(r31) 用作堆栈指针的代理，堆栈指针可以自由增长
或收缩。  如果 llvm-gcc 未传递，也会使用基指针
-fomit-帧指针标志。堆栈指针始终与 16 字节对齐，因此
为 altivec 向量分配的空间将正确对齐。

调用框架的布局如下（顶部是低内存）：

:raw-html:`<table border="1" cellspacing="0">`
:raw-html:`<tr>`
:raw-html:`<td>链接<br><br></td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>参数区<br><br></td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>动态区域<br><br></td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>本地区域<br><br></td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>保存的寄存器区域<br><br></td>`
:raw-html:`</tr>`
:raw-html:`<tr style="border-style: 无隐藏 无隐藏;">`
:raw-html:`<td><br></td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>Previous Frame<br><br></td>`
:raw-html:`</tr>`
:raw-html:`</table>`

*链接*区域被被调用者用来在调用之前保存特殊寄存器
分配自己的框架。  只有三个条目与 LLVM 相关。第一个
条目是前一个堆栈指针（sp），也称为链接。  这允许探测工具
像 gdb 或异常处理程序一样快速扫描堆栈中的帧。  一个
函数 Epilog 还可以使用链接从堆栈中弹出帧。  这
链接区中的第三个条目用于保存 lr 的返回地址
登记。最后，如上所述，最后一个条目用于保存
前一帧指针（r31。）链接区域中的条目的大小为
GPR，因此链接区域在32位模式下为24字节长，在64位模式下为48字节长
位模式。

32位链接区：

:raw-html:`<table  border="1" cellspacing="0">`
:raw-html:`<tr>`
:raw-html:`<td>0</td>`
:raw-html:`<td>Saved SP (r1)</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>4</td>`
:raw-html:`<td>Saved CR</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>8</td>`
:raw-html:`<td>已保存的 LR</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>12</td>`
:raw-html:`<td>保留</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>16</td>`
:raw-html:`<td>保留</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>20</td>`
:raw-html:`<td>Saved FP (r31)</td>`
:raw-html:`</tr>`
:raw-html:`</table>`

64位链接区：

:raw-html:`<table border="1" cellspacing="0">`
:raw-html:`<tr>`
:raw-html:`<td>0</td>`
:raw-html:`<td>Saved SP (r1)</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>8</td>`
:raw-html:`<td>Saved CR</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>16</td>`
:raw-html:`<td>已保存的 LR</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>24</td>`
:raw-html:`<td>保留</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>32</td>`
:raw-html:`<td>保留</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>40</td>`
:raw-html:`<td>Saved FP (r31)</td>`
:raw-html:`</tr>`
:raw-html:`</table>`

*参数区域* 用于存储传递给被调用者的参数
功能。  遵循 PowerPC ABI，前几个参数实际上是
传入寄存器，参数区空间未使用。  然而，如果
没有足够的寄存器或者被调用者是 thunk 或 vararg 函数，
这些寄存器参数可以溢出到参数区域。  因此，
参数区必须足够大，可以存储最大的所有参数
呼叫者发出的呼叫序列。  尺寸也必须足够大
溢出寄存器 r3-r10。  这使得被调用者对调用签名视而不见，
例如 thunk 和 vararg 函数，有足够的空间来缓存参数
寄存器。  因此，参数区域最小为 32 字节（64 字节中为 64 字节）
位模式。）还要注意，由于参数区域是相对于
框架的顶部，被调用者可以使用固定访问其分割参数
距堆栈指针（或基指针）的偏移量。

结合有关链接、参数区域和对齐的信息。一个
堆栈帧在 32 位模式下最小为 64 字节，在 64 位模式下最小为 128 字节。

*动态区域*从大小零开始。  如果函数使用动态分配
然后将空间添加到堆栈中，链接和参数区域移动到
堆栈顶部，并且新空间紧邻链接下方可用，并且
参数区域。  移动链接和参数区域的成本很小
因为只需要复制链接值。  链接值可以很容易地
通过将原始帧大小添加到基指针来获取。  注意
动态空间中的分配需要遵守 16 字节对齐。

*局部区域*是 llvm 编译器为局部变量保留空间的地方。

*保存的寄存器区域*是 llvm 编译器溢出被调用者保存的地方
在进入被调用者时注册。

序言/结语
^^^^^^^^^^^^^

llvm prolog 和 epilog 与 PowerPC ABI 中描述的相同，其中
以下例外情况。  被调用者保存的寄存器在帧结束后溢出
创建的。  这使得 llvm epilog/prolog 支持与其他
目标。  基指针被调用者保存的寄存器r31保存在TOC槽中
联动区。  这简化了基指针的空间分配
可以方便地以编程方式和在调试过程中进行定位。

动态分配
^^^^^^^^^^^^^^^^^^

.. note::

  TODO - More to come.

The NVPTX backend
-----------------

lib/Target/NVPTX 下的 NVPTX 代码生成器是一个开源版本
用于 LLVM 的 NVIDIA NVPTX 代码生成器。  它由 NVIDIA 贡献，
CUDA 编译器 (nvcc) 中使用的代码生成器的端口。  它的目标是
PTX 3.0/3.1 ISA，并且可以针对大于或等于的任何计算能力
2.0（费米）。

该目标是生产质量的，应该完全兼容
NVIDIA 官方工具链。

代码生成器选项：

:raw-html:`<table border="1" cellspacing="0">`
:raw-html:`<tr>`
:raw-html:`<th>选项</th>`
:raw-html:`<th>描述</th>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>sm_20</td>`
:raw-html:`<tdalign="left">将着色器模型/计算能力设置为 2.0</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>sm_21</td>`
:raw-html:`<tdalign="left">将着色器模型/计算能力设置为 2.1</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>sm_30</td>`
:raw-html:`<tdalign="left">将着色器模型/计算能力设置为 3.0</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>sm_35</td>`
:raw-html:`<tdalign="left">将着色器模型/计算能力设置为 3.5</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>ptx30</td>`
:raw-html:`<tdalign="left">目标 PTX 3.0</td>`
:raw-html:`</tr>`
:raw-html:`<tr>`
:raw-html:`<td>ptx31</td>`
:raw-html:`<tdalign="left">目标 PTX 3.1</td>`
:raw-html:`</tr>`
:raw-html:`</table>`

扩展的伯克利数据包过滤器 (eBPF) 后端
--------------------------------------------------

扩展 BPF（或 eBPF）与使用的原始（“经典”）BPF (cBPF) 类似
过滤网络数据包。  这
`bpf() system call <http://man7.org/linux/man-pages/man2/bpf.2.html>`_
执行一系列与 eBPF 相关的操作。  对于 cBPF 和 eBPF
程序，Linux内核在加载之前静态分析程序
他们，以确保它们不会损害正在运行的系统。  eBPF 是
64 位 RISC 指令集，专为一对一映射到 64 位 CPU 而设计。
操作码采用 8 位编码，定义了 87 条指令。  有 10 个
寄存器，按功能分组，如下所述。

::

  R0        return value from in-kernel functions; exit value for eBPF program
  R1 - R5   function call arguments to in-kernel functions
  R6 - R9   callee-saved registers preserved by in-kernel functions
  R10       stack frame pointer (read only)

指令编码（算术和跳转）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
eBPF 重用了经典的大部分操作码编码以简化转换
从经典 BPF 到 eBPF。  对于算术和跳转指令，8 位“代码”
字段分为三个部分：

::

  +----------------+--------+--------------------+
  |   4 bits       |  1 bit |   3 bits           |
  | operation code | source | instruction class  |
  +----------------+--------+--------------------+
  (MSB)                                      (LSB)

三个 LSB 位存储指令类别，它是以下之一：

::

  BPF_LD     0x0
  BPF_LDX    0x1
  BPF_ST     0x2
  BPF_STX    0x3
  BPF_ALU    0x4
  BPF_JMP    0x5
  (unused)   0x6
  BPF_ALU64  0x7

当 BPF_CLASS(code) == BPF_ALU 或 BPF_ALU64 或 BPF_JMP 时，
第 4 位编码源操作数

::

  BPF_X     0x1  use src_reg register as source operand
  BPF_K     0x0  use 32 bit immediate as source operand

和四个MSB位存储操作代码

::

  BPF_ADD   0x0  add
  BPF_SUB   0x1  subtract
  BPF_MUL   0x2  multiply
  BPF_DIV   0x3  divide
  BPF_OR    0x4  bitwise logical OR
  BPF_AND   0x5  bitwise logical AND
  BPF_LSH   0x6  left shift
  BPF_RSH   0x7  right shift (zero extended)
  BPF_NEG   0x8  arithmetic negation
  BPF_MOD   0x9  modulo
  BPF_XOR   0xa  bitwise logical XOR
  BPF_MOV   0xb  move register to register
  BPF_ARSH  0xc  right shift (sign extended)
  BPF_END   0xd  endianness conversion

如果 BPF_CLASS(code) == BPF_JMP，则 BPF_OP(code) 是以下之一

::

  BPF_JA    0x0  unconditional jump
  BPF_JEQ   0x1  jump ==
  BPF_JGT   0x2  jump >
  BPF_JGE   0x3  jump >=
  BPF_JSET  0x4  jump if (DST & SRC)
  BPF_JNE   0x5  jump !=
  BPF_JSGT  0x6  jump signed >
  BPF_JSGE  0x7  jump signed >=
  BPF_CALL  0x8  function call
  BPF_EXIT  0x9  function return

指令编码（加载、存储）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
对于加载和存储指令，8 位“代码”字段分为：

::

  +--------+--------+-------------------+
  | 3 bits | 2 bits |   3 bits          |
  |  mode  |  size  | instruction class |
  +--------+--------+-------------------+
  (MSB)                             (LSB)

尺寸修饰符是其中之一

::

  BPF_W       0x0  word
  BPF_H       0x1  half word
  BPF_B       0x2  byte
  BPF_DW      0x3  double word

模式修饰符是其中之一

::

  BPF_IMM     0x0  immediate
  BPF_ABS     0x1  used to access packet data
  BPF_IND     0x2  used to access packet data
  BPF_MEM     0x3  memory
  (reserved)  0x4
  (reserved)  0x5
  BPF_XADD    0x6  exclusive add


数据包数据访问（BPF_ABS、BPF_IND）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

两个非通用指令：(BPF_ABS | <size> | BPF_LD) 和
(BPF_IND | <size> | BPF_LD) 用于访问数据包数据。
寄存器 R6 是隐式输入，必须包含指向 sk_buff 的指针。
寄存器 R0 是隐式输出，其中包含获取的数据
从数据包中。  寄存器 R1-R5 是暂存寄存器，不得
用于跨 BPF_ABS | 存储数据BPF_LD 或 BPF_IND | BPF_LD
指示。  这些指令具有隐式程序退出条件
以及。  当eBPF程序试图访问超出范围的数据时
如果超出数据包边界，解释器将中止程序的执行。

BPF_IND | BPF_W | BPF_LD 相当于：
  R0 = ntohl(\*(u32 \*) (((struct sk_buff \*) R6)->data + src_reg + imm32))

eBPF 地图
^^^^^^^^^

eBPF 映射用于在内核和用户空间之间共享数据。
Currently implemented types are hash and array, with potential extension to
支持布隆过滤器、基数树等。映射由其类型定义，
maximum number of elements, key size and value size in bytes.  eBPF syscall
支持地图上的创建、更新、查找和删除功能。

函数调用
^^^^^^^^^^^^^^

函数调用参数最多使用五个寄存器 (R1 - R5) 进行传递。
返回值在专用寄存器（R0）中传递。  附加四个
寄存器（R6 - R9）由被调用者保存，这些寄存器中的值
保留在内核函数中。  R0 - R5 是暂存寄存器
kernel functions, and eBPF programs must therefor store/restore values in
如果需要跨函数调用，则使用这些寄存器。  可以访问堆栈
使用只读帧指针 R10。  eBPF 寄存器与硬件 1:1 映射
registers on x86_64 and other 64-bit architectures.  For example, x86_64
内核 JIT 将它们映射为

::

  R0 - rax
  R1 - rdi
  R2 - rsi
  R3 - rdx
  R4 - rcx
  R5 - r8
  R6 - rbx
  R7 - r13
  R8 - r14
  R9 - r15
  R10 - rbp

由于 x86_64 ABI 要求使用 rdi、rsi、rdx、rcx、r8、r9 来传递参数
rbx、r12 - r15 是被调用者保存的。

节目开始
^^^^^^^^^^^^^

eBPF 程序接收单个参数并包含
单个 eBPF 主例程；该程序不包含 eBPF 函数。
函数调用仅限于一组预定义的内核函数。  尺寸
程序仅限于 4K 指令：这确保了快速终止和
有限数量的内核函数调用。  在运行 eBPF 程序之前，
验证程序执行静态分析以防止代码中出现循环
确保有效的寄存器使用和操作数类型。

AMDGPU 后端
------------------

AMDGPU 代码生成器位于“lib/Target/AMDGPU”中
目录。该代码生成器能够针对各种
AMD GPU 处理器。请参阅:doc:`AMDGPUUsage` 了解更多信息。
