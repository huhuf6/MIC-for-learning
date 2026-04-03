module @module {
  util.func public @main$async(%arg0: !hal.buffer_view, %arg1: !hal.fence, %arg2: !hal.fence) -> !hal.buffer_view attributes {inlining_policy = #util.inline.never, iree.abi.model = "coarse-fences", iree.abi.stub} {
    %cst = arith.constant dense_resource<torch_tensor_8_16_torch.float32> : tensor<8x16xf32>
    %0 = hal.tensor.import wait(%arg1) => %arg0 : !hal.buffer_view -> tensor<2x8xf32>
    %1 = flow.dispatch.workgroups(%0, %cst) : (tensor<2x8xf32>, tensor<8x16xf32>) -> tensor<2x16xf32> =
        (%arg3: !iree_tensor_ext.dispatch.tensor<readonly:tensor<2x8xf32>>, %arg4: !iree_tensor_ext.dispatch.tensor<readonly:tensor<8x16xf32>>, %arg5: !iree_tensor_ext.dispatch.tensor<writeonly:tensor<2x16xf32>>) {
      %cst_0 = arith.constant 0.000000e+00 : f32
      %cst_1 = arith.constant 1.000000e+00 : f32
      %cst_2 = arith.constant dense_resource<torch_tensor_16_torch.float32> : tensor<16xf32>
      %cst_3 = arith.constant dense_resource<torch_tensor_16_torch.float32_1> : tensor<16xf32>
      %cst_4 = arith.constant dense_resource<torch_tensor_16_torch.float32_2> : tensor<16xf32>
      %cst_5 = arith.constant dense_resource<torch_tensor_16_torch.float32_3> : tensor<16xf32>
      %4 = iree_tensor_ext.dispatch.tensor.load %arg3, offsets = [0, 0], sizes = [2, 8], strides = [1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<2x8xf32>> -> tensor<2x8xf32>
      %5 = iree_tensor_ext.dispatch.tensor.load %arg4, offsets = [0, 0], sizes = [8, 16], strides = [1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<8x16xf32>> -> tensor<8x16xf32>
      %6 = tensor.empty() : tensor<2x16xf32>
      %7 = linalg.fill ins(%cst_0 : f32) outs(%6 : tensor<2x16xf32>) -> tensor<2x16xf32>
      %8 = linalg.matmul ins(%4, %5 : tensor<2x8xf32>, tensor<8x16xf32>) outs(%7 : tensor<2x16xf32>) -> tensor<2x16xf32>
      %9 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d1)>, affine_map<(d0, d1) -> (d1)>, affine_map<(d0, d1) -> (d1)>, affine_map<(d0, d1) -> (d1)>, affine_map<(d0, d1) -> (d0, d1)>], iterator_types = ["parallel", "parallel"]} ins(%8, %cst_2, %cst_3, %cst_4, %cst_5 : tensor<2x16xf32>, tensor<16xf32>, tensor<16xf32>, tensor<16xf32>, tensor<16xf32>) outs(%6 : tensor<2x16xf32>) {
      ^bb0(%in: f32, %in_6: f32, %in_7: f32, %in_8: f32, %in_9: f32, %out: f32):
        %10 = arith.addf %in, %in_6 : f32
        %11 = arith.cmpf ugt, %10, %cst_0 : f32
        %12 = arith.select %11, %10, %cst_0 : f32
        %13 = arith.mulf %12, %in_7 : f32
        %14 = arith.addf %13, %in_8 : f32
        %15 = arith.negf %14 : f32
        %16 = math.exp %15 : f32
        %17 = arith.addf %16, %cst_1 : f32
        %18 = arith.divf %cst_1, %17 : f32
        %19 = arith.mulf %12, %in_9 : f32
        %20 = arith.mulf %18, %14 : f32
        %21 = arith.addf %20, %19 : f32
        linalg.yield %21 : f32
      } -> tensor<2x16xf32>
      iree_tensor_ext.dispatch.tensor.store %9, %arg5, offsets = [0, 0], sizes = [2, 16], strides = [1, 1] : tensor<2x16xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<2x16xf32>>
      flow.return
    } count() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      flow.return %x, %y, %z : index, index, index
    }
    %2 = hal.tensor.barrier join(%1 : tensor<2x16xf32>) => %arg2 : !hal.fence
    %3 = hal.tensor.export %2 : tensor<2x16xf32> -> !hal.buffer_view
    util.return %3 : !hal.buffer_view
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