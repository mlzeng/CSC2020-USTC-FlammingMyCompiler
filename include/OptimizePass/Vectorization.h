#ifndef SYSYC_VECTORIZATION_H
#define SYSYC_VECTORIZATION_H

#include "LoopFind.h"
#include "PassManager.h"

class Vectorization : public Transform {
public:
  const int MIN_LOOP_ITERS = 400;
  const int VECTORIZATE_INT32_NUM = 8;

public:
  Vectorization(Module *m) : Transform(m) {}
  void run() override;
  bool tryVectorizate(BasicBlock *cond, BasicBlock *body);
  void vectorizate(BasicBlock *body, Instruction *mul, Instruction *iter);

private:
  int vectorizate_loops = 0;
};

#endif // SYSYC_VECTORIZATION_H
