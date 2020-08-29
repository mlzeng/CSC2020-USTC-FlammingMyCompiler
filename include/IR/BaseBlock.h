#ifndef SYSYC_BASEBLOCK_H
#define SYSYC_BASEBLOCK_H

#include "Type.h"
#include "Value.h"

class Function;

class BaseBlock : public Value {
public:
  enum BlockType {
    Basic,
    If,
    While,
  };

  bool isBasicBlock() const { return block_ty_ == Basic; }
  bool isIfBlock() const { return block_ty_ == If; }
  bool isWhileBlock() const { return block_ty_ == While; }

  void setFunction(Function *func) { func_ = func; }
  void setBaseFather(BaseBlock *father) {
    father_ = father;
    this->setFunction(father->getFunction());
  }
  void clearFather() { father_ = nullptr; }

  BaseBlock *getBaseFather() const { return father_; }
  Function *getFunction() const { return func_; }

  void print() override {}

protected:
  // this will auto add this to function's baseblock list
  // don't use this in nested structure
  explicit BaseBlock(BlockType ty, Module *m, const std::string &name,
                     Function *func);

  explicit BaseBlock(BlockType ty, Module *m, const std::string &name)
      : block_ty_(ty), Value(Type::getLabelTy(m), name), father_(nullptr),
        func_(nullptr) {}

  BlockType block_ty_;
  BaseBlock *father_;
  Function *func_;
};

#endif // SYSYC_BASEBLOCK_H
