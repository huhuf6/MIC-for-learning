#===----------------------------------------------------------------------===##
# Passes模块构建配置
#===----------------------------------------------------------------------===##

# 定义Passes模块的源文件
set(SOURCES
  ConstantFoldPass.cpp
  FusionPass.cpp
  LayoutTransformPass.cpp
  NNToRuntimeLoweringPass.cpp
  PipelineLoweringPasses.cpp
)

# 定义Passes模块的头文件
set(HEADERS)

# 创建Passes库
add_mlir_library(MICPasses
  ${SOURCES}
  ${HEADERS}
  LINK_LIBS PUBLIC
  MLIRArithmeticDialect
  MLIRLinalgDialect
  MLIRLinalgTransforms
  MLIRTensorDialect
  MLIRIR
  MLIRSupport
  MLIRPass
  MLIRTransforms
  MLIRFuncDialect
  MICNNDialect
)

# 添加包含目录
target_include_directories(MICPasses PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${MLIR_INCLUDE_DIRS}
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)
