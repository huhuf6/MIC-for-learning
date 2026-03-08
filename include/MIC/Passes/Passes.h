//===----------------------------------------------------------------------===//
// MIC Passes Entry
//===----------------------------------------------------------------------===//

#ifndef MIC_PASSES_PASSES_H
#define MIC_PASSES_PASSES_H

#include <memory>

namespace mlir {
class Pass;

namespace MIC {

std::unique_ptr<Pass> createLowerONNXToNNPass();
std::unique_ptr<Pass> createLowerNNToLinalgPass();
std::unique_ptr<Pass> createLinalgFusePass();
std::unique_ptr<Pass> createLinalgTilePass();
std::unique_ptr<Pass> createLinalgVectorizePass();
std::unique_ptr<Pass> createLinalgToTensorRTPass();
std::unique_ptr<Pass> createNNToRuntimeLoweringPass();

} // namespace MIC
} // namespace mlir

#endif // MIC_PASSES_PASSES_H
