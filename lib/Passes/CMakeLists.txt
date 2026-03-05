add_mlir_library(MICPasses
    FusionPass.cpp
    LayoutTransformPass.cpp
    ConstantFoldPass.cpp

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRPass
    MLIRTransformUtils
    MICNNDialect
    )
