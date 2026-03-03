//===----------------------------------------------------------------------===//
// MIC Neural Network (NN) Dialect Definition
//===----------------------------------------------------------------------===//

#ifndef MIC_DIALECT_NN_DIALECT_H
#define MIC_DIALECT_NN_DIALECT_H

#include "mlir/IR/Dialect.h"

namespace MIC {
namespace NN {

class NNDialect : public mlir::Dialect {
public:
  explicit NNDialect(mlir::MLIRContext *context);
  virtual ~NNDialect();
  
  static llvm::StringRef getDialectNamespace() { return "nn"; }
  
  void initialize();
};

} // namespace NN
} // namespace MIC

#endif // MIC_DIALECT_NN_DIALECT_H
