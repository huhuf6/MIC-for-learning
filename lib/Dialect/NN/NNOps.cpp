//===----------------------------------------------------------------------===//
// MIC Neural Network (NN) Operations Implementation
//===----------------------------------------------------------------------===//

#include "MIC/Dialect/NNOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/TypeUtilities.h"

// 使用mlir命名空间
using namespace mlir;
// 使用MIC::NN命名空间，包含神经网络相关操作
using namespace MIC::NN;

//===----------------------------------------------------------------------===//
// LinearOp实现
// 功能：线性变换操作，实现y = x * w^T + b
// 参数：
//   - input: 输入张量，形状为[*, in_features]
//   - weight: 权重张量，形状为[out_features, in_features]
//   - bias: 偏置张量，形状为[out_features]（可选）
// 返回值：输出张量，形状为[*, out_features]
//===----------------------------------------------------------------------===//
LogicalResult LinearOp::inferReturnTypes(
    MLIRContext *context, Optional<Location> location,
    ValueRange operands, DictionaryAttr attributes,
    RegionRange regions, SmallVectorImpl<Type> &inferredReturnTypes) {
  // 获取输入和权重的类型
  auto inputType = operands[0].getType().dyn_cast<RankedTensorType>();
  auto weightType = operands[1].getType().dyn_cast<RankedTensorType>();
  
  // 检查输入和权重是否为 ranked tensor 类型
  if (!inputType || !weightType) {
    return failure();
  }
  
  // 输入形状: [*, in_features]
  // 权重形状: [out_features, in_features]
  // 输出形状: [*, out_features]
  auto inputShape = inputType.getShape();
  auto weightShape = weightType.getShape();
  
  // 检查输入和权重是否为2维张量
  if (inputShape.size() != 2 || weightShape.size() != 2) {
    return failure();
  }
  
  // 检查输入特征维度是否与权重特征维度匹配
  if (inputShape[1] != weightShape[1]) {
    return failure();
  }
  
  // 计算输出形状
  SmallVector<int64_t, 2> outputShape;
  outputShape.push_back(inputShape[0]);  // 保持批次维度不变
  outputShape.push_back(weightShape[0]);  // 使用权重的输出特征维度
  
  // 设置输出类型
  inferredReturnTypes.push_back(
      RankedTensorType::get(outputShape, inputType.getElementType()));
  return success();
}

//===----------------------------------------------------------------------===//
// Conv2DOp实现
// 功能：2D卷积操作
// 参数：
//   - input: 输入张量，形状为[N, C_in, H_in, W_in]
//   - weight: 权重张量，形状为[C_out, C_in/groups, KH, KW]
//   - bias: 偏置张量，形状为[C_out]（可选）
//   - strides: 卷积步长，默认为[1, 1]
//   - padding: 填充大小，默认为[0, 0]
//   - dilation: 膨胀率，默认为[1, 1]
//   - groups: 分组数，默认为1
// 返回值：输出张量，形状为[N, C_out, H_out, W_out]
//===----------------------------------------------------------------------===//
LogicalResult Conv2DOp::inferReturnTypes(
    MLIRContext *context, Optional<Location> location,
    ValueRange operands, DictionaryAttr attributes,
    RegionRange regions, SmallVectorImpl<Type> &inferredReturnTypes) {
  // 获取输入和权重的类型
  auto inputType = operands[0].getType().dyn_cast<RankedTensorType>();
  auto weightType = operands[1].getType().dyn_cast<RankedTensorType>();
  
  // 检查输入和权重是否为 ranked tensor 类型
  if (!inputType || !weightType) {
    return failure();
  }
  
  // 输入形状: [N, C_in, H_in, W_in]
  // 权重形状: [C_out, C_in/groups, KH, KW]
  auto inputShape = inputType.getShape();
  auto weightShape = weightType.getShape();
  
  // 检查输入和权重是否为4维张量
  if (inputShape.size() != 4 || weightShape.size() != 4) {
    return failure();
  }
  
  // 获取卷积参数
  auto strides = getStrides();     // 步长
  auto padding = getPadding();     // 填充
  auto dilation = getDilation();   // 膨胀率
  auto groups = getGroups();       // 分组数
  
  // 计算输出形状
  int64_t H_out = (inputShape[2] + 2 * padding[0] - dilation[0] * (weightShape[2] - 1) - 1) / strides[0] + 1;
  int64_t W_out = (inputShape[3] + 2 * padding[1] - dilation[1] * (weightShape[3] - 1) - 1) / strides[1] + 1;
  
  // 构建输出形状
  SmallVector<int64_t, 4> outputShape;
  outputShape.push_back(inputShape[0]);    // 保持批次大小不变
  outputShape.push_back(weightShape[0]);    // 使用权重的输出通道数
  outputShape.push_back(H_out);             // 计算得到的输出高度
  outputShape.push_back(W_out);             // 计算得到的输出宽度
  
  // 设置输出类型
  inferredReturnTypes.push_back(
      RankedTensorType::get(outputShape, inputType.getElementType()));
  return success();
}

//===----------------------------------------------------------------------===//
// GELUOp实现
// 功能：高斯误差线性单元激活函数
// 参数：
//   - input: 输入张量
// 返回值：输出张量，形状与输入相同
//===----------------------------------------------------------------------===//
LogicalResult GELUOp::inferReturnTypes(
    MLIRContext *context, Optional<Location> location,
    ValueRange operands, DictionaryAttr attributes,
    RegionRange regions, SmallVectorImpl<Type> &inferredReturnTypes) {
  // GELU操作保持输入和输出形状相同
  inferredReturnTypes.push_back(operands[0].getType());
  return success();
}

//===----------------------------------------------------------------------===//
// LayerNormOp实现
// 功能：层归一化操作
// 参数：
//   - input: 输入张量
//   - scale: 缩放因子张量（可选）
//   - bias: 偏置张量（可选）
//   - eps: epsilon值，用于数值稳定性，默认为1e-5
// 返回值：输出张量，形状与输入相同
//===----------------------------------------------------------------------===//
LogicalResult LayerNormOp::inferReturnTypes(
    MLIRContext *context, Optional<Location> location,
    ValueRange operands, DictionaryAttr attributes,
    RegionRange regions, SmallVectorImpl<Type> &inferredReturnTypes) {
  // LayerNorm操作保持输入和输出形状相同
  inferredReturnTypes.push_back(operands[0].getType());
  return success();
}

//===----------------------------------------------------------------------===//
// AttentionOp实现
// 功能：注意力机制操作
// 参数：
//   - query: 查询张量，形状为[B, T, H, D]
//   - key: 键张量，形状为[B, S, H, D]
//   - value: 值张量，形状为[B, S, H, D]
//   - mask: 掩码张量（可选）
// 返回值：输出张量，形状为[B, T, H, D]
//===----------------------------------------------------------------------===//
LogicalResult AttentionOp::inferReturnTypes(
    MLIRContext *context, Optional<Location> location,
    ValueRange operands, DictionaryAttr attributes,
    RegionRange regions, SmallVectorImpl<Type> &inferredReturnTypes) {
  // 查询形状: [B, T, H, D]
  // 键形状: [B, S, H, D]
  // 值形状: [B, S, H, D]
  // 输出形状: [B, T, H, D]
  auto queryType = operands[0].getType().dyn_cast<RankedTensorType>();
  if (!queryType) {
    return failure();
  }
  
  // Attention操作输出形状与查询张量相同
  inferredReturnTypes.push_back(queryType);
  return success();
}

//===----------------------------------------------------------------------===//
// TableGen生成的实现
// 说明：包含TableGen生成的操作实现代码
//===----------------------------------------------------------------------===//
#include "MIC/Dialect/NNOps.cpp.inc"
