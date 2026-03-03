#===----------------------------------------------------------------------===##
# MIC项目主构建配置
#===----------------------------------------------------------------------===##

cmake_minimum_required(VERSION 3.16)
project(MIC LANGUAGES CXX CUDA)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找LLVM
find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})
include(AddLLVM)

# 查找MLIR
find_package(MLIR REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH ${MLIR_CMAKE_DIR})
include(AddMLIR)

# 包含TableGen模块
include(TableGen)

# 查找CUDA
find_package(CUDA REQUIRED)

# 查找ONNX
find_package(ONNX REQUIRED)

# 禁用TensorRT相关功能，因为通过pip安装的TensorRT不包含开发头文件
set(TensorRT_FOUND FALSE)

# 禁用工具和测试目录的构建
set(LLVM_BUILD_TOOLS OFF)
set(LLVM_BUILD_TESTS OFF)
set(MLIR_BUILD_TOOLS OFF)
set(MLIR_BUILD_TESTS OFF)

# 添加子目录
add_subdirectory(lib/Dialect/NN)
add_subdirectory(lib/Passes)
add_subdirectory(lib/CUDA)
add_subdirectory(lib/TensorRT)
add_subdirectory(tools)
