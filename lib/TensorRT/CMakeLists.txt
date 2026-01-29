add_mlir_library(MICTensorRT
    TensorRTBuilder.cpp
    NetworkConverter.cpp

    DEPENDS

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRPass
    MICNNDialect
    ${CUDA_LIBRARIES}
    ${TensorRT_LIBRARIES}
    )
