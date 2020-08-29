#ifndef SYSYC_DEADCODEELIMATE_H
#define SYSYC_DEADCODEELIMATE_H

#include "PassManager.h"

class DeadCodeEliminate : public Transform {
public:
  DeadCodeEliminate(Module *m) : Transform(m) {}
  void run() override;

  bool isSideEffect(Instruction *inst);
  bool isSideEffectAndCall(Instruction *inst);
  bool isLocalStore(StoreInst *store);
  bool isDeadInstruction(Instruction *inst);
  bool isEqualStoreLoadPtr(StoreInst *store, LoadInst *load);
  bool isStrictEqualStoreLoadPtr(StoreInst *store, LoadInst *load);
  bool isEqualStorePtr(StoreInst *store_a, StoreInst *store_b);
  bool isSideEffectFunc(Function *func) {
    return notSideEffectFunc.find(func) == notSideEffectFunc.end();
  }
  Value *getFirstAddr(Value *v);

  void detectNotSideEffectFunc(Module *m);

  void deleteDeadFunc(Module *m);
  void deleteDeadInst(Function *func);
  void deleteDeadStore(Function *func);
  void deleteDeadRet(Function *func);

  void markUse(Instruction *inst, std::unordered_set<Instruction *> &worklist);

  void printStats();

private:
  int inst_counter = 0;
  int store_counter = 0;
  int func_counter = 0;

  std::unordered_set<Function *> notSideEffectFunc;
};

#endif // SYSYC_DEADCODEELIMATE_H
