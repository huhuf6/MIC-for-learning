add_mlir_library(MICTensorRT
    TensorRTBuilder.cpp
    NetworkConverter.cpp

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRPass
    MICNNDialect
    )

target_link_libraries(MICTensorRT PUBLIC ${CUDA_LIBRARIES} ${TensorRT_LIBRARIES})
