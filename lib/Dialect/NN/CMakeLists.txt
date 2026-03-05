set(LLVM_TARGET_DEFINITIONS ${CMAKE_SOURCE_DIR}/include/MIC/Dialect/NNOps.td)
mlir_tablegen(NN.h.inc -gen-op-decls)
mlir_tablegen(NN.cpp.inc -gen-op-defs)
mlir_tablegen(NNDialect.h.inc -gen-dialect-decls -dialect=nn)
mlir_tablegen(NNDialect.cpp.inc -gen-dialect-defs -dialect=nn)
add_public_tablegen_target(MLIRNNIncGen)

set(LLVM_TARGET_DEFINITIONS ${CMAKE_SOURCE_DIR}/include/MIC/Dialect/NNInterfaces.td)
mlir_tablegen(NNInterfaces.h.inc -gen-op-interface-decls)
mlir_tablegen(NNInterfaces.cpp.inc -gen-op-interface-defs)
add_public_tablegen_target(MLIRNNInterfacesIncGen)

add_mlir_library(MICNNDialect
    NNOps.cpp
    NN.cpp

    DEPENDS
    MLIRNNIncGen
    MLIRNNInterfacesIncGen

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRInferTypeOpInterface
    MLIRSideEffectInterfaces
    )

target_include_directories(MICNNDialect PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
