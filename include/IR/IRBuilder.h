#ifndef SYSYC_IRBUILDER_H
#define SYSYC_IRBUILDER_H

#include "BasicBlock.h"
#include "Instruction.h"
#include "Value.h"

class IRBuilder {
private:
  BasicBlock *BB_;

public:
  IRBuilder(BasicBlock *bb) { this->BB_ = bb; };
  ~IRBuilder() = default;
  BasicBlock *GetInsertBlock() { return this->BB_; }
  void SetInsertPoint(BasicBlock *bb) {
    this->BB_ = bb;
  } //在某个基本块中插入指令
  void insertInstr(Instruction *pos, Instruction *insert) {
    BB_->insertInstr(pos, insert);
  }
  void deleteInstr(Instruction *instr) { BB_->deleteInstr(instr); }

  BinaryInst *CreateAdd(Value *v1, Value *v2) {
    return BinaryInst::createAdd(v1, v2, this->BB_);
  } //创建加法指令（以及其他算术指令）
  BinaryInst *CreateSub(Value *v1, Value *v2) {
    return BinaryInst::createSub(v1, v2, this->BB_);
  }
  BinaryInst *CreateMul(Value *v1, Value *v2) {
    return BinaryInst::createMul(v1, v2, this->BB_);
  }
  BinaryInst *CreateDiv(Value *v1, Value *v2) {
    return BinaryInst::createDiv(v1, v2, this->BB_);
  }
  BinaryInst *CreateRem(Value *v1, Value *v2) {
    return BinaryInst::createRem(v1, v2, this->BB_);
  }
  BinaryInst *CreateAnd(Value *v1, Value *v2) {
    return BinaryInst::createAnd(v1, v2, this->BB_);
  }
  BinaryInst *CreateOr(Value *v1, Value *v2) {
    return BinaryInst::createOr(v1, v2, this->BB_);
  }

  CmpInst *CreateCmpEQ(Value *lhs, Value *rhs, Module *m) {
    return CmpInst::createCmp(CmpInst::EQ, lhs, rhs, this->BB_, m);
  }
  CmpInst *CreateCmpNE(Value *lhs, Value *rhs, Module *m) {
    return CmpInst::createCmp(CmpInst::NE, lhs, rhs, this->BB_, m);
  }
  CmpInst *CreateCmpGT(Value *lhs, Value *rhs, Module *m) {
    return CmpInst::createCmp(CmpInst::GT, lhs, rhs, this->BB_, m);
  }
  CmpInst *CreateCmpGE(Value *lhs, Value *rhs, Module *m) {
    return CmpInst::createCmp(CmpInst::GE, lhs, rhs, this->BB_, m);
  }
  CmpInst *CreateCmpLT(Value *lhs, Value *rhs, Module *m) {
    return CmpInst::createCmp(CmpInst::LT, lhs, rhs, this->BB_, m);
  }
  CmpInst *CreateCmpLE(Value *lhs, Value *rhs, Module *m) {
    return CmpInst::createCmp(CmpInst::LE, lhs, rhs, this->BB_, m);
  }

  CallInst *CreateCall(Function *func, std::vector<Value *> args) {
    return CallInst::create(func, args, this->BB_);
  }

  BranchInst *CreateBr(BasicBlock *if_true) {
    return BranchInst::createBr(if_true, this->BB_);
  }
  BranchInst *CreateCondBr(Value *cond, BasicBlock *if_true,
                           BasicBlock *if_false) {
    return BranchInst::createCondBr(cond, if_true, if_false, this->BB_);
  }

  ReturnInst *CreateRet(Value *val) {
    return ReturnInst::createRet(val, this->BB_);
  }
  ReturnInst *CreateVoidRet() { return ReturnInst::createVoidRet(this->BB_); }

  GetElementPtrInst *CreateGEP(Value *ptr, std::vector<Value *> idxs) {
    return GetElementPtrInst::createGEP(ptr, idxs, this->BB_);
  }

  StoreInst *CreateStore(Value *val, Value *ptr) {
    return StoreInst::createStore(val, ptr, this->BB_);
  }
  LoadInst *CreateLoad(Type *ty, Value *ptr) {
    return LoadInst::createLoad(ty, ptr, this->BB_);
  }
  LoadInst *CreateLoad(Value *ptr) {
    return LoadInst::createLoad(ptr->getType()->getPointerElementType(), ptr,
                                this->BB_);
  }

  AllocaInst *CreateAlloca(Type *ty) {
    return AllocaInst::createAlloca(ty, this->BB_);
  }
  CastInst *CreateZext(Value *val, Type *ty) {
    return CastInst::createZExt(val, ty, this->BB_);
  }
  PhiInst *CreatePhi(Type *ty) { return PhiInst::createPhi(ty, this->BB_); }

  HighIR *CreateBreak(Module *m) { return HighIR::createBreak(m, this->BB_); }
  HighIR *CreateContinue(Module *m) {
    return HighIR::createContinue(m, this->BB_);
  }
};

#endif // SYSYC_IRBUILDER_H
