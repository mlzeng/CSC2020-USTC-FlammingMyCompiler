#ifndef __LOOPMERGE_H__
#define __LOOPMERGE_H__

#include "BBExps.h"
#include "PassManager.h"
#include <memory>

class LoopMerge : public Pass {
  // std::unique_ptr<LoopPattern> patternFinder;
  std::unique_ptr<BBExpsAnalysis_hir> BB2exps;
  WhileBlock *lastWhile = nullptr;
  Value *lastStartVal;
  Value *lastEndVal;
  Value *lastIter;
  std::unordered_set<Value *> lastVar;
  WhileBlock *curWhile;
  Value *curStartVal;
  Value *curEndVal;
  Value *curIter;
  std::unordered_set<Value *> curVar;
  bool mergeAble();
  void merge();
  bool isSimpleAccuPat();
  bool allAccu(BasicBlock *);
  bool isSameAddr(Value *, Value *);
  bool isInvariant(Value *val, BaseBlock *block);
  bool checkSetCurStart(BasicBlock *BB);
  bool isSameAccuPattern();
  bool isSameOffsetPattern(Value *val, Value *val2);
  bool hasNot(Exp_hir &, Value *);
  void clearFormerInformation(Instruction *);
  void insertAsNewBefore(Instruction *, std::_List_iterator<Instruction *>,
                         std::list<Instruction *> &);
  void resetLastPat() { lastWhile = nullptr; }
  void setLastPatternFromCur() {
    lastWhile = curWhile;
    lastIter = curIter;
    lastStartVal = curStartVal;
    lastEndVal = curEndVal;
    lastVar = std::move(curVar);
  }
  std::list<BaseBlock *> &getList(BaseBlock *);
  std::vector<Instruction *> getRelatedIns(Instruction *);

public:
  LoopMerge(Module *m) : Pass(m) {}
  void run() override;
};

#endif