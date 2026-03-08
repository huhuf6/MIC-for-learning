//===----------------------------------------------------------------------===//
// MIC Neural Network (NN) Dialect Operations Header
//===----------------------------------------------------------------------===//

#ifndef MIC_DIALECT_NN_OPS_H
#define MIC_DIALECT_NN_OPS_H

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

namespace MIC {
namespace NN {

class NNDialect;

} // namespace NN
} // namespace MIC

#define GET_OP_CLASSES
#include "NN.h.inc"

#endif // MIC_DIALECT_NN_OPS_H
