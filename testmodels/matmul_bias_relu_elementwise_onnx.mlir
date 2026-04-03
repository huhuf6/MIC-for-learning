module {
  func.func @main_graph(%arg0: !torch.vtensor<[2,8],f32>) -> !torch.vtensor<[2,16],f32> attributes {torch.onnx_meta.ir_version = 10 : si64, torch.onnx_meta.opset_version = 18 : si64, torch.onnx_meta.producer_name = "pytorch", torch.onnx_meta.producer_version = "2.11.0+cpu"} {
    %0 = torch.operator "onnx.Constant"() {torch.onnx.value = dense_resource<weight> : tensor<8x16xf32>} : () -> !torch.vtensor<[8,16],f32> 
    %1 = torch.operator "onnx.Constant"() {torch.onnx.value = dense_resource<bias> : tensor<16xf32>} : () -> !torch.vtensor<[16],f32> 
    %2 = torch.operator "onnx.Constant"() {torch.onnx.value = dense_resource<scale> : tensor<16xf32>} : () -> !torch.vtensor<[16],f32> 
    %3 = torch.operator "onnx.Constant"() {torch.onnx.value = dense_resource<shift> : tensor<16xf32>} : () -> !torch.vtensor<[16],f32> 
    %4 = torch.operator "onnx.Constant"() {torch.onnx.value = dense_resource<gate> : tensor<16xf32>} : () -> !torch.vtensor<[16],f32> 
    %none = torch.constant.none
    %5 = torch.operator "onnx.MatMul"(%arg0, %0) : (!torch.vtensor<[2,8],f32>, !torch.vtensor<[8,16],f32>) -> !torch.vtensor<[2,16],f32> 
    %6 = torch.operator "onnx.Add"(%5, %1) : (!torch.vtensor<[2,16],f32>, !torch.vtensor<[16],f32>) -> !torch.vtensor<[2,16],f32> 
    %7 = torch.operator "onnx.Relu"(%6) : (!torch.vtensor<[2,16],f32>) -> !torch.vtensor<[2,16],f32> 
    %8 = torch.operator "onnx.Mul"(%7, %2) : (!torch.vtensor<[2,16],f32>, !torch.vtensor<[16],f32>) -> !torch.vtensor<[2,16],f32> 
    %9 = torch.operator "onnx.Add"(%8, %3) : (!torch.vtensor<[2,16],f32>, !torch.vtensor<[16],f32>) -> !torch.vtensor<[2,16],f32> 
    %10 = torch.operator "onnx.Sigmoid"(%9) : (!torch.vtensor<[2,16],f32>) -> !torch.vtensor<[2,16],f32> 
    %11 = torch.operator "onnx.Mul"(%10, %9) : (!torch.vtensor<[2,16],f32>, !torch.vtensor<[2,16],f32>) -> !torch.vtensor<[2,16],f32> 
    %12 = torch.operator "onnx.Mul"(%7, %4) : (!torch.vtensor<[2,16],f32>, !torch.vtensor<[16],f32>) -> !torch.vtensor<[2,16],f32> 
    %13 = torch.operator "onnx.Add"(%11, %12) : (!torch.vtensor<[2,16],f32>, !torch.vtensor<[2,16],f32>) -> !torch.vtensor<[2,16],f32> 
    return %13 : !torch.vtensor<[2,16],f32>
  }
}

{-#
  dialect_resources: {
    builtin: {
      weight: "0x080000002575B8BC7DCDBCBC1C38A4BB642C0EBC7E0D8B3CEFC1623C251ACFBB5D472DBDC234D33B19FCCEBC6F5DE53B4BF0C93B23141D3B22C7CA3C07F9B63C640EA2BB699EDDBC41EE0ABD19AE393C2702823C4B3A443C6AC9FEBCC5B6DFBB5BCC173D74D2753C18DB3FBC4B4663BBFB7C703B3DA2E33CD7F3013DA10A9B3C5E3A8ABC140F49BC0FA3253ABD7021BC14CDA23B5C14103CEF56133B8FF9513CDF8E103C6C1906BB82D5813C2DD6BDBB2DA5893AB7542B3CBE983C3DE0A9F0BC46FB01BDE08E5CBC32018F3CE6E8AC3C731A693BE1F396BBBD5E00BCDD06323C377C01BC693712BCFBCC733C8132F93CB6B18B3D86DEFABC6033CABC6712153D94B934BCFD873ABC66BA963C01FFB53C3D55D33C1C2FF2BCC64E523D2A081BBC13E8DB3B717905BDD72334BC6A3B1DBC53BC23BC66D0AEBCF4ABB63C806138BBC103843CF0B4F4BAF321613C7F5989BC749F953799EF893C4C1503BC324EAA3C1EB8EA3B1C38A1BB459F3C3DE2251ABD3D5B82BA9435ABBC8DB69CBCB3CD2F3A61AE683C55D4063D9ED9DEBC7CD0E13BAA592A3C581556BDA6F90ABD9E9495BBA778B73B60D3A13BFE8DC93AF783DD3BFBE5143CEBBA153C4EC88DBC6E02803C5DD897BC336A8FBB057B47BD4424BFBAC92F32BAF1B29D3CB5D4E43BEBF896BCAF4F93BABC0B4CBCD6F317BCB26E1D3DF7E703BCC666223B8FD6BE3C1749973C614BE33C",
      bias: "0x0800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
      scale: "0x080000000000C03F0000C03F0000C03F0000C03F0000C03F0000C03F0000C03F0000C03F0000C03F0000C03F0000C03F0000C03F0000C03F0000C03F0000C03F0000C03F",
      shift: "0x08000000CDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3DCDCCCC3D",
      gate: "0x080000000000003F0000003F0000003F0000003F0000003F0000003F0000003F0000003F0000003F0000003F0000003F0000003F0000003F0000003F0000003F0000003F"
    }
  }
#-}

