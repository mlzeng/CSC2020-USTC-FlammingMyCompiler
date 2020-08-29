#ifndef SYSY_HIRTOMIR_H
#define SYSY_HIRTOMIR_H

#include "PassManager.h"

class HIRToMIR : public Transform {
public:
  HIRToMIR(Module *m) : Transform(m) {}
  void run() override;
  BasicBlock *genBaseBlock(BaseBlock *basebb, BasicBlock *next_bb,
                           BasicBlock *while_entry, BasicBlock *while_exit,
                           Function *func);
};

#endif // SYSY_HIRTOMIR_H
