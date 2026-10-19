# RISC-V 实际指令与伪指令对照

本文依据 `rvv/riscv-asm-manual/src/asm-manual.adoc` 中的标准伪指令表和
CSR 伪指令表整理。

## 1. 基本概念

- **实际指令（base instruction）**：ISA 为它规定了机器编码，CPU 可以直接解码执行。例如
  `add`、`addi`、`auipc`、`jalr`。
- **伪指令（pseudoinstruction）**：汇编器提供的便捷写法，没有独立的机器编码。汇编器把它
  转换为一条或多条实际指令。例如 `mv a0, a1` 转换为 `addi a0, a1, 0`。
- **汇编器指令/伪操作（directive）**：以 `.` 开头并控制汇编过程，例如 `.text`、`.word`、
  `.option`。它们既不是实际指令，也不是本文所说的伪指令。

同一个助记符可能因操作数形式不同而属于不同类别：

```asm
lw a0, 8(sp)       # 实际指令：I 型立即数寻址
lw a0, value       # 加载全局符号的伪指令，通常展开为 auipc + lw

jal ra, target     # 实际指令：显式给出 rd
jal target         # 伪指令：省略的 rd 默认为 ra

fence r, w         # 实际指令
fence              # 伪指令，展开为 fence iorw, iorw
```

下面的展开是规范规定的典型或等价展开。`li`、`la`、`call` 等指令的最终序列可能因
RV32/RV64、PIC 设置、可用扩展和链接器松弛优化而变化。

常用操作数记号：

| 记号 | 含义 |
|---|---|
| `rd` | 目的整数寄存器 |
| `rs`、`rs1`、`rs2`、`rt` | 源寄存器或临时工作寄存器 |
| `frd`、`frs`、`frt` | 浮点寄存器 |
| `vd`、`vs`、`va`、`vb` | 向量寄存器 |
| `x0`/`zero` | 恒为 0 的整数寄存器，写入被丢弃 |
| `x1`/`ra` | 返回地址寄存器 |

## 2. RISC-V 寄存器

RISC-V 的寄存器集合取决于实现的 ISA 扩展。基础整数 ISA 提供 32 个整数寄存器；F/D/Q
扩展使用 32 个浮点寄存器；V 扩展使用 32 个向量寄存器。RV32E 是例外，只提供
`x0-x15`。

### 2.1 整数寄存器 `x0-x31`

RV32 中每个整数寄存器宽 32 位，RV64 中宽 64 位。硬件编码使用 `x0-x31`；ABI 名称描述
寄存器在函数调用中的约定用途。

| 硬件名 | ABI 名称 | 约定用途 | 跨函数调用保存？ |
|---|---|---|---|
| `x0` | `zero` | 恒为 0；写入被忽略 | 不适用，不可修改 |
| `x1` | `ra` | 返回地址 | 否 |
| `x2` | `sp` | 栈指针 | 是 |
| `x3` | `gp` | 全局指针/平台专用 | 固定用途，不可分配 |
| `x4` | `tp` | 线程指针 | 固定用途，不可分配 |
| `x5` | `t0` | 临时寄存器 0 | 否 |
| `x6` | `t1` | 临时寄存器 1 | 否 |
| `x7` | `t2` | 临时寄存器 2 | 否 |
| `x8` | `s0`/`fp` | 保存寄存器 0，或帧指针 | 是 |
| `x9` | `s1` | 保存寄存器 1 | 是 |
| `x10` | `a0` | 参数 0、返回值 0 | 否 |
| `x11` | `a1` | 参数 1、返回值 1 | 否 |
| `x12` | `a2` | 参数 2 | 否 |
| `x13` | `a3` | 参数 3 | 否 |
| `x14` | `a4` | 参数 4 | 否 |
| `x15` | `a5` | 参数 5 | 否 |
| `x16` | `a6` | 参数 6 | 否 |
| `x17` | `a7` | 参数 7 | 否 |
| `x18` | `s2` | 保存寄存器 2 | 是 |
| `x19` | `s3` | 保存寄存器 3 | 是 |
| `x20` | `s4` | 保存寄存器 4 | 是 |
| `x21` | `s5` | 保存寄存器 5 | 是 |
| `x22` | `s6` | 保存寄存器 6 | 是 |
| `x23` | `s7` | 保存寄存器 7 | 是 |
| `x24` | `s8` | 保存寄存器 8 | 是 |
| `x25` | `s9` | 保存寄存器 9 | 是 |
| `x26` | `s10` | 保存寄存器 10 | 是 |
| `x27` | `s11` | 保存寄存器 11 | 是 |
| `x28` | `t3` | 临时寄存器 3 | 否 |
| `x29` | `t4` | 临时寄存器 4 | 否 |
| `x30` | `t5` | 临时寄存器 5 | 否 |
| `x31` | `t6` | 临时寄存器 6 | 否 |

调用者如需在函数调用后继续使用 `ra`、`a0-a7` 或 `t0-t6` 的原值，必须自行保存。
被调用函数使用 `s0-s11` 时必须保存并在返回前恢复。标准 ABI 程序不应修改 `gp` 和 `tp`。

RV32E 只实现 `x0-x15`。其 ILP32E 调用约定相应缩减为 `a0-a5` 六个参数寄存器、
`s0-s1` 两个保存寄存器和 `t0-t2` 三个临时寄存器。

### 2.2 程序计数器 `pc`

`pc` 保存当前执行位置，但它不属于 `x0-x31`，也没有通用整数寄存器编号。顺序执行、分支、
跳转、异常和中断会改变它。普通代码通常通过 `auipc` 获取当前 PC 相关值，通过 `jal`、
`jalr` 和分支指令改变控制流，不能把 `pc` 当作普通 `rd`/`rs` 直接读写。

### 2.3 浮点寄存器 `f0-f31`

浮点寄存器需要相应扩展。F、D、Q 分别支持 32、64、128 位浮点运算；寄存器的可用宽度由
目标 ISA 的最大浮点宽度 FLEN 决定。

| 硬件名 | ABI 名称 | 约定用途 | 硬件浮点 ABI 中保存？ |
|---|---|---|---|
| `f0` | `ft0` | 浮点临时寄存器 0 | 否 |
| `f1` | `ft1` | 浮点临时寄存器 1 | 否 |
| `f2` | `ft2` | 浮点临时寄存器 2 | 否 |
| `f3` | `ft3` | 浮点临时寄存器 3 | 否 |
| `f4` | `ft4` | 浮点临时寄存器 4 | 否 |
| `f5` | `ft5` | 浮点临时寄存器 5 | 否 |
| `f6` | `ft6` | 浮点临时寄存器 6 | 否 |
| `f7` | `ft7` | 浮点临时寄存器 7 | 否 |
| `f8` | `fs0` | 浮点保存寄存器 0 | 是* |
| `f9` | `fs1` | 浮点保存寄存器 1 | 是* |
| `f10` | `fa0` | 浮点参数/返回值 0 | 否 |
| `f11` | `fa1` | 浮点参数/返回值 1 | 否 |
| `f12` | `fa2` | 浮点参数 2 | 否 |
| `f13` | `fa3` | 浮点参数 3 | 否 |
| `f14` | `fa4` | 浮点参数 4 | 否 |
| `f15` | `fa5` | 浮点参数 5 | 否 |
| `f16` | `fa6` | 浮点参数 6 | 否 |
| `f17` | `fa7` | 浮点参数 7 | 否 |
| `f18` | `fs2` | 浮点保存寄存器 2 | 是* |
| `f19` | `fs3` | 浮点保存寄存器 3 | 是* |
| `f20` | `fs4` | 浮点保存寄存器 4 | 是* |
| `f21` | `fs5` | 浮点保存寄存器 5 | 是* |
| `f22` | `fs6` | 浮点保存寄存器 6 | 是* |
| `f23` | `fs7` | 浮点保存寄存器 7 | 是* |
| `f24` | `fs8` | 浮点保存寄存器 8 | 是* |
| `f25` | `fs9` | 浮点保存寄存器 9 | 是* |
| `f26` | `fs10` | 浮点保存寄存器 10 | 是* |
| `f27` | `fs11` | 浮点保存寄存器 11 | 是* |
| `f28` | `ft8` | 浮点临时寄存器 8 | 否 |
| `f29` | `ft9` | 浮点临时寄存器 9 | 否 |
| `f30` | `ft10` | 浮点临时寄存器 10 | 否 |
| `f31` | `ft11` | 浮点临时寄存器 11 | 否 |

`是*` 表示只保证保存宽度不超过目标 ABI 中 ABI_FLEN 的浮点值。例如 LP64D 保证保存不超过
64 位的值，LP64F 只保证不超过 32 位的值。使用纯整数 ABI（ILP32、LP64）时，所有浮点
寄存器都可视为临时寄存器。

### 2.4 向量寄存器 `v0-v31`

V 扩展提供 32 个向量寄存器，每个寄存器宽度为实现定义的 VLEN。向量寄存器没有类似
`a0`、`s0` 的通用 ABI 别名。`v0` 还约定用于向量掩码。

| 寄存器 | 标准调用约定 | 向量调用约定变体 | 变体中保存？ |
|---|---|---|---|
| `v0` | 临时寄存器 | 掩码参数寄存器 | 否 |
| `v1` | 临时寄存器 | 保存寄存器 | 是 |
| `v2` | 临时寄存器 | 保存寄存器 | 是 |
| `v3` | 临时寄存器 | 保存寄存器 | 是 |
| `v4` | 临时寄存器 | 保存寄存器 | 是 |
| `v5` | 临时寄存器 | 保存寄存器 | 是 |
| `v6` | 临时寄存器 | 保存寄存器 | 是 |
| `v7` | 临时寄存器 | 保存寄存器 | 是 |
| `v8` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v9` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v10` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v11` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v12` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v13` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v14` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v15` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v16` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v17` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v18` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v19` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v20` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v21` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v22` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v23` | 临时寄存器 | 参数/返回值寄存器 | 否 |
| `v24` | 临时寄存器 | 保存寄存器 | 是 |
| `v25` | 临时寄存器 | 保存寄存器 | 是 |
| `v26` | 临时寄存器 | 保存寄存器 | 是 |
| `v27` | 临时寄存器 | 保存寄存器 | 是 |
| `v28` | 临时寄存器 | 保存寄存器 | 是 |
| `v29` | 临时寄存器 | 保存寄存器 | 是 |
| `v30` | 临时寄存器 | 保存寄存器 | 是 |
| `v31` | 临时寄存器 | 保存寄存器 | 是 |

标准调用约定不使用向量寄存器传递参数或返回值，并且 `v0-v31` 均不跨调用保存。只有显式采用
向量调用约定变体（例如函数使用 `riscv_vector_cc` 属性）时，才使用表中的变体规则。

### 2.5 浮点和向量控制寄存器

| 寄存器/CSR | 作用 |
|---|---|
| `fflags` | 浮点累计异常标志 |
| `frm` | 浮点动态舍入模式 |
| `fcsr` | 浮点控制与状态寄存器；由 `frm` 和 `fflags` 组成 |
| `vstart` | 向量指令开始处理的元素索引 |
| `vxsat` | 向量定点饱和标志 |
| `vxrm` | 向量定点舍入模式 |
| `vcsr` | 向量控制与状态寄存器；包含 `vxrm` 和 `vxsat` |
| `vl` | 当前向量长度 |
| `vtype` | 当前向量元素宽度、LMUL 和尾部/掩码策略 |
| `vlenb` | VLEN 的字节数，只读 |

psABI 要求 `fcsr`（因此也包括 `frm` 和 `fflags`）具有符合 C11 浮点环境规则的线程存储期；
它不是普通的 callee-saved 整数寄存器。向量状态 `vl`、`vtype`、`vxrm` 和 `vxsat` 不跨调用
保存。函数可以假设入口和返回时 `vstart=0`；显式把 `vstart` 写成非零值的函数，必须在调用
其他函数或返回之前将它清零。

### 2.6 常用特权 CSR

CSR 使用独立的 12 位地址空间，理论上有 4096 个地址。实际存在的 CSR 由特权级以及
浮点、向量、计数器、调试、虚拟化、PMP 等扩展共同决定，因此不存在一张适用于所有 RISC-V
处理器的固定“全部 CSR”表。下面列出汇编和系统软件中最常见的标准 CSR：

| 级别/类别 | 常用 CSR | 作用 |
|---|---|---|
| 用户计数器 | `cycle`、`time`、`instret` | 周期、实时时钟、退休指令计数 |
| 浮点 | `fflags`、`frm`、`fcsr` | 浮点异常与舍入控制 |
| 向量 | `vstart`、`vxsat`、`vxrm`、`vcsr`、`vl`、`vtype`、`vlenb` | 向量执行状态和配置 |
| Supervisor | `sstatus`、`sie`、`stvec`、`scounteren`、`sscratch`、`sepc`、`scause`、`stval`、`sip`、`satp` | S 模式状态、中断、陷阱和地址转换 |
| Machine | `mstatus`、`misa`、`medeleg`、`mideleg`、`mie`、`mtvec`、`mcounteren`、`mscratch`、`mepc`、`mcause`、`mtval`、`mip` | M 模式状态、中断、委托和陷阱处理 |
| 机器计数器 | `mcycle`、`minstret`、`mhpmcounter3-mhpmcounter31` | M 模式性能计数器 |
| PMP | `pmpcfg*`、`pmpaddr*` | 物理内存保护区域配置 |
| Debug | `dcsr`、`dpc`、`dscratch0`、`dscratch1` | 外部调试模式状态 |

CSR 是否存在、是否可读写以及在哪个特权级可访问，必须以目标处理器实现的 ISA/特权规范为准。
例如普通用户程序不能直接写 `mstatus`，而没有 V 扩展的处理器也不会提供向量 CSR。

## 3. 常见实际指令

下表只列出本文伪指令展开时经常出现的实际指令，不是完整的 RISC-V ISA 指令表。

| 实际指令 | 作用 | 使用示例 |
|---|---|---|
| `lui rd, imm20` | 将 20 位立即数放到高位 | `lui a0, 0x12345` |
| `auipc rd, imm20` | 当前 PC 加上高位立即数 | `auipc t0, 1` |
| `addi rd, rs, imm12` | 寄存器加 12 位有符号立即数 | `addi a0, a1, 8` |
| `add/sub rd, rs1, rs2` | 加法/减法 | `add a0, a1, a2` |
| `addiw/subw` | RV64 的 32 位字运算并符号扩展 | `addiw a0, a1, 1` |
| `andi/xori` | 与立即数进行按位与/异或 | `andi a0, a1, 255` |
| `slli/srli/srai` | 逻辑左移、逻辑右移、算术右移 | `slli a0, a1, 8` |
| `slt/sltu/sltiu` | 比较并将结果写为 0 或 1 | `slt a0, a1, a2` |
| `lb/lh/lw/ld` | 从 `imm(rs1)` 加载整数 | `lw a0, 8(sp)` |
| `lbu/lhu/lwu` | 无符号加载 | `lbu a0, 0(a1)` |
| `sb/sh/sw/sd` | 将整数存入 `imm(rs1)` | `sw a0, 8(sp)` |
| `flh/flw/fld/flq` | 加载浮点数 | `flw fa0, 0(a0)` |
| `fsh/fsw/fsd/fsq` | 存储浮点数 | `fsd fa0, 0(a0)` |
| `beq/bne/blt/bge/bltu/bgeu` | 条件分支 | `bne a0, a1, loop` |
| `jal rd, offset` | PC 相对跳转并保存返回地址 | `jal ra, function` |
| `jalr rd, imm(rs1)` | 寄存器间接跳转并保存返回地址 | `jalr ra, 0(t0)` |
| `csrrw/csrrs/csrrc` | 读写、置位或清除 CSR | `csrrs a0, mstatus, t0` |
| `csrrwi/csrrsi/csrrci` | 使用立即数读写 CSR | `csrrsi a0, mstatus, 8` |
| `fence pred, succ` | 内存和 I/O 访问排序 | `fence rw, rw` |

浮点和向量实际指令需要相应的 ISA 扩展；例如 `fsgnj.s` 需要单精度浮点支持，
`vmxor.mm` 需要 V 扩展。

## 4. 地址和全局数据伪指令

| 伪指令 | 典型实际指令展开 | 作用与示例 |
|---|---|---|
| `la rd, symbol`（`nopic`） | `auipc rd, %pcrel_hi(symbol)`<br>`addi rd, rd, %pcrel_lo(label)` | 加载符号地址。例：`la a0, value` |
| `la rd, symbol`（`pic`） | `auipc rd, %got_pcrel_hi(symbol)`<br>RV64：`ld rd, %pcrel_lo(label)(rd)`<br>RV32：`lw rd, %pcrel_lo(label)(rd)` | 从 GOT 加载可动态绑定符号的地址。例：`la a0, external_value` |
| `lla rd, symbol` | `auipc rd, %pcrel_hi(symbol)`<br>`addi rd, rd, %pcrel_lo(label)` | 加载本地符号地址。例：`lla a0, local_value` |
| `lga rd, symbol` | `auipc rd, %got_pcrel_hi(symbol)`<br>RV32 使用 `lw`，RV64 使用 `ld` | 通过 GOT 加载全局符号地址。例：`lga a0, global_value` |
| `lb/lh/lw/ld rd, symbol` | `auipc rd, %pcrel_hi(symbol)`<br>`lb/lh/lw/ld rd, %pcrel_lo(label)(rd)` | 加载全局整数；`rd` 同时作临时地址寄存器。例：`lw a0, counter` |
| `lbu/lhu/lwu rd, symbol` | `auipc rd, %pcrel_hi(symbol)`<br>`lbu/lhu/lwu rd, %pcrel_lo(label)(rd)` | 无符号加载全局整数。例：`lbu a0, flag` |
| `sb/sh/sw/sd rs, symbol, rt` | `auipc rt, %pcrel_hi(symbol)`<br>`sb/sh/sw/sd rs, %pcrel_lo(label)(rt)` | 存储全局整数，`rt` 是临时寄存器。例：`sw a0, counter, t0` |
| `flh/flw/fld/flq frd, symbol, rt` | `auipc rt, %pcrel_hi(symbol)`<br>`flh/flw/fld/flq frd, %pcrel_lo(label)(rt)` | 加载全局浮点数。例：`flw fa0, pi, t0` |
| `fsh/fsw/fsd/fsq frs, symbol, rt` | `auipc rt, %pcrel_hi(symbol)`<br>`fsh/fsw/fsd/fsq frs, %pcrel_lo(label)(rt)` | 存储全局浮点数。例：`fsd fa0, result, t0` |

其中 `label` 是配对 `auipc` 所在位置的局部标签，例如：

```asm
.Laddr:
    auipc a0, %pcrel_hi(value)
    lw    a0, %pcrel_lo(.Laddr)(a0)
```

## 5. 整数伪指令

| 伪指令 | 实际指令展开 | 使用示例 |
|---|---|---|
| `nop` | `addi x0, x0, 0` | `nop`：不改变架构状态 |
| `li rd, immediate` | 由汇编器选择一条或多条 `addi`、`lui`、移位等指令 | `li a0, 42`；`li a1, 0x12345678` |
| `mv rd, rs` | `addi rd, rs, 0` | `mv a0, a1` |
| `not rd, rs` | `xori rd, rs, -1` | `not a0, a1` |
| `neg rd, rs` | `sub rd, x0, rs` | `neg a0, a1` |
| `negw rd, rs` | `subw rd, x0, rs` | `negw a0, a1`（RV64） |
| `sext.b rd, rs` | 无 Zbb：`slli rd, rs, XLEN-8`；`srai rd, rd, XLEN-8` | `sext.b a0, a1` |
| `sext.h rd, rs` | 无 Zbb：`slli rd, rs, XLEN-16`；`srai rd, rd, XLEN-16` | `sext.h a0, a1` |
| `sext.w rd, rs` | `addiw rd, rs, 0` | `sext.w a0, a1`（RV64） |
| `zext.b rd, rs` | `andi rd, rs, 255` | `zext.b a0, a1` |
| `zext.h rd, rs` | 无 Zbb：`slli rd, rs, XLEN-16`；`srli rd, rd, XLEN-16` | `zext.h a0, a1` |
| `zext.w rd, rs` | 无 Zba：`slli rd, rs, XLEN-32`；`srli rd, rd, XLEN-32`<br>有 Zba：`add.uw rd, rs, x0` | `zext.w a0, a1`（RV64） |
| `seqz rd, rs` | `sltiu rd, rs, 1` | `seqz a0, a1`：`a1 == 0` 时 `a0=1` |
| `snez rd, rs` | `sltu rd, x0, rs` | `snez a0, a1`：`a1 != 0` 时 `a0=1` |
| `sltz rd, rs` | `slt rd, rs, x0` | `sltz a0, a1` |
| `sgtz rd, rs` | `slt rd, x0, rs` | `sgtz a0, a1` |
| `sgt rd, rs, rt` | `slt rd, rt, rs` | `sgt a0, a1, a2` |
| `sgtu rd, rs, rt` | `sltu rd, rt, rs` | `sgtu a0, a1, a2` |

`sext.b`、`sext.h` 和 `zext.h` 在启用 Zbb 时存在同名的单条实际指令编码；没有 Zbb 时，
汇编器可按表中序列展开。这是“同一助记符可能兼具真实指令和伪指令身份”的典型情况。

## 6. 浮点伪指令

`h/s/d/q` 分别表示半精度、单精度、双精度和四倍精度。

| 伪指令族 | 实际指令展开 | 使用示例 |
|---|---|---|
| `fmv.{h,s,d,q} frd, frs` | `fsgnj.{h,s,d,q} frd, frs, frs` | `fmv.s fa0, fa1` |
| `fabs.{h,s,d,q} frd, frs` | `fsgnjx.{h,s,d,q} frd, frs, frs` | `fabs.d fa0, fa1` |
| `fneg.{h,s,d,q} frd, frs` | `fsgnjn.{h,s,d,q} frd, frs, frs` | `fneg.s fa0, fa1` |
| `fgt.{h,s,d,q} rd, frs, frt` | `flt.{h,s,d,q} rd, frt, frs` | `fgt.s a0, fa0, fa1` |
| `fge.{h,s,d,q} rd, frs, frt` | `fle.{h,s,d,q} rd, frt, frs` | `fge.d a0, fa0, fa1` |

例如 `fgt.s a0, fa0, fa1` 通过交换源操作数，转换成实际指令：

```asm
flt.s a0, fa1, fa0
```

## 7. 分支、跳转与调用伪指令

| 伪指令 | 实际指令展开 | 使用示例 |
|---|---|---|
| `beqz rs, offset` | `beq rs, x0, offset` | `beqz a0, done` |
| `bnez rs, offset` | `bne rs, x0, offset` | `bnez a0, loop` |
| `blez rs, offset` | `bge x0, rs, offset` | `blez a0, non_positive` |
| `bgez rs, offset` | `bge rs, x0, offset` | `bgez a0, non_negative` |
| `bltz rs, offset` | `blt rs, x0, offset` | `bltz a0, negative` |
| `bgtz rs, offset` | `blt x0, rs, offset` | `bgtz a0, positive` |
| `bgt rs, rt, offset` | `blt rt, rs, offset` | `bgt a0, a1, greater` |
| `ble rs, rt, offset` | `bge rt, rs, offset` | `ble a0, a1, less_equal` |
| `bgtu rs, rt, offset` | `bltu rt, rs, offset` | `bgtu a0, a1, greater_u` |
| `bleu rs, rt, offset` | `bgeu rt, rs, offset` | `bleu a0, a1, less_equal_u` |
| `j offset` | `jal x0, offset` | `j loop` |
| `jump offset, rt` | `auipc rt, high20(offset)`；`jalr x0, low12(offset)(rt)` | `jump far_label, t0` |
| `jal offset` | `jal x1, offset` | `jal function` |
| `jr rs` | `jalr x0, 0(rs)` | `jr t0` |
| `jr offset(rs)` | `jalr x0, offset(rs)` | `jr 8(t0)` |
| `jalr rs` | `jalr x1, 0(rs)` | `jalr t0` |
| `jalr offset(rs)` | `jalr x1, offset(rs)` | `jalr 8(t0)` |
| `jalr rd, rs` | `jalr rd, 0(rs)` | `jalr ra, t0` |
| `ret` | `jalr x0, 0(x1)`，即 `jalr zero, 0(ra)` | `ret` |
| `call offset` | `auipc ra, high20(offset)`；`jalr ra, low12(offset)(ra)` | `call function` |
| `call rt, offset` | `auipc rt, high20(offset)`；`jalr rt, low12(offset)(rt)` | `call t0, function` |
| `tail offset` | 通常为 `auipc t1, high20(offset)`；`jalr x0, low12(offset)(t1)` | `tail function` |
| `tail offset, rt` | `auipc rt, high20(offset)`；`jalr x0, low12(offset)(rt)` | `tail function, t0` |

`call`、`tail` 和 `jump` 通常伴随 `R_RISCV_CALL` 等重定位，链接器确定目标地址后填写高低位，
并可能把两条指令松弛为一条 `jal`。

## 8. 向量伪指令

下列伪指令需要 RISC-V V 扩展。`vm` 表示可选掩码操作数。

| 伪指令 | 实际指令展开 | 使用示例 |
|---|---|---|
| `vfneg.v vd, vs` | `vfsgnjn.vv vd, vs, vs` | `vfneg.v v1, v2` |
| `vfabs.v vd, vs` | `vfsgnjx.vv vd, vs, vs` | `vfabs.v v1, v2` |
| `vmclr.m vd` | `vmxor.mm vd, vd, vd` | `vmclr.m v0` |
| `vmmv.m vd, vs` | `vmand.mm vd, vs, vs` | `vmmv.m v1, v0` |
| `vmnot.m vd, vs` | `vmnand.mm vd, vs, vs` | `vmnot.m v1, v0` |
| `vmset.m vd` | `vmxnor.mm vd, vd, vd` | `vmset.m v0` |
| `vmfge.vv vd, va, vb, vm` | `vmfle.vv vd, vb, va, vm` | `vmfge.vv v0, v1, v2` |
| `vmfgt.vv vd, va, vb, vm` | `vmflt.vv vd, vb, va, vm` | `vmfgt.vv v0, v1, v2` |
| `vmsge.vi vd, va, i, vm` | `vmsgt.vi vd, va, i-1, vm` | `vmsge.vi v0, v1, 5` |
| `vmsgeu.vi vd, va, i, vm` | `vmsgtu.vi vd, va, i-1, vm` | `vmsgeu.vi v0, v1, 5` |
| `vmsgeu.vi vd, va, 0, vm` | `vmseq.vv vd, va, va, vm` | `vmsgeu.vi v0, v1, 0`，活动元素恒为真 |
| `vmsge.vv vd, va, vb, vm` | `vmsle.vv vd, vb, va, vm` | `vmsge.vv v0, v1, v2` |
| `vmsgeu.vv vd, va, vb, vm` | `vmsleu.vv vd, vb, va, vm` | `vmsgeu.vv v0, v1, v2` |
| `vmsgt.vv vd, va, vb, vm` | `vmslt.vv vd, vb, va, vm` | `vmsgt.vv v0, v1, v2` |
| `vmsgtu.vv vd, va, vb, vm` | `vmsltu.vv vd, vb, va, vm` | `vmsgtu.vv v0, v1, v2` |
| `vmslt.vi vd, va, i, vm` | `vmsle.vi vd, va, i-1, vm` | `vmslt.vi v0, v1, 5` |
| `vmsltu.vi vd, va, i, vm` | `vmsleu.vi vd, va, i-1, vm` | `vmsltu.vi v0, v1, 5` |
| `vmsltu.vi vd, va, 0, vm` | `vmsne.vv vd, va, va, vm` | `vmsltu.vi v0, v1, 0`，活动元素恒为假 |
| `vneg.v vd, vs` | `vrsub.vx vd, vs, x0` | `vneg.v v1, v2` |
| `vnot.v vd, vs, vm` | `vxor.vi vd, vs, -1, vm` | `vnot.v v1, v2` |
| `vncvt.x.x.w vd, vs, vm` | `vnsrl.wx vd, vs, x0, vm` | `vncvt.x.x.w v1, v2` |
| `vwcvt.x.x.v vd, vs, vm` | `vwadd.vx vd, vs, x0, vm` | `vwcvt.x.x.v v2, v1` |
| `vwcvtu.x.x.v vd, vs, vm` | `vwaddu.vx vd, vs, x0, vm` | `vwcvtu.x.x.v v2, v1` |
| `vl1r.v vd, (rs1)` | `vl1re8.v vd, (rs1)` | `vl1r.v v3, (a0)` |
| `vl2r.v vd, (rs1)` | `vl2re8.v vd, (rs1)` | `vl2r.v v2, (a0)` |
| `vl4r.v vd, (rs1)` | `vl4re8.v vd, (rs1)` | `vl4r.v v4, (a0)` |
| `vl8r.v vd, (rs1)` | `vl8re8.v vd, (rs1)` | `vl8r.v v8, (a0)` |

向量与标量“大于等于”还存在以下复合展开：

| 伪指令形式 | 实际指令展开 |
|---|---|
| `vmsge{u}.vx vd, va, x`（无掩码） | `vmslt{u}.vx vd, va, x`；`vmnand.mm vd, vd, vd` |
| `vmsge{u}.vx vd, va, x, v0.t`（`vd != v0`） | `vmslt{u}.vx vd, va, x, v0.t`；`vmxor.mm vd, vd, v0` |
| `vmsge{u}.vx vd, va, x, v0.t, vt`（`vd = v0`） | `vmslt{u}.vx vt, va, x`；`vmandn.mm vd, vd, vt` |
| `vmsge{u}.vx vd, va, x, v0.t, vt`（任意 `vd`） | `vmslt{u}.vx vt, va, x`；`vmandn.mm vt, v0, vt`；`vmandn.mm vd, vd, v0`；`vmor.mm vd, vt, vd` |

其中 `{u}` 表示同时包括有符号的 `vmsge.vx` 和无符号的 `vmsgeu.vx`。

## 9. 内存屏障和暂停伪指令

| 伪指令 | 实际指令展开 | 使用示例 |
|---|---|---|
| `fence` | `fence iorw, iorw` | `fence`：约束所有设备输入/输出和内存读写的顺序 |
| `pause` | `fence w, 0` 的标准提示编码 | 自旋等待循环中使用 `pause` |

## 10. CSR 伪指令

| 伪指令 | 实际指令展开 | 使用示例 |
|---|---|---|
| `rdinstret rd` | `csrrs rd, instret, x0` | `rdinstret a0` |
| `rdinstreth rd` | `csrrs rd, instreth, x0` | `rdinstreth a1`（RV32 高 32 位） |
| `rdcycle rd` | `csrrs rd, cycle, x0` | `rdcycle a0` |
| `rdcycleh rd` | `csrrs rd, cycleh, x0` | `rdcycleh a1`（RV32 高 32 位） |
| `rdtime rd` | `csrrs rd, time, x0` | `rdtime a0` |
| `rdtimeh rd` | `csrrs rd, timeh, x0` | `rdtimeh a1`（RV32 高 32 位） |
| `csrr rd, csr` | `csrrs rd, csr, x0` | `csrr a0, mstatus` |
| `csrw csr, rs` | `csrrw x0, csr, rs` | `csrw mtvec, t0` |
| `csrs csr, rs` | `csrrs x0, csr, rs` | `csrs mstatus, t0` |
| `csrc csr, rs` | `csrrc x0, csr, rs` | `csrc mstatus, t0` |
| `csrwi csr, imm` | `csrrwi x0, csr, imm` | `csrwi frm, 1` |
| `csrsi csr, imm` | `csrrsi x0, csr, imm` | `csrsi mstatus, 8` |
| `csrci csr, imm` | `csrrci x0, csr, imm` | `csrci mstatus, 8` |
| `frcsr rd` | `csrrs rd, fcsr, x0` | `frcsr a0` |
| `fscsr rd, rs` | `csrrw rd, fcsr, rs` | `fscsr a0, a1`：写入新值并返回旧值 |
| `fscsr rs` | `csrrw x0, fcsr, rs` | `fscsr a0` |
| `frrm rd` | `csrrs rd, frm, x0` | `frrm a0` |
| `fsrm rd, rs` | `csrrw rd, frm, rs` | `fsrm a0, a1` |
| `fsrm rs` | `csrrw x0, frm, rs` | `fsrm a0` |
| `fsrmi rd, imm` | `csrrwi rd, frm, imm` | `fsrmi a0, 1` |
| `fsrmi imm` | `csrrwi x0, frm, imm` | `fsrmi 1` |
| `frflags rd` | `csrrs rd, fflags, x0` | `frflags a0` |
| `fsflags rd, rs` | `csrrw rd, fflags, rs` | `fsflags a0, a1` |
| `fsflags rs` | `csrrw x0, fflags, rs` | `fsflags a0` |
| `fsflagsi rd, imm` | `csrrwi rd, fflags, imm` | `fsflagsi a0, 1` |
| `fsflagsi imm` | `csrrwi x0, fflags, imm` | `fsflagsi 1` |

## 11. `UNIMP`

手册还单独介绍了用于故意触发非法指令异常的 `unimp`：

| 写法 | 实际编码或实际指令 | 使用示例 |
|---|---|---|
| `unimp`（允许 C 扩展） | 16 位非法编码 `0x0000`，常称 `C.UNIMP` | 放在不应到达的路径：`unimp` |
| `unimp`（不允许 C 扩展） | `csrrw x0, cycle, x0`，编码 `0xC0001073` | `.option norvc` 后使用 `unimp` |

两种形式都会触发非法指令异常，但 CPU 实际看到的是不同的机器编码。

## 12. 如何查看伪指令的实际展开

例如创建 `demo.s`：

```asm
.text
.globl demo
demo:
    li   a0, 42
    mv   a1, a0
    neg  a2, a1
    beqz a2, 1f
    ret
1:
    nop
    ret
```

汇编并要求 `objdump` 不显示别名：

```bash
clang --target=riscv64 -march=rv64i -mabi=lp64 -c demo.s -o demo.o
llvm-objdump -d -M no-aliases demo.o
```

`-M no-aliases` 很重要，否则反汇编器可能把实际机器指令重新显示成 `mv`、`ret` 等伪指令，
让人误以为这些名称拥有独立编码。
