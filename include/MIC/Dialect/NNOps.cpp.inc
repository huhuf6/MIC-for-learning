/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Op Definitions                                                             *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_OP_LIST
#undef GET_OP_LIST

::MIC::NN::AttentionOp,
::MIC::NN::Conv2DOp,
::MIC::NN::GELUOp,
::MIC::NN::LayerNormOp,
::MIC::NN::LinearOp
#endif  // GET_OP_LIST

#ifdef GET_OP_CLASSES
#undef GET_OP_CLASSES


//===----------------------------------------------------------------------===//
// Local Utility Method Definitions
//===----------------------------------------------------------------------===//

namespace MIC {
namespace NN {

static ::mlir::LogicalResult __mlir_ods_local_type_constraint_NNOps0(
    ::mlir::Operation *op, ::mlir::Type type, ::llvm::StringRef valueKind,
    unsigned valueIndex) {
  if (!(((type.isa<::mlir::TensorType>())) && ([](::mlir::Type elementType) { return (true); }(type.cast<::mlir::ShapedType>().getElementType())))) {
    return op->emitOpError(valueKind) << " #" << valueIndex
        << " must be tensor of any type values, but got " << type;
  }
  return ::mlir::success();
}

static ::mlir::LogicalResult __mlir_ods_local_attr_constraint_NNOps0(
    ::mlir::Operation *op, ::mlir::Attribute attr, ::llvm::StringRef attrName) {
  if (attr && !(((attr.isa<::mlir::ArrayAttr>())) && (::llvm::all_of(attr.cast<::mlir::ArrayAttr>(), [&](::mlir::Attribute attr) { return ((attr.isa<::mlir::IntegerAttr>())) && ((attr.cast<::mlir::IntegerAttr>().getType().isSignlessInteger(64))); })))) {
    return op->emitOpError("attribute '") << attrName
        << "' failed to satisfy constraint: 64-bit integer array attribute";
  }
  return ::mlir::success();
}

static ::mlir::LogicalResult __mlir_ods_local_attr_constraint_NNOps1(
    ::mlir::Operation *op, ::mlir::Attribute attr, ::llvm::StringRef attrName) {
  if (attr && !(((attr.isa<::mlir::IntegerAttr>())) && ((attr.cast<::mlir::IntegerAttr>().getType().isSignlessInteger(32))))) {
    return op->emitOpError("attribute '") << attrName
        << "' failed to satisfy constraint: 32-bit signless integer attribute";
  }
  return ::mlir::success();
}

static ::mlir::LogicalResult __mlir_ods_local_attr_constraint_NNOps2(
    ::mlir::Operation *op, ::mlir::Attribute attr, ::llvm::StringRef attrName) {
  if (attr && !(((attr.isa<::mlir::FloatAttr>())) && ((attr.cast<::mlir::FloatAttr>().getType().isF64())))) {
    return op->emitOpError("attribute '") << attrName
        << "' failed to satisfy constraint: 64-bit float attribute";
  }
  return ::mlir::success();
}
} // namespace NN
} // namespace MIC
namespace MIC {
namespace NN {

//===----------------------------------------------------------------------===//
// ::MIC::NN::AttentionOp definitions
//===----------------------------------------------------------------------===//

AttentionOpAdaptor::AttentionOpAdaptor(::mlir::ValueRange values, ::mlir::DictionaryAttr attrs, ::mlir::RegionRange regions) : odsOperands(values), odsAttrs(attrs), odsRegions(regions) {}

AttentionOpAdaptor::AttentionOpAdaptor(AttentionOp &op) : odsOperands(op->getOperands()), odsAttrs(op->getAttrDictionary()), odsRegions(op->getRegions()) {}

::mlir::ValueRange AttentionOpAdaptor::getOperands() {
  return odsOperands;
}

std::pair<unsigned, unsigned> AttentionOpAdaptor::getODSOperandIndexAndLength(unsigned index) {
  bool isVariadic[] = {false, false, false, true};
  int prevVariadicCount = 0;
  for (unsigned i = 0; i < index; ++i)
    if (isVariadic[i]) ++prevVariadicCount;

  // Calculate how many dynamic values a static variadic operand corresponds to.
  // This assumes all static variadic operands have the same dynamic value count.
  int variadicSize = (odsOperands.size() - 3) / 1;
  // `index` passed in as the parameter is the static index which counts each
  // operand (variadic or not) as size 1. So here for each previous static variadic
  // operand, we need to offset by (variadicSize - 1) to get where the dynamic
  // value pack for this static operand starts.
  int start = index + (variadicSize - 1) * prevVariadicCount;
  int size = isVariadic[index] ? variadicSize : 1;
  return {start, size};
}

::mlir::ValueRange AttentionOpAdaptor::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(odsOperands.begin(), valueRange.first),
           std::next(odsOperands.begin(), valueRange.first + valueRange.second)};
}

::mlir::Value AttentionOpAdaptor::query() {
  return *getODSOperands(0).begin();
}

::mlir::Value AttentionOpAdaptor::key() {
  return *getODSOperands(1).begin();
}

::mlir::Value AttentionOpAdaptor::value() {
  return *getODSOperands(2).begin();
}

::mlir::Value AttentionOpAdaptor::mask() {
  auto operands = getODSOperands(3);
  return operands.empty() ? ::mlir::Value() : *operands.begin();
}

::mlir::DictionaryAttr AttentionOpAdaptor::getAttributes() {
  return odsAttrs;
}

::mlir::LogicalResult AttentionOpAdaptor::verify(::mlir::Location loc) {
  return ::mlir::success();
}

std::pair<unsigned, unsigned> AttentionOp::getODSOperandIndexAndLength(unsigned index) {
  bool isVariadic[] = {false, false, false, true};
  int prevVariadicCount = 0;
  for (unsigned i = 0; i < index; ++i)
    if (isVariadic[i]) ++prevVariadicCount;

  // Calculate how many dynamic values a static variadic operand corresponds to.
  // This assumes all static variadic operands have the same dynamic value count.
  int variadicSize = (getOperation()->getNumOperands() - 3) / 1;
  // `index` passed in as the parameter is the static index which counts each
  // operand (variadic or not) as size 1. So here for each previous static variadic
  // operand, we need to offset by (variadicSize - 1) to get where the dynamic
  // value pack for this static operand starts.
  int start = index + (variadicSize - 1) * prevVariadicCount;
  int size = isVariadic[index] ? variadicSize : 1;
  return {start, size};
}

::mlir::Operation::operand_range AttentionOp::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(getOperation()->operand_begin(), valueRange.first),
           std::next(getOperation()->operand_begin(), valueRange.first + valueRange.second)};
}

::mlir::Value AttentionOp::query() {
  return *getODSOperands(0).begin();
}

::mlir::Value AttentionOp::key() {
  return *getODSOperands(1).begin();
}

::mlir::Value AttentionOp::value() {
  return *getODSOperands(2).begin();
}

::mlir::Value AttentionOp::mask() {
  auto operands = getODSOperands(3);
  return operands.empty() ? ::mlir::Value() : *operands.begin();
}

::mlir::MutableOperandRange AttentionOp::queryMutable() {
  auto range = getODSOperandIndexAndLength(0);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

::mlir::MutableOperandRange AttentionOp::keyMutable() {
  auto range = getODSOperandIndexAndLength(1);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

::mlir::MutableOperandRange AttentionOp::valueMutable() {
  auto range = getODSOperandIndexAndLength(2);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

::mlir::MutableOperandRange AttentionOp::maskMutable() {
  auto range = getODSOperandIndexAndLength(3);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

std::pair<unsigned, unsigned> AttentionOp::getODSResultIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::result_range AttentionOp::getODSResults(unsigned index) {
  auto valueRange = getODSResultIndexAndLength(index);
  return {std::next(getOperation()->result_begin(), valueRange.first),
           std::next(getOperation()->result_begin(), valueRange.first + valueRange.second)};
}

::mlir::Value AttentionOp::output() {
  return *getODSResults(0).begin();
}

void AttentionOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value query, ::mlir::Value key, ::mlir::Value value, /*optional*/::mlir::Value mask) {
  odsState.addOperands(query);
  odsState.addOperands(key);
  odsState.addOperands(value);
  if (mask)
    odsState.addOperands(mask);
  odsState.addTypes(output);
}

void AttentionOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value query, ::mlir::Value key, ::mlir::Value value, /*optional*/::mlir::Value mask) {
  odsState.addOperands(query);
  odsState.addOperands(key);
  odsState.addOperands(value);
  if (mask)
    odsState.addOperands(mask);
  assert(resultTypes.size() == 1u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void AttentionOp::build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes) {
  assert(operands.size() >= 3u && "mismatched number of parameters");
  odsState.addOperands(operands);
  odsState.addAttributes(attributes);
  assert(resultTypes.size() == 1u && "mismatched number of return types");
  odsState.addTypes(resultTypes);
}

::mlir::LogicalResult AttentionOp::verify() {
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSOperands(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
    auto valueGroup1 = getODSOperands(1);

    for (auto v : valueGroup1) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
    auto valueGroup2 = getODSOperands(2);

    for (auto v : valueGroup2) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
    auto valueGroup3 = getODSOperands(3);

    if (valueGroup3.size() > 1) {
      return emitOpError("operand group starting at #") << index
          << " requires 0 or 1 element, but found " << valueGroup3.size();
    }

    for (auto v : valueGroup3) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
  }
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSResults(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "result", index++)))
        return ::mlir::failure();
    }
  }
  return ::mlir::success();
}

void AttentionOp::getEffects(::mlir::SmallVectorImpl<::mlir::SideEffects::EffectInstance<::mlir::MemoryEffects::Effect>> &effects) {
}

} // namespace NN
} // namespace MIC
DEFINE_EXPLICIT_TYPE_ID(::MIC::NN::AttentionOp)

namespace MIC {
namespace NN {

//===----------------------------------------------------------------------===//
// ::MIC::NN::Conv2DOp definitions
//===----------------------------------------------------------------------===//

Conv2DOpAdaptor::Conv2DOpAdaptor(::mlir::ValueRange values, ::mlir::DictionaryAttr attrs, ::mlir::RegionRange regions) : odsOperands(values), odsAttrs(attrs), odsRegions(regions) {}

Conv2DOpAdaptor::Conv2DOpAdaptor(Conv2DOp &op) : odsOperands(op->getOperands()), odsAttrs(op->getAttrDictionary()), odsRegions(op->getRegions()) {}

::mlir::ValueRange Conv2DOpAdaptor::getOperands() {
  return odsOperands;
}

std::pair<unsigned, unsigned> Conv2DOpAdaptor::getODSOperandIndexAndLength(unsigned index) {
  bool isVariadic[] = {false, false, true};
  int prevVariadicCount = 0;
  for (unsigned i = 0; i < index; ++i)
    if (isVariadic[i]) ++prevVariadicCount;

  // Calculate how many dynamic values a static variadic operand corresponds to.
  // This assumes all static variadic operands have the same dynamic value count.
  int variadicSize = (odsOperands.size() - 2) / 1;
  // `index` passed in as the parameter is the static index which counts each
  // operand (variadic or not) as size 1. So here for each previous static variadic
  // operand, we need to offset by (variadicSize - 1) to get where the dynamic
  // value pack for this static operand starts.
  int start = index + (variadicSize - 1) * prevVariadicCount;
  int size = isVariadic[index] ? variadicSize : 1;
  return {start, size};
}

::mlir::ValueRange Conv2DOpAdaptor::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(odsOperands.begin(), valueRange.first),
           std::next(odsOperands.begin(), valueRange.first + valueRange.second)};
}

::mlir::Value Conv2DOpAdaptor::input() {
  return *getODSOperands(0).begin();
}

::mlir::Value Conv2DOpAdaptor::weight() {
  return *getODSOperands(1).begin();
}

::mlir::Value Conv2DOpAdaptor::bias() {
  auto operands = getODSOperands(2);
  return operands.empty() ? ::mlir::Value() : *operands.begin();
}

::mlir::DictionaryAttr Conv2DOpAdaptor::getAttributes() {
  return odsAttrs;
}

::mlir::ArrayAttr Conv2DOpAdaptor::stridesAttr() {
  assert(odsAttrs && "no attributes when constructing adapter");
  ::mlir::ArrayAttr attr = odsAttrs.get("strides").cast<::mlir::ArrayAttr>();
  return attr;
}

::mlir::ArrayAttr Conv2DOpAdaptor::strides() {
  auto attr = stridesAttr();
  return attr;
}

::mlir::ArrayAttr Conv2DOpAdaptor::paddingAttr() {
  assert(odsAttrs && "no attributes when constructing adapter");
  ::mlir::ArrayAttr attr = odsAttrs.get("padding").cast<::mlir::ArrayAttr>();
  return attr;
}

::mlir::ArrayAttr Conv2DOpAdaptor::padding() {
  auto attr = paddingAttr();
  return attr;
}

::mlir::ArrayAttr Conv2DOpAdaptor::dilationAttr() {
  assert(odsAttrs && "no attributes when constructing adapter");
  ::mlir::ArrayAttr attr = odsAttrs.get("dilation").cast<::mlir::ArrayAttr>();
  return attr;
}

::mlir::ArrayAttr Conv2DOpAdaptor::dilation() {
  auto attr = dilationAttr();
  return attr;
}

::mlir::IntegerAttr Conv2DOpAdaptor::groupsAttr() {
  assert(odsAttrs && "no attributes when constructing adapter");
  ::mlir::IntegerAttr attr = odsAttrs.get("groups").cast<::mlir::IntegerAttr>();
  return attr;
}

uint32_t Conv2DOpAdaptor::groups() {
  auto attr = groupsAttr();
  return attr.getValue().getZExtValue();
}

::mlir::LogicalResult Conv2DOpAdaptor::verify(::mlir::Location loc) {
  {
    auto tblgen_strides = odsAttrs.get("strides");
    if (!tblgen_strides)
      return emitError(loc, "'nn.conv2d' op ""requires attribute 'strides'");

    if (tblgen_strides && !(((tblgen_strides.isa<::mlir::ArrayAttr>())) && (::llvm::all_of(tblgen_strides.cast<::mlir::ArrayAttr>(), [&](::mlir::Attribute attr) { return ((attr.isa<::mlir::IntegerAttr>())) && ((attr.cast<::mlir::IntegerAttr>().getType().isSignlessInteger(64))); }))))
      return emitError(loc, "'nn.conv2d' op ""attribute 'strides' failed to satisfy constraint: 64-bit integer array attribute");
  }
  {
    auto tblgen_padding = odsAttrs.get("padding");
    if (!tblgen_padding)
      return emitError(loc, "'nn.conv2d' op ""requires attribute 'padding'");

    if (tblgen_padding && !(((tblgen_padding.isa<::mlir::ArrayAttr>())) && (::llvm::all_of(tblgen_padding.cast<::mlir::ArrayAttr>(), [&](::mlir::Attribute attr) { return ((attr.isa<::mlir::IntegerAttr>())) && ((attr.cast<::mlir::IntegerAttr>().getType().isSignlessInteger(64))); }))))
      return emitError(loc, "'nn.conv2d' op ""attribute 'padding' failed to satisfy constraint: 64-bit integer array attribute");
  }
  {
    auto tblgen_dilation = odsAttrs.get("dilation");
    if (!tblgen_dilation)
      return emitError(loc, "'nn.conv2d' op ""requires attribute 'dilation'");

    if (tblgen_dilation && !(((tblgen_dilation.isa<::mlir::ArrayAttr>())) && (::llvm::all_of(tblgen_dilation.cast<::mlir::ArrayAttr>(), [&](::mlir::Attribute attr) { return ((attr.isa<::mlir::IntegerAttr>())) && ((attr.cast<::mlir::IntegerAttr>().getType().isSignlessInteger(64))); }))))
      return emitError(loc, "'nn.conv2d' op ""attribute 'dilation' failed to satisfy constraint: 64-bit integer array attribute");
  }
  {
    auto tblgen_groups = odsAttrs.get("groups");
    if (!tblgen_groups)
      return emitError(loc, "'nn.conv2d' op ""requires attribute 'groups'");

    if (tblgen_groups && !(((tblgen_groups.isa<::mlir::IntegerAttr>())) && ((tblgen_groups.cast<::mlir::IntegerAttr>().getType().isSignlessInteger(32)))))
      return emitError(loc, "'nn.conv2d' op ""attribute 'groups' failed to satisfy constraint: 32-bit signless integer attribute");
  }
  return ::mlir::success();
}

std::pair<unsigned, unsigned> Conv2DOp::getODSOperandIndexAndLength(unsigned index) {
  bool isVariadic[] = {false, false, true};
  int prevVariadicCount = 0;
  for (unsigned i = 0; i < index; ++i)
    if (isVariadic[i]) ++prevVariadicCount;

  // Calculate how many dynamic values a static variadic operand corresponds to.
  // This assumes all static variadic operands have the same dynamic value count.
  int variadicSize = (getOperation()->getNumOperands() - 2) / 1;
  // `index` passed in as the parameter is the static index which counts each
  // operand (variadic or not) as size 1. So here for each previous static variadic
  // operand, we need to offset by (variadicSize - 1) to get where the dynamic
  // value pack for this static operand starts.
  int start = index + (variadicSize - 1) * prevVariadicCount;
  int size = isVariadic[index] ? variadicSize : 1;
  return {start, size};
}

::mlir::Operation::operand_range Conv2DOp::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(getOperation()->operand_begin(), valueRange.first),
           std::next(getOperation()->operand_begin(), valueRange.first + valueRange.second)};
}

::mlir::Value Conv2DOp::input() {
  return *getODSOperands(0).begin();
}

::mlir::Value Conv2DOp::weight() {
  return *getODSOperands(1).begin();
}

::mlir::Value Conv2DOp::bias() {
  auto operands = getODSOperands(2);
  return operands.empty() ? ::mlir::Value() : *operands.begin();
}

::mlir::MutableOperandRange Conv2DOp::inputMutable() {
  auto range = getODSOperandIndexAndLength(0);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

::mlir::MutableOperandRange Conv2DOp::weightMutable() {
  auto range = getODSOperandIndexAndLength(1);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

::mlir::MutableOperandRange Conv2DOp::biasMutable() {
  auto range = getODSOperandIndexAndLength(2);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

std::pair<unsigned, unsigned> Conv2DOp::getODSResultIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::result_range Conv2DOp::getODSResults(unsigned index) {
  auto valueRange = getODSResultIndexAndLength(index);
  return {std::next(getOperation()->result_begin(), valueRange.first),
           std::next(getOperation()->result_begin(), valueRange.first + valueRange.second)};
}

::mlir::Value Conv2DOp::output() {
  return *getODSResults(0).begin();
}

::mlir::ArrayAttr Conv2DOp::stridesAttr() {
  return (*this)->getAttr(stridesAttrName()).cast<::mlir::ArrayAttr>();
}

::mlir::ArrayAttr Conv2DOp::strides() {
  auto attr = stridesAttr();
  return attr;
}

::mlir::ArrayAttr Conv2DOp::paddingAttr() {
  return (*this)->getAttr(paddingAttrName()).cast<::mlir::ArrayAttr>();
}

::mlir::ArrayAttr Conv2DOp::padding() {
  auto attr = paddingAttr();
  return attr;
}

::mlir::ArrayAttr Conv2DOp::dilationAttr() {
  return (*this)->getAttr(dilationAttrName()).cast<::mlir::ArrayAttr>();
}

::mlir::ArrayAttr Conv2DOp::dilation() {
  auto attr = dilationAttr();
  return attr;
}

::mlir::IntegerAttr Conv2DOp::groupsAttr() {
  return (*this)->getAttr(groupsAttrName()).cast<::mlir::IntegerAttr>();
}

uint32_t Conv2DOp::groups() {
  auto attr = groupsAttr();
  return attr.getValue().getZExtValue();
}

void Conv2DOp::stridesAttr(::mlir::ArrayAttr attr) {
  (*this)->setAttr(stridesAttrName(), attr);
}

void Conv2DOp::paddingAttr(::mlir::ArrayAttr attr) {
  (*this)->setAttr(paddingAttrName(), attr);
}

void Conv2DOp::dilationAttr(::mlir::ArrayAttr attr) {
  (*this)->setAttr(dilationAttrName(), attr);
}

void Conv2DOp::groupsAttr(::mlir::IntegerAttr attr) {
  (*this)->setAttr(groupsAttrName(), attr);
}

void Conv2DOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias, ::mlir::ArrayAttr strides, ::mlir::ArrayAttr padding, ::mlir::ArrayAttr dilation, ::mlir::IntegerAttr groups) {
  odsState.addOperands(input);
  odsState.addOperands(weight);
  if (bias)
    odsState.addOperands(bias);
  odsState.addAttribute(stridesAttrName(odsState.name), strides);
  odsState.addAttribute(paddingAttrName(odsState.name), padding);
  odsState.addAttribute(dilationAttrName(odsState.name), dilation);
  odsState.addAttribute(groupsAttrName(odsState.name), groups);
  odsState.addTypes(output);
}

void Conv2DOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias, ::mlir::ArrayAttr strides, ::mlir::ArrayAttr padding, ::mlir::ArrayAttr dilation, ::mlir::IntegerAttr groups) {
  odsState.addOperands(input);
  odsState.addOperands(weight);
  if (bias)
    odsState.addOperands(bias);
  odsState.addAttribute(stridesAttrName(odsState.name), strides);
  odsState.addAttribute(paddingAttrName(odsState.name), padding);
  odsState.addAttribute(dilationAttrName(odsState.name), dilation);
  odsState.addAttribute(groupsAttrName(odsState.name), groups);
  assert(resultTypes.size() == 1u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void Conv2DOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias, ::mlir::ArrayAttr strides, ::mlir::ArrayAttr padding, ::mlir::ArrayAttr dilation, uint32_t groups) {
  odsState.addOperands(input);
  odsState.addOperands(weight);
  if (bias)
    odsState.addOperands(bias);
  odsState.addAttribute(stridesAttrName(odsState.name), strides);
  odsState.addAttribute(paddingAttrName(odsState.name), padding);
  odsState.addAttribute(dilationAttrName(odsState.name), dilation);
  odsState.addAttribute(groupsAttrName(odsState.name), odsBuilder.getIntegerAttr(odsBuilder.getIntegerType(32), groups));
  odsState.addTypes(output);
}

void Conv2DOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias, ::mlir::ArrayAttr strides, ::mlir::ArrayAttr padding, ::mlir::ArrayAttr dilation, uint32_t groups) {
  odsState.addOperands(input);
  odsState.addOperands(weight);
  if (bias)
    odsState.addOperands(bias);
  odsState.addAttribute(stridesAttrName(odsState.name), strides);
  odsState.addAttribute(paddingAttrName(odsState.name), padding);
  odsState.addAttribute(dilationAttrName(odsState.name), dilation);
  odsState.addAttribute(groupsAttrName(odsState.name), odsBuilder.getIntegerAttr(odsBuilder.getIntegerType(32), groups));
  assert(resultTypes.size() == 1u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void Conv2DOp::build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes) {
  assert(operands.size() >= 2u && "mismatched number of parameters");
  odsState.addOperands(operands);
  odsState.addAttributes(attributes);
  assert(resultTypes.size() == 1u && "mismatched number of return types");
  odsState.addTypes(resultTypes);
}

::mlir::LogicalResult Conv2DOp::verify() {
  {
    auto tblgen_strides = (*this)->getAttr(stridesAttrName());
    if (!tblgen_strides)
      return emitOpError("requires attribute 'strides'");

    if (::mlir::failed(__mlir_ods_local_attr_constraint_NNOps0(*this, tblgen_strides, "strides")))
      return ::mlir::failure();
  }
  {
    auto tblgen_padding = (*this)->getAttr(paddingAttrName());
    if (!tblgen_padding)
      return emitOpError("requires attribute 'padding'");

    if (::mlir::failed(__mlir_ods_local_attr_constraint_NNOps0(*this, tblgen_padding, "padding")))
      return ::mlir::failure();
  }
  {
    auto tblgen_dilation = (*this)->getAttr(dilationAttrName());
    if (!tblgen_dilation)
      return emitOpError("requires attribute 'dilation'");

    if (::mlir::failed(__mlir_ods_local_attr_constraint_NNOps0(*this, tblgen_dilation, "dilation")))
      return ::mlir::failure();
  }
  {
    auto tblgen_groups = (*this)->getAttr(groupsAttrName());
    if (!tblgen_groups)
      return emitOpError("requires attribute 'groups'");

    if (::mlir::failed(__mlir_ods_local_attr_constraint_NNOps1(*this, tblgen_groups, "groups")))
      return ::mlir::failure();
  }
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSOperands(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
    auto valueGroup1 = getODSOperands(1);

    for (auto v : valueGroup1) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
    auto valueGroup2 = getODSOperands(2);

    if (valueGroup2.size() > 1) {
      return emitOpError("operand group starting at #") << index
          << " requires 0 or 1 element, but found " << valueGroup2.size();
    }

    for (auto v : valueGroup2) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
  }
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSResults(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "result", index++)))
        return ::mlir::failure();
    }
  }
  return ::mlir::success();
}

void Conv2DOp::getEffects(::mlir::SmallVectorImpl<::mlir::SideEffects::EffectInstance<::mlir::MemoryEffects::Effect>> &effects) {
}

} // namespace NN
} // namespace MIC
DEFINE_EXPLICIT_TYPE_ID(::MIC::NN::Conv2DOp)

namespace MIC {
namespace NN {

//===----------------------------------------------------------------------===//
// ::MIC::NN::GELUOp definitions
//===----------------------------------------------------------------------===//

GELUOpAdaptor::GELUOpAdaptor(::mlir::ValueRange values, ::mlir::DictionaryAttr attrs, ::mlir::RegionRange regions) : odsOperands(values), odsAttrs(attrs), odsRegions(regions) {}

GELUOpAdaptor::GELUOpAdaptor(GELUOp &op) : odsOperands(op->getOperands()), odsAttrs(op->getAttrDictionary()), odsRegions(op->getRegions()) {}

::mlir::ValueRange GELUOpAdaptor::getOperands() {
  return odsOperands;
}

std::pair<unsigned, unsigned> GELUOpAdaptor::getODSOperandIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::ValueRange GELUOpAdaptor::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(odsOperands.begin(), valueRange.first),
           std::next(odsOperands.begin(), valueRange.first + valueRange.second)};
}

::mlir::Value GELUOpAdaptor::input() {
  return *getODSOperands(0).begin();
}

::mlir::DictionaryAttr GELUOpAdaptor::getAttributes() {
  return odsAttrs;
}

::mlir::LogicalResult GELUOpAdaptor::verify(::mlir::Location loc) {
  return ::mlir::success();
}

std::pair<unsigned, unsigned> GELUOp::getODSOperandIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::operand_range GELUOp::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(getOperation()->operand_begin(), valueRange.first),
           std::next(getOperation()->operand_begin(), valueRange.first + valueRange.second)};
}

::mlir::Value GELUOp::input() {
  return *getODSOperands(0).begin();
}

::mlir::MutableOperandRange GELUOp::inputMutable() {
  auto range = getODSOperandIndexAndLength(0);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

std::pair<unsigned, unsigned> GELUOp::getODSResultIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::result_range GELUOp::getODSResults(unsigned index) {
  auto valueRange = getODSResultIndexAndLength(index);
  return {std::next(getOperation()->result_begin(), valueRange.first),
           std::next(getOperation()->result_begin(), valueRange.first + valueRange.second)};
}

::mlir::Value GELUOp::output() {
  return *getODSResults(0).begin();
}

void GELUOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input) {
  odsState.addOperands(input);
  odsState.addTypes(output);
}

void GELUOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input) {
  odsState.addOperands(input);
  assert(resultTypes.size() == 1u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void GELUOp::build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes) {
  assert(operands.size() == 1u && "mismatched number of parameters");
  odsState.addOperands(operands);
  odsState.addAttributes(attributes);
  assert(resultTypes.size() == 1u && "mismatched number of return types");
  odsState.addTypes(resultTypes);
}

::mlir::LogicalResult GELUOp::verify() {
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSOperands(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
  }
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSResults(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "result", index++)))
        return ::mlir::failure();
    }
  }
  return ::mlir::success();
}

void GELUOp::getEffects(::mlir::SmallVectorImpl<::mlir::SideEffects::EffectInstance<::mlir::MemoryEffects::Effect>> &effects) {
}

} // namespace NN
} // namespace MIC
DEFINE_EXPLICIT_TYPE_ID(::MIC::NN::GELUOp)

namespace MIC {
namespace NN {

//===----------------------------------------------------------------------===//
// ::MIC::NN::LayerNormOp definitions
//===----------------------------------------------------------------------===//

LayerNormOpAdaptor::LayerNormOpAdaptor(::mlir::ValueRange values, ::mlir::DictionaryAttr attrs, ::mlir::RegionRange regions) : odsOperands(values), odsAttrs(attrs), odsRegions(regions) {}

LayerNormOpAdaptor::LayerNormOpAdaptor(LayerNormOp &op) : odsOperands(op->getOperands()), odsAttrs(op->getAttrDictionary()), odsRegions(op->getRegions()) {}

::mlir::ValueRange LayerNormOpAdaptor::getOperands() {
  return odsOperands;
}

std::pair<unsigned, unsigned> LayerNormOpAdaptor::getODSOperandIndexAndLength(unsigned index) {
  assert(odsAttrs && "missing segment size attribute for op");
  auto sizeAttr = odsAttrs.get("operand_segment_sizes").cast<::mlir::DenseIntElementsAttr>();

  const uint32_t *sizeAttrValueIt = &*sizeAttr.value_begin<uint32_t>();
  if (sizeAttr.isSplat())
    return {*sizeAttrValueIt * index, *sizeAttrValueIt};

  unsigned start = 0;
  for (unsigned i = 0; i < index; ++i)
    start += sizeAttrValueIt[i];
  return {start, sizeAttrValueIt[index]};
}

::mlir::ValueRange LayerNormOpAdaptor::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(odsOperands.begin(), valueRange.first),
           std::next(odsOperands.begin(), valueRange.first + valueRange.second)};
}

::mlir::Value LayerNormOpAdaptor::input() {
  return *getODSOperands(0).begin();
}

::mlir::Value LayerNormOpAdaptor::scale() {
  auto operands = getODSOperands(1);
  return operands.empty() ? ::mlir::Value() : *operands.begin();
}

::mlir::Value LayerNormOpAdaptor::bias() {
  auto operands = getODSOperands(2);
  return operands.empty() ? ::mlir::Value() : *operands.begin();
}

::mlir::DictionaryAttr LayerNormOpAdaptor::getAttributes() {
  return odsAttrs;
}

::mlir::FloatAttr LayerNormOpAdaptor::epsilonAttr() {
  assert(odsAttrs && "no attributes when constructing adapter");
  ::mlir::FloatAttr attr = odsAttrs.get("epsilon").cast<::mlir::FloatAttr>();
  return attr;
}

::llvm::APFloat LayerNormOpAdaptor::epsilon() {
  auto attr = epsilonAttr();
  return attr.getValue();
}

::mlir::LogicalResult LayerNormOpAdaptor::verify(::mlir::Location loc) {
  {
    auto sizeAttr = odsAttrs.get("operand_segment_sizes").dyn_cast<::mlir::DenseIntElementsAttr>();
    if (!sizeAttr)
      return emitError(loc, "'nn.layer_norm' op ""missing segment sizes attribute 'operand_segment_sizes'");
    auto numElements =
        sizeAttr.getType().cast<::mlir::ShapedType>().getNumElements();
    if (numElements != 3)
      return emitError(loc, "'nn.layer_norm' op ""'operand_segment_sizes' attribute for specifying operand segments must have 3 "
                "elements, but got ") << numElements;
  }
    {
    auto tblgen_epsilon = odsAttrs.get("epsilon");
    if (!tblgen_epsilon)
      return emitError(loc, "'nn.layer_norm' op ""requires attribute 'epsilon'");

    if (tblgen_epsilon && !(((tblgen_epsilon.isa<::mlir::FloatAttr>())) && ((tblgen_epsilon.cast<::mlir::FloatAttr>().getType().isF64()))))
      return emitError(loc, "'nn.layer_norm' op ""attribute 'epsilon' failed to satisfy constraint: 64-bit float attribute");
  }
  return ::mlir::success();
}

std::pair<unsigned, unsigned> LayerNormOp::getODSOperandIndexAndLength(unsigned index) {
  auto sizeAttr =
      (*this)->getAttr(operand_segment_sizesAttrName()).cast<::mlir::DenseIntElementsAttr>();

  const uint32_t *sizeAttrValueIt = &*sizeAttr.value_begin<uint32_t>();
  if (sizeAttr.isSplat())
    return {*sizeAttrValueIt * index, *sizeAttrValueIt};

  unsigned start = 0;
  for (unsigned i = 0; i < index; ++i)
    start += sizeAttrValueIt[i];
  return {start, sizeAttrValueIt[index]};
}

::mlir::Operation::operand_range LayerNormOp::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(getOperation()->operand_begin(), valueRange.first),
           std::next(getOperation()->operand_begin(), valueRange.first + valueRange.second)};
}

::mlir::Value LayerNormOp::input() {
  return *getODSOperands(0).begin();
}

::mlir::Value LayerNormOp::scale() {
  auto operands = getODSOperands(1);
  return operands.empty() ? ::mlir::Value() : *operands.begin();
}

::mlir::Value LayerNormOp::bias() {
  auto operands = getODSOperands(2);
  return operands.empty() ? ::mlir::Value() : *operands.begin();
}

::mlir::MutableOperandRange LayerNormOp::inputMutable() {
  auto range = getODSOperandIndexAndLength(0);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second, ::mlir::MutableOperandRange::OperandSegment(0u, *getOperation()->getAttrDictionary().getNamed(operand_segment_sizesAttrName())));
  return mutableRange;
}

::mlir::MutableOperandRange LayerNormOp::scaleMutable() {
  auto range = getODSOperandIndexAndLength(1);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second, ::mlir::MutableOperandRange::OperandSegment(1u, *getOperation()->getAttrDictionary().getNamed(operand_segment_sizesAttrName())));
  return mutableRange;
}

::mlir::MutableOperandRange LayerNormOp::biasMutable() {
  auto range = getODSOperandIndexAndLength(2);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second, ::mlir::MutableOperandRange::OperandSegment(2u, *getOperation()->getAttrDictionary().getNamed(operand_segment_sizesAttrName())));
  return mutableRange;
}

std::pair<unsigned, unsigned> LayerNormOp::getODSResultIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::result_range LayerNormOp::getODSResults(unsigned index) {
  auto valueRange = getODSResultIndexAndLength(index);
  return {std::next(getOperation()->result_begin(), valueRange.first),
           std::next(getOperation()->result_begin(), valueRange.first + valueRange.second)};
}

::mlir::Value LayerNormOp::output() {
  return *getODSResults(0).begin();
}

::mlir::FloatAttr LayerNormOp::epsilonAttr() {
  return (*this)->getAttr(epsilonAttrName()).cast<::mlir::FloatAttr>();
}

::llvm::APFloat LayerNormOp::epsilon() {
  auto attr = epsilonAttr();
  return attr.getValue();
}

void LayerNormOp::epsilonAttr(::mlir::FloatAttr attr) {
  (*this)->setAttr(epsilonAttrName(), attr);
}

void LayerNormOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input, /*optional*/::mlir::Value scale, /*optional*/::mlir::Value bias, ::mlir::FloatAttr epsilon) {
  odsState.addOperands(input);
  if (scale)
    odsState.addOperands(scale);
  if (bias)
    odsState.addOperands(bias);
  odsState.addAttribute(operand_segment_sizesAttrName(odsState.name), odsBuilder.getI32VectorAttr({1, (scale ? 1 : 0), (bias ? 1 : 0)}));
  odsState.addAttribute(epsilonAttrName(odsState.name), epsilon);
  odsState.addTypes(output);
}

void LayerNormOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input, /*optional*/::mlir::Value scale, /*optional*/::mlir::Value bias, ::mlir::FloatAttr epsilon) {
  odsState.addOperands(input);
  if (scale)
    odsState.addOperands(scale);
  if (bias)
    odsState.addOperands(bias);
  odsState.addAttribute(operand_segment_sizesAttrName(odsState.name), odsBuilder.getI32VectorAttr({1, (scale ? 1 : 0), (bias ? 1 : 0)}));
  odsState.addAttribute(epsilonAttrName(odsState.name), epsilon);
  assert(resultTypes.size() == 1u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void LayerNormOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input, /*optional*/::mlir::Value scale, /*optional*/::mlir::Value bias, ::llvm::APFloat epsilon) {
  odsState.addOperands(input);
  if (scale)
    odsState.addOperands(scale);
  if (bias)
    odsState.addOperands(bias);
  odsState.addAttribute(operand_segment_sizesAttrName(odsState.name), odsBuilder.getI32VectorAttr({1, (scale ? 1 : 0), (bias ? 1 : 0)}));
  odsState.addAttribute(epsilonAttrName(odsState.name), odsBuilder.getFloatAttr(odsBuilder.getF64Type(), epsilon));
  odsState.addTypes(output);
}

void LayerNormOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input, /*optional*/::mlir::Value scale, /*optional*/::mlir::Value bias, ::llvm::APFloat epsilon) {
  odsState.addOperands(input);
  if (scale)
    odsState.addOperands(scale);
  if (bias)
    odsState.addOperands(bias);
  odsState.addAttribute(operand_segment_sizesAttrName(odsState.name), odsBuilder.getI32VectorAttr({1, (scale ? 1 : 0), (bias ? 1 : 0)}));
  odsState.addAttribute(epsilonAttrName(odsState.name), odsBuilder.getFloatAttr(odsBuilder.getF64Type(), epsilon));
  assert(resultTypes.size() == 1u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void LayerNormOp::build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes) {
  assert(operands.size() >= 1u && "mismatched number of parameters");
  odsState.addOperands(operands);
  odsState.addAttributes(attributes);
  assert(resultTypes.size() == 1u && "mismatched number of return types");
  odsState.addTypes(resultTypes);
}

::mlir::LogicalResult LayerNormOp::verify() {
  {
    auto sizeAttr = (*this)->getAttr(operand_segment_sizesAttrName()).dyn_cast<::mlir::DenseIntElementsAttr>();
    if (!sizeAttr)
      return emitOpError("missing segment sizes attribute 'operand_segment_sizes'");
    auto numElements =
        sizeAttr.getType().cast<::mlir::ShapedType>().getNumElements();
    if (numElements != 3)
      return emitOpError("'operand_segment_sizes' attribute for specifying operand segments must have 3 "
                "elements, but got ") << numElements;
  }
    {
    auto tblgen_epsilon = (*this)->getAttr(epsilonAttrName());
    if (!tblgen_epsilon)
      return emitOpError("requires attribute 'epsilon'");

    if (::mlir::failed(__mlir_ods_local_attr_constraint_NNOps2(*this, tblgen_epsilon, "epsilon")))
      return ::mlir::failure();
  }
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSOperands(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
    auto valueGroup1 = getODSOperands(1);

    if (valueGroup1.size() > 1) {
      return emitOpError("operand group starting at #") << index
          << " requires 0 or 1 element, but found " << valueGroup1.size();
    }

    for (auto v : valueGroup1) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
    auto valueGroup2 = getODSOperands(2);

    if (valueGroup2.size() > 1) {
      return emitOpError("operand group starting at #") << index
          << " requires 0 or 1 element, but found " << valueGroup2.size();
    }

    for (auto v : valueGroup2) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
  }
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSResults(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "result", index++)))
        return ::mlir::failure();
    }
  }
  return ::mlir::success();
}

void LayerNormOp::getEffects(::mlir::SmallVectorImpl<::mlir::SideEffects::EffectInstance<::mlir::MemoryEffects::Effect>> &effects) {
}

} // namespace NN
} // namespace MIC
DEFINE_EXPLICIT_TYPE_ID(::MIC::NN::LayerNormOp)

namespace MIC {
namespace NN {

//===----------------------------------------------------------------------===//
// ::MIC::NN::LinearOp definitions
//===----------------------------------------------------------------------===//

LinearOpAdaptor::LinearOpAdaptor(::mlir::ValueRange values, ::mlir::DictionaryAttr attrs, ::mlir::RegionRange regions) : odsOperands(values), odsAttrs(attrs), odsRegions(regions) {}

LinearOpAdaptor::LinearOpAdaptor(LinearOp &op) : odsOperands(op->getOperands()), odsAttrs(op->getAttrDictionary()), odsRegions(op->getRegions()) {}

::mlir::ValueRange LinearOpAdaptor::getOperands() {
  return odsOperands;
}

std::pair<unsigned, unsigned> LinearOpAdaptor::getODSOperandIndexAndLength(unsigned index) {
  bool isVariadic[] = {false, false, true};
  int prevVariadicCount = 0;
  for (unsigned i = 0; i < index; ++i)
    if (isVariadic[i]) ++prevVariadicCount;

  // Calculate how many dynamic values a static variadic operand corresponds to.
  // This assumes all static variadic operands have the same dynamic value count.
  int variadicSize = (odsOperands.size() - 2) / 1;
  // `index` passed in as the parameter is the static index which counts each
  // operand (variadic or not) as size 1. So here for each previous static variadic
  // operand, we need to offset by (variadicSize - 1) to get where the dynamic
  // value pack for this static operand starts.
  int start = index + (variadicSize - 1) * prevVariadicCount;
  int size = isVariadic[index] ? variadicSize : 1;
  return {start, size};
}

::mlir::ValueRange LinearOpAdaptor::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(odsOperands.begin(), valueRange.first),
           std::next(odsOperands.begin(), valueRange.first + valueRange.second)};
}

::mlir::Value LinearOpAdaptor::input() {
  return *getODSOperands(0).begin();
}

::mlir::Value LinearOpAdaptor::weight() {
  return *getODSOperands(1).begin();
}

::mlir::Value LinearOpAdaptor::bias() {
  auto operands = getODSOperands(2);
  return operands.empty() ? ::mlir::Value() : *operands.begin();
}

::mlir::DictionaryAttr LinearOpAdaptor::getAttributes() {
  return odsAttrs;
}

::mlir::LogicalResult LinearOpAdaptor::verify(::mlir::Location loc) {
  return ::mlir::success();
}

std::pair<unsigned, unsigned> LinearOp::getODSOperandIndexAndLength(unsigned index) {
  bool isVariadic[] = {false, false, true};
  int prevVariadicCount = 0;
  for (unsigned i = 0; i < index; ++i)
    if (isVariadic[i]) ++prevVariadicCount;

  // Calculate how many dynamic values a static variadic operand corresponds to.
  // This assumes all static variadic operands have the same dynamic value count.
  int variadicSize = (getOperation()->getNumOperands() - 2) / 1;
  // `index` passed in as the parameter is the static index which counts each
  // operand (variadic or not) as size 1. So here for each previous static variadic
  // operand, we need to offset by (variadicSize - 1) to get where the dynamic
  // value pack for this static operand starts.
  int start = index + (variadicSize - 1) * prevVariadicCount;
  int size = isVariadic[index] ? variadicSize : 1;
  return {start, size};
}

::mlir::Operation::operand_range LinearOp::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(getOperation()->operand_begin(), valueRange.first),
           std::next(getOperation()->operand_begin(), valueRange.first + valueRange.second)};
}

::mlir::Value LinearOp::input() {
  return *getODSOperands(0).begin();
}

::mlir::Value LinearOp::weight() {
  return *getODSOperands(1).begin();
}

::mlir::Value LinearOp::bias() {
  auto operands = getODSOperands(2);
  return operands.empty() ? ::mlir::Value() : *operands.begin();
}

::mlir::MutableOperandRange LinearOp::inputMutable() {
  auto range = getODSOperandIndexAndLength(0);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

::mlir::MutableOperandRange LinearOp::weightMutable() {
  auto range = getODSOperandIndexAndLength(1);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

::mlir::MutableOperandRange LinearOp::biasMutable() {
  auto range = getODSOperandIndexAndLength(2);
  auto mutableRange = ::mlir::MutableOperandRange(getOperation(), range.first, range.second);
  return mutableRange;
}

std::pair<unsigned, unsigned> LinearOp::getODSResultIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::result_range LinearOp::getODSResults(unsigned index) {
  auto valueRange = getODSResultIndexAndLength(index);
  return {std::next(getOperation()->result_begin(), valueRange.first),
           std::next(getOperation()->result_begin(), valueRange.first + valueRange.second)};
}

::mlir::Value LinearOp::output() {
  return *getODSResults(0).begin();
}

void LinearOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type output, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias) {
  odsState.addOperands(input);
  odsState.addOperands(weight);
  if (bias)
    odsState.addOperands(bias);
  odsState.addTypes(output);
}

void LinearOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value input, ::mlir::Value weight, /*optional*/::mlir::Value bias) {
  odsState.addOperands(input);
  odsState.addOperands(weight);
  if (bias)
    odsState.addOperands(bias);
  assert(resultTypes.size() == 1u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void LinearOp::build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes) {
  assert(operands.size() >= 2u && "mismatched number of parameters");
  odsState.addOperands(operands);
  odsState.addAttributes(attributes);
  assert(resultTypes.size() == 1u && "mismatched number of return types");
  odsState.addTypes(resultTypes);
}

::mlir::LogicalResult LinearOp::verify() {
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSOperands(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
    auto valueGroup1 = getODSOperands(1);

    for (auto v : valueGroup1) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
    auto valueGroup2 = getODSOperands(2);

    if (valueGroup2.size() > 1) {
      return emitOpError("operand group starting at #") << index
          << " requires 0 or 1 element, but found " << valueGroup2.size();
    }

    for (auto v : valueGroup2) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
  }
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSResults(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_NNOps0(*this, v.getType(), "result", index++)))
        return ::mlir::failure();
    }
  }
  return ::mlir::success();
}

void LinearOp::getEffects(::mlir::SmallVectorImpl<::mlir::SideEffects::EffectInstance<::mlir::MemoryEffects::Effect>> &effects) {
}

} // namespace NN
} // namespace MIC
DEFINE_EXPLICIT_TYPE_ID(::MIC::NN::LinearOp)


#endif  // GET_OP_CLASSES

