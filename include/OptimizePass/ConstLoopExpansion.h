#ifndef __CONSTLOOPEXPANSION_H__
#define __CONSTLOOPEXPANSION_H__

#include "LoopFind.h"
#include "PassManager.h"

class ConstLoopExpansion : public Pass {
  void tryExpand(BasicBlock *, BasicBlock *);

public:
  ConstLoopExpansion(Module *m) : Pass(m) {}
  void run() override;
};

#endif