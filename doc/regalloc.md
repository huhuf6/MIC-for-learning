# 寄存器为什么要分配
1.ir有无限的寄存器,便于优化   ir寄存器
2.机器上的寄存器有限(物理限制)  CPU物理寄存器
3.关键  
把hot数据存到寄存器,而cold数据存到内存

# 专业术语
1. assignment 
将ir寄存器映射到物理寄存器
2. coalesing
将多个ir寄存器映射到一个物理寄存器
3. move(成本低)
将一个cpu寄存器移动到另一个寄存器
编译phinodes
packing hot data in the finite set of registers
4. spil(成本高)
将register 数据移动到 dram hot->cold
5. restore(成本高)
将dram 数据移动到 register cold->hot

核心目标 :通过上述方法解决ir寄存器和CPU寄存器数量不匹配的问题

# 难点
如何识别哪些是hot哪些cold数据
1. 找到一个映射最小化spill和restore的cost
2. 映射到cpu上的寄存器是hot,映射到dram的寄存器是cold
3. 因为整个函数都需要考虑,所以是全局问题

静态映射 clang/c的做法
1. 编译时决定映射,找到cost小的映射
2. 根据静态映射将ir转为汇编

动态映射 jit的做法
1. 在程序运行中找到一个好的映射
2. 根据映射反馈优化,反复将ir转为汇编

# 如何找到一个好的映射-问题类比
观察到: 映射问题可以简化为图着色问题
好的映射是一个图着色问题
## 图着色问题
图中的每一个顶点,都要赋予一种颜色,任意两个点,如果有相同的颜色,那么他们之间不能存在边，在此规则上,希望找到一种满足规则的映射,且颜色数量最少
## 冲突图创建
1. 分析ir寄存器的 live range(define-最后的use)
2. 将每一个ir 寄存器映射到一个图中的顶点
3. 如果两个顶点对应的ir寄存器的live range overlay,那么在图中给它们加一条边
如果图的颜色数量小于cpu寄存器的数量,那就是理想状态

## spilling
如果找到的颜色数量要大于cpu寄存器的数量,那么放弃一个ir寄存器,它要被放到dram中
难点： 如何决定哪个ir 寄存器是要被放弃的?
1. 不hot的寄存器
2. 有很大的好处(在冲突图中的邻居很多)

整体算法
1. 构建conflict graph
2. 图着色
3. 如果图的颜色数量小于cpu寄存器的数量,那么结束
4. 否则,放弃一个IR 寄存器然后重新开始

问题1：如何“完美的”找一个IR 寄存器放弃
它和：
图着色（graph coloring）
live range overlap
loop frequency
instruction scheduling
rematerialization
coalescing
全部耦合。一个 spill 决策可能导致：
后面更多 spill
instruction 增多
live range 变长
cache miss

这是全局组合优化,所以现实中没有“完美算法”,只有 heuristic。
现实方法:
1.spill 最少使用的值
2.spill loop 外面的值
3.spill live range 长的值
4.rematerialization 重新计算而不是存储
问题2: spill 是唯一方法吗？
live range splitting,将一个ir寄存器的live range切成多段
现代 allocator：
LLVM Greedy RA
PBQP
Linear Scan with splitting
都大量依赖
live interval splitting

## LLVM Greedy Register Allocator：
核心就是：
priority queue
+ spill weight
+ live interval splitting
+ eviction
+ recoloring

# 算法实现
## draw interference graph
live range:从define到use
1. define支配所有的uses
2. use不能支配其他uses

overlap的条件
d1 dominate d2,d2 dominate u1i
d2 dominate d1,d1 dominate u2i

## coloring interference graph
1. 找到图中这样一个顶点,与他相邻的点(有直线相交)都互相相连,那么这个点与它邻居的颜色都不能相同,只能用剩余/新颜色,这样的点最后处理,因为要等邻居节点
当邻居数<颜色数,则可以直接移除 push
2. 如果没有这样的点?SSA理论上必须会有这样的点
3. 把这个点移除 push ,递归的找下一个这样的点
4. 直到剩一个点后,开始染色 pop
5. 如果已有的颜色数量满足,那么完成
6. 否则,进入spill阶段

选择需要spill的寄存器
1. 如果一个clique(clique是指若干个两两互联的顶点组成的簇) > limit,选择其中一个顶点 spill
2. 如果当需要用新寄存器给顶点染色时,此时没有新的CPU寄存器可用,那么把当前顶点 spill
3. spill一个顶点他的live range 特别长

或者其他启发式的算法

## risc-v的约束
1. 函数参数只允许用寄存器存储8个,多的要存在stack/heap中
如@foo(%a,%b)->
@foo(%a',%b')
%a=%a' copy
%b=%b' copy
在计算color时,固定 '%a'到'a0','%b'到'a1'即可

2. caller-saved register not preserved after function calls
如
t0=42,
call @foo() //可能改动t0
那就将对t0的使用改成s0,因为t0是临时寄存器会被callee改变,而s0寄存器要被callee还原

3. 如果ir寄存器使用的数据大于CPU寄存器允许的数据长度
存在stack里
把其split成小的多个寄存器 SROA, scholar replacement of aggregate 聚合体替换

4. 整型寄存器和浮点寄存器是不同的
那么,必须基于类型,给这些ir寄存器染色,如分成两组不同的颜色

# llvm的RA
Greedy Register Allocator
LLVM 为什么不用经典 graph coloring
经典 Chaitin流程：
build interference graph
simplify
spill
rebuild

问题：几十万个 edge 构图非常贵。
spill rebuild 成本高
spill 后：
需要重新建图

split 不自然
高度依赖 live range splitting
graph coloring 不擅长这个。

基于优先队列的ir寄存器 spill
1. spill weight 
“这个 virtual register 值多少钱”
weight 高尽量别 spill  考虑use_count × loop_weight
2. loop depth 执行次数越多,spill 越贵,提高 loop 内变量优先级
3. use density  单位 live range 长度里的 use 密度,保留高 density 的值
4. rematerializable 用计算比load store 更便宜
5. register class 不同类型的数据只能存储在特定的寄存器
spill

为什么 LLVM Machine IR 不是纯 SSA
1. two-address instruction 
add rax, rbx  输入输出必须同寄存器,这不是SSA
2. copies
Machine IR 里大量COPY
v1 -> a0
用于：
calling convention
coalescing
two-address lowering
register constraint
这些 copy导致
live range overlap
3. subregister
寄存器可以部分访问。
例如 x86：
RAX
EAX
AX
AL
AH
其实是同一个物理寄存器的不同部分
LLVM需要建模subregister
例如：
低32位
低16位
低8位
于是 interference不再简单。
例如：
AL 和 AX overlap
AL 和高64位别的部分关系复杂。
4. physreg constraints
某些 instruction 强制使用特定物理寄存器