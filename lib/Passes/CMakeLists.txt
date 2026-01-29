add_mlir_library(MICPasses
    FusionPass.cpp
    LayoutTransformPass.cpp
    ConstantFoldPass.cpp

    DEPENDS
    MICPassesIncGen

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRPass
    MLIRTransformUtils
    MICNNDialect
    )
