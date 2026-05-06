module {
  // CASE 1: truncate producer -> binary consumer
  util.func public @case_truncate(
      %src: tensor<4x4xf32>, %rhs: tensor<4x4xf16>,
      %init_p: tensor<4x4xf16>, %init_c: tensor<4x4xf16>) -> tensor<4x4xf16> {
    %p = linalg.generic {
      indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>,
                       affine_map<(d0, d1) -> (d0, d1)>],
      iterator_types = ["parallel", "parallel"]
    } ins(%src : tensor<4x4xf32>) outs(%init_p : tensor<4x4xf16>) {
    ^bb0(%in: f32, %out: f16):
      %t = arith.truncf %in : f32 to f16
      linalg.yield %t : f16
    } -> tensor<4x4xf16>

    %c = linalg.generic {
      indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>,
                       affine_map<(d0, d1) -> (d0, d1)>,
                       affine_map<(d0, d1) -> (d0, d1)>],
      iterator_types = ["parallel", "parallel"]
    } ins(%p, %rhs : tensor<4x4xf16>, tensor<4x4xf16>) outs(%init_c : tensor<4x4xf16>) {
    ^bb0(%a: f16, %b: f16, %out: f16):
      %r = arith.addf %a, %b : f16
      linalg.yield %r : f16
    } -> tensor<4x4xf16>
    util.return %c : tensor<4x4xf16>
  }
}

// -----

module {
  // CASE 2: 1D producer -> broadcast-like consumer
  util.func public @case_broadcast(
      %src: tensor<8xf32>, %rhs: tensor<4x8xf32>,
      %init_p: tensor<8xf32>, %init_c: tensor<4x8xf32>) -> tensor<4x8xf32> {
    %p = linalg.generic {
      indexing_maps = [affine_map<(d0) -> (d0)>,
                       affine_map<(d0) -> (d0)>],
      iterator_types = ["parallel"]
    } ins(%src : tensor<8xf32>) outs(%init_p : tensor<8xf32>) {
    ^bb0(%in: f32, %out: f32):
      %one = arith.constant 1.0 : f32
      %t = arith.addf %in, %one : f32
      linalg.yield %t : f32
    } -> tensor<8xf32>

    %c = linalg.generic {
      indexing_maps = [affine_map<(d0, d1) -> (d1)>,
                       affine_map<(d0, d1) -> (d0, d1)>,
                       affine_map<(d0, d1) -> (d0, d1)>],
      iterator_types = ["parallel", "parallel"]
    } ins(%p, %rhs : tensor<8xf32>, tensor<4x8xf32>) outs(%init_c : tensor<4x8xf32>) {
    ^bb0(%a: f32, %b: f32, %out: f32):
      %r = arith.addf %a, %b : f32
      linalg.yield %r : f32
    } -> tensor<4x8xf32>
    util.return %c : tensor<4x8xf32>
  }
}

// -----

module {
  // CASE 3: producer fused into multi-reduction consumer
  util.func public @case_multi_reduction(
      %src: tensor<4x3x2xf32>, %rhs: tensor<4x3x2xf32>,
      %init_p: tensor<4x3x2xf32>, %init_c: tensor<4xf32>) -> tensor<4xf32> {
    %p = linalg.generic {
      indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>,
                       affine_map<(d0, d1, d2) -> (d0, d1, d2)>],
      iterator_types = ["parallel", "parallel", "parallel"]
    } ins(%src : tensor<4x3x2xf32>) outs(%init_p : tensor<4x3x2xf32>) {
    ^bb0(%in: f32, %out: f32):
      %two = arith.constant 2.0 : f32
      %t = arith.mulf %in, %two : f32
      linalg.yield %t : f32
    } -> tensor<4x3x2xf32>

    %c = linalg.generic {
      indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>,
                       affine_map<(d0, d1, d2) -> (d0, d1, d2)>,
                       affine_map<(d0, d1, d2) -> (d0)>],
      iterator_types = ["parallel", "reduction", "reduction"]
    } ins(%p, %rhs : tensor<4x3x2xf32>, tensor<4x3x2xf32>) outs(%init_c : tensor<4xf32>) {
    ^bb0(%a: f32, %b: f32, %out: f32):
      %m = arith.mulf %a, %b : f32
      %s = arith.addf %out, %m : f32
      linalg.yield %s : f32
    } -> tensor<4xf32>
    util.return %c : tensor<4xf32>
  }
}
