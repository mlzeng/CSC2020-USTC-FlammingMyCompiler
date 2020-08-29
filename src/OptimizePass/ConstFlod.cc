#include "ConstFlod.h"
#include "ReturnVal.h"

ConstantInt *ConstFlod::compute(Instruction::OpID op, ConstantInt *v1,
                                ConstantInt *v2) {
  int lhs = v1->getValue();
  int rhs = v2->getValue();
  int ret;
  switch (op) {
  case Instruction::Add:
    ret = lhs + rhs;
    break;
  case Instruction::Sub:
    ret = lhs - rhs;
    break;
  case Instruction::Mul:
    ret = lhs * rhs;
    break;
  case Instruction::Div:
    ret = lhs / rhs;
    break;
  case Instruction::Rem:
    ret = lhs % rhs;
    break;
  case Instruction::And:
    ret = lhs & rhs;
    break;
  case Instruction::Or:
    ret = lhs | rhs;
    break;
  default:
    std::cerr << "error in const flod" << std::endl;
    exit_ifnot(_CantFindSuitableOp_compute_ConstFlod, false);
    break;
  }
  return ConstantInt::get(ret, module_);
}

ConstantInt *ConstFlod::compute(Instruction::OpID op, ConstantInt *v1) {
  int hs = v1->getValue();
  int ret;
  switch (op) {
  case Instruction::Not:
    ret = ~hs;
    break;
  case Instruction::Neg:
    ret = -hs;
    break;
  default:
    std::cerr << "error in const flod" << std::endl;
    exit_ifnot(_CantFindSuitableOp_compute_ConstFlod, false);
    break;
  }
  return ConstantInt::get(ret, module_);
}

ConstantInt *ConstFlod::compute(CmpInst::CmpOp op, ConstantInt *v1,
                                ConstantInt *v2) {
  int lhs = v1->getValue();
  int rhs = v2->getValue();
  int ret;
  switch (op) {
  case CmpInst::EQ:
    ret = lhs == rhs;
    break;
  case CmpInst::NE:
    ret = lhs != rhs;
    break;
  case CmpInst::GT:
    ret = lhs > rhs;
    break;
  case CmpInst::GE:
    ret = lhs >= rhs;
    break;
  case CmpInst::LE:
    ret = lhs <= rhs;
    break;
  case CmpInst::LT:
    ret = lhs < rhs;
    break;
  default:
    std::cerr << "error in const flod" << std::endl;
    exit_ifnot(_CantFindSuitableOp_compute_ConstFlod, false);
    break;
  }
  return ConstantInt::get(ret, module_);
}