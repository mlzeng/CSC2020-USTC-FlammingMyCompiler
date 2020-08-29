#ifndef SYSYC_FUNCTION_H
#define SYSYC_FUNCTION_H

#include <cassert>
#include <cstddef>
#include <iterator>
#include <list>
#include <map>

#include "BasicBlock.h"
#include "HighBlock.h"
#include "Instruction.h"
#include "ReturnVal.h"
#include "User.h"
#include <unordered_map>

class Module;
class Argument;
class BaseBlock;
class BasicBlock;
class Type;
class FunctionType;

class Function : public Value {
public:
  Function(FunctionType *ty, const std::string &name, Module *parent);
  ~Function();
  static Function *create(FunctionType *ty, const std::string &name,
                          Module *parent);

  FunctionType *getFunctionType() const;

  Type *getResultType() const;

  void addBasicBlock(BasicBlock *bb);

  void addBasicBlockAfter(std::list<BasicBlock *>::iterator after_pos,
                          BasicBlock *bb);

  unsigned getNumArgs() const;

  unsigned getNumBasicBlocks() const { return basic_blocks_.size(); }

  Module *getParent() const;

  std::list<BasicBlock *> &getBasicBlocks() { return basic_blocks_; }
  std::list<Argument *> &getArgs() { return arguments_; }

  BasicBlock *getEntryBlock() { return *basic_blocks_.begin(); }

  std::list<Argument *>::iterator arg_begin() { return arguments_.begin(); }
  std::list<Argument *>::iterator arg_end() { return arguments_.end(); }

  void removeBasicBlock(BasicBlock *bb);

  void HighIRprint();

  void print() override;

private:
  void buildArgs();

private:
  std::list<BasicBlock *> basic_blocks_; // basic blocks
  std::list<Argument *> arguments_;      // arguments
  Module *parent_;
  int print_cnt_;

private:
  std::list<BaseBlock *> base_blocks_; // base blocks

  bool multithreading_ = false;

public:
  void removeBaseBlock(BaseBlock *bb) { base_blocks_.remove(bb); }
  void addBaseBlock(BaseBlock *basebb);
  void insertBaseBlock(std::list<BaseBlock *>::iterator iter,
                       BaseBlock *basebb) {
    base_blocks_.insert(iter, basebb);
    basebb->clearFather();
  }
  std::list<BaseBlock *> &getBaseBlocks() { return base_blocks_; }

  void setMultithreading(bool stat) { multithreading_ = stat; }
  bool getMultithreading() { return multithreading_; }
  void clearCnt() { print_cnt_ = 0; }
};

// Argument of Function, does not contain actual value
class Argument : public Value {
public:
  /// Argument constructor.
  explicit Argument(Type *ty, const std::string &name = "",
                    Function *f = nullptr, unsigned arg_no = 0)
      : Value(ty, name), parent_(f), arg_no_(arg_no) {}
  ~Argument() {}

  inline const Function *getParent() const { return parent_; }
  inline Function *getParent() { return parent_; }

  /// For example in "void foo(int a, float b)" a is 0 and b is 1.
  unsigned getArgNo() const {
    if (parent_ == nullptr)
      exit(_getArgNo_Argument);
    return arg_no_;
  }

  virtual void print() override;

private:
  Function *parent_;
  unsigned arg_no_; // argument No.
  std::vector<Value *> array_bound_;

public:
  void setArrayBound(std::vector<Value *> &array_bound) {
    array_bound_.assign(array_bound.begin(), array_bound.end());
  }

  std::vector<Value *> &getArrayBound() { return array_bound_; }
};

#endif // SYSYC_FUNCTION_H
