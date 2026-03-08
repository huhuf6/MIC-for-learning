//===----------------------------------------------------------------------===//
// mlir2trt - model to TensorRT engine conversion tool
//===----------------------------------------------------------------------===//

#include "MIC/Passes/Passes.h"
#include "MIC/Dialect/NNDialect.h"
#include "MIC/TensorRT/NetworkConverter.h"
#include "MIC/TensorRT/TensorRTBuilder.h"
#include "NvOnnxParser.h"
#include "mlir/Dialect/Arithmetic/IR/Arithmetic.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/Passes.h"

#include <chrono>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>

namespace {

// ONNX/MLIR 两条导入路径共享的转换选项。
struct ConvertOptions {
  bool enableFP16 = true;
  std::uint64_t workspaceSize = 1ULL << 30; // 1 GB
  bool verbosePipeline = false;
  enum class OnnxImportMode { Auto, TRT, MLIR } onnxImportMode =
      OnnxImportMode::Auto;
  std::string onnxMlirConverter;
  bool keepTempMlir = false;
};

struct ParsedArgs {
  ConvertOptions options;
  std::string inputPath;
};

bool parseArgs(int argc, char **argv, ParsedArgs &parsed) {
  auto parseOnnxImportMode = [](const std::string &v,
                                ConvertOptions::OnnxImportMode &out) {
    if (v == "auto") {
      out = ConvertOptions::OnnxImportMode::Auto;
      return true;
    }
    if (v == "trt") {
      out = ConvertOptions::OnnxImportMode::TRT;
      return true;
    }
    if (v == "mlir") {
      out = ConvertOptions::OnnxImportMode::MLIR;
      return true;
    }
    return false;
  };

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--verbose-pipeline") {
      parsed.options.verbosePipeline = true;
      continue;
    }
    if (arg == "--keep-temp-mlir") {
      parsed.options.keepTempMlir = true;
      continue;
    }
    if (arg.rfind("--onnx-import=", 0) == 0) {
      std::string mode = arg.substr(std::string("--onnx-import=").size());
      if (!parseOnnxImportMode(mode, parsed.options.onnxImportMode))
        return false;
      continue;
    }
    if (arg.rfind("--onnx-mlir-converter=", 0) == 0) {
      parsed.options.onnxMlirConverter =
          arg.substr(std::string("--onnx-mlir-converter=").size());
      continue;
    }
    if (!parsed.inputPath.empty())
      return false;
    parsed.inputPath = arg;
  }
  return !parsed.inputPath.empty();
}

std::string quoteShellArg(const std::string &s) {
  std::string q = "'";
  for (char c : s) {
    if (c == '\'')
      q += "'\\''";
    else
      q += c;
  }
  q += "'";
  return q;
}

bool convertOnnxToMlirViaExternalTool(const std::string &onnxPath,
                                      const ConvertOptions &options,
                                      std::string &mlirPath) {
  if (options.onnxMlirConverter.empty()) {
    std::cerr << "[ONNX->MLIR] 未指定转换器。请提供 --onnx-mlir-converter=<cmd>，"
                 "并保证其参数形式为: <cmd> <input.onnx> <output.mlir>\n";
    return false;
  }

  mlirPath = "/tmp/mlir2trt_onnx_" + std::to_string(::getpid()) + ".mlir";
  const std::string cmd = options.onnxMlirConverter + " " +
                          quoteShellArg(onnxPath) + " " +
                          quoteShellArg(mlirPath);
  std::cout << "[ONNX->MLIR] 执行外部转换器: " << cmd << "\n";
  const int rc = std::system(cmd.c_str());
  if (rc != 0) {
    std::cerr << "[ONNX->MLIR] 外部转换失败，退出码: " << rc << "\n";
    return false;
  }

  std::ifstream f(mlirPath);
  if (!f.good()) {
    std::cerr << "[ONNX->MLIR] 未生成 MLIR 文件: " << mlirPath << "\n";
    return false;
  }
  std::cout << "[ONNX->MLIR] 生成成功: " << mlirPath << "\n";
  return true;
}

void printIrSummary(mlir::ModuleOp module, const std::string &title) {
  int totalOps = 0;
  int nnOps = 0;
  int linalgOps = 0;
  int arithOps = 0;
  int tensorOps = 0;
  int funcCalls = 0;
  int micTrtCalls = 0;

  module.walk([&](mlir::Operation *op) {
    ++totalOps;
    auto ns = op->getName().getDialectNamespace();
    if (ns == "nn")
      ++nnOps;
    else if (ns == "linalg")
      ++linalgOps;
    else if (ns == "arith")
      ++arithOps;
    else if (ns == "tensor")
      ++tensorOps;

    if (auto call = llvm::dyn_cast<mlir::func::CallOp>(op)) {
      ++funcCalls;
      if (call.getCallee().startswith("mic_trt_"))
        ++micTrtCalls;
    }
  });

  std::cout << "    [IR] " << title << ": total_ops=" << totalOps
            << ", nn=" << nnOps << ", linalg=" << linalgOps
            << ", arith=" << arithOps << ", tensor=" << tensorOps
            << ", func.call=" << funcCalls << ", mic_trt_call=" << micTrtCalls
            << '\n';
}

class OnnxParserLogger final : public nvinfer1::ILogger {
public:
  void log(Severity severity, const char *msg) noexcept override {
    if (severity <= Severity::kWARNING && msg)
      std::cerr << "[TensorRT] " << msg << '\n';
  }
};

std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return s;
}

bool hasSuffix(const std::string &s, const std::string &suffixLower) {
  const std::string lower = toLower(s);
  return lower.size() >= suffixLower.size() &&
         lower.compare(lower.size() - suffixLower.size(), suffixLower.size(),
                       suffixLower) == 0;
}

std::string makeEnginePath(const std::string &inputPath) {
  std::size_t pos = inputPath.find_last_of('.');
  if (pos == std::string::npos)
    return inputPath + ".engine";
  return inputPath.substr(0, pos) + ".engine";
}

bool addDefaultDynamicProfile(nvinfer1::INetworkDefinition *network,
                              nvinfer1::IBuilder *trtBuilder,
                              nvinfer1::IBuilderConfig *trtConfig) {
  // 对 ONNX 动态输入设置一组保守的默认 profile 区间。
  auto *profile = trtBuilder->createOptimizationProfile();
  if (!profile)
    return false;

  bool hasDynamicInput = false;
  for (int i = 0; i < network->getNbInputs(); ++i) {
    auto *inputTensor = network->getInput(i);
    if (!inputTensor)
      continue;

    nvinfer1::Dims minDims = inputTensor->getDimensions();
    nvinfer1::Dims optDims = inputTensor->getDimensions();
    nvinfer1::Dims maxDims = inputTensor->getDimensions();
    bool inputIsDynamic = false;

    for (int d = 0; d < minDims.nbDims; ++d) {
      if (minDims.d[d] < 0) {
        inputIsDynamic = true;
        minDims.d[d] = 1;
        optDims.d[d] = 8;
        maxDims.d[d] = 32;
      } else if (minDims.d[d] == 0) {
        minDims.d[d] = 1;
        optDims.d[d] = 1;
        maxDims.d[d] = 1;
      }
    }

    if (!inputIsDynamic)
      continue;

    hasDynamicInput = true;
    const char *inputName = inputTensor->getName();
    if (!profile->setDimensions(inputName, nvinfer1::OptProfileSelector::kMIN,
                                minDims) ||
        !profile->setDimensions(inputName, nvinfer1::OptProfileSelector::kOPT,
                                optDims) ||
        !profile->setDimensions(inputName, nvinfer1::OptProfileSelector::kMAX,
                                maxDims)) {
      return false;
    }
  }

  if (hasDynamicInput)
    trtConfig->addOptimizationProfile(profile);
  return true;
}

bool importOnnxToNetwork(const std::string &inputPath,
                         nvinfer1::INetworkDefinition *network,
                         nvinfer1::IBuilder *trtBuilder,
                         nvinfer1::IBuilderConfig *trtConfig) {
  // ONNX 路径：
  // 1) 直接把 ONNX 解析到 TensorRT Network
  // 2) 若有动态维，补默认 optimization profile
  static OnnxParserLogger parserLogger;
  std::unique_ptr<nvonnxparser::IParser> parser(
      nvonnxparser::createParser(*network, parserLogger));
  if (!parser) {
    std::cerr << "Failed to create TensorRT ONNX parser\n";
    return false;
  }

  if (!parser->parseFromFile(
          inputPath.c_str(),
          static_cast<int>(nvinfer1::ILogger::Severity::kWARNING))) {
    std::cerr << "Failed to parse ONNX file: " << inputPath << '\n';
    const int numErrors = parser->getNbErrors();
    for (int i = 0; i < numErrors; ++i) {
      auto *err = parser->getError(i);
      if (err)
        std::cerr << "  ONNX parser error[" << i << "]: " << err->desc() << '\n';
    }
    return false;
  }

  if (!addDefaultDynamicProfile(network, trtBuilder, trtConfig)) {
    std::cerr << "Failed to configure TensorRT optimization profile\n";
    return false;
  }
  return true;
}

bool importMlirToNetwork(const std::string &inputPath,
                         const ConvertOptions &options,
                         mlir::MIC::TensorRT::TensorRTBuilder &builder) {
  // MLIR 路径：
  // 1) 解析 MLIR 模块
  // 2) 执行降低/优化 pipeline，产出后端运行时调用
  // 3) 将 lowered 后的操作转换为 TensorRT Layer
  mlir::DialectRegistry registry;
  registry.insert<mlir::func::FuncDialect, mlir::linalg::LinalgDialect,
                  mlir::tensor::TensorDialect, mlir::arith::ArithmeticDialect,
                  MIC::NN::NNDialect>();
  mlir::MLIRContext context(registry);
  context.loadAllAvailableDialects();
  mlir::ParserConfig parserConfig(&context);
  mlir::OwningOpRef<mlir::ModuleOp> module =
      mlir::parseSourceFile<mlir::ModuleOp>(inputPath, parserConfig);
  if (!module) {
    std::cerr << "Failed to parse MLIR file: " << inputPath << '\n';
    return false;
  }

  // 目标 pipeline：
  // ONNX -> NN -> Linalg -> (fuse/tile/vectorize) -> TensorRT runtime calls。
  struct PipelineStage {
    const char *name;
    std::function<std::unique_ptr<mlir::Pass>()> makePass;
  };

  const std::vector<PipelineStage> stages = {
      {"Lower ONNX to NN", [] { return mlir::MIC::createLowerONNXToNNPass(); }},
      {"Canonicalize NN IR", [] { return mlir::createCanonicalizerPass(); }},
      {"Lower NN to Linalg", [] { return mlir::MIC::createLowerNNToLinalgPass(); }},
      {"Linalg Fusion", [] { return mlir::MIC::createLinalgFusePass(); }},
      {"Linalg Tiling", [] { return mlir::MIC::createLinalgTilePass(); }},
      {"Linalg Vectorization", [] { return mlir::MIC::createLinalgVectorizePass(); }},
      {"Lower Linalg to TensorRT Calls",
       [] { return mlir::MIC::createLinalgToTensorRTPass(); }},
  };

  std::cout << "\n[MLIR Pipeline] Starting pass pipeline (" << stages.size()
            << " stages)\n";

  for (size_t i = 0; i < stages.size(); ++i) {
    const auto &stage = stages[i];
    if (options.verbosePipeline)
      printIrSummary(*module, std::string("before ") + stage.name);

    std::cout << "  [" << (i + 1) << "/" << stages.size() << "] " << stage.name
              << " ... " << std::flush;

    auto t0 = std::chrono::steady_clock::now();
    mlir::PassManager stagePM(&context);
    stagePM.addPass(stage.makePass());
    if (mlir::failed(stagePM.run(*module))) {
      auto t1 = std::chrono::steady_clock::now();
      const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
      std::cout << "FAILED (" << ms.count() << " ms)\n";
      std::cerr << "[MLIR Pipeline] Stage failed: " << stage.name << "\n";
      return false;
    }

    auto t1 = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    std::cout << "OK (" << ms.count() << " ms)\n";

    if (options.verbosePipeline)
      printIrSummary(*module, std::string("after ") + stage.name);
  }

  std::cout << "[MLIR Pipeline] Completed successfully.\n\n";

  mlir::MIC::TensorRT::NetworkConverter converter(builder);
  bool ok = true;
  module->walk([&](mlir::Operation *op) {
    if (op->getName().getStringRef() == "module")
      return;
    if (mlir::failed(converter.convertOperation(op)))
      ok = false;
  });
  if (!ok)
    std::cerr << "Some MLIR ops could not be converted to TensorRT\n";
  return ok;
}

bool saveEngine(const std::string &enginePath,
                const nvinfer1::IHostMemory &engineBytes) {
  std::ofstream outputFile(enginePath, std::ios::binary);
  if (!outputFile.is_open()) {
    std::cerr << "Failed to open output file: " << enginePath << '\n';
    return false;
  }
  outputFile.write(static_cast<const char *>(engineBytes.data()),
                   engineBytes.size());
  return outputFile.good();
}

int run(const std::string &inputPath, const ConvertOptions &options) {
  // 端到端工作流：
  // 1) 初始化 TensorRT Builder/Network/Config
  // 2) 导入 ONNX 或 MLIR 到 TensorRT Network
  // 3) 构建并序列化 engine 到磁盘
  const std::string enginePath = makeEnginePath(inputPath);
  const bool isOnnxInput = hasSuffix(inputPath, ".onnx");

  std::cout << "Processing model: " << inputPath << '\n';
  std::cout << "Output engine: " << enginePath << '\n';

  mlir::MIC::TensorRT::TensorRTBuilder builder;
  builder.setFP16Mode(options.enableFP16);
  builder.setMaxWorkspaceSize(options.workspaceSize);

  auto *network = builder.getNetwork();
  auto *trtBuilder = builder.getBuilder();
  auto *trtConfig = builder.getConfig();

  bool imported = false;
  if (isOnnxInput) {
    const bool forceMlirPath =
        options.onnxImportMode == ConvertOptions::OnnxImportMode::MLIR;
    const bool useTrtPath = !forceMlirPath;

    if (useTrtPath) {
      std::cout << "[Import] ONNX -> TensorRT Parser 路径\n";
      imported = importOnnxToNetwork(inputPath, network, trtBuilder, trtConfig);
    } else {
      std::cout << "[Import] ONNX -> MLIR -> TensorRT 路径\n";
      std::string generatedMlir;
      imported = convertOnnxToMlirViaExternalTool(inputPath, options, generatedMlir);
      if (imported)
        imported = importMlirToNetwork(generatedMlir, options, builder);
      if (!options.keepTempMlir && !generatedMlir.empty())
        std::remove(generatedMlir.c_str());
    }
  } else {
    imported = importMlirToNetwork(inputPath, options, builder);
  }
  if (!imported)
    return 1;

  std::unique_ptr<nvinfer1::IHostMemory> serializedModel = builder.build();
  if (!serializedModel) {
    std::cerr << "Failed to build TensorRT engine\n";
    return 1;
  }

  if (!saveEngine(enginePath, *serializedModel))
    return 1;

  std::cout << "Engine saved successfully (" << serializedModel->size()
            << " bytes)\n";
  return 0;
}

} // namespace

int main(int argc, char **argv) {
  ParsedArgs parsed;
  if (!parseArgs(argc, argv, parsed)) {
    std::cerr
        << "Usage: mlir2trt [--verbose-pipeline] [--onnx-import=auto|trt|mlir] "
           "[--onnx-mlir-converter=<cmd>] [--keep-temp-mlir] "
           "<model.onnx|model.mlir>\n";
    return 1;
  }

  return run(parsed.inputPath, parsed.options);
}
