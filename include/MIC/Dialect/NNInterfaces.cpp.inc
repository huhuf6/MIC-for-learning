/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Interface Definitions                                                      *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

bool MIC::NN::FusableOpInterface::canFuseWith(mlir::Operation * nextOp) {
      return getImpl()->canFuseWith(getImpl(), getOperation(), nextOp);
  }
mlir::Operation *MIC::NN::FusableOpInterface::fuseWith(mlir::Operation * nextOp) {
      return getImpl()->fuseWith(getImpl(), getOperation(), nextOp);
  }
std::string MIC::NN::NNLayerInterface::getLayerType() {
      return getImpl()->getLayerType(getImpl(), getOperation());
  }
mlir::TypeRange MIC::NN::NNLayerInterface::getInputTypes() {
      return getImpl()->getInputTypes(getImpl(), getOperation());
  }
mlir::TypeRange MIC::NN::NNLayerInterface::getOutputTypes() {
      return getImpl()->getOutputTypes(getImpl(), getOperation());
  }
