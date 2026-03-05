#ifndef MIC_DIALECT_NNOPS_H
#define MIC_DIALECT_NNOPS_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"

#include "NNDialect.h.inc"

#define GET_OP_CLASSES
#include "NN.h.inc"

#endif
