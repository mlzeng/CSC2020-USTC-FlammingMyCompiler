#include "BasicBlock.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.h"

class IRCheck : public Analysis {
public:
  IRCheck(Module *m) : Analysis(m) {}
  ~IRCheck(){};
  void run() override;

private:
  void CheckPraent();
  void CheckPhiPostion();
  void CheckRetBrPostion();
  void CheckTerminate();
  void CheckPredSucc();
  void CheckEntry();
};