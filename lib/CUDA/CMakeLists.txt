#===----------------------------------------------------------------------===##
# CUDA模块构建配置
#===----------------------------------------------------------------------===##

# 查找CUDA
find_package(CUDA REQUIRED)

# 查找CUDA库
find_library(CUDART_LIBRARY cudart ${CUDA_TOOLKIT_ROOT_DIR}/lib64)
find_library(CUDA_RUNTIME_LIBRARY cuda ${CUDA_TOOLKIT_ROOT_DIR}/lib64)

# 定义CUDA模块的源文件
set(SOURCES
  CUDAKernel.cu
)

# 创建CUDA模块
add_library(MICCUDA STATIC
  ${SOURCES}
)

# 链接CUDA库
target_link_libraries(MICCUDA PUBLIC
  ${CUDART_LIBRARY}
  ${CUDA_RUNTIME_LIBRARY}
)

# 设置CUDA编译选项
set_target_properties(MICCUDA PROPERTIES
  CUDA_SEPARABLE_COMPILATION ON
  CUDA_STANDARD 11
  CUDA_STANDARD_REQUIRED ON
)

# 添加包含目录
target_include_directories(MICCUDA PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${CUDA_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)
