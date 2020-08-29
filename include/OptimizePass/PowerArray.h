#include "BasicBlock.h"
#include "ConstFlod.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.h"

class PowerArray : public Transform {
private:
  BasicBlock *bb_;

public:
  PowerArray(Module *m) : Transform(m) {}
  ~PowerArray() {}
  void run() override;
  Value *getPowerArray();
  void replaceWithShl(Value *);

  Value *getGepPtr(Instruction *instr);
  Value *getGepOffset(Instruction *instr);
};