#include "BaseBlock.h"
#include "Function.h"

// this will auto add this to function's baseblock list
// don't use this in nested structure
BaseBlock::BaseBlock(BlockType ty, Module *m, const std::string &name,
                     Function *func)
    : block_ty_(ty), Value(Type::getLabelTy(m), name), father_(nullptr),
      func_(func) {
  func->addBaseBlock(this);
}