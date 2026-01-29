add_mlir_dialect(NN nn)
add_mlir_doc(NNOps NN Dialects/ -gen-op-doc)

add_mlir_interface(NNInterfaces)

add_mlir_library(MICNNDialect
    NNOps.cpp
    NN.cpp

    DEPENDS
    MICOpsIncGen
    MICInterfacesIncGen

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRInferTypeOpInterface
    MLIRSideEffectInterfaces
    )
