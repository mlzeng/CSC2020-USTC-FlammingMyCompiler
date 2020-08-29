#ifndef SYSYC_INSTRUCTION_H
#define SYSYC_INSTRUCTION_H

#include "ReturnVal.h"
#include "Type.h"
#include "User.h"
#include <cassert>
#include <map>
#include <set>

class BasicBlock;
class Function;

class Instruction : public User {
public:
  enum OpID {
    // High IR
    Break,
    Continue,
    // Terminator Instructions
    Ret,
    Br,
    // Standard unary operators
    Neg,
    // Standard binary operators
    Add,
    Sub,
    RSub, // Reverse Subtract
    Mul,
    Div,
    Rem,
    AddAddr, // deprecated
    // Logical operators
    And,
    Or,
    Not,
    // Memory operators
    Alloca,
    Load,
    Store,
    // Shift operators
    Shl,  // <<
    AShr, // arithmetic >>
    LShr, // logical >>
    // Other operators
    Cmp,
    PHI,
    Call,
    GEP,     // GetElementPtr
    ZExt,    // zero extend
    MulAdd,  // a*b+c
    MTSTART, // %thread_id = MTSTART
    MTEND,   // MTEND %thread_id
    // NEON SIMD
    VV, // sum(vector .* vector)
    BIC
  };
  // create instruction, auto insert to bb
  // ty here is result type
  Instruction(Type *ty, OpID id, unsigned num_ops, BasicBlock *parent);
  Instruction(Type *ty, OpID id, unsigned num_ops);

  inline const BasicBlock *getParent() const { return parent_; }
  inline BasicBlock *getParent() { return parent_; }

  void setParent(BasicBlock *parent) { parent_ = parent; }

  // Return the function this instruction belongs to.
  const Function *getFunction() const;
  Function *getFunction() {
    return const_cast<Function *>(
        static_cast<const Instruction *>(this)->getFunction());
  }

  OpID getInstrType() const { return op_id_; }
  bool isVoid() {
    return ((op_id_ == Break) || (op_id_ == Continue) || (op_id_ == Ret) ||
            (op_id_ == Br) || (op_id_ == Store) ||
            (op_id_ == Call && this->getType()->isVoidTy()));
  }

  bool isBreak() { return op_id_ == Break; }
  bool isContinue() { return op_id_ == Continue; }

  bool isPHI() { return op_id_ == PHI; }
  bool isRet() { return op_id_ == Ret; }
  bool isStore() { return op_id_ == Store; }
  bool isLoad() { return op_id_ == Load; }
  bool isAlloca() { return op_id_ == Alloca; }
  bool isBr() { return op_id_ == Br; }
  bool isNeg() { return op_id_ == Neg; }

  bool isAdd() { return op_id_ == Add; }
  bool isSub() { return op_id_ == Sub; }
  bool isRSub() { return op_id_ == RSub; }
  bool isMul() { return op_id_ == Mul; }
  bool isDiv() { return op_id_ == Div; }
  bool isRem() { return op_id_ == Rem; }
  bool isAnd() { return op_id_ == And; }
  bool isOr() { return op_id_ == Or; }
  bool isAddAddr() { return op_id_ == AddAddr; }

  bool isNot() { return op_id_ == Not; }

  bool isCmp() { return op_id_ == Cmp; }

  bool isCall() { return op_id_ == Call; }
  bool isGEP() { return op_id_ == GEP; }
  bool isZExt() { return op_id_ == ZExt; }
  bool isMulAdd() { return op_id_ == MulAdd; }

  bool isShl() { return op_id_ == Shl; }
  bool isAShr() { return op_id_ == AShr; }
  bool isLShr() { return op_id_ == LShr; }

  bool isMtstart() { return op_id_ == MTSTART; }
  bool isMtend() { return op_id_ == MTEND; }

  bool isVV() { return op_id_ == VV; }
  bool isBic() { return op_id_ == BIC; }

  bool isBinary() {
    return (isAdd() || isSub() || isRSub() || isMul() || isDiv() || isRem() ||
            isAnd() || isOr() || isShl() || isAShr() || isLShr()) &&
           (getNumOperand() == 2);
  }

  bool isTerminator() { return isBr() || isRet(); }

  std::string CommentPrint();
  virtual void print() override;

  virtual Instruction *copy(BasicBlock *new_bb) {
    return new Instruction(getType(), op_id_, num_ops_, new_bb);
  }
  // TODO 待确认
  void setOpID(OpID id) { op_id_ = id; }

private:
  BasicBlock *parent_;
  OpID op_id_;
  unsigned num_ops_;
};

class UnaryInst : public Instruction {
private:
  UnaryInst(Type *ty, OpID id, Value *v, BasicBlock *bb);

  UnaryInst(Type *ty, OpID id, BasicBlock *bb) : Instruction(ty, id, 1, bb) {}

public:
  // create unary operator, auto insert to bb
  // static UnaryOperator *Create(OpID id, Value *v, BasicBlock *bb);

  // create neg instruction, auto insert to bb
  static UnaryInst *createNeg(Value *v, BasicBlock *bb);

  // create not instruction, auto insert to bb
  static UnaryInst *createNot(Value *v, BasicBlock *bb);

  Instruction *copy(BasicBlock *new_bb) override {
    return new UnaryInst(getType(), getInstrType(), new_bb);
  }
};

class BinaryInst : public Instruction {
private:
  BinaryInst(Type *ty, OpID id, Value *v1, Value *v2);
  BinaryInst(Type *ty, OpID id, Value *v1, Value *v2, BasicBlock *bb);
  BinaryInst(Type *ty, OpID id, BasicBlock *bb) : Instruction(ty, id, 2, bb) {}

public:
  // create add instruction, auto insert to bb
  static BinaryInst *createAdd(Value *v1, Value *v2, BasicBlock *bb);
  // create mul instruction, don't insert to bb
  static BinaryInst *createAdd(Value *v1, Value *v2) {
    // only v1 can be pointer type
    if (v2->getType()->isPointerTy())
      exit(_GenPhi_Mem2RegPass);
    return new BinaryInst(v1->getType()->isPointerTy() ? v1->getType()
                                                       : v2->getType(),
                          Instruction::Add, v1, v2);
  }

  // create sub instruction, auto insert to bb
  static BinaryInst *createSub(Value *v1, Value *v2, BasicBlock *bb);
  // create sub instruction, don't insert to bb
  static BinaryInst *createSub(Value *v1, Value *v2) {
    return new BinaryInst(v1->getType(), Instruction::Sub, v1, v2);
  }

  // create reverse sub instruction, auto insert to bb
  static BinaryInst *createRSub(Value *v1, Value *v2, BasicBlock *bb);

  // create mul instruction, auto insert to bb
  static BinaryInst *createMul(Value *v1, Value *v2, BasicBlock *bb);
  // create mul instruction, don't insert to bb
  static BinaryInst *createMul(Value *v1, Value *v2) {
    return new BinaryInst(v1->getType(), Instruction::Mul, v1, v2);
  }

  // create Div instruction, auto insert to bb
  static BinaryInst *createDiv(Value *v1, Value *v2, BasicBlock *bb);
  // create div instruction, don't insert to bb
  static BinaryInst *createDiv(Value *v1, Value *v2) {
    return new BinaryInst(v1->getType(), Instruction::Div, v1, v2);
  }

  // create Rem instruction, auto insert to bb
  static BinaryInst *createRem(Value *v1, Value *v2, BasicBlock *bb);

  // create And instruction, auto insert to bb
  static BinaryInst *createAnd(Value *v1, Value *v2, BasicBlock *bb);
  // create And instruction, don't insert to bb
  static BinaryInst *createAnd(Value *v1, Value *v2) {
    return new BinaryInst(v1->getType(), Instruction::And, v1, v2);
  }

  // create Or instruction, auto insert to bb
  static BinaryInst *createOr(Value *v1, Value *v2, BasicBlock *bb);

  // create Shl instruction, auto insert to bb
  static BinaryInst *createShl(Value *v1, Value *v2, BasicBlock *bb);
  // create Shl instruction, don't insert to bb
  static BinaryInst *createShl(Value *v1, Value *v2) {
    return new BinaryInst(v1->getType(), Instruction::Shl, v1, v2);
  }

  // create AShr instruction, auto insert to bb
  static BinaryInst *createAShr(Value *v1, Value *v2, BasicBlock *bb);
  // create AShl instruction, don't insert to bb
  static BinaryInst *createAShr(Value *v1, Value *v2) {
    return new BinaryInst(v1->getType(), Instruction::AShr, v1, v2);
  }

  // create LShr instruction, auto insert to bb
  static BinaryInst *createLShr(Value *v1, Value *v2, BasicBlock *bb);

  Instruction *copy(BasicBlock *new_bb) override {
    return new BinaryInst(getType(), getInstrType(), new_bb);
  }

private:
  void assertValid();
};

class CmpInst : public Instruction {
public:
  enum CmpOp {
    EQ, // ==
    NE, // !=
    GT, // >
    GE, // >=
    LT, // <
    LE  // <=
  };

private:
  CmpInst(Type *ty, CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);
  CmpInst(Type *ty, CmpOp op, BasicBlock *bb)
      : Instruction(ty, Instruction::Cmp, 2, bb), cmp_op_(op) {}

public:
  static CmpInst *createCmp(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb,
                            Module *m);

  CmpOp getCmpOp() { return cmp_op_; }
  void setCmpOp(CmpOp op) { cmp_op_ = op; }

  Instruction *copy(BasicBlock *new_bb) override {
    return new CmpInst(getType(), cmp_op_, new_bb);
  }

private:
  CmpOp cmp_op_;
  void assertValid();
};

class CallInst : public Instruction {
protected:
  CallInst(Function *func, std::vector<Value *> args, BasicBlock *bb);

  CallInst(Function *func, BasicBlock *bb);

public:
  static CallInst *create(Function *func, std::vector<Value *> args,
                          BasicBlock *bb);
  FunctionType *getFunctionType() const;
  Function *getFunction() const;

  Instruction *copy(BasicBlock *new_bb) override {
    return new CallInst(getFunction(), new_bb);
  }
};

class BranchInst : public Instruction {
public:
  using CmpOp = CmpInst::CmpOp;

private:
  BranchInst(Value *cond, BasicBlock *if_true, BasicBlock *if_false,
             BasicBlock *bb);
  BranchInst(BasicBlock *if_true, BasicBlock *bb);
  BranchInst(Type *ty, unsigned num_ops, BasicBlock *bb)
      : Instruction(ty, Instruction::Br, num_ops, bb) {}
  BranchInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true,
             BasicBlock *if_false, BasicBlock *bb);

  CmpOp cmp_op_;

public:
  static BranchInst *createCondBr(Value *cond, BasicBlock *if_true,
                                  BasicBlock *if_false, BasicBlock *bb);
  static BranchInst *createBr(BasicBlock *if_true, BasicBlock *bb);
  static BranchInst *createCmpBr(CmpOp op, Value *lhs, Value *rhs,
                                 BasicBlock *if_true, BasicBlock *if_false,
                                 BasicBlock *bb);

  bool isCondBr() const;
  // lhs, rhs, if_true, if_false
  bool isCmpBr() const { return getNumOperand() == 4; }
  CmpOp getCmpOp() const { return cmp_op_; }
  void fuseCmpInst();
  Instruction *copy(BasicBlock *new_bb) override {
    return new BranchInst(getType(), getNumOperand(), new_bb);
  }
};

class ReturnInst : public Instruction {
private:
  ReturnInst(Value *val, BasicBlock *bb);
  ReturnInst(BasicBlock *bb);
  ReturnInst(Type *ty, unsigned num_ops, BasicBlock *bb)
      : Instruction(ty, Instruction::Ret, num_ops, bb) {}

public:
  static ReturnInst *createRet(Value *val, BasicBlock *bb);
  static ReturnInst *createVoidRet(BasicBlock *bb);
  bool isVoidRet() const;

  Instruction *copy(BasicBlock *new_bb) override {
    return new ReturnInst(getType(), getNumOperand(), new_bb);
  }
};

class GetElementPtrInst : public Instruction {
private:
  GetElementPtrInst(Value *ptr, std::vector<Value *> idxs, BasicBlock *bb);
  GetElementPtrInst(Value *ptr, std::vector<Value *> idxs);
  GetElementPtrInst(Type *ty, unsigned num_ops, BasicBlock *bb, Type *elem_ty)
      : Instruction(ty, Instruction::GEP, num_ops, bb), element_ty_(elem_ty) {}

public:
  static Type *getElementType(Value *ptr, std::vector<Value *> idxs);
  static GetElementPtrInst *createGEP(Value *ptr, std::vector<Value *> idxs,
                                      BasicBlock *bb);
  static GetElementPtrInst *createGEP(Value *ptr, std::vector<Value *> idxs);
  Type *getElementType() const;

  Instruction *copy(BasicBlock *new_bb) override {
    return new GetElementPtrInst(getType(), getNumOperand(), new_bb,
                                 element_ty_);
  }

private:
  Type *element_ty_;
};

class StoreInst : public Instruction {
private:
  StoreInst(Value *val, Value *ptr, BasicBlock *bb);
  StoreInst(Module *m, Value *val, Value *ptr);
  StoreInst(Module *m, Value *val, Value *ptr, Value *offset);

  StoreInst(Type *ty, unsigned num_ops, BasicBlock *bb)
      : Instruction(ty, Instruction::Store, num_ops, bb) {}

public:
  static StoreInst *createStore(Value *val, Value *ptr, BasicBlock *bb);
  static StoreInst *createStore(Module *m, Value *val, Value *ptr);
  static StoreInst *createStore(Module *m, Value *val, Value *ptr,
                                Value *offset);
  // std::map<Value *, std::set<StoreInst *>> &getInstrIn() { return instr_in_;
  // } std::map<Value *, std::set<StoreInst *>> &getInstrOut() { return
  // instr_out_; }

  bool hasOffset() { return getNumOperand() >= 3; }
  bool hasShift() { return getNumOperand() >= 4; }

  void setRVal(Value *v) { setOperand(0, v); }
  void setLVal(Value *v) { setOperand(1, v); }
  void setOffset(Value *v) { setOperand(2, v); }
  void setShift(Value *v) { setOperand(3, v); }

  Value *getRVal() { return this->getOperand(0); }
  Value *getLVal() { return this->getOperand(1); }
  Value *getOffset() { return this->getOperand(2); }
  Value *getShift() { return this->getOperand(3); }

  Instruction *copy(BasicBlock *new_bb) override {
    return new StoreInst(getType(), getNumOperand(), new_bb);
  }
};

class LoadInst : public Instruction {
private:
  LoadInst(Type *ty, Value *ptr, BasicBlock *bb);
  LoadInst(Type *ty, unsigned num_ops, BasicBlock *bb)
      : Instruction(ty, Instruction::Load, num_ops, bb) {}
  LoadInst(Type *ty, Value *ptr);
  LoadInst(Type *ty, Value *ptr, Value *offset);

public:
  static LoadInst *createLoad(Type *ty, Value *ptr, BasicBlock *bb);
  static LoadInst *createLoad(Type *ty, Value *ptr) {
    return new LoadInst(ty, ptr);
  }
  static LoadInst *createLoad(Type *ty, Value *ptr, Value *offset) {
    return new LoadInst(ty, ptr, offset);
  }

  bool hasOffset() { return getNumOperand() >= 2; }
  bool hasShift() { return getNumOperand() >= 3; }
  Type *getLoadType() const;

  void setLVal(Value *v) { setOperand(0, v); }
  void setOffset(Value *v) { setOperand(1, v); }
  void setShift(Value *v) { setOperand(2, v); }

  Value *getLVal() { return this->getOperand(0); }
  Value *getOffset() { return this->getOperand(1); }
  Value *getShift() { return this->getOperand(2); }

  Instruction *copy(BasicBlock *new_bb) override {
    return new LoadInst(getType(), getNumOperand(), new_bb);
  }
};

class AllocaInst : public Instruction {
private:
  AllocaInst(Type *ty, BasicBlock *bb);
  AllocaInst(Type *ty)
      : Instruction(PointerType::get(ty), Instruction::Alloca, 0),
        alloca_ty_(ty) {}
  AllocaInst(Type *ty, unsigned num_ops, BasicBlock *bb, Type *alloca_ty)
      : Instruction(ty, Instruction::Alloca, num_ops, bb),
        alloca_ty_(alloca_ty) {}

public:
  static AllocaInst *createAlloca(Type *ty, BasicBlock *bb);
  static AllocaInst *createAlloca(Type *ty) { return new AllocaInst(ty); }

  Type *getAllocaType() const;
  void setInit() { init_ = true; }
  bool getInit() { return init_; }

  Instruction *copy(BasicBlock *new_bb) override {
    return new AllocaInst(getType(), getNumOperand(), new_bb, alloca_ty_);
  }

private:
  Type *alloca_ty_;
  bool init_ = false;
};

class CastInst : public Instruction {
private:
  CastInst(OpID op, Value *val, Type *ty, BasicBlock *bb);
  CastInst(Type *ty, OpID op, unsigned num_ops, BasicBlock *bb, Type *dest_ty)
      : Instruction(ty, op, num_ops, bb), dest_ty_(dest_ty) {}

public:
  static CastInst *createZExt(Value *val, Type *ty, BasicBlock *bb);

  Type *getDestType() const;

  Instruction *copy(BasicBlock *new_bb) override {
    return new CastInst(getType(), getInstrType(), getNumOperand(), new_bb,
                        dest_ty_);
  }

private:
  Type *dest_ty_;
};

class PhiInst : public Instruction {
private:
  PhiInst(OpID op, std::vector<Value *> vals, std::vector<BasicBlock *> val_bbs,
          Type *ty, BasicBlock *bb);
  PhiInst(Type *ty, OpID op, unsigned num_ops, BasicBlock *bb)
      : Instruction(ty, op, num_ops, bb) {}
  Value *l_val_;

public:
  static PhiInst *createPhi(Type *ty, BasicBlock *bb);
  Value *getLVal() { return l_val_; }
  void setLVal(Value *l_val) { l_val_ = l_val; }
  void setParams(Value *val, Value *pre_bb) {
    this->addOperand(val);
    this->addOperand(pre_bb);
  }

  Instruction *copy(BasicBlock *new_bb) override {
    return new PhiInst(getType(), getInstrType(), getNumOperand(), new_bb);
  }
  // private:
  //     std::vector<Value *> vals;
  //     std::vector<BasicBlock *> val_bbs;
};

class MulAddInst : public Instruction {
private:
  // v1 * v2 + v3
  MulAddInst(Value *v1, Value *v2, Value *v3);
  MulAddInst(Value *v1, Value *v2, Value *v3, BasicBlock *bb);
  MulAddInst(Type *ty, OpID id, BasicBlock *bb) : Instruction(ty, id, 3, bb) {}

public:
  static MulAddInst *createMulAddInst(Value *v1, Value *v2, Value *v3);
  static MulAddInst *createMulAddInst(Value *v1, Value *v2, Value *v3,
                                      BasicBlock *bb);

  Instruction *copy(BasicBlock *new_bb) override {
    return new MulAddInst(getType(), getInstrType(), new_bb);
  }
};

class VVInst : public Instruction {
private:
  VVInst(Value *v1, Value *v2, Value *v3, BasicBlock *parent);
  VVInst(unsigned int32_num, Value *v1, Value *v2, BasicBlock *parent);
  VVInst(unsigned int32_num, Value *v1, Value *v2);
  VVInst(unsigned int32_num, Type *ty, OpID op, unsigned num_ops,
         BasicBlock *bb)
      : Instruction(ty, op, num_ops, bb), int32_num_(int32_num) {}
  unsigned int32_num_;

public:
  static VVInst *createVV(unsigned int32_num, Value *v1, Value *v2) {
    return new VVInst(int32_num, v1, v2);
  }
  static VVInst *createVV(unsigned int32_num, Value *v1, Value *v2,
                          BasicBlock *parent) {
    return new VVInst(int32_num, v1, v2, parent);
  }
  static VVInst *createVV(Value *v1, Value *v2, Value *v3, BasicBlock *parent) {
    return new VVInst(v1, v2, v3, parent);
  }

  unsigned getNumInt32() const { return int32_num_; }

  Instruction *copy(BasicBlock *new_bb) override {
    return new VVInst(getNumInt32(), getType(), getInstrType(), getNumOperand(),
                      new_bb);
  }
};

class BicInst : public Instruction {
private:
  BicInst(Value *v1, Value *v2, Value *v3, BasicBlock *parent);

public:
  static BicInst *createBic(Value *v1, Value *v2, Value *v3,
                            BasicBlock *parent) {
    return new BicInst(v1, v2, v3, parent);
  }
};

class Mtstart : public Instruction {
  // %thread_id = MTSTART %num_threads
private:
  Mtstart(Module *m);

public:
  Mtstart *createMtstart(Module *m);
};

class Mtend : public Instruction {
  // %thread_id = MTSTART %num_threads
private:
  Mtend(Module *m, Mtstart *start);

public:
  Mtend *createMtend(Module *m, Mtstart *start);
};

class HighIR : public Instruction {
private:
  HighIR(Type *ty, OpID id, unsigned num_ops, BasicBlock *parent)
      : Instruction(ty, id, num_ops, parent) {}
  HighIR(Type *ty, OpID id, unsigned num_ops) : Instruction(ty, id, num_ops) {}

public:
  static HighIR *createBreak(Module *m) {
    return new HighIR(Type::getVoidTy(m), Instruction::Break, 0);
  }
  static HighIR *createBreak(Module *m, BasicBlock *parent) {
    return new HighIR(Type::getVoidTy(m), Instruction::Break, 0, parent);
  }

  static HighIR *createContinue(Module *m) {
    return new HighIR(Type::getVoidTy(m), Instruction::Continue, 0);
  }
  static HighIR *createContinue(Module *m, BasicBlock *parent) {
    return new HighIR(Type::getVoidTy(m), Instruction::Continue, 0, parent);
  }
};

#endif // SYSYC_INSTRUCTION_H
