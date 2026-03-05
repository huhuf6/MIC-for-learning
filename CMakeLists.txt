cmake_minimum_required(VERSION 3.18)
project(MIC LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MLIR and LLVM
find_package(MLIR REQUIRED CONFIG PATHS /usr/lib/llvm-15/lib/cmake/mlir)
find_package(LLVM REQUIRED CONFIG PATHS /usr/lib/llvm-15/lib/cmake/llvm)

# Find CUDA
find_package(CUDA REQUIRED)

# Manually set TensorRT paths since find_package doesn't work
set(TensorRT_INCLUDE_DIRS /usr/include/x86_64-linux-gnu)
set(TensorRT_LIBRARIES nvinfer)

# Find ONNX
find_package(ONNX REQUIRED)

# Include LLVM and MLIR CMake modules
list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR} ${MLIR_CMAKE_DIR})
include(TableGen)
include(AddLLVM)
include(AddMLIR)

# Add includes
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${TensorRT_INCLUDE_DIRS})
include_directories(${ONNX_INCLUDE_DIRS})

# Add project includes
include_directories(include)

# Add MLIR and LLVM definitions
add_definitions(${MLIR_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS})

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(tools)

# Add documentation target
add_custom_target(doc
    COMMAND ${CMAKE_COMMAND} -E echo "Building documentation..."
    COMMENT "Documentation target"
)
