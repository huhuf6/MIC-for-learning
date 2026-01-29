include "mlir/IR/DialectBase.td"
include "mlir/IR/OpBase.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "MIC/Dialect/NNInterfaces.td"

define_dialect"NN" {
  let summary = "Neural Network dialect for MIC compiler";
  let description = """
    This dialect provides operations for neural network computations, including
    linear layers, convolutions, attention mechanisms, and activation functions.
  """;
  let cppNamespace = "::MIC::NN";
}

class NNOp<string mnemonic, list<OpTrait> traits = []> :
    Op<NN_Dialect, mnemonic, traits>;

// Linear layer operation
define NNOp<"linear", [Pure, SameOperandsAndResultElementType]> LinearOp:
    let summary = "Linear layer operation";
    let description = """
      Performs a linear transformation: y = x * W^T + b
    """;
    let arguments = (
      ins
        TensorType:$input,
        TensorType:$weight,
        Optional<TensorType>:$bias
    );
    let results = (outs TensorType:$output);
    let assemblyFormat = "`(` $input `,` $weight (`:` $bias^)? `)` attr-dict `:` type($input) `->` type($output)`";

// Conv2D operation
define NNOp<"conv2d", [Pure, SameOperandsAndResultElementType]> Conv2DOp:
    let summary = "2D convolution operation";
    let description = """
      Performs 2D convolution with optional bias.
    """;
    let arguments = (
      ins
        TensorType:$input,
        TensorType:$weight,
        Optional<TensorType>:$bias,
        I64ArrayAttr:$strides,
        I64ArrayAttr:$padding,
        I64ArrayAttr:$dilation,
        I32Attr:$groups
    );
    let results = (outs TensorType:$output);
    let assemblyFormat = "`(` $input `,` $weight (`:` $bias^)? `)` `,` $strides `,` $padding `,` $dilation `,` $groups attr-dict `:` type($input) `->` type($output)`";

// Attention operation
define NNOp<"attention", [Pure, SameOperandsAndResultElementType]> AttentionOp:
    let summary = "Attention mechanism operation";
    let description = """
      Performs multi-head attention computation.
    """;
    let arguments = (
      ins
        TensorType:$query,
        TensorType:$key,
        TensorType:$value,
        Optional<TensorType>:$mask
    );
    let results = (outs TensorType:$output);
    let assemblyFormat = "`(` $query `,` $key `,` $value (`:` $mask^)? `)` attr-dict `:` type($query) `->` type($output)`";

// GELU activation
define NNOp<"gelu", [Pure, SameOperandsAndResultElementType]> GELUOp:
    let summary = "GELU activation function";
    let description = """
      Applies the Gaussian Error Linear Unit (GELU) activation function.
    """;
    let arguments = (ins TensorType:$input);
    let results = (outs TensorType:$output);
    let assemblyFormat = "`(` $input `)` attr-dict `:` type($input) `->` type($output)`";

// LayerNorm operation
define NNOp<"layer_norm", [Pure, SameOperandsAndResultElementType]> LayerNormOp:
    let summary = "Layer normalization operation";
    let description = """
      Performs layer normalization with optional scale and bias.
    """;
    let arguments = (
      ins
        TensorType:$input,
        Optional<TensorType>:$scale,
        Optional<TensorType>:$bias,
        F64Attr:$epsilon
    );
    let results = (outs TensorType:$output);
    let assemblyFormat = "`(` $input (`:` $scale^)? (`:` $bias^)? `)` `,` $epsilon attr-dict `:` type($input) `->` type($output)`";
