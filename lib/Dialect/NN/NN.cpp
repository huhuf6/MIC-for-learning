#include "MIC/Dialect/NNOps.h"
#include "mlir/IR/DialectImplementation.h"

using namespace mlir;
using namespace MIC::NN;

// Dialect implementation
void NNDialect::initialize() {
  addOperations<
    #include "MIC/Dialect/NNOps.cpp.inc"
  >();
}

// Dialect registration
#include "MIC/Dialect/NNOpsDialect.cpp.inc"
