#===----------------------------------------------------------------------===##
# NN方言构建配置
#===----------------------------------------------------------------------===##

# 定义TableGen文件
set(LLVM_TARGET_DEFINITIONS ../../include/MIC/Dialect/NNOps.td)

# 运行TableGen生成操作定义
mlir_tablegen(NNOps.h.inc -gen-op-decls)
mlir_tablegen(NNOps.cpp.inc -gen-op-defs)
mlir_tablegen(NNDialect.cpp.inc -gen-dialect-defs)

# 定义TableGen文件
set(LLVM_TARGET_DEFINITIONS ../../include/MIC/Dialect/NNInterfaces.td)

# 运行TableGen生成接口定义
mlir_tablegen(NNInterfaces.h.inc -gen-op-interface-decls)
mlir_tablegen(NNInterfaces.cpp.inc -gen-op-interface-defs)

# 定义NN方言的源文件
set(SOURCES
  NN.cpp
  NNOps.cpp
)

# 创建NN方言库
add_mlir_library(MICNNDialect
  ${SOURCES}
  LINK_LIBS PUBLIC
  MLIRIR
  MLIRSupport
)

# 添加包含目录
target_include_directories(MICNNDialect PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${MLIR_INCLUDE_DIRS}
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)
