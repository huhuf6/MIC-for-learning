#===----------------------------------------------------------------------===##
# TensorRT模块构建配置
#===----------------------------------------------------------------------===##

# 只有当找到TensorRT时才构建TensorRT模块
if(TensorRT_FOUND)
  # 直接指定TensorRT库文件的路径
  set(TENSORRT_LIBRARY "${TensorRT_LIBRARY_DIRS}/libnvinfer.so.8")
  set(TENSORRT_PLUGIN_LIBRARY "${TensorRT_LIBRARY_DIRS}/libnvinfer_plugin.so.8")
  
  # 查找CUDA库文件
  find_library(CUDA_CUDART_LIBRARY cudart)
  
  # 查找系统库
  find_library(RT_LIBRARY rt)
  find_library(DL_LIBRARY dl)
  
  add_mlir_library(MICTensorRT
      TensorRTBuilder.cpp
      NetworkConverter.cpp

      LINK_LIBS PUBLIC
      MLIRIR
      MLIRPass
      MICNNDialect
      ${CUDA_CUDART_LIBRARY}
      ${TENSORRT_LIBRARY}
      ${TENSORRT_PLUGIN_LIBRARY}
      ${RT_LIBRARY}
      ${DL_LIBRARY}
      )
  
  # 添加包含目录
  target_include_directories(MICTensorRT PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${MLIR_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIRS}
    ${CUDA_INCLUDE_DIRS}
    ${TensorRT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
  )
else()
  message(WARNING "TensorRT not found, skipping MICTensorRT module")
endif()
