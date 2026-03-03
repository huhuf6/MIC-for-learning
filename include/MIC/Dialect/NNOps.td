include "mlir/IR/OpBase.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "MIC/Dialect/NNInterfaces.td"

def NN_Dialect : Dialect {
  let summary = "Neural Network dialect for MIC compiler";
  let description = [{
    This dialect provides operations for neural network computations, including
    linear layers, convolutions, attention mechanisms, and activation functions.
  }];
  let name = "nn";
  let cppNamespace = "::MIC::NN";
}

class NNOp<string mnemonic, list<Trait> traits = []> :
    Op<NN_Dialect, mnemonic, traits>;

// Linear layer operation
def LinearOp : NNOp<"linear", [NoSideEffect, SameOperandsAndResultElementType]> {
    let summary = "Linear layer operation";
    let description = [{
      Performs a linear transformation: y = x * W^T + b
    }];
    let arguments = (
      ins
        AnyTensor:$input,
        AnyTensor:$weight,
        Optional<AnyTensor>:$bias
    );
    let results = (outs AnyTensor:$output);
}

// Conv2D operation
def Conv2DOp : NNOp<"conv2d", [NoSideEffect, SameOperandsAndResultElementType]> {
    let summary = "2D convolution operation";
    let description = [{
      Performs 2D convolution with optional bias.
    }];
    let arguments = (
      ins
        AnyTensor:$input,
        AnyTensor:$weight,
        Optional<AnyTensor>:$bias,
        I64ArrayAttr:$strides,
        I64ArrayAttr:$padding,
        I64ArrayAttr:$dilation,
        I32Attr:$groups
    );
    let results = (outs AnyTensor:$output);
}

// Attention operation
def AttentionOp : NNOp<"attention", [NoSideEffect, SameOperandsAndResultElementType]> {
    let summary = "Attention mechanism operation";
    let description = [{
      Performs multi-head attention computation.
    }];
    let arguments = (
      ins
        AnyTensor:$query,
        AnyTensor:$key,
        AnyTensor:$value,
        Optional<AnyTensor>:$mask
    );
    let results = (outs AnyTensor:$output);
}

// GELU activation
def GELUOp : NNOp<"gelu", [NoSideEffect, SameOperandsAndResultElementType]> {
    let summary = "GELU activation function";
    let description = [{
      Applies the Gaussian Error Linear Unit (GELU) activation function.
    }];
    let arguments = (ins AnyTensor:$input);
    let results = (outs AnyTensor:$output);
}

// LayerNorm operation
def LayerNormOp : NNOp<"layer_norm", [NoSideEffect, SameOperandsAndResultElementType, AttrSizedOperandSegments]> {
    let summary = "Layer normalization operation";
    let description = [{
      Performs layer normalization with optional scale and bias.
    }];
    let arguments = (
      ins
        AnyTensor:$input,
        Optional<AnyTensor>:$scale,
        Optional<AnyTensor>:$bias,
        F64Attr:$epsilon
    );
    let results = (outs AnyTensor:$output);
}
