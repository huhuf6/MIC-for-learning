/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Op Declarations                                                            *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#if defined(GET_OP_CLASSES) || defined(GET_OP_FWD_DEFINES)
#undef GET_OP_FWD_DEFINES
namespace MIC {
namespace NN {
class AttentionOp;
} // namespace NN
} // namespace MIC
namespace MIC {
namespace NN {
class Conv2DOp;
} // namespace NN
} // namespace MIC
namespace MIC {
namespace NN {
class GELUOp;
} // namespace NN
} // namespace MIC
namespace MIC {
namespace NN {
class LayerNormOp;
} // namespace NN
} // namespace MIC
namespace MIC {
namespace NN {
class LinearOp;
} // namespace NN
} // namespace MIC
#endif

#ifdef GET_OP_CLASSES
#undef GET_OP_CLASSES


//===----------------------------------------------------------------------===//
// Local Utility Method Definitions
//===----------------------------------------------------------------------===//

namespace MIC {
namespace NN {

//===----------------------------------------------------------------------===//
// ::MIC::NN::AttentionOp declarations
//===----------------------------------------------------------------------===//

class AttentionOpAdaptor {
public:
  AttentionOpAdaptor(::mlir::ValueRange values, ::mlir::DictionaryAttr attrs = nullptr, ::mlir::RegionRange regions = {});

  AttentionOpAdaptor(AttentionOp &op);

  ::mlir::ValueRange getOperands();
  std::pair<unsigned, unsigned> getODSOperandIndexAndLength(unsigned index);
  ::mlir::ValueRange getODSOperands(unsigned index);
  ::mlir::Value query();
  ::mlir::Value key();
  ::mlir::Value value();
  ::mlir::Value mask();
  ::mlir::DictionaryAttr getAttributes();
  ::mlir::LogicalResult verify(::mlir::Location loc);
private:
  ::mlir::ValueRange odsOperands;
  ::mlir::DictionaryAttr odsAttrs;
  ::mlir::RegionRange odsRegions;
};
class AttentionOp : public ::mlir::Op<AttentionOp, ::mlir::OpTrait::ZeroRegions, ::mlir::OpTrait::OneResult, ::mlir::OpTrait::OneTypedResult<::mlir::TensorType>::Impl, ::mlir::OpTrait::ZeroSuccessors, ::mlir::OpTrait::AtLeastNOperands<3>::Impl, ::mlir::MemoryEffectOpInterface::Trait, ::mlir::OpTrait::SameOperandsAndResultElementType> {
public:
  using Op::Op;
  using Op::print;
  using Adaptor = AttentionOpAdaptor;
  static ::llvm::ArrayRef<::llvm::StringRef> getAttributeNames() {
    return {};
  }

  static constexpr ::llvm::StringLiteral getOperationName() {
    return ::llvm::StringLiteral("nn.attention");
  }

  std::pair<unsigned, unsigned> getODSOperandIndexAndLength(unsigned index);
  ::mlir::Operation::operand_range getODSOperands(unsigned index);
  ::mlir::Value query();
  ::mlir::Value key();
  ::mlir::Value value();
  ::mlir::Value mask();
  ::mlir::MutableOperandRange queryMutable();
  ::mlir::MutableOperandRange keyMutable();
  ::mlir::MutableOperandRange valueMutable();
  ::mlir::MutableOperandRange maskMutable();
  std::pair<unsigned, unsigned> getODSResultIndexAndLength(unsigned index);
  ::mlir::Operation::result_range getODSResults(unsigned index);
  ::mlir::Value output();
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value query, ::mlir::Value key, ::mlir::Value value, /*optional*/::mlir::Value mask);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value query, ::mlir::Value key, ::mlir::Value value, /*optional*/::mlir::Value mask);
  static void build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes = {});
  ::mlir::LogicalResult verify();
  void getEffects(::mlir::SmallVectorImpl<::mlir::SideEffects::EffectInstance<::mlir::MemoryEffects::Effect>> &effects);
public:
};
} // namespace NN
} // namespace MIC
DECLARE_EXPLICIT_TYPE_ID(::MIC::NN::AttentionOp)

namespace MIC {
namespace NN {

//===----------------------------------------------------------------------===//
// ::MIC::NN::Conv2DOp declarations
//===----------------------------------------------------------------------===//

class Conv2DOpAdaptor {
public:
  Conv2DOpAdaptor(::mlir::ValueRange values, ::mlir::DictionaryAttr attrs = nullptr, ::mlir::RegionRange regions = {});

  Conv2DOpAdaptor(Conv2DOp &op);

  ::mlir::ValueRange getOperands();
  std::pair<unsigned, unsigned> getODSOperandIndexAndLength(unsigned index);
  ::mlir::ValueRange getODSOperands(unsigned index);
  ::mlir::Value input();
  ::mlir::Value weight();
  ::mlir::Value bias();
  ::mlir::DictionaryAttr getAttributes();
  ::mlir::ArrayAttr stridesAttr();
  ::mlir::ArrayAttr strides();
  ::mlir::ArrayAttr paddingAttr();
  ::mlir::ArrayAttr padding();
  ::mlir::ArrayAttr dilationAttr();
  ::mlir::ArrayAttr dilation();
  ::mlir::IntegerAttr groupsAttr();
  uint32_t groups();
  ::mlir::LogicalResult verify(::mlir::Location loc);
private:
  ::mlir::ValueRange odsOperands;
  ::mlir::DictionaryAttr odsAttrs;
  ::mlir::RegionRange odsRegions;
};
class Conv2DOp : public ::mlir::Op<Conv2DOp, ::mlir::OpTrait::ZeroRegions, ::mlir::OpTrait::OneResult, ::mlir::OpTrait::OneTypedResult<::mlir::TensorType>::Impl, ::mlir::OpTrait::ZeroSuccessors, ::mlir::OpTrait::AtLeastNOperands<2>::Impl, ::mlir::MemoryEffectOpInterface::Trait, ::mlir::OpTrait::SameOperandsAndResultElementType> {
public:
  using Op::Op;
  using Op::print;
  using Adaptor = Conv2DOpAdaptor;
  static ::llvm::ArrayRef<::llvm::StringRef> getAttributeNames() {
    static ::llvm::StringRef attrNames[] = {::llvm::StringRef("strides"), ::llvm::StringRef("padding"), ::llvm::StringRef("dilation"), ::llvm::StringRef("groups")};
    return ::llvm::makeArrayRef(attrNames);
  }

  ::mlir::StringAttr stridesAttrName() {
    return getAttributeNameForIndex(0);
  }

  static ::mlir::StringAttr stridesAttrName(::mlir::OperationName name) {
    return getAttributeNameForIndex(name, 0);
  }

  ::mlir::StringAttr paddingAttrName() {
    return getAttributeNameForIndex(1);
  }

  static ::mlir::StringAttr paddingAttrName(::mlir::OperationName name) {
    return getAttributeNameForIndex(name, 1);
  }

  ::mlir::StringAttr dilationAttrName() {
    return getAttributeNameForIndex(2);
  }

  static ::mlir::StringAttr dilationAttrName(::mlir::OperationName name) {
    return getAttributeNameForIndex(name, 2);
  }

  ::mlir::StringAttr groupsAttrName() {
    return getAttributeNameForIndex(3);
  }

  static ::mlir::StringAttr groupsAttrName(::mlir::OperationName name) {
    return getAttributeNameForIndex(name, 3);
  }

  static constexpr ::llvm::StringLiteral getOperationName() {
    return ::llvm::StringLiteral("nn.conv2d");
  }

  std::pair<unsigned, unsigned> getODSOperandIndexAndLength(unsigned index);
  ::mlir::Operation::operand_range getODSOperands(unsigned index);
  ::mlir::Value input();
  ::mlir::Value weight();
  ::mlir::Value bias();
  ::mlir::MutableOperandRange inputMutable();
  ::mlir::MutableOperandRange weightMutable();
  ::mlir::MutableOperandRange biasMutable();
  std::pair<unsigned, unsigned> getODSResultIndexAndLength(unsigned index);
  ::mlir::Operation::result_range getODSResults(unsigned index);
  ::mlir::Value output();
  ::mlir::ArrayAttr stridesAttr();
  ::mlir::ArrayAttr strides();
  ::mlir::ArrayAttr paddingAttr();
  ::mlir::ArrayAttr padding();
  ::mlir::ArrayAttr dilationAttr();
  ::mlir::ArrayAttr dilation();
  ::mlir::IntegerAttr groupsAttr();
  uint32_t groups();
  void stridesAttr(::mlir::ArrayAttr attr);
  void paddingAttr(::mlir::ArrayAttr attr);
  void dilationAttr(::mlir::ArrayAttr attr);
  void groupsAttr(::mlir::IntegerAttr attr);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias, ::mlir::ArrayAttr strides, ::mlir::ArrayAttr padding, ::mlir::ArrayAttr dilation, ::mlir::IntegerAttr groups);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias, ::mlir::ArrayAttr strides, ::mlir::ArrayAttr padding, ::mlir::ArrayAttr dilation, ::mlir::IntegerAttr groups);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias, ::mlir::ArrayAttr strides, ::mlir::ArrayAttr padding, ::mlir::ArrayAttr dilation, uint32_t groups);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias, ::mlir::ArrayAttr strides, ::mlir::ArrayAttr padding, ::mlir::ArrayAttr dilation, uint32_t groups);
  static void build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes = {});
  ::mlir::LogicalResult verify();
  void getEffects(::mlir::SmallVectorImpl<::mlir::SideEffects::EffectInstance<::mlir::MemoryEffects::Effect>> &effects);
private:
  ::mlir::StringAttr getAttributeNameForIndex(unsigned index) {
    return getAttributeNameForIndex((*this)->getName(), index);
  }

  static ::mlir::StringAttr getAttributeNameForIndex(::mlir::OperationName name, unsigned index) {
    assert(index < 4 && "invalid attribute index");
    return name.getRegisteredInfo()->getAttributeNames()[index];
  }

public:
};
} // namespace NN
} // namespace MIC
DECLARE_EXPLICIT_TYPE_ID(::MIC::NN::Conv2DOp)

namespace MIC {
namespace NN {

//===----------------------------------------------------------------------===//
// ::MIC::NN::GELUOp declarations
//===----------------------------------------------------------------------===//

class GELUOpAdaptor {
public:
  GELUOpAdaptor(::mlir::ValueRange values, ::mlir::DictionaryAttr attrs = nullptr, ::mlir::RegionRange regions = {});

  GELUOpAdaptor(GELUOp &op);

  ::mlir::ValueRange getOperands();
  std::pair<unsigned, unsigned> getODSOperandIndexAndLength(unsigned index);
  ::mlir::ValueRange getODSOperands(unsigned index);
  ::mlir::Value input();
  ::mlir::DictionaryAttr getAttributes();
  ::mlir::LogicalResult verify(::mlir::Location loc);
private:
  ::mlir::ValueRange odsOperands;
  ::mlir::DictionaryAttr odsAttrs;
  ::mlir::RegionRange odsRegions;
};
class GELUOp : public ::mlir::Op<GELUOp, ::mlir::OpTrait::ZeroRegions, ::mlir::OpTrait::OneResult, ::mlir::OpTrait::OneTypedResult<::mlir::TensorType>::Impl, ::mlir::OpTrait::ZeroSuccessors, ::mlir::OpTrait::OneOperand, ::mlir::MemoryEffectOpInterface::Trait, ::mlir::OpTrait::SameOperandsAndResultElementType> {
public:
  using Op::Op;
  using Op::print;
  using Adaptor = GELUOpAdaptor;
  static ::llvm::ArrayRef<::llvm::StringRef> getAttributeNames() {
    return {};
  }

  static constexpr ::llvm::StringLiteral getOperationName() {
    return ::llvm::StringLiteral("nn.gelu");
  }

  std::pair<unsigned, unsigned> getODSOperandIndexAndLength(unsigned index);
  ::mlir::Operation::operand_range getODSOperands(unsigned index);
  ::mlir::Value input();
  ::mlir::MutableOperandRange inputMutable();
  std::pair<unsigned, unsigned> getODSResultIndexAndLength(unsigned index);
  ::mlir::Operation::result_range getODSResults(unsigned index);
  ::mlir::Value output();
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input);
  static void build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes = {});
  ::mlir::LogicalResult verify();
  void getEffects(::mlir::SmallVectorImpl<::mlir::SideEffects::EffectInstance<::mlir::MemoryEffects::Effect>> &effects);
public:
};
} // namespace NN
} // namespace MIC
DECLARE_EXPLICIT_TYPE_ID(::MIC::NN::GELUOp)

namespace MIC {
namespace NN {

//===----------------------------------------------------------------------===//
// ::MIC::NN::LayerNormOp declarations
//===----------------------------------------------------------------------===//

class LayerNormOpAdaptor {
public:
  LayerNormOpAdaptor(::mlir::ValueRange values, ::mlir::DictionaryAttr attrs = nullptr, ::mlir::RegionRange regions = {});

  LayerNormOpAdaptor(LayerNormOp &op);

  ::mlir::ValueRange getOperands();
  std::pair<unsigned, unsigned> getODSOperandIndexAndLength(unsigned index);
  ::mlir::ValueRange getODSOperands(unsigned index);
  ::mlir::Value input();
  ::mlir::Value scale();
  ::mlir::Value bias();
  ::mlir::DictionaryAttr getAttributes();
  ::mlir::FloatAttr epsilonAttr();
  ::llvm::APFloat epsilon();
  ::mlir::LogicalResult verify(::mlir::Location loc);
private:
  ::mlir::ValueRange odsOperands;
  ::mlir::DictionaryAttr odsAttrs;
  ::mlir::RegionRange odsRegions;
};
class LayerNormOp : public ::mlir::Op<LayerNormOp, ::mlir::OpTrait::ZeroRegions, ::mlir::OpTrait::OneResult, ::mlir::OpTrait::OneTypedResult<::mlir::TensorType>::Impl, ::mlir::OpTrait::ZeroSuccessors, ::mlir::OpTrait::AtLeastNOperands<1>::Impl, ::mlir::MemoryEffectOpInterface::Trait, ::mlir::OpTrait::SameOperandsAndResultElementType, ::mlir::OpTrait::AttrSizedOperandSegments> {
public:
  using Op::Op;
  using Op::print;
  using Adaptor = LayerNormOpAdaptor;
  static ::llvm::ArrayRef<::llvm::StringRef> getAttributeNames() {
    static ::llvm::StringRef attrNames[] = {::llvm::StringRef("epsilon"), ::llvm::StringRef("operand_segment_sizes")};
    return ::llvm::makeArrayRef(attrNames);
  }

  ::mlir::StringAttr epsilonAttrName() {
    return getAttributeNameForIndex(0);
  }

  static ::mlir::StringAttr epsilonAttrName(::mlir::OperationName name) {
    return getAttributeNameForIndex(name, 0);
  }

  ::mlir::StringAttr operand_segment_sizesAttrName() {
    return getAttributeNameForIndex(1);
  }

  static ::mlir::StringAttr operand_segment_sizesAttrName(::mlir::OperationName name) {
    return getAttributeNameForIndex(name, 1);
  }

  static constexpr ::llvm::StringLiteral getOperationName() {
    return ::llvm::StringLiteral("nn.layer_norm");
  }

  std::pair<unsigned, unsigned> getODSOperandIndexAndLength(unsigned index);
  ::mlir::Operation::operand_range getODSOperands(unsigned index);
  ::mlir::Value input();
  ::mlir::Value scale();
  ::mlir::Value bias();
  ::mlir::MutableOperandRange inputMutable();
  ::mlir::MutableOperandRange scaleMutable();
  ::mlir::MutableOperandRange biasMutable();
  std::pair<unsigned, unsigned> getODSResultIndexAndLength(unsigned index);
  ::mlir::Operation::result_range getODSResults(unsigned index);
  ::mlir::Value output();
  ::mlir::FloatAttr epsilonAttr();
  ::llvm::APFloat epsilon();
  void epsilonAttr(::mlir::FloatAttr attr);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input, /*optional*/::mlir::Value scale, /*optional*/::mlir::Value bias, ::mlir::FloatAttr epsilon);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input, /*optional*/::mlir::Value scale, /*optional*/::mlir::Value bias, ::mlir::FloatAttr epsilon);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input, /*optional*/::mlir::Value scale, /*optional*/::mlir::Value bias, ::llvm::APFloat epsilon);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input, /*optional*/::mlir::Value scale, /*optional*/::mlir::Value bias, ::llvm::APFloat epsilon);
  static void build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes = {});
  ::mlir::LogicalResult verify();
  void getEffects(::mlir::SmallVectorImpl<::mlir::SideEffects::EffectInstance<::mlir::MemoryEffects::Effect>> &effects);
private:
  ::mlir::StringAttr getAttributeNameForIndex(unsigned index) {
    return getAttributeNameForIndex((*this)->getName(), index);
  }

  static ::mlir::StringAttr getAttributeNameForIndex(::mlir::OperationName name, unsigned index) {
    assert(index < 2 && "invalid attribute index");
    return name.getRegisteredInfo()->getAttributeNames()[index];
  }

public:
};
} // namespace NN
} // namespace MIC
DECLARE_EXPLICIT_TYPE_ID(::MIC::NN::LayerNormOp)

namespace MIC {
namespace NN {

//===----------------------------------------------------------------------===//
// ::MIC::NN::LinearOp declarations
//===----------------------------------------------------------------------===//

class LinearOpAdaptor {
public:
  LinearOpAdaptor(::mlir::ValueRange values, ::mlir::DictionaryAttr attrs = nullptr, ::mlir::RegionRange regions = {});

  LinearOpAdaptor(LinearOp &op);

  ::mlir::ValueRange getOperands();
  std::pair<unsigned, unsigned> getODSOperandIndexAndLength(unsigned index);
  ::mlir::ValueRange getODSOperands(unsigned index);
  ::mlir::Value input();
  ::mlir::Value weight();
  ::mlir::Value bias();
  ::mlir::DictionaryAttr getAttributes();
  ::mlir::LogicalResult verify(::mlir::Location loc);
private:
  ::mlir::ValueRange odsOperands;
  ::mlir::DictionaryAttr odsAttrs;
  ::mlir::RegionRange odsRegions;
};
class LinearOp : public ::mlir::Op<LinearOp, ::mlir::OpTrait::ZeroRegions, ::mlir::OpTrait::OneResult, ::mlir::OpTrait::OneTypedResult<::mlir::TensorType>::Impl, ::mlir::OpTrait::ZeroSuccessors, ::mlir::OpTrait::AtLeastNOperands<2>::Impl, ::mlir::MemoryEffectOpInterface::Trait, ::mlir::OpTrait::SameOperandsAndResultElementType> {
public:
  using Op::Op;
  using Op::print;
  using Adaptor = LinearOpAdaptor;
  static ::llvm::ArrayRef<::llvm::StringRef> getAttributeNames() {
    return {};
  }

  static constexpr ::llvm::StringLiteral getOperationName() {
    return ::llvm::StringLiteral("nn.linear");
  }

  std::pair<unsigned, unsigned> getODSOperandIndexAndLength(unsigned index);
  ::mlir::Operation::operand_range getODSOperands(unsigned index);
  ::mlir::Value input();
  ::mlir::Value weight();
  ::mlir::Value bias();
  ::mlir::MutableOperandRange inputMutable();
  ::mlir::MutableOperandRange weightMutable();
  ::mlir::MutableOperandRange biasMutable();
  std::pair<unsigned, unsigned> getODSResultIndexAndLength(unsigned index);
  ::mlir::Operation::result_range getODSResults(unsigned index);
  ::mlir::Value output();
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias);
  static void build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias);
  static void build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes = {});
  ::mlir::LogicalResult verify();
  void getEffects(::mlir::SmallVectorImpl<::mlir::SideEffects::EffectInstance<::mlir::MemoryEffects::Effect>> &effects);
public:
};
} // namespace NN
} // namespace MIC
DECLARE_EXPLICIT_TYPE_ID(::MIC::NN::LinearOp)


#endif  // GET_OP_CLASSES

