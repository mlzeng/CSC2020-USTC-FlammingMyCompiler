#ifndef __REFACTORPARTINS_H__
#define __REFACTORPARTINS_H__

#include "PassManager.h"

class RefactorPartIns : public Pass {
  void refactor(Instruction *);

public:
  RefactorPartIns(Module *m) : Pass(m) {}
  void run() override;
};

#endif
