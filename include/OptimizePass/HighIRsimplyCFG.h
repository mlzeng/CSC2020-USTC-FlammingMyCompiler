#include "BasicBlock.h"
#include "Function.h"
#include "HighBlock.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.h"

class HighIRsimplyCFG : public Transform {
private:
  Function *func_;
  std::vector<BaseBlock *> wait_delete_;

public:
  HighIRsimplyCFG(Module *m) : Transform(m) {}
  ~HighIRsimplyCFG() {}

  void run() override;
  void MergeSinglePredecessorBB();
  bool MergeBasicBlock(BasicBlock *bb, BaseBlock *pre_bb);
  void MergeWhileBB(WhileBlock *whilebb);
  void MergeIfBB(IfBlock *ifbb);
  // void removeAllBB();
};
