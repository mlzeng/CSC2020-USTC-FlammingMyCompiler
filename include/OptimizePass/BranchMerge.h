#include "BasicBlock.h"
#include "ConstFlod.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "LoopFind.h"
#include "Module.h"
#include "PassManager.h"
#include "syntax_tree.hh"

// class AriExp;

class ExpVal {
private:
  int low_limit_;
  int high_limit_;
  bool free_int_;

public:
  ExpVal() { free_int_ = true; }
  ExpVal(int const_val) {
    low_limit_ = const_val;
    high_limit_ = const_val;
    free_int_ = false;
  }
  ExpVal(int low, int high) {
    low_limit_ = low;
    high_limit_ = high;
    free_int_ = false;
  }

  void setLowLimit(int val) { low_limit_ = val; }
  void setHighLimit(int val) { high_limit_ = val; }

  int getLowLimit() { return low_limit_; }
  int getHighLimit() { return high_limit_; }

  bool isConst() { return low_limit_ == high_limit_; }
  bool isFreeInt() { return free_int_; }
  bool isBool() { return low_limit_ == 0 && high_limit_ == 1; }
};

class AriExp {
private:
  AriExp *ari_exp1_;
  AriExp *ari_exp2_;

  ExpVal *val_;

public:
  ExpVal *computeExpVal();

  AriExp() {
    ari_exp1_ = nullptr;
    ari_exp2_ = nullptr;
    val_ = new ExpVal();
  }

  AriExp(int const_val) {
    ari_exp1_ = nullptr;
    ari_exp2_ = nullptr;
    val_ = new ExpVal(const_val);
  }

  enum OpID { ADD, SUB, MUL, DIV, REM, AND, OR, EQ, NE, GT, GE, LT, LE };

  AriExp(AriExp *exp1, AriExp *exp2, OpID op);

  void setLopExp(AriExp *exp) { ari_exp1_ = exp; }
  void setRopExp(AriExp *exp) { ari_exp2_ = exp; }

  AriExp *getLopExp() { return ari_exp1_; }
  AriExp *getRopExp() { return ari_exp2_; }

  int getLowLimit() { return val_->getLowLimit(); }
  int getHighLimit() { return val_->getHighLimit(); }

  bool isBoolOpID() {
    return op_ == EQ || op_ == NE || op_ == GT || op_ == GE || op_ == LE ||
           op_ == LT;
  }
  bool isConst() { return val_->isConst(); }
  bool isFreeInt() { return val_->isFreeInt(); }
  bool isBool() { return val_->isFreeInt(); }

private:
  OpID op_;
};

class BranchMerge : public Transform {
private:
public:
  BranchMerge(Module *m, SyntaxCompUnit *root) : Transform(m) {}
  ~BranchMerge() {}
  void run() override;
  void MergeCond() {}
  void simplifyAlgebra(AriExp *exp);
  AriExp *getCondInst(BranchInst *br);
  AriExp *getAriExp(Value *op_val);
};