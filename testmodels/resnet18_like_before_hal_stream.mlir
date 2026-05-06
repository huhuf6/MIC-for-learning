module @module {
  stream.executable private @main$async_dispatch_0 {
    stream.executable.export public @main$async_dispatch_0_slow_memcpy workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_0_slow_memcpy(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}) {
        %c0 = arith.constant 0 : index
        %0 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<3x224x224xf32>>
        %1 = stream.binding.subspan %arg1[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<3x230x230xf32>>
        %2 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [3, 224, 224], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<3x224x224xf32>> -> tensor<3x224x224xf32>
        iree_tensor_ext.dispatch.tensor.store %2, %1, offsets = [0, 3, 3], sizes = [3, 224, 224], strides = [1, 1, 1] : tensor<3x224x224xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<3x230x230xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_1 {
    stream.executable.export public @main$async_dispatch_1_conv_64x112x112x3x7x7_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_1_conv_64x112x112x3x7x7_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %cst_1 = arith.constant dense_resource<torch_tensor_64_torch.float32> : tensor<64xf32>
        %cst_2 = arith.constant dense_resource<torch_tensor_64_torch.float32_2> : tensor<64xf32>
        %cst_3 = arith.constant dense_resource<torch_tensor_64_torch.float32_3> : tensor<64xf32>
        %c0 = arith.constant 0 : index
        %c46678016 = arith.constant 46678016 : index
        %c634816 = arith.constant 634816 : index
        %0 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<3x230x230xf32>>
        %1 = stream.binding.subspan %arg1[%c46678016] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x3x7x7xf32>>
        %2 = stream.binding.subspan %arg2[%c634816] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<64x114x114xf32>>
        %3 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [3, 230, 230], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<3x230x230xf32>> -> tensor<3x230x230xf32>
        %4 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [64, 3, 7, 7], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x3x7x7xf32>> -> tensor<64x3x7x7xf32>
        %5 = tensor.empty() : tensor<64x112x112xf32>
        %6 = linalg.fill ins(%cst_0 : f32) outs(%5 : tensor<64x112x112xf32>) -> tensor<64x112x112xf32>
        %7 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 * 2 + d4, d2 * 2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%3, %4 : tensor<3x230x230xf32>, tensor<64x3x7x7xf32>) outs(%6 : tensor<64x112x112xf32>) {
        ^bb0(%in: f32, %in_4: f32, %out: f32):
          %9 = arith.mulf %in, %in_4 : f32
          %10 = arith.addf %out, %9 : f32
          linalg.yield %10 : f32
        } -> tensor<64x112x112xf32>
        %8 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%7, %cst_1, %cst_2, %cst_3 : tensor<64x112x112xf32>, tensor<64xf32>, tensor<64xf32>, tensor<64xf32>) outs(%5 : tensor<64x112x112xf32>) {
        ^bb0(%in: f32, %in_4: f32, %in_5: f32, %in_6: f32, %out: f32):
          %9 = arith.subf %in, %in_4 : f32
          %10 = arith.mulf %9, %cst : f32
          %11 = arith.mulf %10, %in_5 : f32
          %12 = arith.addf %11, %in_6 : f32
          %13 = arith.cmpf ugt, %12, %cst_0 : f32
          %14 = arith.select %13, %12, %cst_0 : f32
          linalg.yield %14 : f32
        } -> tensor<64x112x112xf32>
        iree_tensor_ext.dispatch.tensor.store %8, %2, offsets = [0, 1, 1], sizes = [64, 112, 112], strides = [1, 1, 1] : tensor<64x112x112xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<64x114x114xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_2 {
    stream.executable.export public @main$async_dispatch_2_conv_64x56x56x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_2_conv_64x56x56x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0xFF800000 : f32
        %c634816 = arith.constant 634816 : index
        %c3961792 = arith.constant 3961792 : index
        %0 = stream.binding.subspan %arg0[%c634816] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x114x114xf32>>
        %1 = stream.binding.subspan %arg1[%c3961792] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<64x56x56xf32>>
        %2 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [64, 114, 114], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x114x114xf32>> -> tensor<64x114x114xf32>
        %3 = tensor.empty() : tensor<64x56x56xf32>
        %4 = tensor.empty() : tensor<3x3xf32>
        %5 = linalg.fill ins(%cst : f32) outs(%3 : tensor<64x56x56xf32>) -> tensor<64x56x56xf32>
        %6 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4) -> (d0, d1 * 2 + d3, d2 * 2 + d4)>, affine_map<(d0, d1, d2, d3, d4) -> (d3, d4)>, affine_map<(d0, d1, d2, d3, d4) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction"]} ins(%2, %4 : tensor<64x114x114xf32>, tensor<3x3xf32>) outs(%5 : tensor<64x56x56xf32>) {
        ^bb0(%in: f32, %in_0: f32, %out: f32):
          %7 = arith.maximumf %out, %in : f32
          linalg.yield %7 : f32
        } -> tensor<64x56x56xf32>
        iree_tensor_ext.dispatch.tensor.store %6, %1, offsets = [0, 0, 0], sizes = [64, 56, 56], strides = [1, 1, 1] : tensor<64x56x56xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<64x56x56xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_3 {
    stream.executable.export public @main$async_dispatch_3_slow_memcpy workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_3_slow_memcpy(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: i32, %arg3: i32) {
        %0 = arith.index_castui %arg2 : i32 to index
        %1 = arith.index_castui %arg3 : i32 to index
        %2:2 = util.assume.int 
            %0[<umin = 3961792, umax = 3961792, udiv = 3961792>, <umin = 861184, umax = 861184, udiv = 861184>, <umin = 1664000, umax = 1664000, udiv = 1664000>], 
            %1[<umin = 4764608, umax = 4764608, udiv = 4764608>, <umin = 1664000, umax = 1664000, udiv = 1664000>, <umin = 2466816, umax = 2466816, udiv = 2466816>]
          : index, index
        %3 = stream.binding.subspan %arg0[%2#0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x56x56xf32>>
        %4 = stream.binding.subspan %arg1[%2#1] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<64x58x58xf32>>
        %5 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0, 0, 0], sizes = [64, 56, 56], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x56x56xf32>> -> tensor<64x56x56xf32>
        iree_tensor_ext.dispatch.tensor.store %5, %4, offsets = [0, 1, 1], sizes = [64, 56, 56], strides = [1, 1, 1] : tensor<64x56x56xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<64x58x58xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_4 {
    stream.executable.export public @main$async_dispatch_4_conv_64x56x56x64x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_4_conv_64x56x56x64x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %cst_1 = arith.constant dense_resource<torch_tensor_64_torch.float32_4> : tensor<64xf32>
        %cst_2 = arith.constant dense_resource<torch_tensor_64_torch.float32_6> : tensor<64xf32>
        %cst_3 = arith.constant dense_resource<torch_tensor_64_torch.float32_7> : tensor<64xf32>
        %c4764608 = arith.constant 4764608 : index
        %c46530560 = arith.constant 46530560 : index
        %c0 = arith.constant 0 : index
        %0 = stream.binding.subspan %arg0[%c4764608] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x58x58xf32>>
        %1 = stream.binding.subspan %arg1[%c46530560] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x64x3x3xf32>>
        %2 = stream.binding.subspan %arg2[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<64x58x58xf32>>
        %3 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [64, 58, 58], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x58x58xf32>> -> tensor<64x58x58xf32>
        %4 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [64, 64, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x64x3x3xf32>> -> tensor<64x64x3x3xf32>
        %5 = tensor.empty() : tensor<64x56x56xf32>
        %6 = linalg.fill ins(%cst_0 : f32) outs(%5 : tensor<64x56x56xf32>) -> tensor<64x56x56xf32>
        %7 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%3, %4 : tensor<64x58x58xf32>, tensor<64x64x3x3xf32>) outs(%6 : tensor<64x56x56xf32>) {
        ^bb0(%in: f32, %in_4: f32, %out: f32):
          %9 = arith.mulf %in, %in_4 : f32
          %10 = arith.addf %out, %9 : f32
          linalg.yield %10 : f32
        } -> tensor<64x56x56xf32>
        %8 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%7, %cst_1, %cst_2, %cst_3 : tensor<64x56x56xf32>, tensor<64xf32>, tensor<64xf32>, tensor<64xf32>) outs(%5 : tensor<64x56x56xf32>) {
        ^bb0(%in: f32, %in_4: f32, %in_5: f32, %in_6: f32, %out: f32):
          %9 = arith.subf %in, %in_4 : f32
          %10 = arith.mulf %9, %cst : f32
          %11 = arith.mulf %10, %in_5 : f32
          %12 = arith.addf %11, %in_6 : f32
          %13 = arith.cmpf ugt, %12, %cst_0 : f32
          %14 = arith.select %13, %12, %cst_0 : f32
          linalg.yield %14 : f32
        } -> tensor<64x56x56xf32>
        iree_tensor_ext.dispatch.tensor.store %8, %2, offsets = [0, 1, 1], sizes = [64, 56, 56], strides = [1, 1, 1] : tensor<64x56x56xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<64x58x58xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_5 {
    stream.executable.export public @main$async_dispatch_5_conv_64x56x56x64x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_5_conv_64x56x56x64x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %cst_1 = arith.constant dense_resource<torch_tensor_64_torch.float32_8> : tensor<64xf32>
        %cst_2 = arith.constant dense_resource<torch_tensor_64_torch.float32_10> : tensor<64xf32>
        %cst_3 = arith.constant dense_resource<torch_tensor_64_torch.float32_11> : tensor<64xf32>
        %c0 = arith.constant 0 : index
        %c3961792 = arith.constant 3961792 : index
        %c46383104 = arith.constant 46383104 : index
        %c861184 = arith.constant 861184 : index
        %0 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x58x58xf32>>
        %1 = stream.binding.subspan %arg1[%c46383104] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x64x3x3xf32>>
        %2 = stream.binding.subspan %arg0[%c3961792] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x56x56xf32>>
        %3 = stream.binding.subspan %arg2[%c861184] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<64x56x56xf32>>
        %4 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [64, 58, 58], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x58x58xf32>> -> tensor<64x58x58xf32>
        %5 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [64, 64, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x64x3x3xf32>> -> tensor<64x64x3x3xf32>
        %6 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0, 0, 0], sizes = [64, 56, 56], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x56x56xf32>> -> tensor<64x56x56xf32>
        %7 = tensor.empty() : tensor<64x56x56xf32>
        %8 = linalg.fill ins(%cst_0 : f32) outs(%7 : tensor<64x56x56xf32>) -> tensor<64x56x56xf32>
        %9 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%4, %5 : tensor<64x58x58xf32>, tensor<64x64x3x3xf32>) outs(%8 : tensor<64x56x56xf32>) {
        ^bb0(%in: f32, %in_4: f32, %out: f32):
          %11 = arith.mulf %in, %in_4 : f32
          %12 = arith.addf %out, %11 : f32
          linalg.yield %12 : f32
        } -> tensor<64x56x56xf32>
        %10 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%9, %cst_1, %cst_2, %cst_3, %6 : tensor<64x56x56xf32>, tensor<64xf32>, tensor<64xf32>, tensor<64xf32>, tensor<64x56x56xf32>) outs(%7 : tensor<64x56x56xf32>) {
        ^bb0(%in: f32, %in_4: f32, %in_5: f32, %in_6: f32, %in_7: f32, %out: f32):
          %11 = arith.subf %in, %in_4 : f32
          %12 = arith.mulf %11, %cst : f32
          %13 = arith.mulf %12, %in_5 : f32
          %14 = arith.addf %13, %in_6 : f32
          %15 = arith.addf %14, %in_7 : f32
          %16 = arith.cmpf ugt, %15, %cst_0 : f32
          %17 = arith.select %16, %15, %cst_0 : f32
          linalg.yield %17 : f32
        } -> tensor<64x56x56xf32>
        iree_tensor_ext.dispatch.tensor.store %10, %3, offsets = [0, 0, 0], sizes = [64, 56, 56], strides = [1, 1, 1] : tensor<64x56x56xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<64x56x56xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_7 {
    stream.executable.export public @main$async_dispatch_7_conv_64x56x56x64x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_7_conv_64x56x56x64x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %cst_1 = arith.constant dense_resource<torch_tensor_64_torch.float32_12> : tensor<64xf32>
        %cst_2 = arith.constant dense_resource<torch_tensor_64_torch.float32_14> : tensor<64xf32>
        %cst_3 = arith.constant dense_resource<torch_tensor_64_torch.float32_15> : tensor<64xf32>
        %c1664000 = arith.constant 1664000 : index
        %c46235648 = arith.constant 46235648 : index
        %c0 = arith.constant 0 : index
        %0 = stream.binding.subspan %arg0[%c1664000] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x58x58xf32>>
        %1 = stream.binding.subspan %arg1[%c46235648] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x64x3x3xf32>>
        %2 = stream.binding.subspan %arg2[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<64x58x58xf32>>
        %3 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [64, 58, 58], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x58x58xf32>> -> tensor<64x58x58xf32>
        %4 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [64, 64, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x64x3x3xf32>> -> tensor<64x64x3x3xf32>
        %5 = tensor.empty() : tensor<64x56x56xf32>
        %6 = linalg.fill ins(%cst_0 : f32) outs(%5 : tensor<64x56x56xf32>) -> tensor<64x56x56xf32>
        %7 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%3, %4 : tensor<64x58x58xf32>, tensor<64x64x3x3xf32>) outs(%6 : tensor<64x56x56xf32>) {
        ^bb0(%in: f32, %in_4: f32, %out: f32):
          %9 = arith.mulf %in, %in_4 : f32
          %10 = arith.addf %out, %9 : f32
          linalg.yield %10 : f32
        } -> tensor<64x56x56xf32>
        %8 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%7, %cst_1, %cst_2, %cst_3 : tensor<64x56x56xf32>, tensor<64xf32>, tensor<64xf32>, tensor<64xf32>) outs(%5 : tensor<64x56x56xf32>) {
        ^bb0(%in: f32, %in_4: f32, %in_5: f32, %in_6: f32, %out: f32):
          %9 = arith.subf %in, %in_4 : f32
          %10 = arith.mulf %9, %cst : f32
          %11 = arith.mulf %10, %in_5 : f32
          %12 = arith.addf %11, %in_6 : f32
          %13 = arith.cmpf ugt, %12, %cst_0 : f32
          %14 = arith.select %13, %12, %cst_0 : f32
          linalg.yield %14 : f32
        } -> tensor<64x56x56xf32>
        iree_tensor_ext.dispatch.tensor.store %8, %2, offsets = [0, 1, 1], sizes = [64, 56, 56], strides = [1, 1, 1] : tensor<64x56x56xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<64x58x58xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_8 {
    stream.executable.export public @main$async_dispatch_8_conv_64x56x56x64x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_8_conv_64x56x56x64x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %cst_1 = arith.constant dense_resource<torch_tensor_64_torch.float32_16> : tensor<64xf32>
        %cst_2 = arith.constant dense_resource<torch_tensor_64_torch.float32_18> : tensor<64xf32>
        %cst_3 = arith.constant dense_resource<torch_tensor_64_torch.float32_19> : tensor<64xf32>
        %c0 = arith.constant 0 : index
        %c861184 = arith.constant 861184 : index
        %c46088192 = arith.constant 46088192 : index
        %c1664000 = arith.constant 1664000 : index
        %0 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x58x58xf32>>
        %1 = stream.binding.subspan %arg1[%c46088192] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x64x3x3xf32>>
        %2 = stream.binding.subspan %arg0[%c861184] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x56x56xf32>>
        %3 = stream.binding.subspan %arg2[%c1664000] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<64x56x56xf32>>
        %4 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [64, 58, 58], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x58x58xf32>> -> tensor<64x58x58xf32>
        %5 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [64, 64, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x64x3x3xf32>> -> tensor<64x64x3x3xf32>
        %6 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0, 0, 0], sizes = [64, 56, 56], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x56x56xf32>> -> tensor<64x56x56xf32>
        %7 = tensor.empty() : tensor<64x56x56xf32>
        %8 = linalg.fill ins(%cst_0 : f32) outs(%7 : tensor<64x56x56xf32>) -> tensor<64x56x56xf32>
        %9 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%4, %5 : tensor<64x58x58xf32>, tensor<64x64x3x3xf32>) outs(%8 : tensor<64x56x56xf32>) {
        ^bb0(%in: f32, %in_4: f32, %out: f32):
          %11 = arith.mulf %in, %in_4 : f32
          %12 = arith.addf %out, %11 : f32
          linalg.yield %12 : f32
        } -> tensor<64x56x56xf32>
        %10 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%9, %cst_1, %cst_2, %cst_3, %6 : tensor<64x56x56xf32>, tensor<64xf32>, tensor<64xf32>, tensor<64xf32>, tensor<64x56x56xf32>) outs(%7 : tensor<64x56x56xf32>) {
        ^bb0(%in: f32, %in_4: f32, %in_5: f32, %in_6: f32, %in_7: f32, %out: f32):
          %11 = arith.subf %in, %in_4 : f32
          %12 = arith.mulf %11, %cst : f32
          %13 = arith.mulf %12, %in_5 : f32
          %14 = arith.addf %13, %in_6 : f32
          %15 = arith.addf %14, %in_7 : f32
          %16 = arith.cmpf ugt, %15, %cst_0 : f32
          %17 = arith.select %16, %15, %cst_0 : f32
          linalg.yield %17 : f32
        } -> tensor<64x56x56xf32>
        iree_tensor_ext.dispatch.tensor.store %10, %3, offsets = [0, 0, 0], sizes = [64, 56, 56], strides = [1, 1, 1] : tensor<64x56x56xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<64x56x56xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_10 {
    stream.executable.export public @main$async_dispatch_10_conv_128x28x28x64x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_10_conv_128x28x28x64x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c2466816 = arith.constant 2466816 : index
        %c45760512 = arith.constant 45760512 : index
        %c46771392 = arith.constant 46771392 : index
        %c46770880 = arith.constant 46770880 : index
        %c46770368 = arith.constant 46770368 : index
        %c0 = arith.constant 0 : index
        %0 = stream.binding.subspan %arg0[%c2466816] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x58x58xf32>>
        %1 = stream.binding.subspan %arg1[%c45760512] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x64x3x3xf32>>
        %2 = stream.binding.subspan %arg1[%c46771392] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %3 = stream.binding.subspan %arg1[%c46770880] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %4 = stream.binding.subspan %arg1[%c46770368] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %5 = stream.binding.subspan %arg2[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<128x30x30xf32>>
        %6 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [64, 58, 58], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x58x58xf32>> -> tensor<64x58x58xf32>
        %7 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [128, 64, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x64x3x3xf32>> -> tensor<128x64x3x3xf32>
        %8 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %9 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %10 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %11 = tensor.empty() : tensor<128x28x28xf32>
        %12 = linalg.fill ins(%cst_0 : f32) outs(%11 : tensor<128x28x28xf32>) -> tensor<128x28x28xf32>
        %13 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 * 2 + d4, d2 * 2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%6, %7 : tensor<64x58x58xf32>, tensor<128x64x3x3xf32>) outs(%12 : tensor<128x28x28xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %15 = arith.mulf %in, %in_1 : f32
          %16 = arith.addf %out, %15 : f32
          linalg.yield %16 : f32
        } -> tensor<128x28x28xf32>
        %14 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%13, %8, %9, %10 : tensor<128x28x28xf32>, tensor<128xf32>, tensor<128xf32>, tensor<128xf32>) outs(%11 : tensor<128x28x28xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %out: f32):
          %15 = arith.subf %in, %in_1 : f32
          %16 = arith.mulf %15, %cst : f32
          %17 = arith.mulf %16, %in_2 : f32
          %18 = arith.addf %17, %in_3 : f32
          %19 = arith.cmpf ugt, %18, %cst_0 : f32
          %20 = arith.select %19, %18, %cst_0 : f32
          linalg.yield %20 : f32
        } -> tensor<128x28x28xf32>
        iree_tensor_ext.dispatch.tensor.store %14, %5, offsets = [0, 1, 1], sizes = [128, 28, 28], strides = [1, 1, 1] : tensor<128x28x28xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<128x30x30xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_11 {
    stream.executable.export public @main$async_dispatch_11_conv_128x28x28x128x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_11_conv_128x28x28x128x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.000000e+00 : f32
        %c0 = arith.constant 0 : index
        %c45170688 = arith.constant 45170688 : index
        %c460800 = arith.constant 460800 : index
        %0 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x30x30xf32>>
        %1 = stream.binding.subspan %arg1[%c45170688] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x128x3x3xf32>>
        %2 = stream.binding.subspan %arg2[%c460800] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<128x28x28xf32>>
        %3 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [128, 30, 30], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x30x30xf32>> -> tensor<128x30x30xf32>
        %4 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [128, 128, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x128x3x3xf32>> -> tensor<128x128x3x3xf32>
        %5 = tensor.empty() : tensor<128x28x28xf32>
        %6 = linalg.fill ins(%cst : f32) outs(%5 : tensor<128x28x28xf32>) -> tensor<128x28x28xf32>
        %7 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%3, %4 : tensor<128x30x30xf32>, tensor<128x128x3x3xf32>) outs(%6 : tensor<128x28x28xf32>) {
        ^bb0(%in: f32, %in_0: f32, %out: f32):
          %8 = arith.mulf %in, %in_0 : f32
          %9 = arith.addf %out, %8 : f32
          linalg.yield %9 : f32
        } -> tensor<128x28x28xf32>
        iree_tensor_ext.dispatch.tensor.store %7, %2, offsets = [0, 0, 0], sizes = [128, 28, 28], strides = [1, 1, 1] : tensor<128x28x28xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<128x28x28xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_12 {
    stream.executable.export public @main$async_dispatch_12_matmul_like_128x28x28x64_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_12_matmul_like_128x28x28x64_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c1664000 = arith.constant 1664000 : index
        %c460800 = arith.constant 460800 : index
        %c46055424 = arith.constant 46055424 : index
        %c46769856 = arith.constant 46769856 : index
        %c46769344 = arith.constant 46769344 : index
        %c46768832 = arith.constant 46768832 : index
        %c46772928 = arith.constant 46772928 : index
        %c46772416 = arith.constant 46772416 : index
        %c46771904 = arith.constant 46771904 : index
        %c0 = arith.constant 0 : index
        %0 = stream.binding.subspan %arg0[%c1664000] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x56x56xf32>>
        %1 = stream.binding.subspan %arg1[%c46055424] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x64xf32>>
        %2 = stream.binding.subspan %arg0[%c460800] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x28x28xf32>>
        %3 = stream.binding.subspan %arg1[%c46769856] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %4 = stream.binding.subspan %arg1[%c46769344] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %5 = stream.binding.subspan %arg1[%c46768832] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %6 = stream.binding.subspan %arg1[%c46772928] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %7 = stream.binding.subspan %arg1[%c46772416] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %8 = stream.binding.subspan %arg1[%c46771904] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %9 = stream.binding.subspan %arg2[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<128x28x28xf32>>
        %10 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0], sizes = [128, 64], strides = [1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x64xf32>> -> tensor<128x64xf32>
        %11 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0, 0, 0], sizes = [128, 28, 28], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x28x28xf32>> -> tensor<128x28x28xf32>
        %12 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %13 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %14 = iree_tensor_ext.dispatch.tensor.load %5, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %15 = iree_tensor_ext.dispatch.tensor.load %6, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %16 = iree_tensor_ext.dispatch.tensor.load %7, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %17 = iree_tensor_ext.dispatch.tensor.load %8, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %18 = tensor.empty() : tensor<128x28x28xf32>
        %19 = linalg.fill ins(%cst_0 : f32) outs(%18 : tensor<128x28x28xf32>) -> tensor<128x28x28xf32>
        %20 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [64, 28, 28], strides = [1, 2, 2] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<64x56x56xf32>> -> tensor<64x28x28xf32>
        %21 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3) -> (d3, d1, d2)>, affine_map<(d0, d1, d2, d3) -> (d0, d3)>, affine_map<(d0, d1, d2, d3) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction"]} ins(%20, %10 : tensor<64x28x28xf32>, tensor<128x64xf32>) outs(%19 : tensor<128x28x28xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %23 = arith.mulf %in, %in_1 : f32
          %24 = arith.addf %out, %23 : f32
          linalg.yield %24 : f32
        } -> tensor<128x28x28xf32>
        %22 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%11, %12, %13, %14, %21, %15, %16, %17 : tensor<128x28x28xf32>, tensor<128xf32>, tensor<128xf32>, tensor<128xf32>, tensor<128x28x28xf32>, tensor<128xf32>, tensor<128xf32>, tensor<128xf32>) outs(%18 : tensor<128x28x28xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %in_4: f32, %in_5: f32, %in_6: f32, %in_7: f32, %out: f32):
          %23 = arith.subf %in_4, %in_5 : f32
          %24 = arith.mulf %23, %cst : f32
          %25 = arith.mulf %24, %in_6 : f32
          %26 = arith.subf %in, %in_1 : f32
          %27 = arith.mulf %26, %cst : f32
          %28 = arith.addf %25, %in_7 : f32
          %29 = arith.mulf %27, %in_2 : f32
          %30 = arith.addf %29, %in_3 : f32
          %31 = arith.addf %30, %28 : f32
          %32 = arith.cmpf ugt, %31, %cst_0 : f32
          %33 = arith.select %32, %31, %cst_0 : f32
          linalg.yield %33 : f32
        } -> tensor<128x28x28xf32>
        iree_tensor_ext.dispatch.tensor.store %22, %9, offsets = [0, 0, 0], sizes = [128, 28, 28], strides = [1, 1, 1] : tensor<128x28x28xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<128x28x28xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_13 {
    stream.executable.export public @main$async_dispatch_13_slow_memcpy workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_13_slow_memcpy(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: i32, %arg3: i32) {
        %0 = arith.index_castui %arg2 : i32 to index
        %1 = arith.index_castui %arg3 : i32 to index
        %2:2 = util.assume.int 
            %0[<umin = 0, umax = 0>, <umin = 862208, umax = 862208, udiv = 862208>], 
            %1[<umin = 862208, umax = 862208, udiv = 862208>, <umin = 1263616, umax = 1263616, udiv = 1263616>]
          : index, index
        %3 = stream.binding.subspan %arg0[%2#0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x28x28xf32>>
        %4 = stream.binding.subspan %arg1[%2#1] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<128x30x30xf32>>
        %5 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0, 0, 0], sizes = [128, 28, 28], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x28x28xf32>> -> tensor<128x28x28xf32>
        iree_tensor_ext.dispatch.tensor.store %5, %4, offsets = [0, 1, 1], sizes = [128, 28, 28], strides = [1, 1, 1] : tensor<128x28x28xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<128x30x30xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_14 {
    stream.executable.export public @main$async_dispatch_14_conv_128x28x28x128x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_14_conv_128x28x28x128x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c862208 = arith.constant 862208 : index
        %c44580864 = arith.constant 44580864 : index
        %c46768320 = arith.constant 46768320 : index
        %c46767808 = arith.constant 46767808 : index
        %c46767296 = arith.constant 46767296 : index
        %c401408 = arith.constant 401408 : index
        %0 = stream.binding.subspan %arg0[%c862208] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x30x30xf32>>
        %1 = stream.binding.subspan %arg1[%c44580864] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x128x3x3xf32>>
        %2 = stream.binding.subspan %arg1[%c46768320] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %3 = stream.binding.subspan %arg1[%c46767808] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %4 = stream.binding.subspan %arg1[%c46767296] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %5 = stream.binding.subspan %arg2[%c401408] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<128x30x30xf32>>
        %6 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [128, 30, 30], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x30x30xf32>> -> tensor<128x30x30xf32>
        %7 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [128, 128, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x128x3x3xf32>> -> tensor<128x128x3x3xf32>
        %8 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %9 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %10 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %11 = tensor.empty() : tensor<128x28x28xf32>
        %12 = linalg.fill ins(%cst_0 : f32) outs(%11 : tensor<128x28x28xf32>) -> tensor<128x28x28xf32>
        %13 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%6, %7 : tensor<128x30x30xf32>, tensor<128x128x3x3xf32>) outs(%12 : tensor<128x28x28xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %15 = arith.mulf %in, %in_1 : f32
          %16 = arith.addf %out, %15 : f32
          linalg.yield %16 : f32
        } -> tensor<128x28x28xf32>
        %14 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%13, %8, %9, %10 : tensor<128x28x28xf32>, tensor<128xf32>, tensor<128xf32>, tensor<128xf32>) outs(%11 : tensor<128x28x28xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %out: f32):
          %15 = arith.subf %in, %in_1 : f32
          %16 = arith.mulf %15, %cst : f32
          %17 = arith.mulf %16, %in_2 : f32
          %18 = arith.addf %17, %in_3 : f32
          %19 = arith.cmpf ugt, %18, %cst_0 : f32
          %20 = arith.select %19, %18, %cst_0 : f32
          linalg.yield %20 : f32
        } -> tensor<128x28x28xf32>
        iree_tensor_ext.dispatch.tensor.store %14, %5, offsets = [0, 1, 1], sizes = [128, 28, 28], strides = [1, 1, 1] : tensor<128x28x28xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<128x30x30xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_15 {
    stream.executable.export public @main$async_dispatch_15_conv_128x28x28x128x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_15_conv_128x28x28x128x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c401408 = arith.constant 401408 : index
        %c0 = arith.constant 0 : index
        %c43991040 = arith.constant 43991040 : index
        %c46766784 = arith.constant 46766784 : index
        %c46766272 = arith.constant 46766272 : index
        %c46765760 = arith.constant 46765760 : index
        %c862208 = arith.constant 862208 : index
        %0 = stream.binding.subspan %arg0[%c401408] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x30x30xf32>>
        %1 = stream.binding.subspan %arg1[%c43991040] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x128x3x3xf32>>
        %2 = stream.binding.subspan %arg1[%c46766784] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %3 = stream.binding.subspan %arg1[%c46766272] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %4 = stream.binding.subspan %arg1[%c46765760] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>>
        %5 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x28x28xf32>>
        %6 = stream.binding.subspan %arg2[%c862208] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<128x28x28xf32>>
        %7 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [128, 30, 30], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x30x30xf32>> -> tensor<128x30x30xf32>
        %8 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [128, 128, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x128x3x3xf32>> -> tensor<128x128x3x3xf32>
        %9 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %10 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %11 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [128], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128xf32>> -> tensor<128xf32>
        %12 = iree_tensor_ext.dispatch.tensor.load %5, offsets = [0, 0, 0], sizes = [128, 28, 28], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x28x28xf32>> -> tensor<128x28x28xf32>
        %13 = tensor.empty() : tensor<128x28x28xf32>
        %14 = linalg.fill ins(%cst_0 : f32) outs(%13 : tensor<128x28x28xf32>) -> tensor<128x28x28xf32>
        %15 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%7, %8 : tensor<128x30x30xf32>, tensor<128x128x3x3xf32>) outs(%14 : tensor<128x28x28xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %17 = arith.mulf %in, %in_1 : f32
          %18 = arith.addf %out, %17 : f32
          linalg.yield %18 : f32
        } -> tensor<128x28x28xf32>
        %16 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%15, %9, %10, %11, %12 : tensor<128x28x28xf32>, tensor<128xf32>, tensor<128xf32>, tensor<128xf32>, tensor<128x28x28xf32>) outs(%13 : tensor<128x28x28xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %in_4: f32, %out: f32):
          %17 = arith.subf %in, %in_1 : f32
          %18 = arith.mulf %17, %cst : f32
          %19 = arith.mulf %18, %in_2 : f32
          %20 = arith.addf %19, %in_3 : f32
          %21 = arith.addf %20, %in_4 : f32
          %22 = arith.cmpf ugt, %21, %cst_0 : f32
          %23 = arith.select %22, %21, %cst_0 : f32
          linalg.yield %23 : f32
        } -> tensor<128x28x28xf32>
        iree_tensor_ext.dispatch.tensor.store %16, %6, offsets = [0, 0, 0], sizes = [128, 28, 28], strides = [1, 1, 1] : tensor<128x28x28xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<128x28x28xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_17 {
    stream.executable.export public @main$async_dispatch_17_conv_256x14x14x128x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_17_conv_256x14x14x128x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c1263616 = arith.constant 1263616 : index
        %c42680320 = arith.constant 42680320 : index
        %c46761664 = arith.constant 46761664 : index
        %c46760640 = arith.constant 46760640 : index
        %c46759616 = arith.constant 46759616 : index
        %c0 = arith.constant 0 : index
        %0 = stream.binding.subspan %arg0[%c1263616] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x30x30xf32>>
        %1 = stream.binding.subspan %arg1[%c42680320] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x128x3x3xf32>>
        %2 = stream.binding.subspan %arg1[%c46761664] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %3 = stream.binding.subspan %arg1[%c46760640] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %4 = stream.binding.subspan %arg1[%c46759616] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %5 = stream.binding.subspan %arg2[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<256x16x16xf32>>
        %6 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [128, 30, 30], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x30x30xf32>> -> tensor<128x30x30xf32>
        %7 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [256, 128, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x128x3x3xf32>> -> tensor<256x128x3x3xf32>
        %8 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %9 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %10 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %11 = tensor.empty() : tensor<256x14x14xf32>
        %12 = linalg.fill ins(%cst_0 : f32) outs(%11 : tensor<256x14x14xf32>) -> tensor<256x14x14xf32>
        %13 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 * 2 + d4, d2 * 2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%6, %7 : tensor<128x30x30xf32>, tensor<256x128x3x3xf32>) outs(%12 : tensor<256x14x14xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %15 = arith.mulf %in, %in_1 : f32
          %16 = arith.addf %out, %15 : f32
          linalg.yield %16 : f32
        } -> tensor<256x14x14xf32>
        %14 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%13, %8, %9, %10 : tensor<256x14x14xf32>, tensor<256xf32>, tensor<256xf32>, tensor<256xf32>) outs(%11 : tensor<256x14x14xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %out: f32):
          %15 = arith.subf %in, %in_1 : f32
          %16 = arith.mulf %15, %cst : f32
          %17 = arith.mulf %16, %in_2 : f32
          %18 = arith.addf %17, %in_3 : f32
          %19 = arith.cmpf ugt, %18, %cst_0 : f32
          %20 = arith.select %19, %18, %cst_0 : f32
          linalg.yield %20 : f32
        } -> tensor<256x14x14xf32>
        iree_tensor_ext.dispatch.tensor.store %14, %5, offsets = [0, 1, 1], sizes = [256, 14, 14], strides = [1, 1, 1] : tensor<256x14x14xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<256x16x16xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_18 {
    stream.executable.export public @main$async_dispatch_18_conv_256x14x14x256x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_18_conv_256x14x14x256x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.000000e+00 : f32
        %c0 = arith.constant 0 : index
        %c40321024 = arith.constant 40321024 : index
        %c262144 = arith.constant 262144 : index
        %0 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x16x16xf32>>
        %1 = stream.binding.subspan %arg1[%c40321024] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x256x3x3xf32>>
        %2 = stream.binding.subspan %arg2[%c262144] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<256x14x14xf32>>
        %3 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [256, 16, 16], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x16x16xf32>> -> tensor<256x16x16xf32>
        %4 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [256, 256, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x256x3x3xf32>> -> tensor<256x256x3x3xf32>
        %5 = tensor.empty() : tensor<256x14x14xf32>
        %6 = linalg.fill ins(%cst : f32) outs(%5 : tensor<256x14x14xf32>) -> tensor<256x14x14xf32>
        %7 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%3, %4 : tensor<256x16x16xf32>, tensor<256x256x3x3xf32>) outs(%6 : tensor<256x14x14xf32>) {
        ^bb0(%in: f32, %in_0: f32, %out: f32):
          %8 = arith.mulf %in, %in_0 : f32
          %9 = arith.addf %out, %8 : f32
          linalg.yield %9 : f32
        } -> tensor<256x14x14xf32>
        iree_tensor_ext.dispatch.tensor.store %7, %2, offsets = [0, 0, 0], sizes = [256, 14, 14], strides = [1, 1, 1] : tensor<256x14x14xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<256x14x14xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_19 {
    stream.executable.export public @main$async_dispatch_19_matmul_like_256x14x14x128_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_19_matmul_like_256x14x14x128_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c862208 = arith.constant 862208 : index
        %c262144 = arith.constant 262144 : index
        %c43859968 = arith.constant 43859968 : index
        %c46758592 = arith.constant 46758592 : index
        %c46757568 = arith.constant 46757568 : index
        %c46756544 = arith.constant 46756544 : index
        %c46764736 = arith.constant 46764736 : index
        %c46763712 = arith.constant 46763712 : index
        %c46762688 = arith.constant 46762688 : index
        %c0 = arith.constant 0 : index
        %0 = stream.binding.subspan %arg0[%c862208] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x28x28xf32>>
        %1 = stream.binding.subspan %arg1[%c43859968] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x128xf32>>
        %2 = stream.binding.subspan %arg0[%c262144] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x14x14xf32>>
        %3 = stream.binding.subspan %arg1[%c46758592] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %4 = stream.binding.subspan %arg1[%c46757568] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %5 = stream.binding.subspan %arg1[%c46756544] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %6 = stream.binding.subspan %arg1[%c46764736] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %7 = stream.binding.subspan %arg1[%c46763712] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %8 = stream.binding.subspan %arg1[%c46762688] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %9 = stream.binding.subspan %arg2[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<256x14x14xf32>>
        %10 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0], sizes = [256, 128], strides = [1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x128xf32>> -> tensor<256x128xf32>
        %11 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0, 0, 0], sizes = [256, 14, 14], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x14x14xf32>> -> tensor<256x14x14xf32>
        %12 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %13 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %14 = iree_tensor_ext.dispatch.tensor.load %5, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %15 = iree_tensor_ext.dispatch.tensor.load %6, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %16 = iree_tensor_ext.dispatch.tensor.load %7, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %17 = iree_tensor_ext.dispatch.tensor.load %8, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %18 = tensor.empty() : tensor<256x14x14xf32>
        %19 = linalg.fill ins(%cst_0 : f32) outs(%18 : tensor<256x14x14xf32>) -> tensor<256x14x14xf32>
        %20 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [128, 14, 14], strides = [1, 2, 2] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<128x28x28xf32>> -> tensor<128x14x14xf32>
        %21 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3) -> (d3, d1, d2)>, affine_map<(d0, d1, d2, d3) -> (d0, d3)>, affine_map<(d0, d1, d2, d3) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction"]} ins(%20, %10 : tensor<128x14x14xf32>, tensor<256x128xf32>) outs(%19 : tensor<256x14x14xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %23 = arith.mulf %in, %in_1 : f32
          %24 = arith.addf %out, %23 : f32
          linalg.yield %24 : f32
        } -> tensor<256x14x14xf32>
        %22 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%11, %12, %13, %14, %21, %15, %16, %17 : tensor<256x14x14xf32>, tensor<256xf32>, tensor<256xf32>, tensor<256xf32>, tensor<256x14x14xf32>, tensor<256xf32>, tensor<256xf32>, tensor<256xf32>) outs(%18 : tensor<256x14x14xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %in_4: f32, %in_5: f32, %in_6: f32, %in_7: f32, %out: f32):
          %23 = arith.subf %in_4, %in_5 : f32
          %24 = arith.mulf %23, %cst : f32
          %25 = arith.mulf %24, %in_6 : f32
          %26 = arith.subf %in, %in_1 : f32
          %27 = arith.mulf %26, %cst : f32
          %28 = arith.addf %25, %in_7 : f32
          %29 = arith.mulf %27, %in_2 : f32
          %30 = arith.addf %29, %in_3 : f32
          %31 = arith.addf %30, %28 : f32
          %32 = arith.cmpf ugt, %31, %cst_0 : f32
          %33 = arith.select %32, %31, %cst_0 : f32
          linalg.yield %33 : f32
        } -> tensor<256x14x14xf32>
        iree_tensor_ext.dispatch.tensor.store %22, %9, offsets = [0, 0, 0], sizes = [256, 14, 14], strides = [1, 1, 1] : tensor<256x14x14xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<256x14x14xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_20 {
    stream.executable.export public @main$async_dispatch_20_slow_memcpy workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_20_slow_memcpy(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: i32, %arg3: i32) {
        %0 = arith.index_castui %arg2 : i32 to index
        %1 = arith.index_castui %arg3 : i32 to index
        %2:2 = util.assume.int 
            %0[<umin = 0, umax = 0>, <umin = 462848, umax = 462848, udiv = 462848>], 
            %1[<umin = 462848, umax = 462848, udiv = 462848>, <umin = 663552, umax = 663552, udiv = 663552>]
          : index, index
        %3 = stream.binding.subspan %arg0[%2#0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x14x14xf32>>
        %4 = stream.binding.subspan %arg1[%2#1] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<256x16x16xf32>>
        %5 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0, 0, 0], sizes = [256, 14, 14], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x14x14xf32>> -> tensor<256x14x14xf32>
        iree_tensor_ext.dispatch.tensor.store %5, %4, offsets = [0, 1, 1], sizes = [256, 14, 14], strides = [1, 1, 1] : tensor<256x14x14xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<256x16x16xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_21 {
    stream.executable.export public @main$async_dispatch_21_conv_256x14x14x256x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_21_conv_256x14x14x256x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c462848 = arith.constant 462848 : index
        %c37961728 = arith.constant 37961728 : index
        %c46755520 = arith.constant 46755520 : index
        %c46754496 = arith.constant 46754496 : index
        %c46753472 = arith.constant 46753472 : index
        %c200704 = arith.constant 200704 : index
        %0 = stream.binding.subspan %arg0[%c462848] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x16x16xf32>>
        %1 = stream.binding.subspan %arg1[%c37961728] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x256x3x3xf32>>
        %2 = stream.binding.subspan %arg1[%c46755520] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %3 = stream.binding.subspan %arg1[%c46754496] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %4 = stream.binding.subspan %arg1[%c46753472] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %5 = stream.binding.subspan %arg2[%c200704] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<256x16x16xf32>>
        %6 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [256, 16, 16], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x16x16xf32>> -> tensor<256x16x16xf32>
        %7 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [256, 256, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x256x3x3xf32>> -> tensor<256x256x3x3xf32>
        %8 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %9 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %10 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %11 = tensor.empty() : tensor<256x14x14xf32>
        %12 = linalg.fill ins(%cst_0 : f32) outs(%11 : tensor<256x14x14xf32>) -> tensor<256x14x14xf32>
        %13 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%6, %7 : tensor<256x16x16xf32>, tensor<256x256x3x3xf32>) outs(%12 : tensor<256x14x14xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %15 = arith.mulf %in, %in_1 : f32
          %16 = arith.addf %out, %15 : f32
          linalg.yield %16 : f32
        } -> tensor<256x14x14xf32>
        %14 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%13, %8, %9, %10 : tensor<256x14x14xf32>, tensor<256xf32>, tensor<256xf32>, tensor<256xf32>) outs(%11 : tensor<256x14x14xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %out: f32):
          %15 = arith.subf %in, %in_1 : f32
          %16 = arith.mulf %15, %cst : f32
          %17 = arith.mulf %16, %in_2 : f32
          %18 = arith.addf %17, %in_3 : f32
          %19 = arith.cmpf ugt, %18, %cst_0 : f32
          %20 = arith.select %19, %18, %cst_0 : f32
          linalg.yield %20 : f32
        } -> tensor<256x14x14xf32>
        iree_tensor_ext.dispatch.tensor.store %14, %5, offsets = [0, 1, 1], sizes = [256, 14, 14], strides = [1, 1, 1] : tensor<256x14x14xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<256x16x16xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_22 {
    stream.executable.export public @main$async_dispatch_22_conv_256x14x14x256x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_22_conv_256x14x14x256x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c200704 = arith.constant 200704 : index
        %c0 = arith.constant 0 : index
        %c35602432 = arith.constant 35602432 : index
        %c46752448 = arith.constant 46752448 : index
        %c46751424 = arith.constant 46751424 : index
        %c46750400 = arith.constant 46750400 : index
        %c462848 = arith.constant 462848 : index
        %0 = stream.binding.subspan %arg0[%c200704] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x16x16xf32>>
        %1 = stream.binding.subspan %arg1[%c35602432] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x256x3x3xf32>>
        %2 = stream.binding.subspan %arg1[%c46752448] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %3 = stream.binding.subspan %arg1[%c46751424] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %4 = stream.binding.subspan %arg1[%c46750400] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>>
        %5 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x14x14xf32>>
        %6 = stream.binding.subspan %arg2[%c462848] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<256x14x14xf32>>
        %7 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [256, 16, 16], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x16x16xf32>> -> tensor<256x16x16xf32>
        %8 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [256, 256, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x256x3x3xf32>> -> tensor<256x256x3x3xf32>
        %9 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %10 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %11 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [256], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256xf32>> -> tensor<256xf32>
        %12 = iree_tensor_ext.dispatch.tensor.load %5, offsets = [0, 0, 0], sizes = [256, 14, 14], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x14x14xf32>> -> tensor<256x14x14xf32>
        %13 = tensor.empty() : tensor<256x14x14xf32>
        %14 = linalg.fill ins(%cst_0 : f32) outs(%13 : tensor<256x14x14xf32>) -> tensor<256x14x14xf32>
        %15 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%7, %8 : tensor<256x16x16xf32>, tensor<256x256x3x3xf32>) outs(%14 : tensor<256x14x14xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %17 = arith.mulf %in, %in_1 : f32
          %18 = arith.addf %out, %17 : f32
          linalg.yield %18 : f32
        } -> tensor<256x14x14xf32>
        %16 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%15, %9, %10, %11, %12 : tensor<256x14x14xf32>, tensor<256xf32>, tensor<256xf32>, tensor<256xf32>, tensor<256x14x14xf32>) outs(%13 : tensor<256x14x14xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %in_4: f32, %out: f32):
          %17 = arith.subf %in, %in_1 : f32
          %18 = arith.mulf %17, %cst : f32
          %19 = arith.mulf %18, %in_2 : f32
          %20 = arith.addf %19, %in_3 : f32
          %21 = arith.addf %20, %in_4 : f32
          %22 = arith.cmpf ugt, %21, %cst_0 : f32
          %23 = arith.select %22, %21, %cst_0 : f32
          linalg.yield %23 : f32
        } -> tensor<256x14x14xf32>
        iree_tensor_ext.dispatch.tensor.store %16, %6, offsets = [0, 0, 0], sizes = [256, 14, 14], strides = [1, 1, 1] : tensor<256x14x14xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<256x14x14xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_24 {
    stream.executable.export public @main$async_dispatch_24_conv_512x7x7x256x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_24_conv_512x7x7x256x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c663552 = arith.constant 663552 : index
        %c30359552 = arith.constant 30359552 : index
        %c46742208 = arith.constant 46742208 : index
        %c46740160 = arith.constant 46740160 : index
        %c46738112 = arith.constant 46738112 : index
        %c0 = arith.constant 0 : index
        %0 = stream.binding.subspan %arg0[%c663552] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x16x16xf32>>
        %1 = stream.binding.subspan %arg1[%c30359552] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x256x3x3xf32>>
        %2 = stream.binding.subspan %arg1[%c46742208] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %3 = stream.binding.subspan %arg1[%c46740160] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %4 = stream.binding.subspan %arg1[%c46738112] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %5 = stream.binding.subspan %arg2[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<512x9x9xf32>>
        %6 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [256, 16, 16], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x16x16xf32>> -> tensor<256x16x16xf32>
        %7 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [512, 256, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x256x3x3xf32>> -> tensor<512x256x3x3xf32>
        %8 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %9 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %10 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %11 = tensor.empty() : tensor<512x7x7xf32>
        %12 = linalg.fill ins(%cst_0 : f32) outs(%11 : tensor<512x7x7xf32>) -> tensor<512x7x7xf32>
        %13 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 * 2 + d4, d2 * 2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%6, %7 : tensor<256x16x16xf32>, tensor<512x256x3x3xf32>) outs(%12 : tensor<512x7x7xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %15 = arith.mulf %in, %in_1 : f32
          %16 = arith.addf %out, %15 : f32
          linalg.yield %16 : f32
        } -> tensor<512x7x7xf32>
        %14 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%13, %8, %9, %10 : tensor<512x7x7xf32>, tensor<512xf32>, tensor<512xf32>, tensor<512xf32>) outs(%11 : tensor<512x7x7xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %out: f32):
          %15 = arith.subf %in, %in_1 : f32
          %16 = arith.mulf %15, %cst : f32
          %17 = arith.mulf %16, %in_2 : f32
          %18 = arith.addf %17, %in_3 : f32
          %19 = arith.cmpf ugt, %18, %cst_0 : f32
          %20 = arith.select %19, %18, %cst_0 : f32
          linalg.yield %20 : f32
        } -> tensor<512x7x7xf32>
        iree_tensor_ext.dispatch.tensor.store %14, %5, offsets = [0, 1, 1], sizes = [512, 7, 7], strides = [1, 1, 1] : tensor<512x7x7xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<512x9x9xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_25 {
    stream.executable.export public @main$async_dispatch_25_conv_512x7x7x512x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_25_conv_512x7x7x512x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.000000e+00 : f32
        %c0 = arith.constant 0 : index
        %c20922368 = arith.constant 20922368 : index
        %c165888 = arith.constant 165888 : index
        %0 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x9x9xf32>>
        %1 = stream.binding.subspan %arg1[%c20922368] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x512x3x3xf32>>
        %2 = stream.binding.subspan %arg2[%c165888] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<512x7x7xf32>>
        %3 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [512, 9, 9], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x9x9xf32>> -> tensor<512x9x9xf32>
        %4 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [512, 512, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x512x3x3xf32>> -> tensor<512x512x3x3xf32>
        %5 = tensor.empty() : tensor<512x7x7xf32>
        %6 = linalg.fill ins(%cst : f32) outs(%5 : tensor<512x7x7xf32>) -> tensor<512x7x7xf32>
        %7 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%3, %4 : tensor<512x9x9xf32>, tensor<512x512x3x3xf32>) outs(%6 : tensor<512x7x7xf32>) {
        ^bb0(%in: f32, %in_0: f32, %out: f32):
          %8 = arith.mulf %in, %in_0 : f32
          %9 = arith.addf %out, %8 : f32
          linalg.yield %9 : f32
        } -> tensor<512x7x7xf32>
        iree_tensor_ext.dispatch.tensor.store %7, %2, offsets = [0, 0, 0], sizes = [512, 7, 7], strides = [1, 1, 1] : tensor<512x7x7xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<512x7x7xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_26 {
    stream.executable.export public @main$async_dispatch_26_matmul_like_512x7x7x256_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_26_matmul_like_512x7x7x256_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c462848 = arith.constant 462848 : index
        %c165888 = arith.constant 165888 : index
        %c35078144 = arith.constant 35078144 : index
        %c46736064 = arith.constant 46736064 : index
        %c46734016 = arith.constant 46734016 : index
        %c46731968 = arith.constant 46731968 : index
        %c46748352 = arith.constant 46748352 : index
        %c46746304 = arith.constant 46746304 : index
        %c46744256 = arith.constant 46744256 : index
        %c0 = arith.constant 0 : index
        %0 = stream.binding.subspan %arg0[%c462848] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x14x14xf32>>
        %1 = stream.binding.subspan %arg1[%c35078144] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x256xf32>>
        %2 = stream.binding.subspan %arg0[%c165888] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x7x7xf32>>
        %3 = stream.binding.subspan %arg1[%c46736064] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %4 = stream.binding.subspan %arg1[%c46734016] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %5 = stream.binding.subspan %arg1[%c46731968] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %6 = stream.binding.subspan %arg1[%c46748352] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %7 = stream.binding.subspan %arg1[%c46746304] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %8 = stream.binding.subspan %arg1[%c46744256] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %9 = stream.binding.subspan %arg2[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<512x7x7xf32>>
        %10 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0], sizes = [512, 256], strides = [1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x256xf32>> -> tensor<512x256xf32>
        %11 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0, 0, 0], sizes = [512, 7, 7], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x7x7xf32>> -> tensor<512x7x7xf32>
        %12 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %13 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %14 = iree_tensor_ext.dispatch.tensor.load %5, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %15 = iree_tensor_ext.dispatch.tensor.load %6, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %16 = iree_tensor_ext.dispatch.tensor.load %7, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %17 = iree_tensor_ext.dispatch.tensor.load %8, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %18 = tensor.empty() : tensor<512x7x7xf32>
        %19 = linalg.fill ins(%cst_0 : f32) outs(%18 : tensor<512x7x7xf32>) -> tensor<512x7x7xf32>
        %20 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [256, 7, 7], strides = [1, 2, 2] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<256x14x14xf32>> -> tensor<256x7x7xf32>
        %21 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3) -> (d3, d1, d2)>, affine_map<(d0, d1, d2, d3) -> (d0, d3)>, affine_map<(d0, d1, d2, d3) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction"]} ins(%20, %10 : tensor<256x7x7xf32>, tensor<512x256xf32>) outs(%19 : tensor<512x7x7xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %23 = arith.mulf %in, %in_1 : f32
          %24 = arith.addf %out, %23 : f32
          linalg.yield %24 : f32
        } -> tensor<512x7x7xf32>
        %22 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%11, %12, %13, %14, %21, %15, %16, %17 : tensor<512x7x7xf32>, tensor<512xf32>, tensor<512xf32>, tensor<512xf32>, tensor<512x7x7xf32>, tensor<512xf32>, tensor<512xf32>, tensor<512xf32>) outs(%18 : tensor<512x7x7xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %in_4: f32, %in_5: f32, %in_6: f32, %in_7: f32, %out: f32):
          %23 = arith.subf %in_4, %in_5 : f32
          %24 = arith.mulf %23, %cst : f32
          %25 = arith.mulf %24, %in_6 : f32
          %26 = arith.subf %in, %in_1 : f32
          %27 = arith.mulf %26, %cst : f32
          %28 = arith.addf %25, %in_7 : f32
          %29 = arith.mulf %27, %in_2 : f32
          %30 = arith.addf %29, %in_3 : f32
          %31 = arith.addf %30, %28 : f32
          %32 = arith.cmpf ugt, %31, %cst_0 : f32
          %33 = arith.select %32, %31, %cst_0 : f32
          linalg.yield %33 : f32
        } -> tensor<512x7x7xf32>
        iree_tensor_ext.dispatch.tensor.store %22, %9, offsets = [0, 0, 0], sizes = [512, 7, 7], strides = [1, 1, 1] : tensor<512x7x7xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<512x7x7xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_27 {
    stream.executable.export public @main$async_dispatch_27_slow_memcpy workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_27_slow_memcpy(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}) {
        %c0 = arith.constant 0 : index
        %c266240 = arith.constant 266240 : index
        %0 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x7x7xf32>>
        %1 = stream.binding.subspan %arg1[%c266240] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<512x9x9xf32>>
        %2 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [512, 7, 7], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x7x7xf32>> -> tensor<512x7x7xf32>
        iree_tensor_ext.dispatch.tensor.store %2, %1, offsets = [0, 1, 1], sizes = [512, 7, 7], strides = [1, 1, 1] : tensor<512x7x7xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<512x9x9xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_28 {
    stream.executable.export public @main$async_dispatch_28_conv_512x7x7x512x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_28_conv_512x7x7x512x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c266240 = arith.constant 266240 : index
        %c11485184 = arith.constant 11485184 : index
        %c46729920 = arith.constant 46729920 : index
        %c46727872 = arith.constant 46727872 : index
        %c46725824 = arith.constant 46725824 : index
        %c100352 = arith.constant 100352 : index
        %0 = stream.binding.subspan %arg0[%c266240] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x9x9xf32>>
        %1 = stream.binding.subspan %arg1[%c11485184] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x512x3x3xf32>>
        %2 = stream.binding.subspan %arg1[%c46729920] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %3 = stream.binding.subspan %arg1[%c46727872] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %4 = stream.binding.subspan %arg1[%c46725824] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %5 = stream.binding.subspan %arg2[%c100352] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<512x9x9xf32>>
        %6 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [512, 9, 9], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x9x9xf32>> -> tensor<512x9x9xf32>
        %7 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [512, 512, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x512x3x3xf32>> -> tensor<512x512x3x3xf32>
        %8 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %9 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %10 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %11 = tensor.empty() : tensor<512x7x7xf32>
        %12 = linalg.fill ins(%cst_0 : f32) outs(%11 : tensor<512x7x7xf32>) -> tensor<512x7x7xf32>
        %13 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%6, %7 : tensor<512x9x9xf32>, tensor<512x512x3x3xf32>) outs(%12 : tensor<512x7x7xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %15 = arith.mulf %in, %in_1 : f32
          %16 = arith.addf %out, %15 : f32
          linalg.yield %16 : f32
        } -> tensor<512x7x7xf32>
        %14 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%13, %8, %9, %10 : tensor<512x7x7xf32>, tensor<512xf32>, tensor<512xf32>, tensor<512xf32>) outs(%11 : tensor<512x7x7xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %out: f32):
          %15 = arith.subf %in, %in_1 : f32
          %16 = arith.mulf %15, %cst : f32
          %17 = arith.mulf %16, %in_2 : f32
          %18 = arith.addf %17, %in_3 : f32
          %19 = arith.cmpf ugt, %18, %cst_0 : f32
          %20 = arith.select %19, %18, %cst_0 : f32
          linalg.yield %20 : f32
        } -> tensor<512x7x7xf32>
        iree_tensor_ext.dispatch.tensor.store %14, %5, offsets = [0, 1, 1], sizes = [512, 7, 7], strides = [1, 1, 1] : tensor<512x7x7xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<512x9x9xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_29 {
    stream.executable.export public @main$async_dispatch_29_conv_512x7x7x512x3x3_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_29_conv_512x7x7x512x3x3_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.999994993 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c100352 = arith.constant 100352 : index
        %c0 = arith.constant 0 : index
        %c2048000 = arith.constant 2048000 : index
        %c46723776 = arith.constant 46723776 : index
        %c46721728 = arith.constant 46721728 : index
        %c46719680 = arith.constant 46719680 : index
        %c432128 = arith.constant 432128 : index
        %0 = stream.binding.subspan %arg0[%c100352] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x9x9xf32>>
        %1 = stream.binding.subspan %arg1[%c2048000] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x512x3x3xf32>>
        %2 = stream.binding.subspan %arg1[%c46723776] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %3 = stream.binding.subspan %arg1[%c46721728] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %4 = stream.binding.subspan %arg1[%c46719680] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %5 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x7x7xf32>>
        %6 = stream.binding.subspan %arg2[%c432128] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<512x8x8xf32>>
        %7 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [512, 9, 9], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x9x9xf32>> -> tensor<512x9x9xf32>
        %8 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0, 0, 0], sizes = [512, 512, 3, 3], strides = [1, 1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x512x3x3xf32>> -> tensor<512x512x3x3xf32>
        %9 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %10 = iree_tensor_ext.dispatch.tensor.load %3, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %11 = iree_tensor_ext.dispatch.tensor.load %4, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %12 = iree_tensor_ext.dispatch.tensor.load %5, offsets = [0, 0, 0], sizes = [512, 7, 7], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x7x7xf32>> -> tensor<512x7x7xf32>
        %13 = tensor.empty() : tensor<512x7x7xf32>
        %14 = linalg.fill ins(%cst_0 : f32) outs(%13 : tensor<512x7x7xf32>) -> tensor<512x7x7xf32>
        %15 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2, d3, d4, d5) -> (d3, d1 + d4, d2 + d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d3, d4, d5)>, affine_map<(d0, d1, d2, d3, d4, d5) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel", "reduction", "reduction", "reduction"]} ins(%7, %8 : tensor<512x9x9xf32>, tensor<512x512x3x3xf32>) outs(%14 : tensor<512x7x7xf32>) {
        ^bb0(%in: f32, %in_1: f32, %out: f32):
          %17 = arith.mulf %in, %in_1 : f32
          %18 = arith.addf %out, %17 : f32
          linalg.yield %18 : f32
        } -> tensor<512x7x7xf32>
        %16 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>, affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} ins(%15, %9, %10, %11, %12 : tensor<512x7x7xf32>, tensor<512xf32>, tensor<512xf32>, tensor<512xf32>, tensor<512x7x7xf32>) outs(%13 : tensor<512x7x7xf32>) {
        ^bb0(%in: f32, %in_1: f32, %in_2: f32, %in_3: f32, %in_4: f32, %out: f32):
          %17 = arith.subf %in, %in_1 : f32
          %18 = arith.mulf %17, %cst : f32
          %19 = arith.mulf %18, %in_2 : f32
          %20 = arith.addf %19, %in_3 : f32
          %21 = arith.addf %20, %in_4 : f32
          %22 = arith.cmpf ugt, %21, %cst_0 : f32
          %23 = arith.select %22, %21, %cst_0 : f32
          linalg.yield %23 : f32
        } -> tensor<512x7x7xf32>
        iree_tensor_ext.dispatch.tensor.store %16, %6, offsets = [0, 0, 0], sizes = [512, 7, 7], strides = [1, 1, 1] : tensor<512x7x7xf32> -> !iree_tensor_ext.dispatch.tensor<readwrite:tensor<512x8x8xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_30 {
    stream.executable.export public @main$async_dispatch_30_reduction_512x49_i1xf32xf32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_30_reduction_512x49_i1xf32xf32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 4.900000e+01 : f32
        %cst_0 = arith.constant 0.000000e+00 : f32
        %c7 = arith.constant 7 : index
        %c432128 = arith.constant 432128 : index
        %c0 = arith.constant 0 : index
        %c2048 = arith.constant 2048 : index
        %0 = stream.binding.subspan %arg0[%c432128] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x8x8xf32>>
        %1 = stream.binding.subspan %arg1[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<512xf32>>
        %2 = stream.binding.subspan %arg2[%c2048] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<f32>>
        %3 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0, 0], sizes = [512, 8, 8], strides = [1, 1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512x8x8xf32>> -> tensor<512x8x8xf32>
        %4 = tensor.empty() : tensor<f32>
        %5 = tensor.empty() : tensor<512xf32>
        %6 = linalg.fill ins(%cst_0 : f32) outs(%5 : tensor<512xf32>) -> tensor<512xf32>
        %7 = tensor.empty() : tensor<49xi1>
        %8:2 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d1)>, affine_map<(d0, d1) -> (d0)>, affine_map<(d0, d1) -> ()>], iterator_types = ["parallel", "reduction"]} ins(%7 : tensor<49xi1>) outs(%6, %4 : tensor<512xf32>, tensor<f32>) {
        ^bb0(%in: i1, %out: f32, %out_1: f32):
          %9 = linalg.index 0 : index
          %10 = linalg.index 1 : index
          %11 = arith.remsi %10, %c7 : index
          %12 = arith.divsi %10, %c7 : index
          %extracted = tensor.extract %3[%9, %12, %11] : tensor<512x8x8xf32>
          %13 = arith.cmpi ult, %12, %c7 : index
          %14 = arith.select %13, %extracted, %cst_0 : f32
          %15 = arith.cmpi ult, %11, %c7 : index
          %16 = arith.select %15, %14, %cst_0 : f32
          %17 = arith.addf %16, %out : f32
          linalg.yield %17, %cst : f32, f32
        } -> (tensor<512xf32>, tensor<f32>)
        iree_tensor_ext.dispatch.tensor.store %8#0, %1, offsets = [0], sizes = [512], strides = [1] : tensor<512xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<512xf32>>
        iree_tensor_ext.dispatch.tensor.store %8#1, %2, offsets = [], sizes = [], strides = [] : tensor<f32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<f32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_31 {
    stream.executable.export public @main$async_dispatch_31_elementwise_512_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_31_elementwise_512_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}) {
        %c2048 = arith.constant 2048 : index
        %c0 = arith.constant 0 : index
        %c2112 = arith.constant 2112 : index
        %0 = stream.binding.subspan %arg0[%c2048] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<f32>>
        %1 = stream.binding.subspan %arg0[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>>
        %2 = stream.binding.subspan %arg1[%c2112] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<512xf32>>
        %3 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [], sizes = [], strides = [] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<f32>> -> tensor<f32>
        %4 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0], sizes = [512], strides = [1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<512xf32>> -> tensor<512xf32>
        %5 = tensor.empty() : tensor<512xf32>
        %6 = linalg.generic {indexing_maps = [affine_map<(d0) -> ()>, affine_map<(d0) -> (d0)>, affine_map<(d0) -> (d0)>], iterator_types = ["parallel"]} ins(%3, %4 : tensor<f32>, tensor<512xf32>) outs(%5 : tensor<512xf32>) {
        ^bb0(%in: f32, %in_0: f32, %out: f32):
          %7 = arith.divf %in_0, %in : f32
          linalg.yield %7 : f32
        } -> tensor<512xf32>
        iree_tensor_ext.dispatch.tensor.store %6, %2, offsets = [0], sizes = [512], strides = [1] : tensor<512xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<512xf32>>
        return
      }
    }
  }
  stream.executable private @main$async_dispatch_32 {
    stream.executable.export public @main$async_dispatch_32_matmul_1x1000x512_f32 workgroups() -> (index, index, index) {
      %x, %y, %z = iree_tensor_ext.dispatch.workgroup_count_from_slice()
      stream.return %x, %y, %z : index, index, index
    }
    builtin.module {
      func.func @main$async_dispatch_32_matmul_1x1000x512_f32(%arg0: !stream.binding {stream.alignment = 64 : index}, %arg1: !stream.binding {stream.alignment = 64 : index}, %arg2: !stream.binding {stream.alignment = 64 : index}) {
        %cst = arith.constant 0.000000e+00 : f32
        %c2112 = arith.constant 2112 : index
        %c0 = arith.constant 0 : index
        %c46715648 = arith.constant 46715648 : index
        %0 = stream.binding.subspan %arg0[%c2112] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<1x512xf32>>
        %1 = stream.binding.subspan %arg1[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<1000x512xf32>>
        %2 = stream.binding.subspan %arg1[%c46715648] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<readonly:tensor<1x1000xf32>>
        %3 = stream.binding.subspan %arg2[%c0] : !stream.binding -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<1x1000xf32>>
        %4 = iree_tensor_ext.dispatch.tensor.load %0, offsets = [0, 0], sizes = [1, 512], strides = [1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<1x512xf32>> -> tensor<1x512xf32>
        %5 = iree_tensor_ext.dispatch.tensor.load %1, offsets = [0, 0], sizes = [1000, 512], strides = [1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<1000x512xf32>> -> tensor<1000x512xf32>
        %6 = iree_tensor_ext.dispatch.tensor.load %2, offsets = [0, 0], sizes = [1, 1000], strides = [1, 1] : !iree_tensor_ext.dispatch.tensor<readonly:tensor<1x1000xf32>> -> tensor<1x1000xf32>
        %7 = tensor.empty() : tensor<1x1000xf32>
        %8 = linalg.fill ins(%cst : f32) outs(%7 : tensor<1x1000xf32>) -> tensor<1x1000xf32>
        %9 = linalg.matmul indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d2)>, affine_map<(d0, d1, d2) -> (d1, d2)>, affine_map<(d0, d1, d2) -> (d0, d1)>] ins(%4, %5 : tensor<1x512xf32>, tensor<1000x512xf32>) outs(%8 : tensor<1x1000xf32>) -> tensor<1x1000xf32>
        %10 = linalg.generic {indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> (d0, d1)>], iterator_types = ["parallel", "parallel"]} ins(%9, %6 : tensor<1x1000xf32>, tensor<1x1000xf32>) outs(%7 : tensor<1x1000xf32>) {
        ^bb0(%in: f32, %in_0: f32, %out: f32):
          %11 = arith.addf %in, %in_0 : f32
          linalg.yield %11 : f32
        } -> tensor<1x1000xf32>
        iree_tensor_ext.dispatch.tensor.store %10, %3, offsets = [0, 0], sizes = [1, 1000], strides = [1, 1] : tensor<1x1000xf32> -> !iree_tensor_ext.dispatch.tensor<writeonly:tensor<1x1000xf32>>
        return
      }
    }
  }
  util.global private @__constant_tensor_1000x512xf32 : !stream.resource<constant>
  util.initializer {
    %c0 = arith.constant 0 : index
    %c0_i64 = arith.constant 0 : i64
    %0 = stream.timepoint.immediate => !stream.timepoint
    %buffer_cst = util.buffer.constant {alignment = 64 : index} : !util.buffer = #util.composite<46773440xi8, [
    dense_resource<__auto.constant_1000_512_torch.float32> : tensor<1000x512xf32>,
    dense_resource<__auto.constant_512_512_3_3_torch.float32$2> : tensor<512x512x3x3xf32>,
    dense_resource<__auto.constant_512_512_3_3_torch.float32$1> : tensor<512x512x3x3xf32>,
    dense_resource<__auto.constant_512_512_3_3_torch.float32> : tensor<512x512x3x3xf32>,
    dense_resource<__auto.constant_512_256_3_3_torch.float32> : tensor<512x256x3x3xf32>,
    dense_resource<__auto.constant_512_256_1_1_torch.float32> : tensor<512x256x1x1xf32>,
    dense_resource<__auto.constant_256_256_3_3_torch.float32$2> : tensor<256x256x3x3xf32>,
    dense_resource<__auto.constant_256_256_3_3_torch.float32$1> : tensor<256x256x3x3xf32>,
    dense_resource<__auto.constant_256_256_3_3_torch.float32> : tensor<256x256x3x3xf32>,
    dense_resource<__auto.constant_256_128_3_3_torch.float32> : tensor<256x128x3x3xf32>,
    dense_resource<__auto.constant_256_128_1_1_torch.float32> : tensor<256x128x1x1xf32>,
    dense_resource<__auto.constant_128_128_3_3_torch.float32$2> : tensor<128x128x3x3xf32>,
    dense_resource<__auto.constant_128_128_3_3_torch.float32$1> : tensor<128x128x3x3xf32>,
    dense_resource<__auto.constant_128_128_3_3_torch.float32> : tensor<128x128x3x3xf32>,
    dense_resource<__auto.constant_128_64_3_3_torch.float32> : tensor<128x64x3x3xf32>,
    dense_resource<__auto.constant_128_64_1_1_torch.float32> : tensor<128x64x1x1xf32>,
    dense_resource<__auto.constant_64_64_3_3_torch.float32$3> : tensor<64x64x3x3xf32>,
    dense_resource<__auto.constant_64_64_3_3_torch.float32$2> : tensor<64x64x3x3xf32>,
    dense_resource<__auto.constant_64_64_3_3_torch.float32$1> : tensor<64x64x3x3xf32>,
    dense_resource<__auto.constant_64_64_3_3_torch.float32> : tensor<64x64x3x3xf32>,
    dense_resource<__auto.constant_64_3_7_7_torch.float32> : tensor<64x3x7x7xf32>,
    dense_resource<torch_tensor_1000_torch.float32> : tensor<1000xf32>,
    dense<0> : vector<32xi8>,
    dense_resource<torch_tensor_512_torch.float32_19> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_18> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_16> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_15> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_14> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_12> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_11> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_10> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_8> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_7> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_6> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_4> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_3> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32_2> : tensor<512xf32>,
    dense_resource<torch_tensor_512_torch.float32> : tensor<512xf32>,
    dense_resource<torch_tensor_256_torch.float32_19> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_18> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_16> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_15> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_14> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_12> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_11> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_10> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_8> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_7> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_6> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_4> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_3> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32_2> : tensor<256xf32>,
    dense_resource<torch_tensor_256_torch.float32> : tensor<256xf32>,
    dense_resource<torch_tensor_128_torch.float32_19> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_18> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_16> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_15> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_14> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_12> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_11> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_10> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_8> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_7> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_6> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_4> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_3> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32_2> : tensor<128xf32>,
    dense_resource<torch_tensor_128_torch.float32> : tensor<128xf32>,
]>
    %c46773440 = arith.constant 46773440 : index
    %did_map, %result = stream.resource.try_map %buffer_cst[%c0] : !util.buffer -> i1, !stream.resource<constant>{%c46773440}
    cf.cond_br %did_map, ^bb2(%0, %result : !stream.timepoint, !stream.resource<constant>), ^bb1
  ^bb1:  // pred: ^bb0
    %1 = stream.resource.alloc uninitialized : !stream.resource<constant>{%c46773440}
    %file = stream.file.constant %buffer_cst[%c0 for %c46773440] : !util.buffer{%c46773440} -> !stream.file
    %2 = stream.file.read await(%0) => %file[%c0_i64], %1[%c0], %c46773440 : !stream.file -> !stream.resource<constant>{%c46773440} => !stream.timepoint
    cf.br ^bb2(%2, %1 : !stream.timepoint, !stream.resource<constant>)
  ^bb2(%3: !stream.timepoint, %4: !stream.resource<constant>):  // 2 preds: ^bb0, ^bb1
    %5 = stream.timepoint.await sync %3 => %4 : !stream.resource<constant>{%c46773440}
    util.global.store %5, @__constant_tensor_1000x512xf32 : !stream.resource<constant>
    util.return
  }
  util.func public @main$async(%arg0: !hal.buffer_view, %arg1: !hal.fence, %arg2: !hal.fence) -> !hal.buffer_view attributes {inlining_policy = #util.inline.never, iree.abi.model = "coarse-fences", iree.abi.stub} {
    %c663552_i32 = arith.constant 663552 : i32
    %c462848_i32 = arith.constant 462848 : i32
    %c1263616_i32 = arith.constant 1263616 : i32
    %c862208_i32 = arith.constant 862208 : i32
    %c2466816_i32 = arith.constant 2466816 : i32
    %c1664000_i32 = arith.constant 1664000 : i32
    %c861184_i32 = arith.constant 861184 : i32
    %c4764608_i32 = arith.constant 4764608 : i32
    %c3961792_i32 = arith.constant 3961792 : i32
    %c0_i32 = arith.constant 0 : i32
    %c4000 = arith.constant 4000 : index
    %c100352 = arith.constant 100352 : index
    %c165888 = arith.constant 165888 : index
    %c200704 = arith.constant 200704 : index
    %c262144 = arith.constant 262144 : index
    %c131072 = arith.constant 131072 : index
    %c401408 = arith.constant 401408 : index
    %c460800 = arith.constant 460800 : index
    %c861184 = arith.constant 861184 : index
    %c-8388608_i32 = arith.constant -8388608 : i32
    %c3326976 = arith.constant 3326976 : index
    %c0 = arith.constant 0 : index
    %c634800 = arith.constant 634800 : index
    %c602112 = arith.constant 602112 : index
    %c0_i8 = arith.constant 0 : i8
    %c224 = arith.constant 224 : index
    %c3 = arith.constant 3 : index
    %c1 = arith.constant 1 : index
    %c634816 = arith.constant 634816 : index
    %c4764608 = arith.constant 4764608 : index
    %c1664000 = arith.constant 1664000 : index
    %c2466816 = arith.constant 2466816 : index
    %c862208 = arith.constant 862208 : index
    %c1263616 = arith.constant 1263616 : index
    %c462848 = arith.constant 462848 : index
    %c663552 = arith.constant 663552 : index
    %c266240 = arith.constant 266240 : index
    %c432128 = arith.constant 432128 : index
    %c5625792 = arith.constant 5625792 : index
    %c46773440 = arith.constant 46773440 : index
    %__constant_tensor_1000x512xf32 = util.global.load immutable @__constant_tensor_1000x512xf32 : !stream.resource<constant>
    %element_type_f32 = hal.element_type<f32> : i32
    %dense_row_major = hal.encoding_type<dense_row_major> : i32
    hal.buffer_view.assert<%arg0 : !hal.buffer_view> message("tensor") shape([%c1, %c3, %c224, %c224]) type(%element_type_f32) encoding(%dense_row_major)
    %0 = stream.tensor.import %arg0 : !hal.buffer_view -> tensor<1x3x224x224xf32> in !stream.resource<external>{%c602112}
    %1 = stream.timepoint.import %arg1 : (!hal.fence) => !stream.timepoint
    %result, %result_timepoint = stream.resource.alloca uninitialized await(%1) => !stream.resource<external>{%c4000} => !stream.timepoint
    %result_0, %result_timepoint_1 = stream.resource.alloca uninitialized await(%1) => !stream.resource<transient>{%c5625792} => !stream.timepoint
    %2 = stream.timepoint.join max(%result_timepoint, %result_timepoint_1) => !stream.timepoint
    %3 = stream.cmd.execute await(%2) => with(%0 as %arg3: !stream.resource<external>{%c602112}, %__constant_tensor_1000x512xf32 as %arg4: !stream.resource<constant>{%c46773440}, %result as %arg5: !stream.resource<external>{%c4000}, %result_0 as %arg6: !stream.resource<transient>{%c5625792}) {
      stream.cmd.fill %c0_i8, %arg6[%c0 for %c634800] : i8 -> !stream.resource<transient>{%c5625792}
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_0::@main$async_dispatch_0_slow_memcpy {
          ro %arg3[%c0 for %c602112] : !stream.resource<external>{%c602112},
          rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c-8388608_i32, %arg6[%c634816 for %c3326976] : i32 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_1::@main$async_dispatch_1_conv_64x112x112x3x7x7_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_2::@main$async_dispatch_2_conv_64x56x56x3x3_f32 {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c4764608 for %c861184] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_3::@main$async_dispatch_3_slow_memcpy(%c3961792_i32, %c4764608_i32 : i32, i32) {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c0 for %c861184] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_4::@main$async_dispatch_4_conv_64x56x56x64x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_5::@main$async_dispatch_5_conv_64x56x56x64x3x3_f32 {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
          wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c1664000 for %c861184] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_3::@main$async_dispatch_3_slow_memcpy(%c861184_i32, %c1664000_i32 : i32, i32) {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c0 for %c861184] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_7::@main$async_dispatch_7_conv_64x56x56x64x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_8::@main$async_dispatch_8_conv_64x56x56x64x3x3_f32 {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
          wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c2466816 for %c861184] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_3::@main$async_dispatch_3_slow_memcpy(%c1664000_i32, %c2466816_i32 : i32, i32) {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c0 for %c460800] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_10::@main$async_dispatch_10_conv_128x28x28x64x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_11::@main$async_dispatch_11_conv_128x28x28x128x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_12::@main$async_dispatch_12_matmul_like_128x28x28x64_f32 {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
          wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c862208 for %c460800] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_13::@main$async_dispatch_13_slow_memcpy(%c0_i32, %c862208_i32 : i32, i32) {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c401408 for %c460800] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_14::@main$async_dispatch_14_conv_128x28x28x128x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_15::@main$async_dispatch_15_conv_128x28x28x128x3x3_f32 {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
          wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c1263616 for %c460800] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_13::@main$async_dispatch_13_slow_memcpy(%c862208_i32, %c1263616_i32 : i32, i32) {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c0 for %c262144] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_17::@main$async_dispatch_17_conv_256x14x14x128x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_18::@main$async_dispatch_18_conv_256x14x14x256x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_19::@main$async_dispatch_19_matmul_like_256x14x14x128_f32 {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
          wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c462848 for %c262144] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_20::@main$async_dispatch_20_slow_memcpy(%c0_i32, %c462848_i32 : i32, i32) {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c200704 for %c262144] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_21::@main$async_dispatch_21_conv_256x14x14x256x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_22::@main$async_dispatch_22_conv_256x14x14x256x3x3_f32 {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
          wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c663552 for %c262144] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_20::@main$async_dispatch_20_slow_memcpy(%c462848_i32, %c663552_i32 : i32, i32) {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c0 for %c165888] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_24::@main$async_dispatch_24_conv_512x7x7x256x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_25::@main$async_dispatch_25_conv_512x7x7x512x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_26::@main$async_dispatch_26_matmul_like_512x7x7x256_f32 {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
          wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c266240 for %c165888] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_27::@main$async_dispatch_27_slow_memcpy {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c100352 for %c165888] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.concurrent {
        stream.cmd.dispatch @main$async_dispatch_28::@main$async_dispatch_28_conv_512x7x7x512x3x3_f32 {
          ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
          ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
          rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
        }
        stream.cmd.fill %c0_i8, %arg6[%c432128 for %c131072] : i8 -> !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_29::@main$async_dispatch_29_conv_512x7x7x512x3x3_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        rw %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_30::@main$async_dispatch_30_reduction_512x49_i1xf32xf32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_31::@main$async_dispatch_31_elementwise_512_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        wo %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792}
      }
      stream.cmd.dispatch @main$async_dispatch_32::@main$async_dispatch_32_matmul_1x1000x512_f32 {
        ro %arg6[%c0 for %c5625792] : !stream.resource<transient>{%c5625792},
        ro %arg4[%c0 for %c46773440] : !stream.resource<constant>{%c46773440},
        wo %arg5[%c0 for %c4000] : !stream.resource<external>{%c4000}
      }
    } => !stream.timepoint
    %4 = stream.resource.dealloca await(%3) => %result_0 : !stream.resource<transient>{%c5625792} => !stream.timepoint
    stream.timepoint.chain_external %4 => (%arg2 : !hal.fence)
    %5 = stream.tensor.export %result : tensor<1x1000xf32> in !stream.resource<external>{%c4000} -> !hal.buffer_view
    util.return %5 : !hal.buffer_view
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