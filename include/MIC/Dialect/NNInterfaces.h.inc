/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Interface Declarations                                                     *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

namespace MIC {
namespace NN {
class FusableOpInterface;
namespace detail {
struct FusableOpInterfaceInterfaceTraits {
  struct Concept {
    bool (*canFuseWith)(const Concept *impl, ::mlir::Operation *, mlir::Operation *);
    mlir::Operation *(*fuseWith)(const Concept *impl, ::mlir::Operation *, mlir::Operation *);
  };
  template<typename ConcreteOp>
  class Model : public Concept {
  public:
    using Interface = ::MIC::NN::FusableOpInterface;
    Model() : Concept{canFuseWith, fuseWith} {}

    static inline bool canFuseWith(const Concept *impl, ::mlir::Operation *tablegen_opaque_val, mlir::Operation * nextOp);
    static inline mlir::Operation *fuseWith(const Concept *impl, ::mlir::Operation *tablegen_opaque_val, mlir::Operation * nextOp);
  };
  template<typename ConcreteOp>
  class FallbackModel : public Concept {
  public:
    using Interface = ::MIC::NN::FusableOpInterface;
    FallbackModel() : Concept{canFuseWith, fuseWith} {}

    static inline bool canFuseWith(const Concept *impl, ::mlir::Operation *tablegen_opaque_val, mlir::Operation * nextOp);
    static inline mlir::Operation *fuseWith(const Concept *impl, ::mlir::Operation *tablegen_opaque_val, mlir::Operation * nextOp);
  };
  template<typename ConcreteModel, typename ConcreteOp>
  class ExternalModel : public FallbackModel<ConcreteModel> {
  public:
  };
};template <typename ConcreteOp>
struct FusableOpInterfaceTrait;

} // namespace detail
class FusableOpInterface : public ::mlir::OpInterface<FusableOpInterface, detail::FusableOpInterfaceInterfaceTraits> {
public:
  using ::mlir::OpInterface<FusableOpInterface, detail::FusableOpInterfaceInterfaceTraits>::OpInterface;
  template <typename ConcreteOp>
  struct Trait : public detail::FusableOpInterfaceTrait<ConcreteOp> {};
  bool canFuseWith(mlir::Operation * nextOp);
  mlir::Operation *fuseWith(mlir::Operation * nextOp);
};
namespace detail {
  template <typename ConcreteOp>
  struct FusableOpInterfaceTrait : public ::mlir::OpInterface<FusableOpInterface, detail::FusableOpInterfaceInterfaceTraits>::Trait<ConcreteOp> {
  };
}// namespace detail
template<typename ConcreteOp>
bool detail::FusableOpInterfaceInterfaceTraits::Model<ConcreteOp>::canFuseWith(const Concept *impl, ::mlir::Operation *tablegen_opaque_val, mlir::Operation * nextOp) {
  return (llvm::cast<ConcreteOp>(tablegen_opaque_val)).canFuseWith(nextOp);
}
template<typename ConcreteOp>
mlir::Operation *detail::FusableOpInterfaceInterfaceTraits::Model<ConcreteOp>::fuseWith(const Concept *impl, ::mlir::Operation *tablegen_opaque_val, mlir::Operation * nextOp) {
  return (llvm::cast<ConcreteOp>(tablegen_opaque_val)).fuseWith(nextOp);
}
template<typename ConcreteOp>
bool detail::FusableOpInterfaceInterfaceTraits::FallbackModel<ConcreteOp>::canFuseWith(const Concept *impl, ::mlir::Operation *tablegen_opaque_val, mlir::Operation * nextOp) {
  return static_cast<const ConcreteOp *>(impl)->canFuseWith(tablegen_opaque_val, nextOp);
}
template<typename ConcreteOp>
mlir::Operation *detail::FusableOpInterfaceInterfaceTraits::FallbackModel<ConcreteOp>::fuseWith(const Concept *impl, ::mlir::Operation *tablegen_opaque_val, mlir::Operation * nextOp) {
  return static_cast<const ConcreteOp *>(impl)->fuseWith(tablegen_opaque_val, nextOp);
}
} // namespace NN
} // namespace MIC
namespace MIC {
namespace NN {
class NNLayerInterface;
namespace detail {
struct NNLayerInterfaceInterfaceTraits {
  struct Concept {
    std::string (*getLayerType)(const Concept *impl, ::mlir::Operation *);
    mlir::TypeRange (*getInputTypes)(const Concept *impl, ::mlir::Operation *);
    mlir::TypeRange (*getOutputTypes)(const Concept *impl, ::mlir::Operation *);
  };
  template<typename ConcreteOp>
  class Model : public Concept {
  public:
    using Interface = ::MIC::NN::NNLayerInterface;
    Model() : Concept{getLayerType, getInputTypes, getOutputTypes} {}

    static inline std::string getLayerType(const Concept *impl, ::mlir::Operation *tablegen_opaque_val);
    static inline mlir::TypeRange getInputTypes(const Concept *impl, ::mlir::Operation *tablegen_opaque_val);
    static inline mlir::TypeRange getOutputTypes(const Concept *impl, ::mlir::Operation *tablegen_opaque_val);
  };
  template<typename ConcreteOp>
  class FallbackModel : public Concept {
  public:
    using Interface = ::MIC::NN::NNLayerInterface;
    FallbackModel() : Concept{getLayerType, getInputTypes, getOutputTypes} {}

    static inline std::string getLayerType(const Concept *impl, ::mlir::Operation *tablegen_opaque_val);
    static inline mlir::TypeRange getInputTypes(const Concept *impl, ::mlir::Operation *tablegen_opaque_val);
    static inline mlir::TypeRange getOutputTypes(const Concept *impl, ::mlir::Operation *tablegen_opaque_val);
  };
  template<typename ConcreteModel, typename ConcreteOp>
  class ExternalModel : public FallbackModel<ConcreteModel> {
  public:
  };
};template <typename ConcreteOp>
struct NNLayerInterfaceTrait;

} // namespace detail
class NNLayerInterface : public ::mlir::OpInterface<NNLayerInterface, detail::NNLayerInterfaceInterfaceTraits> {
public:
  using ::mlir::OpInterface<NNLayerInterface, detail::NNLayerInterfaceInterfaceTraits>::OpInterface;
  template <typename ConcreteOp>
  struct Trait : public detail::NNLayerInterfaceTrait<ConcreteOp> {};
  std::string getLayerType();
  mlir::TypeRange getInputTypes();
  mlir::TypeRange getOutputTypes();
};
namespace detail {
  template <typename ConcreteOp>
  struct NNLayerInterfaceTrait : public ::mlir::OpInterface<NNLayerInterface, detail::NNLayerInterfaceInterfaceTraits>::Trait<ConcreteOp> {
  };
}// namespace detail
template<typename ConcreteOp>
std::string detail::NNLayerInterfaceInterfaceTraits::Model<ConcreteOp>::getLayerType(const Concept *impl, ::mlir::Operation *tablegen_opaque_val) {
  return (llvm::cast<ConcreteOp>(tablegen_opaque_val)).getLayerType();
}
template<typename ConcreteOp>
mlir::TypeRange detail::NNLayerInterfaceInterfaceTraits::Model<ConcreteOp>::getInputTypes(const Concept *impl, ::mlir::Operation *tablegen_opaque_val) {
  return (llvm::cast<ConcreteOp>(tablegen_opaque_val)).getInputTypes();
}
template<typename ConcreteOp>
mlir::TypeRange detail::NNLayerInterfaceInterfaceTraits::Model<ConcreteOp>::getOutputTypes(const Concept *impl, ::mlir::Operation *tablegen_opaque_val) {
  return (llvm::cast<ConcreteOp>(tablegen_opaque_val)).getOutputTypes();
}
template<typename ConcreteOp>
std::string detail::NNLayerInterfaceInterfaceTraits::FallbackModel<ConcreteOp>::getLayerType(const Concept *impl, ::mlir::Operation *tablegen_opaque_val) {
  return static_cast<const ConcreteOp *>(impl)->getLayerType(tablegen_opaque_val);
}
template<typename ConcreteOp>
mlir::TypeRange detail::NNLayerInterfaceInterfaceTraits::FallbackModel<ConcreteOp>::getInputTypes(const Concept *impl, ::mlir::Operation *tablegen_opaque_val) {
  return static_cast<const ConcreteOp *>(impl)->getInputTypes(tablegen_opaque_val);
}
template<typename ConcreteOp>
mlir::TypeRange detail::NNLayerInterfaceInterfaceTraits::FallbackModel<ConcreteOp>::getOutputTypes(const Concept *impl, ::mlir::Operation *tablegen_opaque_val) {
  return static_cast<const ConcreteOp *>(impl)->getOutputTypes(tablegen_opaque_val);
}
} // namespace NN
} // namespace MIC
