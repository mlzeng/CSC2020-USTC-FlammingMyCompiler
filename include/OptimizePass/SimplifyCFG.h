#include "BasicBlock.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.h"

class SimplifyCFG : public Transform {
private:
  Function *func_;
  SimplifyCFG();

public:
  SimplifyCFG(Module *m) : Transform(m) {}
  ~SimplifyCFG(){};
  void run() override;
  void RemoveNoPredecessorBB();
  void MergeSinglePredecessorBB();
  void EliminateSinglePredecessorPhi();
  void EliminateSingleUnCondBrBB();
  void RemoveSelfLoopBB();
};