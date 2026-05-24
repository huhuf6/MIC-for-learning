SSA 单一静态赋值
1.一个寄存器只被允许赋值一次(虚拟)
2.便于优化
保证一个寄存器中的值没有干扰
常量传播
mem2reg 将内存使用转为寄存器使用
遇到循环,条件分支,使用phi节点

算法流程
Step1 找 promotable alloca 满足entry block 中只被 load/store 使用,地址不逃逸
Step2 收集 Def Blocks     store指令
Step3 计算 Dominance Frontier 
工作队列：

W = DefBlocks

循环：

while W not empty:
    X = pop(W)

    for Y in DF(X):
        if Y 没有 x 的 PHI:
            insert PHI at Y

            if Y 不是定义块:
                push(Y)

这就是：

Iterated Dominance Frontier

Step4 插入 PHI
Step5 Rename（SSA Rename）  重命名def的地方,在phi节点插入新的命名来源

Rename(B):

    for phi in B:
        newName = newVersion(x)
        push(x, newName)

    for inst in B:

        if load x:
            replace with top(x)

        if store v -> x:
            newName = v
            push(x, newName)
            delete store

    for succ in successors(B):
        update successor phi incoming

    for child in DomTree(B):
        Rename(child)

    pop local definitions

Step6 删除 load/store/alloca
load
load x
→
当前 SSA version
store
store v -> x
→
产生新的 SSA version
PHI
PHI
→
新的 SSA definition