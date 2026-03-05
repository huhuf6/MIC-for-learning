add_mlir_library(MICCUDA
    LayerNormPlugin.cpp
    CUDAKernel.cu

    LINK_LIBS PUBLIC
    MLIRIR
    MICNNDialect
    )

target_link_libraries(MICCUDA PUBLIC ${CUDA_LIBRARIES} ${TensorRT_LIBRARIES})

# Set CUDA properties
set_property(TARGET MICCUDA PROPERTY CUDA_SEPARABLE_COMPILATION ON)
