#include "BasicBlock.h"
#include "Function.h"
#include "HighBlock.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.h"

class MergeCond : public Transform {
private:
  Function *func_;
  enum mother_type { FUNCTION, WHILE_BODY, IF_THEN, IF_ELSE };

public:
  MergeCond(Module *m) : Transform(m) {}
  ~MergeCond() {}

  void run() override;
  void runFun();
  bool runIf(IfBlock *ifbb, mother_type ty);
  void runWhile(WhileBlock *whilebb, mother_type ty);
};