#ifndef SYSYC_GLOBALVARIABLELOCAL_H
#define SYSYC_GLOBALVARIABLELOCAL_H

#include "PassManager.h"
#include <map>
#include <set>

class GlobalVariableLocal : public Transform {
public:
  GlobalVariableLocal(Module *m) : Transform(m) {}
  void run() override;
  void localGlobalVariable(GlobalVariable *global_var, Function *func);
  bool genArrayInitStores(Instruction *ptr, Constant *init_var);
  void genGEPLocal(GlobalVariable *global_var, Function *func);
  void createFuncSucc();
  void detectRecursive();
  void visitFunc(Function *entry, std::set<Function *> &visited);

private:
  std::list<Instruction *> init_instr_;
  BasicBlock *insert_bb_;
  std::map<Function *, std::set<Function *>> func_succ_;
  std::set<Function *> recursive_func_;
};

#endif // SYSYC_GLOBALVARIABLELOCAL_H