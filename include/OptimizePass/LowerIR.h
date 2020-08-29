#ifndef SYSYC_LOWERIR_H
#define SYSYC_LOWERIR_H

#include "PassManager.h"

class LowerIR : public Transform {
public:
  LowerIR(Module *m) : Transform(m) {}
  void run() override;
  void fuseCmpBr(BasicBlock *bb);
  void fuseMulAdd(BasicBlock *bb);
  void splitGEP(BasicBlock *bb);
  void convertMulDivToShift(BasicBlock *bb);
  void convertRemToAnd(BasicBlock *bb);
  void fuseConstShift(BasicBlock *bb);
  void fuseAddLoadStore(BasicBlock *bb);
  // addr + offset < shift, 1 <= shift <= 31
  void fuseShiftLoadStore(BasicBlock *bb);
  void removeUnusedOp(BasicBlock *bb);
  void fuseShiftArithmetic(BasicBlock *bb);
  void splitRem(BasicBlock *bb);
};

#endif // SYSYC_LOWERIR_H
