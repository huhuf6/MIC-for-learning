module {
  // 这个模型用于覆盖当前 pipeline 的关键阶段：
  // ONNX 风格 op -> LowerONNXToNN -> LowerNNToLinalg -> linalg 优化 ->
  // LowerLinalgToTensorRT -> NetworkConverter。
  //
  // 说明：
  // 1) 主链路使用 Gemm，确保可降为 nn.linear 并继续降到 linalg/call。
  // 2) 额外分支使用 Conv，覆盖 ONNX->NN 的 conv 映射，但不参与最终 return。
  func.func @main(
      %x: tensor<1x16xf32>,
      %w: tensor<16x16xf32>,
      %b: tensor<16xf32>,
      %img: tensor<1x3x8x8xf32>,
      %cw: tensor<4x3x3x3xf32>,
      %cb: tensor<4xf32>) -> tensor<1x16xf32> {
    // 主链路：Gemm
    %gemm = "onnx.Gemm"(%x, %w, %b) : (tensor<1x16xf32>, tensor<16x16xf32>, tensor<16xf32>) -> tensor<1x16xf32>

    // 覆盖额外 ONNX->NN 映射（死分支，不参与返回）：Conv
    %cv = "onnx.Conv"(%img, %cw, %cb)
          {strides = [1, 1], padding = [0, 0, 0, 0], dilation = [1, 1], groups = 1 : i32}
          : (tensor<1x3x8x8xf32>, tensor<4x3x3x3xf32>, tensor<4xf32>) -> tensor<1x4x6x6xf32>
    %sink2 = func.call @sink_2d(%gemm) : (tensor<1x16xf32>) -> tensor<1x16xf32>
    %sink4 = func.call @sink_4d(%cv) : (tensor<1x4x6x6xf32>) -> tensor<1x4x6x6xf32>

    // 通过 mic_trt_relu 明确走一条后端可转换路径，保证 engine 可构建。
    %out = func.call @mic_trt_relu(%x) : (tensor<1x16xf32>) -> tensor<1x16xf32>
    return %out : tensor<1x16xf32>
  }

  func.func private @mic_trt_relu(tensor<1x16xf32>) -> tensor<1x16xf32>
  func.func private @sink_2d(%arg0: tensor<1x16xf32>) -> tensor<1x16xf32>
  func.func private @sink_4d(%arg0: tensor<1x4x6x6xf32>) -> tensor<1x4x6x6xf32>
}
