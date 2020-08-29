#include "BasicBlock.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.h"

class Exp {
private:
  Value *val_;
  Exp *lhs_;
  Exp *rhs_;
  int cost_depth_;
  int print_depth_ = 0;

public:
  enum OpID {
    Assign,
    Add,
    Sub,
    Mul,
    Div,
    Rem,
    And,
    Or,
    GT,
    GE,
    LT,
    LE,
    EQ,
    NE
  };
  OpID op_;

  Exp(Value *val) : val_(val), op_(Assign), cost_depth_(0) {}
  Exp(Exp *lhs, Exp *rhs, OpID op) : lhs_(lhs), rhs_(rhs), op_(op) {
    auto depth1 = lhs_->cost_depth_;
    auto depth2 = rhs_->cost_depth_;
    cost_depth_ = (depth1 > depth2) ? depth1 : depth2;
  }
  bool isAssign() { return op_ == Assign; }
  bool isAdd() { return op_ == Add; }
  bool isSub() { return op_ == Sub; }
  bool isMul() { return op_ == Mul; }
  bool isDiv() { return op_ == Div; }
  bool isRem() { return op_ == Rem; }
  bool isAnd() { return op_ == And; }
  bool isOr() { return op_ == Or; }
  bool isGT() { return op_ == GT; }
  bool isGE() { return op_ == GE; }
  bool isLT() { return op_ == LT; }
  bool isLE() { return op_ == LE; }
  bool isEQ() { return op_ == EQ; }
  bool isNE() { return op_ == NE; }
  OpID getOp() { return op_; }

  Exp *getLSubexp() { return lhs_; }
  Exp *getRSubexp() { return rhs_; }
  Value *getVal() { return val_; }
  int getCostDepth() { return cost_depth_; }
  static Exp *create(Value *root);
  void setPrintDepth(int depth) { print_depth_ = depth; }
  void print();
};

class CondSimplify : public Transform {
private:
  Function *func_;
  // BasicBlock *bb_;

public:
  CondSimplify(Module *m) : Transform(m) {}
  ~CondSimplify(){};
  void run() override;
  // recog cond
  void EmliteCond();
  Exp *genExp(BranchInst *br);
  Value *ExpSimplify(Exp *exp);
  std::pair<Value *, int> item_1(Exp *exp);
  std::pair<Value *, Value *> item_2(Exp *exp);
  // std::pair<Value *, int>item_3( Exp *exp );
  // change add to assign
  std::pair<Value *, Value *> Accumulate(StoreInst *str);
  bool AddrCmp(Instruction *addr1, Instruction *addr2);
  bool expCmp(Exp *exp1, Exp *exp2);
  void BranchMerge();
};
