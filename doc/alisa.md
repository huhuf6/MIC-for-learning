# alisa 别名分析

mlir中bufferization的别名分析
1. 基于op的显式表达 alias relation
根据ir判断是否使用了alisa
2. Bufferization 里的 alias analysis
One-Shot Bufferize：维护了alisainfo 表,存储哪些是alisa,哪些是equivalent
3. 通过 OpInterface 推导 alias  即BufferizableOpInterface
op自己告诉analysisi operand/result 是否 alias
4. memref dialect 本身的 alias 来源
如
%sub = memref.subview %A[...]
%1 = memref.cast %0
bufferization.to_memref
bufferization.to_memref

如何判断是否安全,还要做RAW分析,所以在遍历use-def chain,构建alisa set时,
根据liveness/dominance,分析是否由RAW,如果有,则不能是alisa

# llvm 的AA 

# llvm和mlir 的AA不同
在llvm中,只包含指针语义,所有的别名分析只能基于pointer,因为llvm的数据流只有标量、指针、结构体、数组,是贴近机器指令的SSA,指令也都是alloca/store/mul/div/cmp/intrisinc等,不是operation SSA，不含高层语义

mlir::AliasAnalysis
各 dialect自己注册
ExternalModel
告诉系统
MayAlias
NoAlias
MustAlias
例如：
memref dialect 会实现 subview aliases source