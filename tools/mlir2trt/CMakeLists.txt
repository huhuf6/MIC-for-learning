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
  MICCUDA
  ${CUDA_LIBRARIES}
  ${TensorRT_LIBRARIES}
  ${ONNX_LIBRARIES}
  )

install(TARGETS mlir2trt
  RUNTIME DESTINATION bin
  )
