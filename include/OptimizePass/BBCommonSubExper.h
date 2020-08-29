#include "BasicBlock.h"
#include "ConstFlod.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.h"

class Arglist {
public:
  std::vector<Value *> args_;
  const bool operator<(const Arglist &a) const;
  const bool operator>(const Arglist &a) const;
  const bool operator==(const Arglist &a) const;
};

class BBCommonSubExper : public Transform {
private:
  BasicBlock *bb_;
  std::set<std::string> eliminatable_call_;

public:
  BBCommonSubExper(Module *m) : Transform(m) {}
  ~BBCommonSubExper() {}
  void run() override;
  void CommonSubExperElimination();
  Value *getBinaryLop(Instruction *instr);
  Value *getBinaryRop(Instruction *instr);
  Value *getLoadPtr(Instruction *instr);
  Value *getLoadOffset(Instruction *instr);
  int getLoadOffsetShl(Instruction *instr);

  Value *getGepPtr(Instruction *instr);
  Value *getGepOffset(Instruction *instr);

  Arglist *getCallArgs(CallInst *instr);
  std::string getFuncID(CallInst *instr);
  bool eliminatableCall(std::string func_id);
  void getEliminatableCall();
};