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

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: mlir2trt model.mlir" << std::endl;
    return 1;
  }

  std::string inputPath = argv[1];
  std::string enginePath = inputPath.substr(0, inputPath.find_last_of('.')) + ".engine";

  std::cout << "Processing model: " << inputPath << std::endl;
  std::cout << "Output engine: " << enginePath << std::endl;

  mlir::MLIRContext context;

  std::cout << "MLIR context initialized" << std::endl;

  mlir::MIC::TensorRT::TensorRTBuilder* builder = new mlir::MIC::TensorRT::TensorRTBuilder();
  std::cout << "TensorRT builder created" << std::endl;

  builder->setFP16Mode(true);
  builder->setMaxWorkspaceSize(1ULL << 30);

  auto* network = builder->getNetwork();
  std::cout << "Network created with " << network->getNbInputs() << " inputs and " << network->getNbOutputs() << " outputs" << std::endl;

  mlir::MIC::TensorRT::NetworkConverter* converter = new mlir::MIC::TensorRT::NetworkConverter(*builder);
  std::cout << "Network converter created" << std::endl;

  delete converter;
  delete builder;

  std::cout << "Done!" << std::endl;
  return 0;
}
