//===----------------------------------------------------------------------===//
// mlir2trt - MLIR to TensorRT Converter Tool
// 
// 工作流程:
// 1. 解析MLIR文件 - 读取.mlir格式的神经网络模型
// 2. 应用MLIR优化passes - 对MLIR IR进行常量折叠、操作融合等优化
// 3. 遍历MLIR操作并转换为TensorRT层 - 使用NetworkConverter将每个MLIR操作转为TensorRT层
// 4. 构建并序列化TensorRT引擎 - 生成优化后的.engine文件
// 5. (可选) CUDA加速推理 - 使用TensorRT Runtime执行引擎进行推理
// 
// 使用方法:
//   ./mlir2trt model.mlir    # 输入MLIR文件，输出model.engine
//===----------------------------------------------------------------------===//

#include "mlir/IR/MLIRContext.h"
#include "mlir/Parser.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/Passes.h"
#include "MIC/Dialect/NNOps.h"
#include "MIC/TensorRT/TensorRTBuilder.h"
#include "MIC/TensorRT/NetworkConverter.h"

#include <fstream>
#include <iostream>
#include <string>

//======================================================================//
// 步骤9: CUDA加速推理执行 (需要在有GPU的环境中运行)
//======================================================================//
// TensorRT引擎执行流程:
// 1. 反序列化引擎: 从.engine文件加载序列化引擎
// 2. 创建执行上下文: IExecutionContext用于执行推理
// 3. 分配输入/输出缓冲区: GPU显存分配
// 4. 执行推理: 异步或同步执行
// 5. 复制输出结果: 从GPU显存拷贝到CPU内存
//
// 注意: 当前环境没有NVIDIA GPU,以下代码被注释
//======================================================================//
// void runInference(const std::string& enginePath) {
//   // 步骤9.1: 读取引擎文件
//   std::ifstream engineFile(enginePath, std::ios::binary);
//   if (!engineFile) {
//     std::cerr << "Failed to open engine file" << std::endl;
//     return;
//   }
//   
//   engineFile.seekg(0, std::ifstream::end);
//   size_t engineSize = engineFile.tellg();
//   engineFile.seekg(0, std::ifstream::beg);
//   
//   std::vector<char> engineData(engineSize);
//   engineFile.read(engineData.data(), engineSize);
//   engineFile.close();
// 
//   // 步骤9.2: 创建TensorRT运行时
//   nvinfer1::IRuntime* runtime = nvinfer1::createInferRuntime(gLogger);
//   
//   // 步骤9.3: 反序列化引擎
//   nvinfer1::ICudaEngine* engine = runtime->deserializeCudaEngine(engineData.data(), engineSize);
//   
//   // 步骤9.4: 创建执行上下文
//   nvinfer1::IExecutionContext* context = engine->createExecutionContext();
//   
//   // 步骤9.5: 准备输入数据 (示例)
//   // 实际应用中需要准备真实的输入数据
//   // float* inputData = new float[inputSize];
//   // cudaMalloc(&inputD/devicePtr, inputSize * sizeof(float));
//   // cudaMemcpy(inputDevicePtr, inputData, inputSize * sizeof(float), cudaMemcpyHostToDevice);
//   
//   // 步骤9.6: 执行推理
//   // void* buffers[2];
//   // buffers[0] = inputDevicePtr;
//   // buffers[1] = outputDevicePtr;
//   // context->executeV2(buffers);
//   
//   // 步骤9.7: 复制输出结果
//   // cudaMemcpy(outputData, outputDevicePtr, outputSize * sizeof(float), cudaMemcpyDeviceToHost);
//   
//   // 步骤9.8: 清理资源
//   // delete[] inputData;
//   // cudaFree(inputDevicePtr);
//   // cudaFree(outputDevicePtr);
//   // delete context;
//   // delete engine;
//   // delete runtime;
// }
//======================================================================//

//======================================================================//
// main - 程序入口
//======================================================================//
// 参数:
//   argc - 命令行参数个数
//   argv - 命令行参数数组
//         argv[1] = 输入的.mlir模型文件路径
// 返回值: 0表示成功，非0表示失败
//======================================================================//
int main(int argc, char **argv) {
  //----------------------------------------------------------------------//
  // 步骤1: 检查命令行参数
  //----------------------------------------------------------------------//
  if (argc != 2) {
    std::cerr << "Usage: mlir2trt model.mlir" << std::endl;
    return 1;
  }

  // 从命令行获取输入输出文件路径
  // 输入: xxx.mlir -> 输出: xxx.engine
  std::string inputPath = argv[1];
  std::string enginePath = inputPath.substr(0, inputPath.find_last_of('.')) + ".engine";

  std::cout << "Processing model: " << inputPath << std::endl;
  std::cout << "Output engine: " << enginePath << std::endl;

  //----------------------------------------------------------------------//
  // 步骤2: 初始化MLIR上下文
  //----------------------------------------------------------------------//
  // MLIRContext是MLIR的核心上下文对象,负责管理:
  // - 已加载的dialect (方言)
  // - 操作(operations)的注册
  // - 类型的管理等
  mlir::MLIRContext context;

  std::cout << "MLIR context initialized" << std::endl;

  //----------------------------------------------------------------------//
  // 步骤3: 解析MLIR文件
  //----------------------------------------------------------------------//
  // parseSourceFile() 解析.mlir文件,返回一个mlir::ModuleOp模块操作
  // ModuleOp是MLIR中的顶层操作,包含整个模型的IR表示
  // 
  // 参数:
  //   inputPath - MLIR文件路径
  //   &context  - MLIR上下文指针
  // 返回值: OwningOpRef<ModuleOp> - 模块操作的智能指针
  std::cout << "Parsing MLIR file..." << std::endl;
  
  mlir::OwningOpRef<mlir::ModuleOp> module = mlir::parseSourceFile<mlir::ModuleOp>(inputPath, &context);
  if (!module) {
    std::cerr << "Failed to parse MLIR file: " << inputPath << std::endl;
    return 1;
  }
  std::cout << "MLIR file parsed successfully" << std::endl;

  //----------------------------------------------------------------------//
  // 步骤4: 应用MLIR优化Passes
  //----------------------------------------------------------------------//
  // 可选的优化步骤:
  // - ConstantFoldPass: 常量折叠,将编译时可确定的计算结果直接替换
  //   例如: 常量 2 + 3 直接替换为 5
  // 
  // - FusionPass: 操作融合,将多个操作合并为一个优化后的操作
  //   例如: 将 Linear + GELU 融合为一个 FusedLinearGELU 操作
  //   减少内核启动开销,提高内存访问效率
  // 
  // - LayoutTransformPass: 布局转换,优化内存布局以提高性能
  //   例如: 将NHWC布局转换为NCHW布局以利用向量化
  //
  // PassManager工作流程:
  // 1. 创建PassManager并绑定到MLIRContext
  // 2. 添加需要的passes
  // 3. 执行passes.pipeline (优化后的IR)
  std::cout << "Applying optimization passes..." << std::endl;
  
  mlir::PassManager pm(&context);
  
  // 添加优化passes (按顺序执行)
  // pm.addPass(mlir::MIC::createConstantFoldPass());  // 先进行常量折叠
  // pm.addPass(mlir::MIC::createFusionPass());        // 再进行操作融合
  // pm.addPass(mlir::MIC::createLayoutTransformPass()); // 最后进行布局转换
  
  if (mlir::failed(pm.run(*module))) {
    std::cerr << "Optimization passes failed" << std::endl;
    return 1;
  }
  std::cout << "Optimization passes completed" << std::endl;

  //----------------------------------------------------------------------//
  // 步骤5: 创建TensorRT构建器
  //----------------------------------------------------------------------//
  // TensorRTBuilder负责:
  // - 创建TensorRT网络定义 (INetworkDefinition)
  // - 配置构建选项 (FP16/INT8精度,工作空间大小等)
  // - 最终生成序列化的引擎 (IHostMemory)
  //
  // 内部实现:
  // 1. 调用 nvinfer1::createInferBuilder() 创建IBuilder
  // 2. 调用 builder->createNetworkV2() 创建INetworkDefinition
  // 3. 调用 builder->createBuilderConfig() 创建IBuilderConfig
  std::cout << "Creating TensorRT builder..." << std::endl;
  
  mlir::MIC::TensorRT::TensorRTBuilder builder;
  std::cout << "TensorRT builder created" << std::endl;

  //----------------------------------------------------------------------//
  // 步骤6: 配置TensorRT构建选项
  //----------------------------------------------------------------------//
  // setFP16Mode: 启用FP16半精度推理
  //   优点: 加速推理、减少显存使用
  //   适用: 对精度要求不太高的场景
  builder.setFP16Mode(true);
  
  // setINT8Mode: 启用INT8量化推理
  //   优点: 进一步加速、减少显存
  //   需要: 校准数据集
  // builder.setINT8Mode(true);
  
  // setMaxBatchSize: 设置最大批处理大小
  //   影响: 批处理越大吞吐量越高
  // builder.setMaxBatchSize(32);
  
  // setMaxWorkspaceSize: 设置GPU工作空间大小(1GB = 1ULL << 30)
  //   TensorRT在构建引擎时需要临时显存空间进行优化
  //   更大的工作空间可以获得更好的优化效果
  builder.setMaxWorkspaceSize(1ULL << 30);

  // 获取网络定义对象,用于添加层
  auto* network = builder.getNetwork();
  std::cout << "TensorRT network created (empty)" << std::endl;

  //----------------------------------------------------------------------//
  // 步骤7: 转换MLIR操作为TensorRT层
  //----------------------------------------------------------------------//
  // NetworkConverter遍历MLIR模块中的每个操作,
  // 并将其转换为对应的TensorRT层:
  //
  // MLIR操作              TensorRT层                  转换函数
  // -----------------------------------------------------------------
  // LinearOp          -> IFullyConnectedLayer    convertLinearOp()
  // Conv2DOp          -> IConvolutionLayer       convertConv2DOp()
  // GELUOp            -> IActivationLayer       convertGELUOp()
  // LayerNormOp       -> INormalizationLayer    convertLayerNormOp()
  // ReluOp            -> IActivationLayer       convertReluOp()
  // AddOp             -> IElementWiseLayer     convertAddOp()
  // MulOp             -> IElementWiseLayer     convertMulOp()
  // MatMulOp          -> IMatrixMultiplyLayer   convertMatMulOp()
  // ReshapeOp         -> IShuffleLayer         convertReshapeOp()
  // TransposeOp       -> IShuffleLayer         convertTransposeOp()
  // SoftmaxOp         -> ISoftMaxLayer         convertSoftmaxOp()
  //
  // 转换过程详解:
  // 1. 遍历module中的所有操作 (使用 module->getOps())
  // 2. 使用dyn_cast<>判断操作类型
  // 3. 从MLIR操作中获取操作数 (operands)
  // 4. 从valueMap中查找对应的TensorRT ITensor
  // 5. 调用TensorRT API创建对应的层
  // 6. 获取层的输出ITensor
  // 7. 将结果存入valueMap供后续操作使用
  //
  // 转换示例 (LinearOp):
  //   MLIR: %result = mic.linear(%input, %weight, %bias)
  //   TensorRT: IFullyConnectedLayer(input, weight, bias) -> output
  std::cout << "Converting MLIR operations to TensorRT layers..." << std::endl;
  
  mlir::MIC::TensorRT::NetworkConverter converter(builder);
  
  // 遍历模块中的所有操作并转换
  int convertedOps = 0;
  module->walk([&](mlir::Operation* op) {
    // 跳过最外层的module操作
    if (op->getName().getStringRef() == "module") {
      return;
    }
    
    if (mlir::succeeded(converter.convertOperation(op))) {
      convertedOps++;
      std::cout << "  Converted: " << op->getName().getStringRef().str() << std::endl;
    } else {
      std::cerr << "  Warning: Failed to convert operation: " << op->getName().getStringRef().str() << std::endl;
    }
  });
  
  std::cout << "Converted " << convertedOps << " operations to TensorRT layers" << std::endl;

  //----------------------------------------------------------------------//
  // 步骤8: 构建并序列化TensorRT引擎
  //----------------------------------------------------------------------//
  // build() 内部调用TensorRT构建流程:
  //
  // 1. TensorRT优化器分析网络图
  //    - 消除冗余操作
  //    - 重新计算张量维度
  //    - 识别可融合的操作
  //
  // 2. 层融合优化
  //    - Conv + BN + ReLU -> ConvBNReLU fusion
  //    - MatMul + Add -> FullyConnected fusion
  //    - 多个 eltwise 操作融合
  //
  // 3. 内核自动选择
  //    - 为每个层选择最优的CUDA kernel
  //    - 考虑数据类型、tensor形状等因素
  //
  // 4. 内存优化
  //    - 张量重用
  //    - 内存池分配
  //
  // 5. 生成序列化引擎
  //    - 输出plan格式的二进制数据
  //    - 包含网络结构和优化后的内核
  //
  // 返回值: IHostMemory (序列化后的引擎数据)
  std::cout << "Building TensorRT engine..." << std::endl;
  
  auto serializedModel = builder.build();
  if (!serializedModel) {
    std::cerr << "Failed to build TensorRT engine" << std::endl;
    return 1;
  }
  std::cout << "TensorRT engine built successfully" << std::endl;

  //----------------------------------------------------------------------//
  // 步骤9: 保存引擎文件
  //----------------------------------------------------------------------//
  // 将序列化的引擎数据写入.engine文件
  // .engine文件是TensorRT的二进制格式,包含:
  // - 网络结构定义 (层类型、连接关系)
  // - 优化后的GPU内核 (编译后的CUDA code)
  // - 权重数据 (训练好的参数)
  // - 推理配置 (批处理大小、精度等)
  //
  // .engine文件特点:
  // - 平台: 针对特定GPU架构编译
  // - 不可移植: 不同GPU需要重新构建
  // - 加载快速: 无需重新解析和优化
  std::cout << "Saving engine to file: " << enginePath << std::endl;
  
  std::ofstream outputFile(enginePath, std::ios::binary);
  if (!outputFile.is_open()) {
    std::cerr << "Failed to open output file: " << enginePath << std::endl;
    return 1;
  }
  
  outputFile.write(static_cast<const char*>(serializedModel->data()), serializedModel->size());
  outputFile.close();
  
  std::cout << "Engine saved successfully (" << serializedModel->size() << " bytes)" << std::endl;

  //----------------------------------------------------------------------//
  // 步骤10: (可选) CUDA加速推理执行
  //----------------------------------------------------------------------//
  // 在实际部署中,可以使用生成的.engine文件进行推理:
  //
  // std::cout << "Running inference with CUDA..." << std::endl;
  // runInference(enginePath);
  //
  // 推理执行流程:
  // 1. 反序列化引擎 (从文件加载)
  // 2. 创建执行上下文
  // 3. 分配GPU显存
  // 4. 复制输入数据到GPU
  // 5. 执行推理 (executeV2)
  // 6. 复制输出数据到CPU
  // 7. 释放资源

  std::cout << "Done! Model converted successfully." << std::endl;
  std::cout << "Engine file: " << enginePath << std::endl;
  
  return 0;
}
