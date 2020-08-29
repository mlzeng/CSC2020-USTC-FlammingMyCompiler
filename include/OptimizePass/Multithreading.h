#include "BasicBlock.h"
#include "ConstFlod.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "LoopFind.h"
#include "Module.h"
#include "PassManager.h"
#include <memory>

class Multithreading : public Pass {
  std::unique_ptr<LoopFind> finder;
  Value *start, *end;
  Instruction *accu;
  Instruction *comparer;
  Instruction *phi;
  int accuValue = -1;
  int multithreadCount = 0;
  bool findAccumulator(BBset_t *);
  void wrapMultithreading(BasicBlock *loopBase, BBset_t *loop);

public:
  Multithreading(Module *m) : Pass(m) {}
  ~Multithreading() {}
  void run() override;
};