cmake_minimum_required(VERSION 3.18)
project(MIC LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MLIR and LLVM
find_package(MLIR REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

# Find TensorRT
find_package(CUDA REQUIRED)
find_package(TensorRT REQUIRED)

# Find ONNX
find_package(ONNX REQUIRED)

# Add MLIR and LLVM includes
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

# Add CUDA and TensorRT definitions
add_definitions(${CUDA_DEFINITIONS})

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(test)

# Add documentation target
add_custom_target(doc
    COMMAND ${CMAKE_COMMAND} -E echo "Building documentation..."
    COMMENT "Documentation target"
)
