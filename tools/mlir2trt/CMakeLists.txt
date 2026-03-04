#===----------------------------------------------------------------------===##
# mlir2trt工具构建配置
#===----------------------------------------------------------------------===##

if(TensorRT_FOUND)
  add_llvm_executable(mlir2trt mlir2trt.cpp)

  llvm_update_compile_flags(mlir2trt)
  target_link_libraries(mlir2trt PRIVATE
    MLIRIR
    MLIRPass
    MLIRSupport
    MLIRParser
    MLIRTransforms
    MLIRLLVMIR
    MICNNDialect
    MICPasses
    MICTensorRT
    ${TensorRT_LIBRARIES}
    dl
    )

  target_include_directories(mlir2trt PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${MLIR_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIRS}
    ${TensorRT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  install(TARGETS mlir2trt
    RUNTIME DESTINATION bin
    )
else()
  message(WARNING "TensorRT not found, skipping mlir2trt tool")
endif()
