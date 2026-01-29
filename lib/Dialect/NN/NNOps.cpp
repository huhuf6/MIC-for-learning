#include "MIC/Dialect/NNOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/TypeUtilities.h"

using namespace mlir;
using namespace MIC::NN;

// LinearOp implementation
LogicalResult LinearOp::inferReturnTypes(
    MLIRContext *context, Optional<Location> location,
    ValueRange operands, DictionaryAttr attributes,
    RegionRange regions, SmallVectorImpl<Type> &inferredReturnTypes) {
  auto inputType = operands[0].getType().dyn_cast<RankedTensorType>();
  auto weightType = operands[1].getType().dyn_cast<RankedTensorType>();
  
  if (!inputType || !weightType) {
    return failure();
  }
  
  // Input shape: [*, in_features]
  // Weight shape: [out_features, in_features]
  // Output shape: [*, out_features]
  auto inputShape = inputType.getShape();
  auto weightShape = weightType.getShape();
  
  if (inputShape.size() != 2 || weightShape.size() != 2) {
    return failure();
  }
  
  if (inputShape[1] != weightShape[1]) {
    return failure();
  }
  
  SmallVector<int64_t, 2> outputShape;
  outputShape.push_back(inputShape[0]);
  outputShape.push_back(weightShape[0]);
  
  inferredReturnTypes.push_back(
      RankedTensorType::get(outputShape, inputType.getElementType()));
  return success();
}

// Conv2DOp implementation
LogicalResult Conv2DOp::inferReturnTypes(
    MLIRContext *context, Optional<Location> location,
    ValueRange operands, DictionaryAttr attributes,
    RegionRange regions, SmallVectorImpl<Type> &inferredReturnTypes) {
  auto inputType = operands[0].getType().dyn_cast<RankedTensorType>();
  auto weightType = operands[1].getType().dyn_cast<RankedTensorType>();
  
  if (!inputType || !weightType) {
    return failure();
  }
  
  // Input shape: [N, C_in, H_in, W_in]
  // Weight shape: [C_out, C_in/groups, KH, KW]
  auto inputShape = inputType.getShape();
  auto weightShape = weightType.getShape();
  
  if (inputShape.size() != 4 || weightShape.size() != 4) {
    return failure();
  }
  
  auto strides = getStrides();
  auto padding = getPadding();
  auto dilation = getDilation();
  auto groups = getGroups();
  
  // Calculate output shape
  int64_t H_out = (inputShape[2] + 2 * padding[0] - dilation[0] * (weightShape[2] - 1) - 1) / strides[0] + 1;
  int64_t W_out = (inputShape[3] + 2 * padding[1] - dilation[1] * (weightShape[3] - 1) - 1) / strides[1] + 1;
  
  SmallVector<int64_t, 4> outputShape;
  outputShape.push_back(inputShape[0]);
  outputShape.push_back(weightShape[0]);
  outputShape.push_back(H_out);
  outputShape.push_back(W_out);
  
  inferredReturnTypes.push_back(
      RankedTensorType::get(outputShape, inputType.getElementType()));
  return success();
}

// GELUOp implementation
LogicalResult GELUOp::inferReturnTypes(
    MLIRContext *context, Optional<Location> location,
    ValueRange operands, DictionaryAttr attributes,
    RegionRange regions, SmallVectorImpl<Type> &inferredReturnTypes) {
  inferredReturnTypes.push_back(operands[0].getType());
  return success();
}

// LayerNormOp implementation
LogicalResult LayerNormOp::inferReturnTypes(
    MLIRContext *context, Optional<Location> location,
    ValueRange operands, DictionaryAttr attributes,
    RegionRange regions, SmallVectorImpl<Type> &inferredReturnTypes) {
  inferredReturnTypes.push_back(operands[0].getType());
  return success();
}

// AttentionOp implementation
LogicalResult AttentionOp::inferReturnTypes(
    MLIRContext *context, Optional<Location> location,
    ValueRange operands, DictionaryAttr attributes,
    RegionRange regions, SmallVectorImpl<Type> &inferredReturnTypes) {
  // Query shape: [B, T, H, D]
  // Key shape: [B, S, H, D]
  // Value shape: [B, S, H, D]
  // Output shape: [B, T, H, D]
  auto queryType = operands[0].getType().dyn_cast<RankedTensorType>();
  if (!queryType) {
    return failure();
  }
  
  inferredReturnTypes.push_back(queryType);
  return success();
}

// TableGen generated implementation
#include "MIC/Dialect/NNOps.cpp.inc"
