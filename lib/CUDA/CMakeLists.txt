add_mlir_library(MICCUDA
    LayerNormPlugin.cpp
    CUDAKernel.cu

    DEPENDS

    LINK_LIBS PUBLIC
    MLIRIR
    MICNNDialect
    ${CUDA_LIBRARIES}
    ${TensorRT_LIBRARIES}
    )

# Set CUDA properties
set_property(TARGET MICCUDA PROPERTY CUDA_SEPARABLE_COMPILATION ON)
