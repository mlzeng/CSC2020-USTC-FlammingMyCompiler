#ifndef SYSYC_CONSTFLOD_H
#define SYSYC_CONSTFLOD_H

#include <vector>

#include "Constant.h"
#include "Instruction.h"
#include "Module.h"

class ConstFlod {
public:
  ConstFlod(Module *m) : module_(m) {}

  ConstantInt *compute(Instruction::OpID op, ConstantInt *v1, ConstantInt *v2);
  ConstantInt *compute(Instruction::OpID op, ConstantInt *v1); // not neg
  ConstantInt *compute(CmpInst::CmpOp op, ConstantInt *v1, ConstantInt *v2);

private:
  Module *module_;
};

#endif
