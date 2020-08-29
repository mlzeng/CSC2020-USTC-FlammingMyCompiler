#ifndef __ACCUMULATEPATTERN_H__
#define __ACCUMULATEPATTERN_H__

#include "BBExps.h"
#include "PassManager.h"
#include <memory>

class AccumulatePattern : public Pass {
  using exp_t = std::tuple<Value *, Instruction *, Exp_hir>;
  std::unique_ptr<BBExpsAnalysis_hir> BB2exps;
  WhileBlock *curWhile;
  Value *curStartVal;
  Value *curEndVal;
  Value *curIter;
  Value *AccuAddr = nullptr;
  std::unordered_set<Value *> curVar;
  std::vector<BasicBlock *> _BasicBBs;
  bool nestedLoop(IfBlock *BB);
  bool isSimpleIterPattern();
  bool hasSimpleAccuPattern();
  bool isInvariant(Value *);
  bool isSameAddr(Value *, Value *);
  bool isSameOffsetPattern(Value *val, Value *val2);
  bool isAssignPlus(exp_t &);
  bool hasNot(Exp_hir &, Value *);
  void moveAccuOut();
  BasicBlock *getNewPreBasicBlock(BaseBlock *);
  BasicBlock *getNewPostBasicBlock(BaseBlock *);
  std::vector<Instruction *> getRelatedInsInOrder(Value *);

public:
  AccumulatePattern(Module *m) : Pass(m) {}
  void run() override;
};

#endif
