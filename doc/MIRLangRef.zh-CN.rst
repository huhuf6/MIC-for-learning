========================================
机器 IR (MIR) 格式参考手册
========================================

.. contents::
   :local:

.. warning::
  This is a work in progress.

介绍
============

本文档是 Machine IR (MIR) 序列化的参考手册
format. MIR is a human readable serialization format that is used to represent
LLVM 的:ref:`机器特定的中间表示
<机器代码表示>`。

MIR 序列化格式旨在用于测试代码
LLVM 中的一代传递。

概述
========

MIR 序列化格式使用 YAML 容器。 YAML 是一个标准
数据序列化语言，完整的 YAML 语言规范可以在以下位置阅读：
`yaml.org
<http://www.yaml.org/spec/1.2/spec.html#Introduction>`_。

MIR 文件被分成一系列“YAML 文档”_。第一个文件
可以包含可选的嵌入式 LLVM IR 模块，其余文档
包含序列化的机器函数。

.. _YAML documents: http://www.yaml.org/spec/1.2/spec.html#id2800132

MIR 测试指南
=================

您可以通过两种不同的方式使用 MIR 格式进行测试：

- 您可以使用以下命令编写调用单个代码生成过程的 MIR 测试
  ``-run-pass`` option in llc.

- 您可以将 llc 的“-stop-after”选项与现有或新的 LLVM 程序集一起使用
  tests and check the MIR output of a specific code generation pass.

测试单独的代码生成通行证
-----------------------------------------

llc 中的“-run-pass”选项允许您创建仅调用的 MIR 测试
单个代码生成过程。当使用此选项时，llc 将解析
输入MIR文件，运行指定的代码生成过程，并输出
生成的 MIR 代码。

您可以使用“-stop-after”或“-stop-after”生成用于测试的输入 MIR 文件
llc 中的``-stop-before`` 选项。例如，如果您想编写一个测试
对于后寄存器分配伪指令扩展过程，您可以
在“-stop-after”选项中指定机器复制传播过程，因为它
在我们尝试测试的通道之前运行：

   ``llc -stop-after=machine-cp bug-trigger.ll > test.mir``

如果同一遍运行多次，则可以包含运行索引
名称后加逗号。

   ``llc -stop-after=dead-mi-elimination,1 bug-trigger.ll > test.mir``

生成输入 MIR 文件后，您必须添加一个使用的运行行
它的“-run-pass”选项。为了测试后寄存器分配
X86-64 上的伪指令扩展传递，如图所示的运行行
可以使用以下：

    ``# RUN: llc -o - %s -mtriple=x86_64-- -run-pass=postrapseudos | FileCheck %s``

MIR 文件与目标相关，因此必须将它们放置在目标中
specific test directories (``lib/CodeGen/TARGETNAME``). They also need to
specify a target triple or a target architecture either in the run line or in
嵌入式 LLVM IR 模块。

简化 MIR 文件
^^^^^^^^^^^^^^^^^^^^^

来自``-stop-after``/``-stop-before`` 的 MIR 代码非常冗长；
简化后，测试更易于访问且面向未来：

- 对 llc 使用“-simplify-mir”选项。

- 机器功能属性通常有默认值或测试仅有效
  as well with default values. Typical candidates for this are: `alignment:`,
  `exposesReturnsTwice`, `legalized`, `regBankSelected`, `selected`.
  The whole `frameInfo` section is often unnecessary if there is no special
  frame usage in the function. `tracksRegLiveness` on the other hand is often
  necessary for some passes that care about block livein lists.

- （全局）“liveins:”列表通常只对早期感兴趣
  instruction selection passes and can be removed when testing later passes.
  The per-block `liveins:` on the other hand are necessary if
  `tracksRegLiveness` is true.

- 如果满足以下条件，则可以删除块“后继：”列表中的分支概率数据：
  test doesn't depend on it. Example:
  `successors: %bb.1(0x40000000), %bb.2(0x40000000)` can be replaced with
  `successors: %bb.1, %bb.2`.

- MIR 代码包含整个 IR 模块。这是必要的，因为有
  no equivalents in MIR for global variables, references to external functions,
  function attributes, metadata, debug info. Instead some MIR data references
  the IR constructs. You can often remove them if the test doesn't depend on
  them.

- Alias Analysis is performed on IR values. These are referenced by memory
  operands in MIR. Example: `:: (load 8 from %ir.foobar, !alias.scope !9)`.
  If the test doesn't depend on (good) alias analysis the references can be
  dropped: `:: (load 8)`

- MIR 块可以参考 IR 块进行调试打印、配置文件信息
  or debug locations. Example: `bb.42.myblock` in MIR references the IR block
  `myblock`. It is usually possible to drop the `.myblock` reference and simply
  use `bb.42`.

- 如果没有内存操作数或块引用 IR，则
  IR function can be replaced by a parameterless dummy function like
  `define @func() { ret void }`.

- 可以删除 MIR 文件的整个 IR 部分，只要它
  contains dummy functions (see above). The .mir loader will create the
  IR functions automatically in this case.

.. _limitations:

局限性
-----------

目前，MIR 格式在声明方面存在一些限制
可以序列化：

- 特定于目标的“MachineFunctionInfo”中的特定于目标的状态
  subclasses isn't serialized at the moment.

- 特定于目标的“MachineConstantPoolValue”子类（在 ARM 和
  SystemZ backends) aren't serialized at the moment.

- “MCSymbol”机器操作数不支持临时或本地符号。

- “MachineModuleInfo” 中的许多状态都没有序列化 - 只有 CFI
  instructions and the variable debug information from MMI is serialized right
  now.

这些限制限制了您可以使用 MIR 格式测试的内容。
目前，想要测试一些取决于状态的行为的测试
临时或本地“MCSymbol”操作数或异常处理状态
MMI，不能使用MIR格式。除此之外，还测试某些行为
这取决于目标特定“MachineFunctionInfo”的状态或
``MachineConstantPoolValue`` 子类目前无法使用 MIR 格式。

High Level Structure
====================

.. _embedded-module:

Embedded Module
---------------

When the first YAML document contains a `YAML block literal string`_, the MIR
解析器会将此字符串视为 LLVM 汇编语言字符串
代表嵌入式 LLVM IR 模块。
以下是包含 LLVM 模块的 YAML 文档示例：

.. code-block:: llvm

       define i32 @inc(i32* %x) {
       entry:
         %0 = load i32, i32* %x
         %1 = add i32 %0, 1
         store i32 %1, i32* %x
         ret i32 %1
       }

.. _YAML block literal string: http://www.yaml.org/spec/1.2/spec.html#id2795688

机器功能
-----------------

其余的 YAML 文档包含机器功能。这是一个例子
此类 YAML 文档的：

.. code-block:: text

     ---
     name:            inc
     tracksRegLiveness: true
     liveins:
       - { reg: '$rdi' }
     callSites:
       - { bb: 0, offset: 3, fwdArgRegs:
           - { arg: 0, reg: '$edi' } }
     body: |
       bb.0.entry:
         liveins: $rdi

         $eax = MOV32rm $rdi, 1, _, 0, _
         $eax = INC32r killed $eax, implicit-def dead $eflags
         MOV32mr killed $rdi, 1, _, 0, _, $eax
         CALL64pcrel32 @foo <regmask...>
         RETQ $eax
     ...

上面的文档由代表各种属性的属性组成
机器函数中的属性和数据结构。

属性“name”是必需的，其值应与
该机器函数所基于的函数的名称。

属性“body”是一个“YAML 块文字字符串”_。其值代表
该函数的机器基本块及其机器指令。

属性“callSites”是调用站点信息的表示，
keeps track of call instructions and registers used to transfer call arguments.

机器指令格式参考
=====================================

机器基本块及其指令使用自定义的、
人类可读的序列化语言。该语言用于
对应于机器函数主体的“YAML 块文字字符串”。

使用该语言的源字符串包含机器基本列表
块，这将在下面的部分中描述。

机器基本块
--------------------

机器基本块在单个块定义源构造中定义
包含块的 ID。
The example below defines two blocks that have an ID of zero and one:

.. code-block:: text

    bb.0:
      <instructions>
    bb.1:
      <instructions>

机器基本块也可以有一个名称。应该在ID后面指定
在块的定义中：

.. code-block:: text

    bb.0.entry:       ; This block's name is "entry"
       <instructions>

该块的名称应与该块的 IR 块的名称相同
机器块是基于。

.. _block-references:

块参考
^^^^^^^^^^^^^^^^

机器基本块通过其 ID 号进行识别。个人
使用以下语法引用块：

.. code-block:: text

    %bb.<id>

例子：

.. code-block:: llvm

    %bb.0

还支持以下语法，但首选前一种语法
块参考：

.. code-block:: text

    %bb.<id>[.<name>]

例子：

.. code-block:: llvm

    %bb.1.then

后继者
^^^^^^^^^^

机器基本块的后继者必须在任何之前指定
instructions:

.. code-block:: text

    bb.0.entry:
      successors: %bb.1.then, %bb.2.else
      <instructions>
    bb.1.then:
      <instructions>
    bb.2.else:
      <instructions>

分支权重可以在后继块后面的括号中指定。
下面的示例定义了一个具有两个具有分支权重的后继者的块
32 和 16：

.. code-block:: text

    bb.0.entry:
      successors: %bb.1.then(32), %bb.2.else(16)

.. _bb-liveins:

住在寄存器中
^^^^^^^^^^^^^^^^^

必须在任何之前指定寄存器中的机器基本块
说明：

.. code-block:: text

    bb.0.entry:
      liveins: $edi, $esi

寄存器和后继者中的生存列表可以为空。语言也
允许多个活在寄存器和后继列表中 - 它们被组合成
解析器的一个列表。

杂项属性
^^^^^^^^^^^^^^^^^^^^^^^^

属性“IsAddressTaken”、“IsLandingPad”、
``IsInlineAsmBrIndirectTarget`` 和 ``Alignment`` 可以在括号中指定
在块的定义之后：

.. code-block:: text

    bb.0.entry (address-taken):
      <instructions>
    bb.2.else (align 4):
      <instructions>
    bb.3(landing-pad, align 4):
      <instructions>
    bb.4 (inlineasm-br-indirect-target):
      <instructions>

.. TODO: Describe the way the reference to an unnamed LLVM IR block can be
   preserved.

“对齐方式”以字节为单位指定，并且必须是 2 的幂。

.. _mir-instructions:

机器说明书
--------------------

机器指令由名称组成，
:ref:`机器操作数 <machine-operands>`,
:ref:`指令标志 <instruction-flags>` 和机器内存操作数。

指令的名称通常在操作数之前指定。例子
下面显示了单机 X86 ``RETQ`` 指令的实例
操作数：

.. code-block:: text

    RETQ $eax

然而，如果机器指令有一个或多个明确定义的寄存器
操作数，必须在其后指定指令名称。例子
下面显示了 AArch64 ``LDPXpost`` 指令的一个实例，其中包含三个
定义的寄存器操作数：

.. code-block:: text

    $sp, $fp, $lr = LDPXpost $sp, 2

指令名称使用来自的确切定义进行序列化
目标的“*InstrInfo.td”文件，并且它们区分大小写。这意味着
类似的指令名称如“TSTri”和“tSTRi”代表不同的指令
机器指令。

.. _instruction-flags:

指令标志
^^^^^^^^^^^^^^^^^

可以在之前指定标志“frame-setup”或“frame-destroy”
指令名称：

.. code-block:: text

    $fp = frame-setup ADDXri $sp, 0, 0

.. code-block:: text

    $x21, $x20 = frame-destroy LDPXi $sp

.. _registers:

捆绑说明
^^^^^^^^^^^^^^^^^^^^

捆绑指令的语法如下：

.. code-block:: text

    BUNDLE implicit-def $r0, implicit-def $r1, implicit $r2 {
      $r0 = SOME_OP $r2
      $r1 = ANOTHER_OP internal $r0
    }

第一条指令通常是包头。 ``{`` 之间的指令
和 ``}`` 与第一条指令捆绑在一起。

.. _mir-registers:

寄存器
---------

寄存器是机器指令中的关键原语之一
序列化语言。它们主要用于
:ref:`注册机器操作数 <register-operands>`,
但它们也可以用在许多其他地方，比如
:ref:`基本块位于列表 <bb-liveins>`中。

物理寄存器通过其名称和“$”前缀符号来标识。
他们使用以下语法：

.. code-block:: text

    $<name>

下面的示例显示了三个 X86 物理寄存器：

.. code-block:: text

    $eax
    $r15
    $eflags

The virtual registers are identified by their ID number and by the '%' sigil.
他们使用以下语法：

.. code-block:: text

    %<id>

例子：

.. code-block:: text

    %0

空寄存器使用下划线（'``_``'）表示。他们也可以是
使用 '``$noreg``' 命名寄存器表示，尽管以前的语法
是优选的。

.. _machine-operands:

机器操作数
----------------

有十八种不同的机器操作数，它们都可以
连载了。

立即数操作数
^^^^^^^^^^^^^^^^^^

立即数机器操作数是无类型的 64 位有符号整数。这
下面的示例显示了 X86 ``MOV32ri`` 指令的实例，该指令具有
立即机器操作数``-42``：

.. code-block:: text

    $eax = MOV32ri -42

立即数操作数也用于表示子寄存器索引，当
机器指令具有以下操作码之一：

- ``EXTRACT_SUBREG``

- ``INSERT_SUBREG``

- ``REG_SEQUENCE``

- ``SUBREG_TO_REG``

如果这是真的，则根据目标打印机器操作数。

例如：

在AArch64RegisterInfo.td中：

.. code-block:: text

  def sub_32 : SubRegIndex<32>;

如果第三个操作数是值为“15”的立即数（取决于目标
值），基于指令的操作码和操作数的索引
将打印为``%subreg.sub_32``：

.. code-block:: text

    %1:gpr64 = SUBREG_TO_REG 0, %0, %subreg.sub_32

对于> 64位的整数，我们使用特殊的机器操作数“MO_CImmediate”，
它使用“APInt”（LLVM 的
任意精度整数）。

.. TODO: Describe the FPIMM immediate operands.

.. _register-operands:

寄存器操作数
^^^^^^^^^^^^^^^^^

:ref:`register <registers>` 原语用于表示寄存器
机器操作数。寄存器操作数也可以有可选的
:ref:`注册标志 <register-flags>`,
:ref:`子寄存器索引 <subregister-indices>`,
以及对绑定寄存器操作数的引用。
寄存器操作数的完整语法如下所示：

.. code-block:: text

    [<flags>] <register> [ :<subregister-idx-name> ] [ (tied-def <tied-op>) ]

此示例显示了 X86“XOR32rr”指令的实例，该指令具有
5个具有不同寄存器标志的寄存器操作数：

.. code-block:: text

  dead $eax = XOR32rr undef $eax, undef $eax, implicit-def dead $eflags, implicit-def $al

.. _register-flags:

寄存器标志
~~~~~~~~~~~~~~

下表显示了所有可能的寄存器标志以及
相应的内部``llvm::RegState``表示：

.. list-table::
   :header-rows: 1

   * - Flag
     - Internal Value

   * - ``implicit``
     - ``RegState::Implicit``

   * - ``implicit-def``
     - ``RegState::ImplicitDefine``

   * - ``def``
     - ``RegState::Define``

   * - ``dead``
     - ``RegState::Dead``

   * - ``killed``
     - ``RegState::Kill``

   * - ``undef``
     - ``RegState::Undef``

   * - ``internal``
     - ``RegState::InternalRead``

   * - ``early-clobber``
     - ``RegState::EarlyClobber``

   * - ``debug-use``
     - ``RegState::Debug``

   * - ``renamable``
     - ``RegState::Renamable``

.. _subregister-indices:

子寄存器索引
~~~~~~~~~~~~~~~~~~~

寄存器机操作数可以通过使用引用寄存器的一部分
子寄存器索引。下面的示例显示了“COPY”的一个实例
使用 X86 ``sub_8bit`` 子寄存器索引复制 8 的伪指令
从32位虚拟寄存器0到8位虚拟寄存器1的低位：

.. code-block:: text

    %1 = COPY %0:sub_8bit

子寄存器索引的名称是特定于目标的，并且通常是
在目标的“*RegisterInfo.td”文件中定义。

常量池指数
^^^^^^^^^^^^^^^^^^^^^

常量池索引 (CPI) 操作数使用其在
函数的“MachineConstantPool”和一个偏移量。

例如，索引为 1、偏移量为 8 的 CPI：

.. code-block:: text

    %1:gr64 = MOV64ri %const.1 + 8

对于索引为 0 且偏移量为 -12 的 CPI：

.. code-block:: text

    %1:gr64 = MOV64ri %const.0 - 12

常量池条目绑定到 LLVM IR “Constant” 或特定于目标的
“机器常量池值”。当序列化所有函数的常量时
使用以下格式：

.. code-block:: text

    constants:
      - id:               <index>
        value:            <value>
        alignment:        <alignment>
        isTargetSpecific: <target-specific>

在哪里：
  - ``<index>`` is a 32-bit unsigned integer;
  - ``<value>`` is a `LLVM IR Constant
    <https://www.llvm.org/docs/LangRef.html#constants>`_;
  - ``<alignment>`` is a 32-bit unsigned integer specified in bytes, and must be
    power of two;
  - ``<target-specific>`` is either true or false.

例子：

.. code-block:: text

    constants:
      - id:               0
        value:            'double 3.250000e+00'
        alignment:        8
      - id:               1
        value:            'g-(LPC0+8)'
        alignment:        4
        isTargetSpecific: true

全局值操作数
^^^^^^^^^^^^^^^^^^^^^

全局值机器操作数引用全局值
:ref:`嵌入式 LLVM IR 模块 <嵌入式模块>`。
下面的示例显示了 X86“MOV64rm”指令的实例，该指令具有
名为“G”的全局值操作数：

.. code-block:: text

    $rax = MOV64rm $rip, 1, _, @G, _

命名的全局值使用带有“@”前缀的标识符来表示。
如果标识符与正则表达式不匹配
`[-a-zA-Z$._][-a-zA-Z$._0-9]*`，则必须用引号引起该标识符。

未命名的全局值使用无符号数值表示
'@' 前缀，如以下示例所示：``@0``、``@989``。

目标相关索引操作数
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

目标索引操作数是特定于目标的索引和偏移量。这
特定于目标的索引是使用特定于目标的名称和正或
负偏移。

例如，“amdgpu-constdata-start”与索引“0”关联
在 AMDGPU 后端。因此，如果我们有一个索引为 0 的目标索引操作数
和偏移量 8：

.. code-block:: text

    $sgpr2 = S_ADD_U32 _, target-index(amdgpu-constdata-start) + 8, implicit-def _, implicit-def _

跳转表索引操作数
^^^^^^^^^^^^^^^^^^^^^^^^^

索引为 0 的跳转表索引操作数打印如下：

.. code-block:: text

    tBR_JTr killed $r0, %jump-table.0

机器跳转表条目包含“MachineBasicBlocks”列表。当序列化所有函数的跳转表条目时，使用以下格式：

.. code-block:: text

    jumpTable:
      kind:             <kind>
      entries:
        - id:             <index>
          blocks:         [ <bbreference>, <bbreference>, ... ]

其中``<kind>``描述如何表示和发出跳转表（纯地址、重定位、PIC等），每个``<index>``是一个32位无符号整数，``blocks``包含机器基本块引用<block-references>`的列表。

例子：

.. code-block:: text

    jumpTable:
      kind:             inline
      entries:
        - id:             0
          blocks:         [ '%bb.3', '%bb.9', '%bb.4.d3' ]
        - id:             1
          blocks:         [ '%bb.7', '%bb.7', '%bb.4.d3', '%bb.5' ]

外部符号操作数
^^^^^^^^^^^^^^^^^^^^^^^^^

外部符号操作数使用带有“&”的标识符来表示
前缀。标识符用“”包围，如果有任何则转义
其中包含特殊的不可打印字符。

例子：

.. code-block:: text

    CALL64pcrel32 &__stack_chk_fail, csr_64, implicit $rsp, implicit-def $rsp

MCSymbol 操作数
^^^^^^^^^^^^^^^^^

MCSymbol 操作数持有指向“MCSymbol”的指针。对于限制
有关 MIR 中此操作数的信息，请参阅:ref:`限制 <限制>`。

语法是：

.. code-block:: text

    EH_LABEL <mcsymbol Ltmp1>

调试指令参考操作数
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

调试指令引用操作数是一对索引，引用一个
指令和该指令中的操作数；看
:ref:`指令引用位置 <instruction-referencing-locations>`。

下面的示例使用对指令 1、操作数 0 的引用：

.. code-block:: text

    DBG_INSTR_REF !123, !DIExpression(DW_OP_LLVM_arg, 0), dbg-instr-ref(1, 0), debug-location !456

CFI指数操作数
^^^^^^^^^^^^^^^^^

CFI 索引操作数保存每个函数边表的索引，
``MachineFunction::getFrameInstructions()``，引用所有框架
“MachineFunction”中的指令。 ``CFI_INSTRUCTION`` 可能看起来像这样
包含多个操作数，但它包含的唯一操作数是 CFI 索引。
其他操作数由“MCCFIInstruction”对象跟踪。

语法是：

.. code-block:: text

    CFI_INSTRUCTION offset $w30, -16

稍后可能会在 MC 层中发出：

.. code-block:: text

    .cfi_offset w30, -16

固有ID操作数
^^^^^^^^^^^^^^^^^^^^

固有 ID 操作数包含通用固有 ID 或目标特定 ID。

“returnaddress” 内在函数的语法是：

.. code-block:: text

   $x0 = COPY intrinsic(@llvm.returnaddress)

谓词操作数
^^^^^^^^^^^^^^^^^^

谓词操作数包含来自 ``CmpInst::Predicate`` 的 IR 谓词，例如
“ICMP_EQ”等

对于 int eq 谓词“ICMP_EQ”，语法为：

.. code-block:: text

   %2:gpr(s32) = G_ICMP intpred(eq), %0, %1

.. TODO: Describe the parsers default behaviour when optional YAML attributes
   are missing.
.. TODO: Describe the syntax for virtual register YAML definitions.
.. TODO: Describe the machine function's YAML flag attributes.
.. TODO: Describe the syntax for the register mask machine operands.
.. TODO: Describe the frame information YAML mapping.
.. TODO: Describe the syntax of the stack object machine operands and their
   YAML definitions.
.. TODO: Describe the syntax of the block address machine operands.
.. TODO: Describe the syntax of the metadata machine operands, and the
   instructions debug location attribute.
.. TODO: Describe the syntax of the register live out machine operands.
.. TODO: Describe the syntax of the machine memory operands.

评论
^^^^^^^^

机器操作数可以有C/C++风格的注释，即附注的注释
在 ``/*`` 和 ``*/`` 之间以提高例如的可读性立即数操作数。
在下面的示例中，ARM 指令 EOR 和 BCC 以及立即数操作数
``14`` 和 ``0`` 已用其条件代码 (CC) 注释
定义，即“always”和“eq”条件代码：

.. code-block:: text

  dead renamable $r2, $cpsr = tEOR killed renamable $r2, renamable $r1, 14 /* CC::always */, $noreg
  t2Bcc %bb.4, 0 /* CC:eq */, killed $cpsr

由于这些注释是注释，因此 MI 解析器会忽略它们。
可以通过覆盖 InstrInfo 的钩子来添加或自定义注释
``createMIROperandComment()``。

调试信息构造
---------------------

MIR 文件中的大部分调试信息都可以在元数据中找到
的嵌入式模块。在机器功能中，该元数据被称为
通过各种构造来描述源位置和变量位置。

来源地点
^^^^^^^^^^^^^^^^

每个 MIR 指令可以选择有一个对
``DILocation`` 元数据节点，位于所有操作数和符号之后，但之前
内存操作数：

.. code-block:: text

   $rbp = MOV64rr $rdi, debug-location !12

源位置附件与 ``!dbg`` 元数据同义
LLVM-IR 中的附件。缺少源位置附件将
由机器指令中的空“DebugLoc”对象表示。

固定可变位置
^^^^^^^^^^^^^^^^^^^^^^^^

有多种指定变量位置的方法。最简单的是
描述永久位于堆栈上的变量。在堆栈中
或机器函数的固定堆栈属性、变量、范围和
提供任何符合条件的位置修饰符：

.. code-block:: text

    - { id: 0, name: offset.addr, offset: -24, size: 8, alignment: 8, stack-id: default,
     4  debug-info-variable: '!1', debug-info-expression: '!DIExpression()',
        debug-info-location: '!2' }

在哪里：

- ``debug-info-variable`` 标识 DILocalVariable 元数据节点，

- ``debug-info-expression`` 将限定符添加到变量位置，

- “debug-info-location” 标识 DILocation 元数据节点。

这些元数据属性对应于“llvm.dbg.declare”的操作数
IR 内在函数，请参阅:ref:`源代码级调试<format_common_intrinsics>`
文档。

不同的可变位置
^^^^^^^^^^^^^^^^^^^^^^^^^^

指定了并不总是位于堆栈或更改位置的变量
使用“DBG_VALUE”元机器指令。它是同义的
``llvm.dbg.value`` IR 内在函数，写为：

.. code-block:: text

    DBG_VALUE $rax, $noreg, !123, !DIExpression(), debug-location !456

分别对应的操作数：

1. 标识机器位置，例如寄存器、立即数或帧索引，

2. 如果要向第一个操作数添加额外的间接级别，则 $noreg 或立即值为零，

3. 标识“DILocalVariable”元数据节点，

4. 指定限定变量位置的表达式，无论是内联还是作为元数据节点引用，

虽然源位置标识了“DILocation”的范围
多变的。第二个操作数 (``IsIndirect``) 已弃用并被删除。
变量位置的所有附加限定符应通过
表达式元数据。

.. _instruction-referencing-locations:

指令参考位置
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

该实验功能旨在分离变量的规范
*值*来自变量取该值的程序点。变化
变量值的发生方式与“DBG_VALUE”元指令相同
但使用“DBG_INSTR_REF”。变量值由一对标识
指令号和操作数号。考虑下面的例子：

.. code-block:: text

    $rbp = MOV64ri 0, debug-instr-number 1, debug-location !12
    DBG_INSTR_REF !123, !DIExpression(DW_OP_LLVM_arg, 0), dbg-instr-ref(1, 0), debug-location !456

指令编号直接附加到机器指令上
可选的“debug-instr-number”附件，位于可选的之前
“调试位置”附件。代码中``$rbp``中定义的值
上面将由“<1, 0>”对来标识。

上面``DBG_INSTR_REF``的第3个操作数记录了指令
和操作数编号``<1, 0>``，标识由``MOV64ri`` 定义的值。
“DBG_INSTR_REF”的前两个操作数与“DBG_VALUE_LIST”相同，
和“DBG_INSTR_REF”的位置记录了变量在哪里
以同样的方式指定值。

有关如何使用这些结构的更多信息，请参阅
:doc:`InstrRefDebugInfo`。相关文档:doc:`SourceLevelDebugging` 和
:doc:`HowToUpdateDebugInfo` 也可能很有用。
