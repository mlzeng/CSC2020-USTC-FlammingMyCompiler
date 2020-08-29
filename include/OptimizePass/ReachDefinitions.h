#ifndef SYSYC_REACHDEFINITIONS_H
#define SYSYC_REACHDEFINITIONS_H

#include "BasicBlock.h"
#include "Function.h"
#include "Instruction.h"
#include "Module.h"

class ReachDefinitionsPass {
private:
  Module *module_;

public:
  ReachDefinitionsPass(Module *m);
  ~ReachDefinitionsPass();

  void runModule(); // get IN and OUT
  void runFunction(Function *f);
  void runBasicBlock(BasicBlock *bb);
  void runStorePhiInst(Instruction *instr,
                       std::map<Value *, std::set<Value *>> &in);
};

#endif