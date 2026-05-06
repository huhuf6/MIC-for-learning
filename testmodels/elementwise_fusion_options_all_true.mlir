#map = affine_map<(d0, d1) -> (d0, d1)>
module {
  util.func public @case_truncate(%arg0: tensor<4x4xf32>, %arg1: tensor<4x4xf16>, %arg2: tensor<4x4xf16>, %arg3: tensor<4x4xf16>) -> tensor<4x4xf16> {
    %0 = tensor.empty() : tensor<4x4xf16>
    %1 = linalg.generic {indexing_maps = [#map, #map, #map], iterator_types = ["parallel", "parallel"]} ins(%arg0, %arg1 : tensor<4x4xf32>, tensor<4x4xf16>) outs(%0 : tensor<4x4xf16>) {
    ^bb0(%in: f32, %in_0: f16, %out: f16):
      %2 = arith.truncf %in : f32 to f16
      %3 = arith.addf %2, %in_0 : f16
      linalg.yield %3 : f16
    } -> tensor<4x4xf16>
    util.return %1 : tensor<4x4xf16>
  }
}

// -----
#map = affine_map<(d0, d1) -> (d1)>
#map1 = affine_map<(d0, d1) -> (d0, d1)>
module {
  util.func public @case_broadcast(%arg0: tensor<8xf32>, %arg1: tensor<4x8xf32>, %arg2: tensor<8xf32>, %arg3: tensor<4x8xf32>) -> tensor<4x8xf32> {
    %cst = arith.constant 1.000000e+00 : f32
    %0 = tensor.empty() : tensor<4x8xf32>
    %1 = linalg.generic {indexing_maps = [#map, #map1, #map1], iterator_types = ["parallel", "parallel"]} ins(%arg0, %arg1 : tensor<8xf32>, tensor<4x8xf32>) outs(%0 : tensor<4x8xf32>) {
    ^bb0(%in: f32, %in_0: f32, %out: f32):
      %2 = arith.addf %in, %cst : f32
      %3 = arith.addf %2, %in_0 : f32
      linalg.yield %3 : f32
    } -> tensor<4x8xf32>
    util.return %1 : tensor<4x8xf32>
  }
}

// -----
#map = affine_map<(d0, d1, d2) -> (d0, d1, d2)>
#map1 = affine_map<(d0, d1, d2) -> (d0)>
module {
  util.func public @case_multi_reduction(%arg0: tensor<4x3x2xf32>, %arg1: tensor<4x3x2xf32>, %arg2: tensor<4x3x2xf32>, %arg3: tensor<4xf32>) -> tensor<4xf32> {
    %cst = arith.constant 2.000000e+00 : f32
    %0 = tensor.empty() : tensor<4x3x2xf32>
    %1 = linalg.generic {indexing_maps = [#map, #map], iterator_types = ["parallel", "parallel", "parallel"]} ins(%arg0 : tensor<4x3x2xf32>) outs(%0 : tensor<4x3x2xf32>) {
    ^bb0(%in: f32, %out: f32):
      %3 = arith.mulf %in, %cst : f32
      linalg.yield %3 : f32
    } -> tensor<4x3x2xf32>
    %2 = linalg.generic {indexing_maps = [#map, #map, #map1], iterator_types = ["parallel", "reduction", "reduction"]} ins(%1, %arg1 : tensor<4x3x2xf32>, tensor<4x3x2xf32>) outs(%arg3 : tensor<4xf32>) {
    ^bb0(%in: f32, %in_0: f32, %out: f32):
      %3 = arith.mulf %in, %in_0 : f32
      %4 = arith.addf %out, %3 : f32
      linalg.yield %4 : f32
    } -> tensor<4xf32>
    util.return %2 : tensor<4xf32>
  }
}

