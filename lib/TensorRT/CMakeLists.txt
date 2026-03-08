#===----------------------------------------------------------------------===##
# TensorRT模块构建配置
#===----------------------------------------------------------------------===##

if(TensorRT_FOUND)
  # 查找TensorRT库文件
  find_library(TENSORRT_LIBRARY NAMES nvinfer PATHS ${TensorRT_LIBRARY_DIRS} NO_DEFAULT_PATH)
  find_library(TENSORRT_PLUGIN_LIBRARY NAMES nvinfer_plugin PATHS ${TensorRT_LIBRARY_DIRS} NO_DEFAULT_PATH)
  find_library(TENSORRT_PARSERS_LIBRARY NAMES nvparsers PATHS ${TensorRT_LIBRARY_DIRS} NO_DEFAULT_PATH)
  find_library(TENSORRT_ONNX_PARSER_LIBRARY NAMES nvonnxparser PATHS ${TensorRT_LIBRARY_DIRS} NO_DEFAULT_PATH)
  
  message(STATUS "Found TensorRT library: ${TENSORRT_LIBRARY}")
  message(STATUS "Found TensorRT plugin library: ${TENSORRT_PLUGIN_LIBRARY}")
  
  add_library(MICTensorRT STATIC
      TensorRTBuilder.cpp
      NetworkConverter.cpp
      )
  
  target_link_libraries(MICTensorRT PUBLIC
      MLIRIR
      MLIRFuncDialect
      MLIRPass
      MICNNDialect
      ${CUDA_LIBRARIES}
      ${TENSORRT_LIBRARY}
      ${TENSORRT_PLUGIN_LIBRARY}
      ${TENSORRT_PARSERS_LIBRARY}
      ${TENSORRT_ONNX_PARSER_LIBRARY}
      dl
      )
  
  target_include_directories(MICTensorRT PUBLIC
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
