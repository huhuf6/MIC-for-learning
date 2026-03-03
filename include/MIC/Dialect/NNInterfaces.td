include "mlir/IR/OpBase.td"

// NN dialect interfaces

def NNLayerInterface : OpInterface<"NNLayerInterface"> {
  let description = "Interface for NN layer operations";
  let cppNamespace = "::MIC::NN";

  let methods = [
    InterfaceMethod<
      "Get layer type",
      "std::string",
      "getLayerType"
    >,
    InterfaceMethod<
      "Get input types",
      "mlir::TypeRange",
      "getInputTypes",
      (ins)
    >,
    InterfaceMethod<
      "Get output types",
      "mlir::TypeRange",
      "getOutputTypes",
      (ins)
    >
  ];
}

// Fusable operation interface
def FusableOpInterface : OpInterface<"FusableOpInterface"> {
  let description = "Interface for operations that can be fused";
  let cppNamespace = "::MIC::NN";

  let methods = [
    InterfaceMethod<
      "Check if can fuse with next op",
      "bool",
      "canFuseWith",
      (ins "mlir::Operation *":$nextOp)
    >,
    InterfaceMethod<
      "Fuse with next op",
      "mlir::Operation *",
      "fuseWith",
      (ins "mlir::Operation *":$nextOp)
    >
  ];
}
