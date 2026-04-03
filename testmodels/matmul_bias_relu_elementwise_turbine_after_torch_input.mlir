module @module {
  util.func public @main$async(%arg0: !hal.buffer_view, %arg1: !hal.fence, %arg2: !hal.fence) -> !hal.buffer_view attributes {inlining_policy = #util.inline.never, iree.abi.model = "coarse-fences", iree.abi.stub} {
    %cst = arith.constant 0.000000e+00 : f32
    %cst_0 = arith.constant 1.000000e+00 : f32
    %cst_1 = arith.constant dense_resource<torch_tensor_16_torch.float32_3> : tensor<16xf32>
    %cst_2 = arith.constant dense_resource<torch_tensor_16_torch.float32_2> : tensor<16xf32>
    %cst_3 = arith.constant dense_resource<torch_tensor_16_torch.float32_1> : tensor<16xf32>
    %cst_4 = arith.constant dense_resource<torch_tensor_16_torch.float32> : tensor<16xf32>
    %cst_5 = arith.constant dense_resource<torch_tensor_8_16_torch.float32> : tensor<8x16xf32>
    %0 = hal.tensor.import wait(%arg1) => %arg0 : !hal.buffer_view -> tensor<2x8xf32>
    %1 = tensor.empty() : tensor<2x16xf32>
    %2 = linalg.fill ins(%cst : f32) outs(%1 : tensor<2x16xf32>) -> tensor<2x16xf32>
    %3 = linalg.matmul ins(%0, %cst_5 : tensor<2x8xf32>, tensor<8x16xf32>) outs(%2 : tensor<2x16xf32>) -> tensor<2x16xf32>
    %4 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d1)>, affine_map<(d0, d1) -> (d0, d1)>], iterator_types = ["parallel", "parallel"]} ins(%3, %cst_4 : tensor<2x16xf32>, tensor<16xf32>) outs(%1 : tensor<2x16xf32>) {
    ^bb0(%in: f32, %in_6: f32, %out: f32):
      %14 = arith.addf %in, %in_6 : f32
      linalg.yield %14 : f32
    } -> tensor<2x16xf32>
    %5 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d0, d1)>], iterator_types = ["parallel", "parallel"]} ins(%4 : tensor<2x16xf32>) outs(%1 : tensor<2x16xf32>) {
    ^bb0(%in: f32, %out: f32):
      %14 = arith.cmpf ugt, %in, %cst : f32
      %15 = arith.select %14, %in, %cst : f32
      linalg.yield %15 : f32
    } -> tensor<2x16xf32>
    %6 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d1)>, affine_map<(d0, d1) -> (d0, d1)>], iterator_types = ["parallel", "parallel"]} ins(%5, %cst_3 : tensor<2x16xf32>, tensor<16xf32>) outs(%1 : tensor<2x16xf32>) {
    ^bb0(%in: f32, %in_6: f32, %out: f32):
      %14 = arith.mulf %in, %in_6 : f32
      linalg.yield %14 : f32
    } -> tensor<2x16xf32>
    %7 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d1)>, affine_map<(d0, d1) -> (d0, d1)>], iterator_types = ["parallel", "parallel"]} ins(%6, %cst_2 : tensor<2x16xf32>, tensor<16xf32>) outs(%1 : tensor<2x16xf32>) {
    ^bb0(%in: f32, %in_6: f32, %out: f32):
      %14 = arith.addf %in, %in_6 : f32
      linalg.yield %14 : f32
    } -> tensor<2x16xf32>
    %8 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d0, d1)>], iterator_types = ["parallel", "parallel"]} ins(%7 : tensor<2x16xf32>) outs(%1 : tensor<2x16xf32>) {
    ^bb0(%in: f32, %out: f32):
      %14 = arith.negf %in : f32
      %15 = math.exp %14 : f32
      %16 = arith.addf %15, %cst_0 : f32
      %17 = arith.divf %cst_0, %16 : f32
      linalg.yield %17 : f32
    } -> tensor<2x16xf32>
    %9 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d0, d1)>], iterator_types = ["parallel", "parallel"]} ins(%8, %7 : tensor<2x16xf32>, tensor<2x16xf32>) outs(%1 : tensor<2x16xf32>) {
    ^bb0(%in: f32, %in_6: f32, %out: f32):
      %14 = arith.mulf %in, %in_6 : f32
      linalg.yield %14 : f32
    } -> tensor<2x16xf32>
    %10 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d1)>, affine_map<(d0, d1) -> (d0, d1)>], iterator_types = ["parallel", "parallel"]} ins(%5, %cst_1 : tensor<2x16xf32>, tensor<16xf32>) outs(%1 : tensor<2x16xf32>) {
    ^bb0(%in: f32, %in_6: f32, %out: f32):
      %14 = arith.mulf %in, %in_6 : f32
      linalg.yield %14 : f32
    } -> tensor<2x16xf32>
    %11 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d0, d1)>], iterator_types = ["parallel", "parallel"]} ins(%9, %10 : tensor<2x16xf32>, tensor<2x16xf32>) outs(%1 : tensor<2x16xf32>) {
    ^bb0(%in: f32, %in_6: f32, %out: f32):
      %14 = arith.addf %in, %in_6 : f32
      linalg.yield %14 : f32
    } -> tensor<2x16xf32>
    %12 = hal.tensor.barrier join(%11 : tensor<2x16xf32>) => %arg2 : !hal.fence
    %13 = hal.tensor.export %12 : tensor<2x16xf32> -> !hal.buffer_view
    util.return %13 : !hal.buffer_view
  }
  util.func public @main(%arg0: !hal.buffer_view) -> !hal.buffer_view attributes {iree.abi.stub} {
    %0 = util.null : !hal.fence
    %c-1_i32 = arith.constant -1 : i32
    %c0 = arith.constant 0 : index
    %device_0 = hal.devices.get %c0 : !hal.device
    %fence = hal.fence.create device(%device_0 : !hal.device) flags("None") : !hal.fence
    %1 = util.call @main$async(%arg0, %0, %fence) : (!hal.buffer_view, !hal.fence, !hal.fence) -> !hal.buffer_view
    %status = hal.fence.await until([%fence]) timeout_millis(%c-1_i32) flags("None") : i32
    util.return %1 : !hal.buffer_view
  }
}