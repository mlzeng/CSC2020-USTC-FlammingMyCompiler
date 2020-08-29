#include "BasicBlock.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.h"

class SparseCondConstPropagation : Transform {
private:
public:
  SparseCondConstPropagation(Module *m) : Transform(m) {}
  ~SparseCondConstPropagation() {}

  void run() override;
  void VisitPhi(PhiInst *phi);
  void VisitInst(Instruction *instr);
};
