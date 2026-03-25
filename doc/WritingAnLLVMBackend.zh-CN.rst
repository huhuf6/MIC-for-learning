=======================
编写 LLVM 后端
=======================

.. toctree::
   :hidden:

   HowToUseInstrMappings

.. contents::
   :local:

介绍
============

本文档描述了编写可转换的编译器后端的技术
LLVM 中间表示 (IR) 为指定机器进行编码或
其他语言。  用于特定机器的代码可以采用以下形式
汇编代码或二进制代码（可用于 JIT 编译器）。

LLVM 的后端具有一个独立于目标的代码生成器，可以
为多种类型的目标 CPU 创建输出 --- 包括 X86、PowerPC、
ARM 和 SPARC。  后端还可用于生成针对 SPU 的代码
Cell 处理器或 GPU 的支持计算内核的执行。

该文档重点介绍在子目录中找到的现有示例
下载的 LLVM 版本中的“llvm/lib/Target”。  特别是，本文件
重点关注创建静态编译器（发出文本的编译器）的示例
程序集）用于 SPARC 目标，因为 SPARC 有相当标准的
RISC指令集和直接调用等特点
惯例。

观众
--------

本文档的读者是任何需要编写 LLVM 后端的人
为特定硬件或软件目标生成代码。

必备阅读
--------------------

在阅读本文档之前，必须先阅读以下重要文档：

* `LLVM 语言参考手册 <LangRef.html>`_ --- 参考手册
  the LLVM assembly language.

* :doc:`CodeGenerator` --- 组件指南（类和代码
  generation algorithms) for translating the LLVM internal representation into
  machine code for a specified target.  Pay particular attention to the
  descriptions of code generation stages: Instruction Selection, Scheduling and
  Formation, SSA-based Optimization, Register Allocation, Prolog/Epilog Code
  Insertion, Late Machine Code Optimizations, and Code Emission.

* :doc:`TableGen/index` --- 描述 TableGen 的文档
  (``tblgen``) application that manages domain-specific information to support
  LLVM code generation.  TableGen processes input from a target description
  file (``.td`` suffix) and generates C++ code that can be used for code
  generation.

* :doc:`WritingAnLLVMPass` --- 汇编打印机是一个``FunctionPass``，如
  are several ``SelectionDAG`` processing steps.

要遵循本文档中的 SPARC 示例，请拥有一份“SPARC
架构手册，版本 8 <http://www.sparc.org/standards/V8.pdf>`_
参考。  有关ARM指令集的详细信息，请参阅《ARM指令集》
架构参考手册<http://infocenter.arm.com/>`_。  有关更多信息
GNU 汇编器格式（``GAS``），请参阅“使用 As”
<http://sourceware.org/binutils/docs/as/index.html>`_，特别是对于
装配打印机。  “Using As”包含目标机器依赖的列表
特点。

基本步骤
-----------

为 LLVM 编写一个编译器后端，将 LLVM IR 转换为代码
指定目标（机器或其他语言），请按照下列步骤操作：

* 创建描述“TargetMachine”类的子类
  characteristics of your target machine.  Copy existing examples of specific
  ``TargetMachine`` class and header files; for example, start with
  ``SparcTargetMachine.cpp`` and ``SparcTargetMachine.h``, but change the file
  names for your target.  Similarly, change code that references "``Sparc``" to
  reference your target.

* 描述目标的寄存器组。  使用 TableGen 生成代码
  register definition, register aliases, and register classes from a
  target-specific ``RegisterInfo.td`` input file.  You should also write
  additional code for a subclass of the ``TargetRegisterInfo`` class that
  represents the class register file data used for register allocation and also
  describes the interactions between registers.

* 描述目标的指令集。  使用TableGen生成代码
  for target-specific instructions from target-specific versions of
  ``TargetInstrFormats.td`` and ``TargetInstrInfo.td``.  You should write
  additional code for a subclass of the ``TargetInstrInfo`` class to represent
  machine instructions supported by the target machine.

* 描述LLVM IR从有向无环的选择和转换
  Graph (DAG) representation of instructions to native target-specific
  instructions.  Use TableGen to generate code that matches patterns and
  selects instructions based on additional information in a target-specific
  version of ``TargetInstrInfo.td``.  Write code for ``XXXISelDAGToDAG.cpp``,
  where ``XXX`` identifies the specific target, to perform pattern matching and
  DAG-to-DAG instruction selection.  Also write code in ``XXXISelLowering.cpp``
  to replace or remove operations and data types that are not supported
  natively in a SelectionDAG.

* 为汇编打印机编写代码，将 LLVM IR 转换为 GAS 格式
  your target machine.  You should add assembly strings to the instructions
  defined in your target-specific version of ``TargetInstrInfo.td``.  You
  should also write code for a subclass of ``AsmPrinter`` that performs the
  LLVM-to-assembly conversion and a trivial subclass of ``TargetAsmInfo``.

*可选地，添加对子目标的支持（即具有不同的变体）
  capabilities).  You should also write code for a subclass of the
  ``TargetSubtarget`` class, which allows you to use the ``-mcpu=`` and
  ``-mattr=`` command-line options.

* 可选地，添加 JIT 支持并创建一个机器代码发射器（
  ``TargetJITInfo``) that is used to emit binary code directly into memory.

在“.cpp”和“.h”中。文件，首先存根这些方法，然后
稍后再实施。  最初，您可能不知道哪些私人成员
该类需要以及哪些组件需要子类化。

预赛
-------------

要实际创建编译器后端，您需要创建和修改一些
文件。  这里讨论绝对最小值。  但要实际使用 LLVM
目标无关的代码生成器，您必须执行中描述的步骤
:doc:`LLVM 目标无关代码生成器 <CodeGenerator>` 文档。

首先，您应该在“lib/Target”下创建一个子目录来保存所有
与您的目标相关的文件。  如果您的目标名为“Dummy”，请创建
目录“lib/Target/Dummy”。

在这个新目录中，创建一个“CMakeLists.txt”。  最简单的方法是复制一个
另一个目标的 ``CMakeLists.txt`` 并修改它。  它至少应该包含
“LLVM_TARGET_DEFINITIONS”变量。该库可以命名为“LLVMDummy”
（例如，请参阅 MIPS 目标）。  或者，您可以拆分库
进入“LLVMDummyCodeGen”和“LLVMDummyAsmPrinter”，后者
应该在“lib/Target/Dummy”下面的子目录中实现（例如，
请参阅 PowerPC 目标）。

请注意，这两个命名方案被硬编码到“llvm-config”中。  使用
任何其他命名方案都会混淆“llvm-config”并产生大量
（看似无关）链接“llc”时出现链接器错误。

为了让你的目标真正做某事，你需要实现一个子类
“目标机器”。  此实现通常应该在文件中
“lib/Target/DummyTargetMachine.cpp”，但“lib/Target”中的任何文件
目录将被构建并且应该可以工作。  使用LLVM的目标独立代码
生成器，你应该做所有当前机器后端所做的事情：创建一个
“LLVMTargetMachine”的子类。  （要从头开始创建目标，请创建一个
“TargetMachine”的子类。）

要让 LLVM 实际构建并链接您的目标，您需要运行“cmake”
带有``-DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=Dummy``。这将建立你的
目标，而不需要将其添加到所有目标的列表中。

一旦您的目标稳定，您可以将其添加到“LLVM_ALL_TARGETS”变量中
位于主“CMakeLists.txt”中。

目标机
==============

“LLVMTargetMachine” 被设计为实现目标的基类
LLVM 与目标无关的代码生成器。  “LLVMTargetMachine” 类
应该由一个具体的目标类来专门化，该目标类实现了各种
虚拟方法。  ``LLVMTargetMachine`` 被定义为 的子类
“include/llvm/Target/TargetMachine.h”中的“TargetMachine”。  的
``TargetMachine`` 类实现 (``TargetMachine.cpp``) 也处理
许多命令行选项。

要创建“LLVMTargetMachine”的具体目标特定子类，请启动
通过复制现有的“TargetMachine”类和标头。  你应该命名
您为反映特定目标而创建的文件。  例如，对于
SPARC 目标，将文件命名为“SparcTargetMachine.h”并
“SparcTargetMachine.cpp”。

对于目标机器“XXX”，“XXXTargetMachine”的实现必须
have access methods to obtain objects that represent target components.  这些
methods are named ``get*Info``, and are intended to obtain the instruction set
(``getInstrInfo``), register set (``getRegisterInfo``), stack frame layout
(``getFrameInfo``) 和类似信息。  ``XXXTargetMachine`` 还必须
实现“getDataLayout”方法来访问特定于目标的对象
数据特征，例如数据类型大小和对齐要求。

例如，对于 SPARC 目标，头文件“SparcTargetMachine.h”
声明几个“get*Info”和“getDataLayout”方法的原型
只需返回一个类成员即可。

.. code-block:: c++

  namespace llvm {

  class Module;

  class SparcTargetMachine : public LLVMTargetMachine {
    const DataLayout DataLayout;       // Calculates type size & alignment
    SparcSubtarget Subtarget;
    SparcInstrInfo InstrInfo;
    TargetFrameInfo FrameInfo;

  protected:
    virtual const TargetAsmInfo *createTargetAsmInfo() const;

  public:
    SparcTargetMachine(const Module &M, const std::string &FS);

    virtual const SparcInstrInfo *getInstrInfo() const {return &InstrInfo; }
    virtual const TargetFrameInfo *getFrameInfo() const {return &FrameInfo; }
    virtual const TargetSubtarget *getSubtargetImpl() const{return &Subtarget; }
    virtual const TargetRegisterInfo *getRegisterInfo() const {
      return &InstrInfo.getRegisterInfo();
    }
    virtual const DataLayout *getDataLayout() const { return &DataLayout; }

    // Pass Pipeline Configuration
    virtual bool addInstSelector(PassManagerBase &PM, bool Fast);
    virtual bool addPreEmitPass(PassManagerBase &PM, bool Fast);
  };

  } // end namespace llvm

*``getInstrInfo()``
*``getRegisterInfo()``
*``getFrameInfo()``
*``getDataLayout()``
*``getSubtargetImpl()``

对于某些目标，还需要支持以下方法：

*``getTargetLowering()``
*``getJITInfo()``

某些架构（例如 GPU）不支持跳转到任意位置
使用屏蔽执行和循环来定位程序并实现分支
循环体周围的特殊说明。为了避免CFG修改
引入了此类硬件无法处理的不可简化的控制流，目标
初始化时必须调用setRequiresStructuredCFG(true)。

此外，“XXXTargetMachine”构造函数应指定一个
``TargetDescription`` 字符串，确定目标的数据布局
机器，包括诸如指针大小、对齐方式和
字节序。  例如，“SparcTargetMachine”的构造函数包含
以下：

.. code-block:: c++

  SparcTargetMachine::SparcTargetMachine(const Module &M, const std::string &FS)
    : DataLayout("E-p:32:32-f128:128:128"),
      Subtarget(M, FS), InstrInfo(Subtarget),
      FrameInfo(TargetFrameInfo::StackGrowsDown, 8, 0) {
  }

连字符分隔“TargetDescription”字符串的各个部分。

* 字符串中的大写“``E``”表示大端目标数据模型。
  A lower-case "``e``" indicates little-endian.

* “``p:``”后面是指针信息：大小、ABI 对齐方式和
  preferred alignment.  If only two figures follow "``p:``", then the first
  value is pointer size, and the second value is both ABI and preferred
  alignment.

* 然后是数字类型对齐的字母：“``i``”，“``f``”，“``v``”，或
  "``a``" (corresponding to integer, floating point, vector, or aggregate).
  "``i``", "``v``", or "``a``" are followed by ABI alignment and preferred
  alignment. "``f``" is followed by three values: the first indicates the size
  of a long double, then ABI alignment, and then ABI preferred alignment.

目标注册
===================

您还必须使用“TargetRegistry”注册您的目标，这就是
其他 LLVM 工具用于在运行时查找和使用您的目标。  的
``TargetRegistry`` 可以直接使用，但对于大多数目标都有帮助程序
应该为您处理工作的模板。

所有目标都应该声明一个全局“Target”对象，用于
代表注册期间的目标。  然后，在目标的“TargetInfo”中
库，目标应该定义该对象并使用``RegisterTarget``
模板来注册目标。  例如Sparc注册码
看起来像这样：

.. code-block:: c++

  Target llvm::getTheSparcTarget();

  extern "C" void LLVMInitializeSparcTargetInfo() {
    RegisterTarget<Triple::sparc, /*HasJIT=*/false>
      X(getTheSparcTarget(), "sparc", "Sparc");
  }

这允许“TargetRegistry”按名称或目标查找目标
三重。  此外，大多数目标还将注册附加功能，这些功能
可以在单独的库中找到。  这些注册步骤是分开的，
因为有些客户可能希望只链接目标的某些部分——
JIT 代码生成器不需要使用汇编打印机，因为
示例。  以下是注册 Sparc 装配打印机的示例：

.. code-block:: c++

  extern "C" void LLVMInitializeSparcAsmPrinter() {
    RegisterAsmPrinter<SparcAsmPrinter> X(getTheSparcTarget());
  }

有关详细信息，请参阅“`llvm/Target/TargetRegistry.h
</doxygen/TargetRegistry_8h-source.html>`_"。

寄存器集和寄存器类
=================================

您应该描述一个具体的特定于目标的类来代表
目标机器的寄存器文件。  这个类称为“XXXRegisterInfo”
（其中“XXX”标识目标）并表示类寄存器文件
用于寄存器分配的数据。  它还描述了交互
寄存器之间。

您还需要定义寄存器类来对相关寄存器进行分类。  一个
应该为所有被处理的寄存器组添加寄存器类
某些指令的方式相同。  典型的例子是寄存器类
整数、浮点或向量寄存器。  寄存器分配器允许
指令使用指定寄存器类中的任何寄存器来执行
以类似的方式进行指导。  寄存器类分配虚拟寄存器
来自这些集合的指令，并注册类让
与目标无关的寄存器分配器自动选择实际的
寄存器。

寄存器的大部分代码，包括寄存器定义、寄存器
别名和寄存器类由 TableGen 生成
``XXXRegisterInfo.td`` 输入文件并放置在``XXXGenRegisterInfo.h.inc`` 中
和 ``XXXGenRegisterInfo.inc`` 输出文件。  中的一些代码
``XXXRegisterInfo`` 的实现需要手动编码。

定义寄存器
-------------------

“XXXRegisterInfo.td” 文件通常以寄存器定义开头
目标机器。  使用“Register”类（在“Target.td”中指定）
为每个寄存器定义一个对象。  指定的字符串“n”成为
寄存器的“名称”。  基本的“Register”对象没有任何
子注册并且不指定任何别名。

.. code-block:: text

  class Register<string n> {
    string Namespace = "";
    string AsmName = n;
    string Name = n;
    int SpillSize = 0;
    int SpillAlignment = 0;
    list<Register> Aliases = [];
    list<Register> SubRegs = [];
    list<int> DwarfNumbers = [];
  }

例如，在``X86RegisterInfo.td``文件中，有寄存器定义
使用“Register”类，例如：

.. code-block:: text

  def AL : Register<"AL">, DwarfRegNum<[0, 0, 0]>;

这定义了寄存器“AL”并为其赋值（使用“DwarfRegNum”）
由``gcc``、``gdb`` 或调试信息编写器用来识别
注册。  对于寄存器“AL”，“DwarfRegNum”采用包含 3 个值的数组
代表 3 种不同的模式：第一个元素用于 X86-64，第二个元素用于
X86-32 上的异常处理 (EH)，第三个是通用的。 -1是特殊的
矮数表示gcc编号未定义，-2表示
寄存器号对于该模式无效。

从前面描述的“X86RegisterInfo.td”文件中的行来看，TableGen
在“X86GenRegisterInfo.inc”文件中生成此代码：

.. code-block:: c++

  static const unsigned GR8[] = { X86::AL, ... };

  const unsigned AL_AliasSet[] = { X86::AX, X86::EAX, X86::RAX, 0 };

  const TargetRegisterDesc RegisterDescriptors[] = {
    ...
  { "AL", "AL", AL_AliasSet, Empty_SubRegsSet, Empty_SubRegsSet, AL_SuperRegsSet }, ...

TableGen 从寄存器信息文件中生成一个“TargetRegisterDesc”对象
对于每个寄存器。  “TargetRegisterDesc” 定义于
``include/llvm/Target/TargetRegisterInfo.h`` 具有以下字段：

.. code-block:: c++

  struct TargetRegisterDesc {
    const char     *AsmName;      // Assembly language name for the register
    const char     *Name;         // Printable name for the reg (for debugging)
    const unsigned *AliasSet;     // Register Alias Set
    const unsigned *SubRegs;      // Sub-register set
    const unsigned *ImmSubRegs;   // Immediate sub-register set
    const unsigned *SuperRegs;    // Super-register set
  };

TableGen 使用整个目标描述文件 (``.td``) 来确定文本
寄存器的名称（在 ``AsmName`` 和 ``Name`` 字段中
``TargetRegisterDesc``）以及其他寄存器与定义的关系
注册（在其他“TargetRegisterDesc”字段中）。  在这个例子中，其他
定义将寄存器“``AX``”、“``EAX``”和“``RAX``”建立为
彼此之间存在别名，因此 TableGen 生成一个以 null 结尾的数组
(``AL_AliasSet``) 此寄存器别名集。

“Register”类通常用作更复杂的基类
类。  在“Target.td”中，“Register”类是
``RegisterWithSubRegs`` 类，用于定义需要的寄存器
在“SubRegs”列表中指定子寄存器，如下所示：

.. code-block:: text

  class RegisterWithSubRegs<string n, list<Register> subregs> : Register<n> {
    let SubRegs = subregs;
  }

在“SparcRegisterInfo.td”中，为 SPARC 定义了附加寄存器类：
``Register`` 子类，``SparcReg``，以及更多子类：``Ri``，``Rf``，
和“路”。  SPARC 寄存器由 5 位 ID 号标识，这是一个
这些子类共有的特征。  请注意使用“``let``”表达式
覆盖最初在超类中定义的值（例如“SubRegs”
``Rd`` 类中的字段）。

.. code-block:: text

  class SparcReg<string n> : Register<n> {
    field bits<5> Num;
    let Namespace = "SP";
  }
  // Ri - 32-bit integer registers
  class Ri<bits<5> num, string n> :
  SparcReg<n> {
    let Num = num;
  }
  // Rf - 32-bit floating-point registers
  class Rf<bits<5> num, string n> :
  SparcReg<n> {
    let Num = num;
  }
  // Rd - Slots in the FP register file for 64-bit floating-point values.
  class Rd<bits<5> num, string n, list<Register> subregs> : SparcReg<n> {
    let Num = num;
    let SubRegs = subregs;
  }

在“SparcRegisterInfo.td”文件中，有一些寄存器定义
利用“Register”的这些子类，例如：

.. code-block:: text

  def G0 : Ri< 0, "G0">, DwarfRegNum<[0]>;
  def G1 : Ri< 1, "G1">, DwarfRegNum<[1]>;
  ...
  def F0 : Rf< 0, "F0">, DwarfRegNum<[32]>;
  def F1 : Rf< 1, "F1">, DwarfRegNum<[33]>;
  ...
  def D0 : Rd< 0, "F0", [F0, F1]>, DwarfRegNum<[32]>;
  def D1 : Rd< 2, "F2", [F2, F3]>, DwarfRegNum<[34]>;

上面显示的最后两个寄存器（“D0”和“D1”）是双精度的
作为单精度对的别名的浮点寄存器
浮点子寄存器。  除了别名之外，子寄存器和
定义的寄存器的超级寄存器关系位于a的字段中
寄存器的``TargetRegisterDesc``。

定义寄存器类
-------------------------

“RegisterClass”类（在“Target.td”中指定）用于定义一个
代表一组相关寄存器的对象，还定义了
寄存器的默认分配顺序。  目标描述文件
使用“Target.td”的“XXXRegisterInfo.td”可以构造寄存器类
使用以下类：

.. code-block:: text

  class RegisterClass<string namespace,
  list<ValueType> regTypes, int alignment, dag regList> {
    string Namespace = namespace;
    list<ValueType> RegTypes = regTypes;
    int Size = 0;  // spill size, in bits; zero lets tblgen pick the size
    int Alignment = alignment;

    // CopyCost is the cost of copying a value between two registers
    // default value 1 means a single instruction
    // A negative value means copying is extremely expensive or impossible
    int CopyCost = 1;
    dag MemberList = regList;

    // for register classes that are subregisters of this class
    list<RegisterClass> SubRegClassList = [];

    code MethodProtos = [{}];  // to insert arbitrary code
    code MethodBodies = [{}];
  }

要定义“RegisterClass”，请使用以下 4 个参数：

* 定义的第一个参数是命名空间的名称。

* 第二个参数是“ValueType”寄存器类型值的列表，这些值是
  defined in ``include/llvm/CodeGen/ValueTypes.td``.  Defined values include
  integer types (such as ``i16``, ``i32``, and ``i1`` for Boolean),
  floating-point types (``f32``, ``f64``), and vector types (for example,
  ``v8i16`` for an ``8 x i16`` vector).  All registers in a ``RegisterClass``
  must have the same ``ValueType``, but some registers may store vector data in
  different configurations.  For example a register that can process a 128-bit
  vector may be able to handle 16 8-bit integer elements, 8 16-bit integers, 4
  32-bit integers, and so on.

* “RegisterClass” 定义的第三个参数指定
  alignment required of the registers when they are stored or loaded to
  memory.

* 最后一个参数“regList”指定此类中的寄存器。
  If an alternative allocation order method is not specified, then ``regList``
  also defines the order of allocation used by the register allocator.  Besides
  simply listing registers with ``(add R0, R1, ...)``, more advanced set
  operators are available.  See ``include/llvm/Target/Target.td`` for more
  information.

在“SparcRegisterInfo.td”中，定义了三个“RegisterClass”对象：
“FPRegs”、“DFPRegs”和“IntRegs”。  对于所有三个寄存器类别，
第一个参数用字符串“``SP``”定义命名空间。  ``FPRegs``
定义了一组 32 个单精度浮点寄存器（``F0`` 到
``F31``）； ``DFPRegs`` 定义一组 16 个双精度寄存器
（``D0-D15``）。

.. code-block:: text

  // F0, F1, F2, ..., F31
  def FPRegs : RegisterClass<"SP", [f32], 32, (sequence "F%u", 0, 31)>;

  def DFPRegs : RegisterClass<"SP", [f64], 64,
                              (add D0, D1, D2, D3, D4, D5, D6, D7, D8,
                                   D9, D10, D11, D12, D13, D14, D15)>;

  def IntRegs : RegisterClass<"SP", [i32], 32,
      (add L0, L1, L2, L3, L4, L5, L6, L7,
           I0, I1, I2, I3, I4, I5,
           O0, O1, O2, O3, O4, O5, O7,
           G1,
           // Non-allocatable regs:
           G2, G3, G4,
           O6,        // stack ptr
           I6,        // frame ptr
           I7,        // return address
           G0,        // constant zero
           G5, G6, G7 // reserved for kernel
      )>;

将“SparcRegisterInfo.td”与 TableGen 一起使用会生成多个输出文件
旨在包含在您编写的其他源代码中。
“SparcRegisterInfo.td”生成“SparcGenRegisterInfo.h.inc”，它应该
包含在头文件中以实现 SPARC 寄存器
您编写的实现（``SparcRegisterInfo.h``）。  在
``SparcGenRegisterInfo.h.inc`` 定义了一个新结构，称为
使用“TargetRegisterInfo”作为基础的“SparcGenRegisterInfo”。  它还
根据定义的寄存器类指定类型：“DFPRegsClass”，
“FPRegsClass”和“IntRegsClass”。

“SparcRegisterInfo.td” 还会生成“SparcGenRegisterInfo.inc”，即
包含在 SPARC 寄存器“SparcRegisterInfo.cpp”的底部
实施。  下面的代码仅显示生成的整数寄存器和
相关的寄存器类。  “IntRegs”中寄存器的顺序反映了
目标描述文件中``IntRegs`` 定义中的顺序。

.. code-block:: c++

  // IntRegs Register Class...
  static const unsigned IntRegs[] = {
    SP::L0, SP::L1, SP::L2, SP::L3, SP::L4, SP::L5,
    SP::L6, SP::L7, SP::I0, SP::I1, SP::I2, SP::I3,
    SP::I4, SP::I5, SP::O0, SP::O1, SP::O2, SP::O3,
    SP::O4, SP::O5, SP::O7, SP::G1, SP::G2, SP::G3,
    SP::G4, SP::O6, SP::I6, SP::I7, SP::G0, SP::G5,
    SP::G6, SP::G7,
  };

  // IntRegsVTs Register Class Value Types...
  static const MVT::ValueType IntRegsVTs[] = {
    MVT::i32, MVT::Other
  };

  namespace SP {   // Register class instances
    DFPRegsClass    DFPRegsRegClass;
    FPRegsClass     FPRegsRegClass;
    IntRegsClass    IntRegsRegClass;
  ...
    // IntRegs Sub-register Classes...
    static const TargetRegisterClass* const IntRegsSubRegClasses [] = {
      NULL
    };
  ...
    // IntRegs Super-register Classes..
    static const TargetRegisterClass* const IntRegsSuperRegClasses [] = {
      NULL
    };
  ...
    // IntRegs Register Class sub-classes...
    static const TargetRegisterClass* const IntRegsSubclasses [] = {
      NULL
    };
  ...
    // IntRegs Register Class super-classes...
    static const TargetRegisterClass* const IntRegsSuperclasses [] = {
      NULL
    };

    IntRegsClass::IntRegsClass() : TargetRegisterClass(IntRegsRegClassID,
      IntRegsVTs, IntRegsSubclasses, IntRegsSuperclasses, IntRegsSubRegClasses,
      IntRegsSuperRegClasses, 4, 4, 1, IntRegs, IntRegs + 32) {}
  }

寄存器分配器将避免使用保留寄存器，并且被调用者保存
在使用所有易失性寄存器之前，不会使用寄存器。  那
通常足够好，但在某些情况下可能需要提供自定义
分配订单。

实现“TargetRegisterInfo”的子类
----------------------------------------------

最后一步是传递“XXXRegisterInfo”的代码部分，其中
实现“TargetRegisterInfo.h”中描述的接口（参见
:ref:`TargetRegisterInfo`)。  这些函数返回 ``0``、``NULL`` 或
``false``，除非被覆盖。  这是被覆盖的函数列表
对于“SparcRegisterInfo.cpp”中的 SPARC 实现：

* ``getCalleeSavedRegs`` --- 返回被调用者保存的寄存器列表
  order of the desired callee-save stack frame offset.

* ``getReservedRegs`` --- 返回由物理寄存器索引的位集
  numbers, indicating if a particular register is unavailable.

* ``hasFP`` --- 返回一个布尔值，指示函数是否应该有
  dedicated frame pointer register.

* ``eliminateCallFramePseudoInstr`` --- 如果调用帧设置或销毁伪
  instructions are used, this can be called to eliminate them.

* ``eliminateFrameIndex`` --- 消除抽象帧索引
  instructions that may use them.

* ``emitPrologue`` --- 将序言代码插入到函数中。

* ``emitEpilogue`` --- 将尾声代码插入到函数中。

.. _instruction-set:

指令集
===============

在代码生成的早期阶段，LLVM IR 代码被转换为
“SelectionDAG”，其节点是“SDNode”类的实例
包含目标指令。  ``SDNode`` 有操作码、操作数、类型
要求和操作属性。  例如，是一个操作
可交换的，从内存加载操作。  各种操作节点
类型在“include/llvm/CodeGen/SelectionDAGNodes.h”文件中描述
（“ISD”命名空间中“NodeType”枚举的值）。

TableGen 使用以下目标描述（``.td``）输入文件来
生成大部分指令定义代码：

*``Target.td`` --- 其中``指令``、``操作数``、``InstrInfo`` 和
  other fundamental classes are defined.

* ``TargetSelectionDAG.td`` --- 由``SelectionDAG`` 指令选择使用
  generators, contains ``SDTC*`` classes (selection DAG type constraint),
  definitions of ``SelectionDAG`` nodes (such as ``imm``, ``cond``, ``bb``,
  ``add``, ``fadd``, ``sub``), and pattern support (``Pattern``, ``Pat``,
  ``PatFrag``, ``PatLeaf``, ``ComplexPattern``.

* ``XXXInstrFormats.td`` --- 特定于目标的定义模式
  instructions.

* ``XXXInstrInfo.td`` --- 指令模板的目标特定定义，
  condition codes, and instructions of an instruction set.  For architecture
  modifications, a different file name may be used.  For example, for Pentium
  with SSE instruction, this file is ``X86InstrSSE.td``, and for Pentium with
  MMX, this file is ``X86InstrMMX.td``.

还有一个特定于目标的“XXX.td”文件，其中“XXX”是
目标。  “XXX.td” 文件包含其他“.td” 输入文件，但是
其内容仅对子目标直接重要。

您应该描述一个具体的特定目标类“XXXInstrInfo”
表示目标机器支持的机器指令。
“XXXInstrInfo”包含一个“XXXInstrDescriptor”对象数组，每个对象
它描述了一条指令。  指令描述符定义：

* 操作码助记符
* 操作数的数量
* 隐式寄存器定义和用途列表
* 与目标无关的属性（例如内存访问，是可交换的）
* 特定于目标的标志

指令类（在``Target.td``中定义）主要用作
更复杂的教学课程。

.. code-block:: text

  class Instruction {
    string Namespace = "";
    dag OutOperandList;    // A dag containing the MI def operand list.
    dag InOperandList;     // A dag containing the MI use operand list.
    string AsmString = ""; // The .s format to print the instruction with.
    list<dag> Pattern;     // Set to the DAG pattern for this instruction.
    list<Register> Uses = [];
    list<Register> Defs = [];
    list<Predicate> Predicates = [];  // predicates turned into isel match code
    ... remainder not shown for space ...
  }

一个“SelectionDAG”节点（“SDNode”）应该包含一个代表一个对象的对象
在“XXXInstrInfo.td”中定义的特定于目标的指令。  的
指令对象应该代表架构手册中的指令
目标机器的信息（例如 SPARC 的 SPARC 架构手册）
目标）。

架构手册中的单个指令通常被建模为多个
目标指令，取决于其操作数。  例如，手册可能
描述采用寄存器或立即数操作数的加法指令。  安
LLVM 目标可以使用两个名为“ADDri”的指令对此进行建模，
``ADDrr``。

您应该为每个指令类别定义一个类并定义每个操作码
作为具有适当参数的类别的子类，例如固定
操作码和扩展操作码的二进制编码。  你应该映射寄存器
位到对其进行编码的指令的位（对于 JIT）。
此外，您还应该指定当出现以下情况时应如何打印指令：
使用自动装配打印机。

正如 SPARC 架构手册第 8 版中所述，有以下三种
主要的 32 位指令格式。  格式 1 仅适用于 ``CALL``
指示。  格式 2 用于条件代码和“SETHI”（设置高
寄存器的位）指令。  格式3用于其他指令。

这些格式中的每一种在“SparcInstrFormat.td”中都有相应的类。
``InstSP`` 是其他指令类的基类。  附加底座
类被指定为更精确的格式：例如
``SparcInstrFormat.td``，``F2_1`` 用于``SETHI``，``F2_2`` 用于
分支机构。  还有其他三个基类：``F3_1`` 用于寄存器/寄存器
操作，“F3_2”用于寄存器/立即操作，“F3_3”用于
浮点运算。  ``SparcInstrInfo.td`` 还添加了基类
``Pseudo`` 用于合成 SPARC 指令。

“SparcInstrInfo.td”主要由操作数和指令定义组成
对于 SPARC 目标。  在“SparcInstrInfo.td”中，以下目标
描述文件条目“LDrr”定义了加载整数指令
从内存地址到寄存器的字（“LD” SPARC 操作码）。  第一个
参数，值 3 (``11``\ :sub:`2`) 是这个的操作值
经营类别。  第二个参数 (``000000``\ :sub:`2`) 是
``LD``/Load Word 的具体操作值。  第三个参数是
输出目的地，它是一个寄存器操作数并在“Register”中定义
目标描述文件（``IntRegs``）。

.. code-block:: text

  def LDrr : F3_1 <3, 0b000000, (outs IntRegs:$rd), (ins (MEMrr $rs1, $rs2):$addr),
                   "ld [$addr], $dst",
                   [(set i32:$dst, (load ADDRrr:$addr))]>;

第四个参数是输入源，使用地址操作数
``MEMrr`` 之前在``SparcInstrInfo.td`` 中定义：

.. code-block:: text

  def MEMrr : Operand<i32> {
    let PrintMethod = "printMemOperand";
    let MIOperandInfo = (ops IntRegs, IntRegs);
  }

第五个参数是装配打印机使用的字符串，可以是
保留为空字符串，直到实现装配打印机接口。
第六个也是最后一个参数是用于匹配指令的模式
在 CodeGenerator 中描述的 SelectionDAG 选择阶段期间。
该参数将在下一节“指令选择器”中详细介绍。

对于不同的操作数类型，指令类定义不会重载，
因此寄存器、内存或寄存器需要单独版本的指令
立即值操作数。  例如，执行 Load Integer 指令
对于从立即操作数到寄存器的字，以下指令
类定义为：

.. code-block:: text

  def LDri : F3_2 <3, 0b000000, (outs IntRegs:$rd), (ins (MEMri $rs1, $simm13):$addr),
                   "ld [$addr], $dst",
                   [(set i32:$rd, (load ADDRri:$addr))]>;

为如此多的类似指令编写这些定义可能会涉及很多工作
剪切和粘贴。  在“.td”文件中，“multiclass”指令启用
创建模板以一次定义多个指令类（使用
``defm`` 指令）。  例如在“SparcInstrInfo.td”中，“multiclass”
模式``F3_12``被定义为每次创建2个指令类
``F3_12`` 被调用：

.. code-block:: text

  multiclass F3_12 <string OpcStr, bits<6> Op3Val, SDNode OpNode> {
    def rr  : F3_1 <2, Op3Val,
                   (outs IntRegs:$rd), (ins IntRegs:$rs1, IntRegs:$rs1),
                   !strconcat(OpcStr, " $rs1, $rs2, $rd"),
                   [(set i32:$rd, (OpNode i32:$rs1, i32:$rs2))]>;
    def ri  : F3_2 <2, Op3Val,
                   (outs IntRegs:$rd), (ins IntRegs:$rs1, i32imm:$simm13),
                   !strconcat(OpcStr, " $rs1, $simm13, $rd"),
                   [(set i32:$rd, (OpNode i32:$rs1, simm13:$simm13))]>;
  }

因此，当“defm”指令用于“XOR”和“ADD”时
指令，如下所示，它创建了四个指令对象：``XORrr``，
“XORri”、“ADDrr”和“ADDri”。

.. code-block:: text

  defm XOR   : F3_12<"xor", 0b000011, xor>;
  defm ADD   : F3_12<"add", 0b000000, add>;

“SparcInstrInfo.td”还包括条件代码的定义
由分支指令引用。  以下定义在
“SparcInstrInfo.td”表示 SPARC 条件代码的位位置。
例如，第 10 位表示“大于”条件
整数，第 22\ :sup:`nd` 位表示“大于”条件
漂浮。

.. code-block:: text

  def ICC_NE  : ICC_VAL< 9>;  // Not Equal
  def ICC_E   : ICC_VAL< 1>;  // Equal
  def ICC_G   : ICC_VAL<10>;  // Greater
  ...
  def FCC_U   : FCC_VAL<23>;  // Unordered
  def FCC_G   : FCC_VAL<22>;  // Greater
  def FCC_UG  : FCC_VAL<21>;  // Unordered or Greater
  ...

（请注意，``Sparc.h`` 还定义了对应于相同 SPARC 的枚举
条件代码。  必须小心确保“Sparc.h”中的值
对应于``SparcInstrInfo.td``中的值。  即“SPCC::ICC_NE = 9”，
``SPCC::FCC_U = 23`` 等等。）

指令操作数映射
---------------------------

代码生成器后端将指令操作数映射到
指示。  每当指令编码“Inst”中的一位被分配时
没有具体值的字段，来自“outs”或“ins”列表的操作数
预计会有一个匹配的名称。然后该操作数填充未定义的
场。例如，Sparc 目标将“XNORrr”指令定义为
``F3_1`` 格式指令具有三个操作数：输出 ``$rd`` 和
输入“$rs1”和“$rs2”。

.. code-block:: text

  def XNORrr  : F3_1<2, 0b000111,
                     (outs IntRegs:$rd), (ins IntRegs:$rs1, IntRegs:$rs2),
                     "xnor $rs1, $rs2, $rd",
                     [(set i32:$rd, (not (xor i32:$rs1, i32:$rs2)))]>;

“SparcInstrFormats.td”中的指令模板显示了
``F3_1`` 是``InstSP``。

.. code-block:: text

  class InstSP<dag outs, dag ins, string asmstr, list<dag> pattern> : Instruction {
    field bits<32> Inst;
    let Namespace = "SP";
    bits<2> op;
    let Inst{31-30} = op;
    dag OutOperandList = outs;
    dag InOperandList = ins;
    let AsmString   = asmstr;
    let Pattern = pattern;
  }

``InstSP`` 定义了 ``op`` 字段，并使用它来定义位 30 和 31
指令，但不为其赋值。

.. code-block:: text

  class F3<dag outs, dag ins, string asmstr, list<dag> pattern>
      : InstSP<outs, ins, asmstr, pattern> {
    bits<5> rd;
    bits<6> op3;
    bits<5> rs1;
    let op{1} = 1;   // Op = 2 or 3
    let Inst{29-25} = rd;
    let Inst{24-19} = op3;
    let Inst{18-14} = rs1;
  }

``F3`` 定义了 ``rd``、``op3`` 和 ``rs1`` 字段，并在
指令，并且再次不赋值。

.. code-block:: text

  class F3_1<bits<2> opVal, bits<6> op3val, dag outs, dag ins,
             string asmstr, list<dag> pattern> : F3<outs, ins, asmstr, pattern> {
    bits<8> asi = 0; // asi not currently used
    bits<5> rs2;
    let op         = opVal;
    let op3        = op3val;
    let Inst{13}   = 0;     // i field = 0
    let Inst{12-5} = asi;   // address space identifier
    let Inst{4-0}  = rs2;
  }

``F3_1`` 为 ``op`` 和 ``op3`` 字段赋值，并定义 ``rs2``
场。  因此，“F3_1”格式指令将需要定义
``rd``、``rs1`` 和 ``rs2`` 以完全指定指令编码。

然后，“XNORrr”指令在其指令中提供这三个操作数。
OutOperandList 和 InOperandList，绑定到相应的字段，以及
从而完成指令编码。

对于某些指令，单个操作数可能包含子操作数。如图所示
早些时候，指令“LDrr”使用类型为“MEMrr”的输入操作数。这个
操作数类型包含两个寄存器子操作数，由
“MIOperandInfo” 值为“(ops IntRegs, IntRegs)”。

.. code-block:: text

  def LDrr : F3_1 <3, 0b000000, (outs IntRegs:$rd), (ins (MEMrr $rs1, $rs2):$addr),
                   "ld [$addr], $dst",
                   [(set i32:$dst, (load ADDRrr:$addr))]>;

由于该指令也是“F3_1”格式，因此它期望操作数名为
还有“rd”、“rs1”和“rs2”。为了实现这一点，需要一个复杂的操作数
可以选择为其每个子操作数命名。在这个例子中
“MEMrr”的第一个子操作数名为“$rs1”，第二个子操作数名为“$rs2”，
整个操作数也被命名为“$addr”。

当特定指令未使用该指令的所有操作数时
格式定义时，常数值可以改为绑定到一个或全部。对于
例如，“RDASR”指令只需要一个寄存器操作数，所以我们
将常量零分配给``rs2``：

.. code-block:: text

  let rs2 = 0 in
    def RDASR : F3_1<2, 0b101000,
                     (outs IntRegs:$rd), (ins ASRRegs:$rs1),
                     "rd $rs1, $rd", []>;

指令操作数名称映射
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TableGen 还将生成一个名为 getNamedOperandIdx() 的函数，该函数
可用于根据其在 MachineInstr 中查找操作数的索引
TableGen 名称。  设置指令中的 UseNamedOperandTable 位
TableGen 定义会将其所有操作数添加到枚举中
llvm::XXX:OpName 命名空间，并将其条目添加到 OperandMap 中
表，可以使用 getNamedOperandIdx() 查询

.. code-block:: text

  int DstIndex = SP::getNamedOperandIdx(SP::XNORrr, SP::OpName::dst); // => 0
  int BIndex = SP::getNamedOperandIdx(SP::XNORrr, SP::OpName::b);     // => 1
  int CIndex = SP::getNamedOperandIdx(SP::XNORrr, SP::OpName::c);     // => 2
  int DIndex = SP::getNamedOperandIdx(SP::XNORrr, SP::OpName::d);     // => -1

  ...

OpName 枚举中的条目逐字取自 TableGen 定义，
因此具有小写名称的操作数在枚举中将具有小写条目。

要在后端包含 getNamedOperandIdx() 函数，您需要
在 XXXInstrInfo.cpp 和 XXXInstrInfo.h 中定义一些预处理器宏。
例如：

XXXInstrInfo.cpp：

.. code-block:: c++

  #define GET_INSTRINFO_NAMED_OPS // For getNamedOperandIdx() function
  #include "XXXGenInstrInfo.inc"

XXXInstrInfo.h：

.. code-block:: c++

  #define GET_INSTRINFO_OPERAND_ENUM // For OpName enum
  #include "XXXGenInstrInfo.inc"

  namespace XXX {
    int16_t getNamedOperandIdx(uint16_t Opcode, uint16_t NamedIndex);
  } // End namespace XXX

指令操作数类型
^^^^^^^^^^^^^^^^^^^^^^^^^

TableGen 还将生成一个由所有命名操作数组成的枚举
在后端的 llvm::XXX::OpTypes 命名空间中定义的类型。
一些常见的立即操作数类型（例如 i8、i32、i64、f32、f64）
为“include/llvm/Target/Target.td”中的所有目标定义，并且是
在每个 Target 的 OpTypes 枚举中可用。  此外，仅出现命名的操作数类型
在枚举中：忽略匿名类型。
例如，X86后端定义了``brtarget``和``brtarget8``，两者都
TableGen``Operand`` 类的实例，代表分支目标
操作数：

.. code-block:: text

  def brtarget : Operand<OtherVT>;
  def brtarget8 : Operand<OtherVT>;

这导致：

.. code-block:: c++

  namespace X86 {
  namespace OpTypes {
  enum OperandType {
    ...
    brtarget,
    brtarget8,
    ...
    i32imm,
    i64imm,
    ...
    OPERAND_TYPE_LIST_END
  } // End namespace OpTypes
  } // End namespace X86

在典型的 TableGen 方式中，要使用枚举，您需要定义一个
预处理器宏：

.. code-block:: c++

  #define GET_INSTRINFO_OPERAND_TYPES_ENUM // For OpTypes enum
  #include "XXXGenInstrInfo.inc"


指令调度
----------------------

可以使用 MCDesc::getSchedClass() 查询指令行程。的
值可以通过生成的 llvm::XXX::Sched 命名空间中的枚举来命名
由 XXXGenInstrInfo.inc 中的 TableGen 提供。时间表课程的名称是
与 XXXSchedule.td 中提供的相同，加上默认的 NoItinerary 类。

调度模型由 TableGen 通过 SubtargetEmitter 生成，
使用“CodeGenSchedModels”类。这与行程不同
指定机器资源使用的方法。  工具``utils/schedcover.py``
可用于确定哪些指令已被覆盖
时间表模型描述和哪些没有。第一步是使用
下面的说明创建输出文件。然后运行“schedcover.py”
输出文件：

.. code-block:: shell

  $ <src>/utils/schedcover.py <build>/lib/Target/AArch64/tblGenSubtarget.with
  instruction, default, CortexA53Model, CortexA57Model, CycloneModel, ExynosM3Model, FalkorModel, KryoModel, ThunderX2T99Model, ThunderXT8XModel
  ABSv16i8, WriteV, , , CyWriteV3, M3WriteNMISC1, FalkorWr_2VXVY_2cyc, KryoWrite_2cyc_XY_XY_150ln, ,
  ABSv1i64, WriteV, , , CyWriteV3, M3WriteNMISC1, FalkorWr_1VXVY_2cyc, KryoWrite_2cyc_XY_noRSV_67ln, ,
  ...

要捕获生成调度模型的调试输出，请更改为
适当的目标目录并使用以下命令：
带有“subtarget-emitter”调试选项的命令：

.. code-block:: shell

  $ <build>/bin/llvm-tblgen -debug-only=subtarget-emitter -gen-subtarget \
    -I <src>/lib/Target/<target> -I <src>/include \
    -I <src>/lib/Target <src>/lib/Target/<target>/<target>.td \
    -o <build>/lib/Target/<target>/<target>GenSubtargetInfo.inc.tmp \
    > tblGenSubtarget.dbg 2>&1

其中``<build>``是构建目录，``src``是源目录，
``<target>`` 是目标的名称。
要仔细检查上述命令是否是所需要的，可以捕获
使用以下命令从构建中获取精确的 TableGen 命令：

.. code-block:: shell

  $ VERBOSE=1 make ...

并在输出中搜索“llvm-tblgen”命令。


指令关系映射
----------------------------

此 TableGen 功能用于将指令相互关联。  它是
当您有多种指令格式并且需要时特别有用
选择指令后在它们之间进行切换。  整个功能是驱动的
通过可以在“XXXInstrInfo.td”文件中定义的关系模型
根据目标特定指令集。  定义关系模型
使用“InstrMapping”类作为基础。  TableGen解析所有模型
并使用指定的信息生成指令关系图。
关系映射作为“XXXGenInstrInfo.inc”文件中的表发出
以及查询它们的函数。  有关如何进行的详细信息
使用此功能，请参考:doc:`HowToUseInstrMappings`。

实现“TargetInstrInfo”的子类
-------------------------------------------

最后一步是传递“XXXInstrInfo”的代码部分，它实现了
“TargetInstrInfo.h” 中描述的接口（请参阅：ref:`TargetInstrInfo`）。
这些函数返回“0”或布尔值或断言，除非被覆盖。
以下是 SPARC 实现中覆盖的函数列表
“SparcInstrInfo.cpp”：

* ``isLoadFromStackSlot`` --- 如果指定的机器指令是直接
  load from a stack slot, return the register number of the destination and the
  ``FrameIndex`` of the stack slot.

* ``isStoreToStackSlot`` --- 如果指定的机器指令是直接
  store to a stack slot, return the register number of the destination and the
  ``FrameIndex`` of the stack slot.

* ``copyPhysReg`` --- 在一对物理寄存器之间复制值。

* ``storeRegToStackSlot`` --- 将寄存器值存储到堆栈槽中。

* ``loadRegFromStackSlot`` --- 从堆栈槽加载寄存器值。

* ``storeRegToAddr`` --- 将寄存器值存储到内存中。

* ``loadRegFromAddr`` --- 从内存加载寄存器值。

* ``foldMemoryOperand`` --- 尝试组合任何加载或操作的指令
  store instruction for the specified operand(s).

分支折叠和 If 转换
--------------------------------

可以通过组合指令或消除指令来提高性能
从未达到的指令。  中的“analyzeBranch”方法
``XXXInstrInfo`` 可以用来检查条件指令和
删除不必要的说明。  ``analyzeBranch`` 查看 a 的末尾
机器基本块（MBB）以提供改进机会，例如分支
折叠和 if 转换。  “BranchFolder” 和 “IfConverter” 机器
函数传递（参见源文件“BranchFolding.cpp”和
``lib/CodeGen`` 目录中的 ``IfConversion.cpp`` ）调用 ``analyzeBranch``
改进表示指令的控制流图。

“analyzeBranch”（适用于 ARM、Alpha 和 X86）的几种实现可以是
作为您自己的“analyzeBranch”实现的模型进行检查。  自 SPARC
没有实现有用的“analyzeBranch”，ARM 目标实现是
如下所示。

``analyzeBranch`` 返回一个布尔值并采用四个参数：

* ``MachineBasicBlock &MBB`` --- 要检查的传入块。

* ``MachineBasicBlock *&TBB`` --- 返回的目标块。  对于一个
  conditional branch that evaluates to true, ``TBB`` is the destination.

* ``MachineBasicBlock *&FBB`` --- 对于计算结果为的条件分支
  false, ``FBB`` is returned as the destination.

* ``std::vector<MachineOperand> &Cond`` --- 计算 a 的操作数列表
  condition for a conditional branch.

在最简单的情况下，如果一个块结束时没有分支，那么它就会失败
到后继块。  没有为“TBB”指定目标块
或 ``FBB``，因此两个参数都返回 ``NULL``。  的开始
``analyzeBranch``（请参阅下面的 ARM 目标代码）显示该函数
参数和最简单情况的代码。

.. code-block:: c++

  bool ARMInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *&TBB,
                                   MachineBasicBlock *&FBB,
                                   std::vector<MachineOperand> &Cond) const
  {
    MachineBasicBlock::iterator I = MBB.end();
    if (I == MBB.begin() || !isUnpredicatedTerminator(--I))
      return false;

如果块以单个无条件分支指令结束，则
``analyzeBranch``（如下所示）应该返回该分支的目的地
“TBB”参数。

.. code-block:: c++

    if (LastOpc == ARM::B || LastOpc == ARM::tB) {
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    }

如果一个块以两个无条件分支结束，那么第二个分支是
从未达到。  在这种情况下，如下所示，删除最后一个分支
指令并返回“TBB”参数中的倒数第二个分支。

.. code-block:: c++

    if ((SecondLastOpc == ARM::B || SecondLastOpc == ARM::tB) &&
        (LastOpc == ARM::B || LastOpc == ARM::tB)) {
      TBB = SecondLastInst->getOperand(0).getMBB();
      I = LastInst;
      I->eraseFromParent();
      return false;
    }

一个块可能以一条失败的条件分支指令结束
如果条件评估为 false，则转到后继块。  在那种情况下，
“analyzeBranch”（如下所示）应该返回该分支的目的地
“TBB”参数中的条件分支和操作数列表
``Cond`` 参数来评估条件。

.. code-block:: c++

    if (LastOpc == ARM::Bcc || LastOpc == ARM::tBcc) {
      // Block ends with fall-through condbranch.
      TBB = LastInst->getOperand(0).getMBB();
      Cond.push_back(LastInst->getOperand(1));
      Cond.push_back(LastInst->getOperand(2));
      return false;
    }

如果一个块以条件分支和随后的无条件分支结束
分支，然后``analyzeBranch``（如下所示）应该返回条件
分支目的地（假设它对应于条件评估
“TBB`` 参数中的“``true``”）和无条件分支目标
在“FBB”中（对应于“false”的条件评估）。  一个
用于评估条件的操作数列表应在“Cond”中返回
参数。

.. code-block:: c++

    unsigned SecondLastOpc = SecondLastInst->getOpcode();

    if ((SecondLastOpc == ARM::Bcc && LastOpc == ARM::B) ||
        (SecondLastOpc == ARM::tBcc && LastOpc == ARM::tB)) {
      TBB =  SecondLastInst->getOperand(0).getMBB();
      Cond.push_back(SecondLastInst->getOperand(1));
      Cond.push_back(SecondLastInst->getOperand(2));
      FBB = LastInst->getOperand(0).getMBB();
      return false;
    }

对于最后两种情况（以单个条件分支结束或以
一个条件分支和一个无条件分支），操作数在
``Cond`` 参数可以传递给其他指令的方法来创建新的
分支或执行其他操作。  ``analyzeBranch`` 的实现
需要辅助方法``removeBranch``和``insertBranch``来管理
后续操作。

在大多数情况下，``analyzeBranch`` 应返回 false 表示成功。
``analyzeBranch`` 仅当该方法对什么感到困惑时才应返回 true
例如，如果一个块具有三个终止分支。
如果“analyzeBranch”遇到它不能遇到的终止符，它可能会返回 true
句柄，例如间接分支。

.. _instruction-selector:

指令选择器
====================

LLVM 使用“SelectionDAG”来表示 LLVM IR 指令，以及
“SelectionDAG” 理想地代表本机目标指令。  编码期间
生成，执行指令选择传递以转换非本地
DAG 指令转换为本机特定于目标的指令。  所描述的通行证
``XXXISelDAGToDAG.cpp`` 中用于匹配模式并执行 DAG-to-DAG
指令选择。  可选地，可以定义一个通行证（在
``XXXBranchSelector.cpp``) 为分支执行类似的 DAG 到 DAG 操作
说明。  后来，``XXXISelLowering.cpp``中的代码替换或删除
本地不支持（合法化）的操作和数据类型
“选择DAG”。

TableGen 使用以下目标生成用于指令选择的代码
描述输入文件：

* ``XXXInstrInfo.td`` --- 包含指令的定义
  target-specific instruction set, generates ``XXXGenDAGISel.inc``, which is
  included in ``XXXISelDAGToDAG.cpp``.

* ``XXXCallingConv.td`` --- 包含调用和返回值约定
  for the target architecture, and it generates ``XXXGenCallingConv.inc``,
  which is included in ``XXXISelLowering.cpp``.

指令选择过程的实现必须包含一个标头
声明“FunctionPass”类或“FunctionPass”的子类。  在
“XXXTargetMachine.cpp”，通行证管理器 (PM) 应添加每条指令
选择传递到要运行的传递队列中。

LLVM 静态编译器（``llc``）是一个用于可视化
DAG 的内容。  在特定之前或之后显示``SelectionDAG``
处理阶段，使用“llc”的命令行选项，描述于
:ref:`SelectionDAG 流程`。

为了描述指令选择器的行为，您应该添加降低模式
LLVM 代码转换为“SelectionDAG”作为指令的最后一个参数
定义在``XXXInstrInfo.td``中。  例如，在“SparcInstrInfo.td”中，
该条目定义了寄存器存储操作，最后一个参数描述了
具有存储 DAG 运算符的模式。

.. code-block:: text

  def STrr  : F3_1< 3, 0b000100, (outs), (ins MEMrr:$addr, IntRegs:$src),
                   "st $src, [$addr]", [(store i32:$src, ADDRrr:$addr)]>;

“ADDRrr” 是一种内存模式，也在“SparcInstrInfo.td” 中定义：

.. code-block:: text

  def ADDRrr : ComplexPattern<i32, 2, "SelectADDRrr", [], []>;

“ADDRrr”的定义引用了“SelectADDRrr”，它是一个函数
在讲师选择器的实现中定义（例如
``SparcISelDAGToDAG.cpp``）。

在“lib/Target/TargetSelectionDAG.td”中，定义了存储的 DAG 运算符
下面：

.. code-block:: text

  def store : PatFrag<(ops node:$val, node:$ptr),
                      (unindexedstore node:$val, node:$ptr)> {
    let IsStore = true;
    let IsTruncStore = false;
  }

``XXXInstrInfo.td`` 还生成（在 ``XXXGenDAGISel.inc`` 中）
用于调用适当处理方法的``SelectCode``方法
以获得指示。  在此示例中，``SelectCode`` 调用``Select_ISD_STORE``
对于“ISD::STORE”操作码。

.. code-block:: c++

  SDNode *SelectCode(SDValue N) {
    ...
    MVT::ValueType NVT = N.getNode()->getValueType(0);
    switch (N.getOpcode()) {
    case ISD::STORE: {
      switch (NVT) {
      default:
        return Select_ISD_STORE(N);
        break;
      }
      break;
    }
    ...

“STrr” 的模式是匹配的，因此在“XXXGenDAGISel.inc” 的其他地方，
为“Select_ISD_STORE”创建“STrr”的代码。  “Emit_22”方法
也在``XXXGenDAGISel.inc``中生成来完成这个的处理
指示。

.. code-block:: c++

  SDNode *Select_ISD_STORE(const SDValue &N) {
    SDValue Chain = N.getOperand(0);
    if (Predicate_store(N.getNode())) {
      SDValue N1 = N.getOperand(1);
      SDValue N2 = N.getOperand(2);
      SDValue CPTmp0;
      SDValue CPTmp1;

      // Pattern: (st:void i32:i32:$src,
      //           ADDRrr:i32:$addr)<<P:Predicate_store>>
      // Emits: (STrr:void ADDRrr:i32:$addr, IntRegs:i32:$src)
      // Pattern complexity = 13  cost = 1  size = 0
      if (SelectADDRrr(N, N2, CPTmp0, CPTmp1) &&
          N1.getNode()->getValueType(0) == MVT::i32 &&
          N2.getNode()->getValueType(0) == MVT::i32) {
        return Emit_22(N, SP::STrr, CPTmp0, CPTmp1);
      }
  ...

SelectionDAG 合法化阶段
-------------------------------

合法化阶段将 DAG 转换为使用本机的类型和操作
得到目标的支持。  对于本机不支持的类型和操作，您
需要将代码添加到特定于目标的“XXXTargetLowering”实现中
将不支持的类型和操作转换为支持的类型和操作。

在 XXXTargetLowering 类的构造函数中，首先使用
``addRegisterClass`` 方法指定支持哪些类型以及哪些
寄存器类与它们相关联。  寄存器类的代码
由 TableGen 从“XXXRegisterInfo.td”生成并放置在
“XXXGenRegisterInfo.h.inc”。  例如，实施
SparcTargetLowering 类的构造函数（在“SparcISelLowering.cpp”中）
从以下代码开始：

.. code-block:: c++

  addRegisterClass(MVT::i32, SP::IntRegsRegisterClass);
  addRegisterClass(MVT::f32, SP::FPRegsRegisterClass);
  addRegisterClass(MVT::f64, SP::DFPRegsRegisterClass);

您应该检查“ISD”命名空间中的节点类型
(``include/llvm/CodeGen/SelectionDAGNodes.h``) 并确定哪些操作
目标本身支持。  对于**没有**具有本机的操作
支持，向 XXXTargetLowering 类的构造函数添加回调，
所以指令选择过程知道要做什么。  “目标降低”
类回调方法（在“llvm/Target/TargetLowering.h”中声明）是：

* ``setOperationAction`` --- 一般操作。
* ``setLoadExtAction`` --- 加载扩展。
* ``setTruncStoreAction`` --- 截断存储。
* ``setIndexedLoadAction`` --- 索引加载。
* ``setIndexedStoreAction`` --- 索引存储。
* ``setConvertAction`` --- 类型转换。
* ``setCondCodeAction`` --- 支持给定的条件代码。

注意：在旧版本中，使用“setLoadXAction”而不是
“setLoadExtAction”。  另外，在旧版本中，“setCondCodeAction”可能不会
得到支持。  检查您的版本以了解具体有哪些方法
支持。

这些回调用于确定操作是否有效
具有指定的类型（或多个类型）。  在所有情况下，第三个参数都是
``LegalAction`` 类型枚举值：``Promote``、``Expand``、``Custom`` 或
“合法”。  “SparcISelLowering.cpp”包含所有四个的示例
“法律行动”值。

推动
^^^^^^^

对于没有对给定类型的本机支持的操作，指定的类型
可以升级为受支持的更大类型。  例如，SPARC 确实
不支持布尔值（“i1”类型）的符号扩展加载，因此
“SparcISelLowering.cpp”下面的第三个参数“Promote”发生变化
加载之前将``i1`` 将值键入为大类型。

.. code-block:: c++

  setLoadExtAction(ISD::SEXTLOAD, MVT::i1, Promote);

扩张
^^^^^^

对于没有本机支持的类型，可能需要进一步细分值，
而不是晋升。  对于没有本机支持的操作，组合
可以使用其他操作来达到类似的效果。  在 SPARC 中，
浮点正弦和余弦三角运算通过扩展支持
其他操作，如第三个参数“Expand”所示，以
“设置操作动作”：

.. code-block:: c++

  setOperationAction(ISD::FSIN, MVT::f32, Expand);
  setOperationAction(ISD::FCOS, MVT::f32, Expand);

风俗
^^^^^^

对于某些操作，简单的类型提升或操作扩展可能是
不足。  在某些情况下，必须实现特殊的内在函数。

例如，一个常数值可能需要特殊处理，或者一个操作
可能需要溢出和恢复堆栈中的寄存器并使用
寄存器分配器。

如下面的“SparcISelLowering.cpp”代码所示，执行类型转换
从浮点值到有符号整数，首先
应使用“Custom”作为第三个参数来调用“setOperationAction”：

.. code-block:: c++

  setOperationAction(ISD::FP_TO_SINT, MVT::i32, Custom);

在“LowerOperation”方法中，对于每个“Custom”操作，都有一个案例
应添加语句以指示要调用什么函数。  在下文中
代码中，“FP_TO_SINT”操作码将调用“LowerFP_TO_SINT”方法：

.. code-block:: c++

  SDValue SparcTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) {
    switch (Op.getOpcode()) {
    case ISD::FP_TO_SINT: return LowerFP_TO_SINT(Op, DAG);
    ...
    }
  }

最后，实现了“LowerFP_TO_SINT”方法，使用 FP 寄存器来
将浮点值转换为整数。

.. code-block:: c++

  static SDValue LowerFP_TO_SINT(SDValue Op, SelectionDAG &DAG) {
    assert(Op.getValueType() == MVT::i32);
    Op = DAG.getNode(SPISD::FTOI, MVT::f32, Op.getOperand(0));
    return DAG.getNode(ISD::BITCAST, MVT::i32, Op);
  }

合法的
^^^^^

``Legal`` ``LegalizeAction`` 枚举值仅指示操作
**是**本机支持的。  ``Legal`` 代表默认条件，所以它
很少使用。  在“SparcISelLowering.cpp”中，“CTPOP”的操作（
对整数中设置的位进行计数的操作）本身仅支持
SPARC v9。  以下代码启用“Expand”转换技术
non-v9 SPARC implementations.

.. code-block:: c++

  setOperationAction(ISD::CTPOP, MVT::i32, Expand);
  ...
  if (TM.getSubtarget<SparcSubtarget>().isV9())
    setOperationAction(ISD::CTPOP, MVT::i32, Legal);

调用约定
-------------------

为了支持特定于目标的调用约定，“XXXGenCallingConv.td”使用
定义在的接口（例如“CCIfType”和“CCAssignToReg”）
“lib/Target/TargetCallingConv.td”。  TableGen可以获取目标描述符
文件``XXXGenCallingConv.td``并生成头文件
“XXXGenCallingConv.inc”，通常包含在
“XXXISelLowering.cpp”。  您可以使用以下接口
``TargetCallingConv.td`` 指定：

* 参数分配的顺序。

* 参数和返回值放置的位置（即在堆栈上或在
  registers).

* 可以使用哪些寄存器。

* 调用者或被调用者是否展开堆栈。

以下示例演示了“CCIfType”和“CCIfType”的使用
``CCAssignToReg`` 接口。  如果“CCIfType”谓词为真（即
如果当前参数的类型为“f32”或“f64”），则操作为
执行。  在这种情况下，“CCAssignToReg”操作分配参数
第一个可用寄存器的值：“R0”或“R1”。

.. code-block:: text

  CCIfType<[f32,f64], CCAssignToReg<[R0, R1]>>

“SparcCallingConv.td”包含特定于目标的返回值的定义
调用约定 (``RetCC_Sparc32``) 和基本的 32 位 C 调用约定
（``CC_Sparc32``）。  “RetCC_Sparc32”的定义（如下所示）表示
哪些寄存器用于指定的标量返回类型。  单精度
浮点数返回到寄存器“F0”，双精度浮点数返回到
注册“D0”。  32 位整数在寄存器“I0”或“I1”中返回。

.. code-block:: text

  def RetCC_Sparc32 : CallingConv<[
    CCIfType<[i32], CCAssignToReg<[I0, I1]>>,
    CCIfType<[f32], CCAssignToReg<[F0]>>,
    CCIfType<[f64], CCAssignToReg<[D0]>>
  ]>;

“SparcCallingConv.td”中“CC_Sparc32”的定义介绍了
``CCAssignToStack``，将值分配给指定的堆栈槽
尺寸和对齐方式。  在下面的示例中，第一个参数 4 表示
槽的大小，第二个参数也是4，表示栈
沿 4 字节单元对齐。  （特殊情况：如果大小为零，则 ABI
使用尺寸；如果对齐为零，则使用 ABI 对齐。）

.. code-block:: text

  def CC_Sparc32 : CallingConv<[
    // All arguments get passed in integer registers if there is space.
    CCIfType<[i32, f32, f64], CCAssignToReg<[I0, I1, I2, I3, I4, I5]>>,
    CCAssignToStack<4, 4>
  ]>;

``CCDelegateTo`` 是另一个常用的接口，它试图找到一个
指定的子调用约定，如果找到匹配，则调用它。  在
下面的例子（在“X86CallingConv.td”中），定义
“RetCC_X86_32_C”以“CCDelegateTo”结尾。  当前值变为后
分配给寄存器“ST0”或“ST1”，“RetCC_X86Common”是
调用。

.. code-block:: text

  def RetCC_X86_32_C : CallingConv<[
    CCIfType<[f32], CCAssignToReg<[ST0, ST1]>>,
    CCIfType<[f64], CCAssignToReg<[ST0, ST1]>>,
    CCDelegateTo<RetCC_X86Common>
  ]>;

``CCIfCC`` 是一个尝试将给定名称与当前名称相匹配的接口
调用约定。  如果名称标识当前的调用约定，
然后调用指定的操作。  在以下示例中（在
“X86CallingConv.td”），如果使用“Fast”调用约定，则
``RetCC_X86_32_Fast`` 被调用。  如果“SSECall”调用约定是
使用，然后调用``RetCC_X86_32_SSE``。

.. code-block:: text

  def RetCC_X86_32 : CallingConv<[
    CCIfCC<"CallingConv::Fast", CCDelegateTo<RetCC_X86_32_Fast>>,
    CCIfCC<"CallingConv::X86_SSECall", CCDelegateTo<RetCC_X86_32_SSE>>,
    CCDelegateTo<RetCC_X86_32_C>
  ]>;

``CCAssignToRegAndStack`` 与 ``CCAssignToReg`` 相同，但也分配
当使用某些寄存器时，堆栈槽。基本上，它的工作原理如下：
``CCIf <CCAssignToReg <regList>，CCAssignToStack <大小，对齐>>``。

.. code-block:: text

  class CCAssignToRegAndStack<list<Register> regList, int size, int align>
      : CCAssignToReg<regList> {
    int Size = size;
    int Align = align;
  }

其他调用约定接口包括：

* ``CCIf <predicate, action>`` --- 如果谓词匹配，则应用该操作。

* ``CCIfInReg <action>`` --- 如果参数标有“``inreg``”
  attribute, then apply the action.

* ``CCIfNest <action>`` --- 如果参数标有“``nest``”
  attribute, then apply the action.

* ``CCIfNotVarArg <action>`` --- 如果当前函数不接受
  variable number of arguments, apply the action.

* ``CCAssignToRegWithShadow <registerList, ShadowList>`` --- 类似于
  ``CCAssignToReg``, but with a shadow list of registers.

* ``CCPassByVal <size,align>`` --- 将值分配给堆栈槽
  minimum specified size and alignment.

* ``CCPromoteToType <type>`` --- 将当前值提升为指定值
  type.

* ``CallingConv <[actions]>`` --- 定义每个调用约定
  supported.

装配打印机
================

在代码发射阶段，代码生成器可以利用 LLVM 传递来
产生装配输出。  为此，您需要实现以下代码：
可将 LLVM IR 转换为目标的 GAS 格式汇编语言的打印机
机，使用以下步骤：

* 定义目标的所有汇编字符串，将它们添加到
  instructions defined in the ``XXXInstrInfo.td`` file.  (See
  :ref:`instruction-set`.)  TableGen will produce an output file
  (``XXXGenAsmWriter.inc``) with an implementation of the ``printInstruction``
  method for the ``XXXAsmPrinter`` class.

* 编写“XXXTargetAsmInfo.h”，其中包含以下内容的基本声明
  the ``XXXTargetAsmInfo`` class (a subclass of ``TargetAsmInfo``).

* 编写“XXXTargetAsmInfo.cpp”，其中包含特定于目标的值
  ``TargetAsmInfo`` properties and sometimes new implementations for methods.

* 编写``XXXAsmPrinter.cpp``，它实现了``AsmPrinter``类
  performs the LLVM-to-assembly conversion.

“XXXTargetAsmInfo.h”中的代码通常是一个简单的声明
用于“XXXTargetAsmInfo.cpp”中的“XXXTargetAsmInfo”类。  同样，
``XXXTargetAsmInfo.cpp`` 通常有一些 ``XXXTargetAsmInfo`` 的声明
覆盖“TargetAsmInfo.cpp”中默认值的替换值。
例如在“SparcTargetAsmInfo.cpp”中：

.. code-block:: c++

  SparcTargetAsmInfo::SparcTargetAsmInfo(const SparcTargetMachine &TM) {
    Data16bitsDirective = "\t.half\t";
    Data32bitsDirective = "\t.word\t";
    Data64bitsDirective = 0;  // .xword is only supported by V9.
    ZeroDirective = "\t.skip\t";
    CommentString = "!";
    ConstantPoolSection = "\t.section \".rodata\",#alloc\n";
  }

X86 汇编打印机实现 (``X86TargetAsmInfo``) 是一个示例
其中目标特定的``TargetAsmInfo``类使用重写的方法：
“展开内联汇编”。

``AsmPrinter`` 的特定于目标的实现是用
``XXXAsmPrinter.cpp``，它实现了转换的``AsmPrinter``类
LLVM 到可打印的程序集。  实施必须包括以下内容
具有“AsmPrinter”声明的标头和
“MachineFunctionPass” 类。  “MachineFunctionPass” 是一个子类
“功能通行证”。

.. code-block:: c++

  #include "llvm/CodeGen/AsmPrinter.h"
  #include "llvm/CodeGen/MachineFunctionPass.h"

作为``FunctionPass``，``AsmPrinter`` 首先调用``doInitialization`` 来设置
打开“AsmPrinter”。  在“SparcAsmPrinter”中，“Mangler”对象是
实例化以处理变量名称。

在“XXXAsmPrinter.cpp”中，“runOnMachineFunction”方法（在
必须为“XXXAsmPrinter”实现“MachineFunctionPass”。  在
“MachineFunctionPass”，“runOnFunction”方法调用
“runOnMachineFunction”。  针对特定目标的实施
``runOnMachineFunction`` 有所不同，但通常执行以下操作来处理每个
机器功能：

* 调用``SetupMachineFunction``来执行初始化。

* 调用``EmitConstantPool`` 打印出（到输出流）常量
  have been spilled to memory.

* 调用``EmitJumpTableInfo``打印出当前使用的跳转表
  function.

* 打印出当前函数的标签。

* 打印出函数的代码，包括基本块标签和
  assembly for the instruction (using ``printInstruction``)

``XXXAsmPrinter`` 实现还必须包含由
TableGen 在``XXXGenAsmWriter.inc`` 文件中输出。  代码在
“XXXGenAsmWriter.inc”包含“printInstruction”的实现
可以调用这些方法的方法：

*``打印操作数``
*``printMemOperand``
*``printCCOperand``（用于条件语句）
*``打印数据指令``
*``打印声明``
*``printImplicitDef``
*``printInlineAsm``

“printDeclare”、“printImplicitDef” 的实现，
“AsmPrinter.cpp”中的“printInlineAsm”和“printLabel”通常是
足以打印组件并且不需要被覆盖。

``printOperand`` 方法是用长``switch``/``case`` 实现的
操作数类型声明：寄存器、立即数、基本块、外部
符号、全局地址、常量池索引或跳转表索引。  对于一个
带有内存地址操作数的指令，“printMemOperand”方法
应实施以产生正确的输出。  同样，
``printCCOperand`` 应该用于打印条件操作数。

``doFinalization`` 应该在 ``XXXAsmPrinter`` 中被覆盖，并且应该是
调用关闭装配打印机。  在“doFinalization”期间，全局
变量和常量被打印到输出。

子目标支持
=================

子目标支持用于通知指令的代码生成过程
为给定的芯片组设置变体。  例如，LLVM SPARC
提供的实现涵盖了 SPARC 微处理器的三个主要版本
架构：版本 8（V8，这是一个 32 位架构）、版本 9（V9，一个
64 位架构）和 UltraSPARC 架构。  V8有16个
双精度浮点寄存器也可用作 32
单精度或 8 个四精度寄存器。  V8 也是纯粹的大端字节序。
V9 有 32 个双精度浮点寄存器，也可用作 16 个
四精度寄存器，但不能用作单精度寄存器。
UltraSPARC 架构将 V9 与 UltraSPARC 视觉指令集相结合
扩展。

如果需要子目标支持，您应该实施特定于目标的
您的架构的 ``XXXSubtarget`` 类。  这个类应该处理
命令行选项``-mcpu=`` 和``-mattr=``。

TableGen 使用“Target.td”和“Sparc.td”文件中的定义来
在“SparcGenSubtarget.inc”中生成代码。  在“Target.td”中，如下所示，
定义了“SubtargetFeature”接口。  前 4 个字符串参数
``SubtargetFeature`` 接口是一个功能名称，一个 XXXSubtarget 字段集
由该功能、XXXSubtarget 字段的值以及该功能的描述组成
功能。  （第五个参数是暗示其存在的功能列表，
它的默认值是一个空数组。）

如果该字段的值为字符串“true”或“false”，则该字段
假定为 bool，并且只有一个 SubtargetFeature 应该引用它。
否则，假定它是一个整数。整数值可以是名称
一个枚举常量。如果多个特征使用相同的整数字段，则
字段将设置为共享的所有已启用功能的最大值
领域。

.. code-block:: text

  class SubtargetFeature<string n, string f, string v, string d,
                         list<SubtargetFeature> i = []> {
    string Name = n;
    string FieldName = f;
    string Value = v;
    string Desc = d;
    list<SubtargetFeature> Implies = i;
  }

在“Sparc.td”文件中，“SubtargetFeature”用于定义
以下功能。

.. code-block:: text

  def FeatureV9 : SubtargetFeature<"v9", "IsV9", "true",
                       "Enable SPARC-V9 instructions">;
  def FeatureV8Deprecated : SubtargetFeature<"deprecated-v8",
                       "UseV8DeprecatedInsts", "true",
                       "Enable deprecated V8 instructions in V9 mode">;
  def FeatureVIS : SubtargetFeature<"vis", "IsVIS", "true",
                       "Enable UltraSPARC Visual Instruction Set extensions">;

在“Sparc.td”的其他地方，定义了“Proc”类，然后用于
定义可能具有先前的特定 SPARC 处理器子类型
描述的特征。

.. code-block:: text

  class Proc<string Name, list<SubtargetFeature> Features>
    : Processor<Name, NoItineraries, Features>;

  def : Proc<"generic",         []>;
  def : Proc<"v8",              []>;
  def : Proc<"supersparc",      []>;
  def : Proc<"sparclite",       []>;
  def : Proc<"f934",            []>;
  def : Proc<"hypersparc",      []>;
  def : Proc<"sparclite86x",    []>;
  def : Proc<"sparclet",        []>;
  def : Proc<"tsc701",          []>;
  def : Proc<"v9",              [FeatureV9]>;
  def : Proc<"ultrasparc",      [FeatureV9, FeatureV8Deprecated]>;
  def : Proc<"ultrasparc3",     [FeatureV9, FeatureV8Deprecated]>;
  def : Proc<"ultrasparc3-vis", [FeatureV9, FeatureV8Deprecated, FeatureVIS]>;

从“Target.td”和“Sparc.td”文件中，得到的结果
``SparcGenSubtarget.inc`` 指定枚举值来标识特征，
表示 CPU 功能和 CPU 子类型的常量数组，以及
``ParseSubtargetFeatures`` method that parses the features string that sets
指定的子目标选项。  生成的“SparcGenSubtarget.inc”文件
应包含在“SparcSubtarget.cpp”中。  The target-specific
“XXXSubtarget”方法的实现应遵循以下伪代码：

.. code-block:: c++

  XXXSubtarget::XXXSubtarget(const Module &M, const std::string &FS) {
    // Set the default features
    // Determine default and user specified characteristics of the CPU
    // Call ParseSubtargetFeatures(FS, CPU) to parse the features string
    // Perform any additional operations
  }

即时支持
===========

目标机器的实现可选地包括即时（JIT）
以二进制形式发出机器代码和辅助结构的代码生成器
输出可以直接写入内存。  为此，请实施 JIT 代码
通过执行以下步骤生成：

* 编写一个包含机器函数pass的``XXXCodeEmitter.cpp``文件
  that transforms target-machine instructions into relocatable machine
  code.

* 编写一个“XXXJITInfo.cpp”文件来实现 JIT 接口
  target-specific code-generation activities, such as emitting machine code and
  stubs.

* 修改“XXXTargetMachine”，使其提供“TargetJITInfo”对象
  through its ``getJITInfo`` method.

编写 JIT 支持代码有多种不同的方法。  对于
例如，TableGen 和目标描述符文件可用于创建 JIT
代码生成器，但不是强制性的。  对于 Alpha 和 PowerPC 目标
机器上，TableGen 用于生成``XXXGenCodeEmitter.inc``，其中
包含机器指令的二进制编码和
``getBinaryCodeForInstr`` 方法来访问这些代码。  其他即时生产
实现没有。

“XXXJITInfo.cpp”和“XXXCodeEmitter.cpp”都必须包含
``llvm/CodeGen/MachineCodeEmitter.h`` 头文件定义
“MachineCodeEmitter” 类包含多个回调函数的代码
将数据（以字节、字、字符串等形式）写入输出流。

机器代码发射器
--------------------

In ``XXXCodeEmitter.cpp``, a target-specific of the ``Emitter`` class is
作为函数 pass 实现（``MachineFunctionPass`` 的子类）。  的
``runOnMachineFunction`` 的特定于目标的实现（由调用
``runOnFunction`` in ``MachineFunctionPass``) iterates through the
``MachineBasicBlock`` calls ``emitInstruction`` to process each instruction and
emit binary code.  ``emitInstruction`` is largely implemented with case
statements on the instruction types defined in ``XXXInstrInfo.h``.  对于
example, in ``X86CodeEmitter.cpp``, the ``emitInstruction`` method is built
around the following ``switch``/``case`` statements:

.. code-block:: c++

  switch (Desc->TSFlags & X86::FormMask) {
  case X86II::Pseudo:  // for not yet implemented instructions
     ...               // or pseudo-instructions
     break;
  case X86II::RawFrm:  // for instructions with a fixed opcode value
     ...
     break;
  case X86II::AddRegFrm: // for instructions that have one register operand
     ...                 // added to their opcode
     break;
  case X86II::MRMDestReg:// for instructions that use the Mod/RM byte
     ...                 // to specify a destination (register)
     break;
  case X86II::MRMDestMem:// for instructions that use the Mod/RM byte
     ...                 // to specify a destination (memory)
     break;
  case X86II::MRMSrcReg: // for instructions that use the Mod/RM byte
     ...                 // to specify a source (register)
     break;
  case X86II::MRMSrcMem: // for instructions that use the Mod/RM byte
     ...                 // to specify a source (memory)
     break;
  case X86II::MRM0r: case X86II::MRM1r:  // for instructions that operate on
  case X86II::MRM2r: case X86II::MRM3r:  // a REGISTER r/m operand and
  case X86II::MRM4r: case X86II::MRM5r:  // use the Mod/RM byte and a field
  case X86II::MRM6r: case X86II::MRM7r:  // to hold extended opcode data
     ...
     break;
  case X86II::MRM0m: case X86II::MRM1m:  // for instructions that operate on
  case X86II::MRM2m: case X86II::MRM3m:  // a MEMORY r/m operand and
  case X86II::MRM4m: case X86II::MRM5m:  // use the Mod/RM byte and a field
  case X86II::MRM6m: case X86II::MRM7m:  // to hold extended opcode data
     ...
     break;
  case X86II::MRMInitReg: // for instructions whose source and
     ...                  // destination are the same register
     break;
  }

这些 case 语句的实现通常首先发出操作码并
然后获取操作数。  然后根据操作数，辅助方法可能
被调用来处理操作数。  例如，在“X86CodeEmitter.cpp”中，
对于“X86II::AddRegFrm”情况，发出的第一个数据（通过“emitByte”）是
添加到寄存器操作数的操作码。  然后是一个代表
提取机器操作数“MO1”。  辅助方法如
“isImmediate”、“isGlobalAddress”、“isExternalSymbol”、
``isConstantPoolIndex`` 和 ``isJumpTableIndex`` 确定操作数类型。
（“X86CodeEmitter.cpp”也有私有方法，例如“emitConstant”，
“emitGlobalAddress”、“emitExternalSymbolAddress”、“emitConstPoolAddress”、
和将数据发送到输出流的``emitJumpTableAddress``。）

.. code-block:: c++

  case X86II::AddRegFrm:
    MCE.emitByte(BaseOpcode + getX86RegNum(MI.getOperand(CurOp++).getReg()));

    if (CurOp != NumOps) {
      const MachineOperand &MO1 = MI.getOperand(CurOp++);
      unsigned Size = X86InstrInfo::sizeOfImm(Desc);
      if (MO1.isImmediate())
        emitConstant(MO1.getImm(), Size);
      else {
        unsigned rt = Is64BitMode ? X86::reloc_pcrel_word
          : (IsPIC ? X86::reloc_picrel_word : X86::reloc_absolute_word);
        if (Opcode == X86::MOV64ri)
          rt = X86::reloc_absolute_dword;  // FIXME: add X86II flag?
        if (MO1.isGlobalAddress()) {
          bool NeedStub = isa<Function>(MO1.getGlobal());
          bool isLazy = gvNeedsLazyPtr(MO1.getGlobal());
          emitGlobalAddress(MO1.getGlobal(), rt, MO1.getOffset(), 0,
                            NeedStub, isLazy);
        } else if (MO1.isExternalSymbol())
          emitExternalSymbolAddress(MO1.getSymbolName(), rt);
        else if (MO1.isConstantPoolIndex())
          emitConstPoolAddress(MO1.getIndex(), rt);
        else if (MO1.isJumpTableIndex())
          emitJumpTableAddress(MO1.getIndex(), rt);
      }
    }
    break;

在前面的示例中，“XXXCodeEmitter.cpp”使用变量“rt”，该变量
是一个“RelocationType”枚举，可用于重新定位地址（例如
例如，具有 PIC 基址偏移的全局地址）。  “RelocationType” 枚举
该目标是在特定于目标的简短“XXXRelocations.h”中定义的
文件。  ``RelocationType`` 由中定义的``relocate`` 方法使用
``XXXJITInfo.cpp`` 重写引用的全局符号的地址。

例如，``X86Relocations.h`` 指定以下重定位类型
X86 地址。  在所有四种情况下，重定位的值都会添加到
值已经在内存中。  对于“reloc_pcrel_word”和“reloc_picrel_word”，
还有一个额外的初始调整。

.. code-block:: c++

  enum RelocationType {
    reloc_pcrel_word = 0,    // add reloc value after adjusting for the PC loc
    reloc_picrel_word = 1,   // add reloc value after adjusting for the PIC base
    reloc_absolute_word = 2, // absolute relocation; no additional adjustment
    reloc_absolute_dword = 3 // absolute relocation; no additional adjustment
  };

目标 JIT 信息
---------------

``XXXJITInfo.cpp`` 实现特定于目标的 JIT 接口
代码生成活动，例如发出机器代码和存根。  在
至少，目标特定版本的 XXXJITInfo 实现以下内容：

* ``getLazyResolverFunction`` --- 初始化 JIT，给目标一个
  function that is used for compilation.

* ``emitFunctionStub`` --- 返回具有指定地址的本机函数
  for a callback function.

* ``relocate`` --- 更改引用的全局变量的地址，基于
  relocation types.

* 回调函数是函数存根的包装器，在调用时使用
  real target is not initially known.

``getLazyResolverFunction`` 通常实现起来很简单。  它使得
传入参数作为全局“JITCompilerFunction”并返回
将使用函数包装器的回调函数。  对于 Alpha 目标
（在“AlphaJITInfo.cpp”中），“getLazyResolverFunction”实现是
简单地说：

.. code-block:: c++

  TargetJITInfo::LazyResolverFn AlphaJITInfo::getLazyResolverFunction(
                                              JITCompilerFn F) {
    JITCompilerFunction = F;
    return AlphaCompilationCallback;
  }

对于 X86 目标，``getLazyResolverFunction`` 实现有点
更复杂，因为它返回不同的回调函数
具有 SSE 指令和 XMM 寄存器的处理器。

回调函数最初保存并随后恢复被调用者寄存器
值、传入参数以及帧和返回地址。  回调
函数需要对寄存器或堆栈进行低级访问，因此通常
用汇编程序实现。
