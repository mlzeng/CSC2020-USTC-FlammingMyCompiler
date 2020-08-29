#include "RefactorPartIns.h"
#include "ReturnVal.h"

void RefactorPartIns::run() {
  for (auto func : m_->getFunctions()) {
    if (func->getBasicBlocks().size() == 0)
      continue;
    for (auto BB : func->getBasicBlocks()) {
      for (auto ins : BB->getInstructions()) {
        refactor(ins);
      }
    }
  }
}

void RefactorPartIns::refactor(Instruction *ins) {
  auto &v = ins->getOperands();
  if (v.size() == 0 || !dynamic_cast<ConstantInt *>(v[0]))
    return;
  Value *tmp;
  switch (ins->getInstrType()) {
  case Instruction::OpID::PHI: {
    for (unsigned r = ins->getNumOperand() - 2, l = 0; l < r;) {
      if (dynamic_cast<ConstantInt *>(v[l]) == nullptr) {
        l += 2;
        continue;
      }
      if (dynamic_cast<ConstantInt *>(v[r]) != nullptr) {
        r -= 2;
        continue;
      }
      v[r]->removeUse(ins, r);
      v[r + 1]->removeUse(ins, r + 1);
      v[l]->removeUse(ins, l);
      v[l + 1]->removeUse(ins, l + 1);
      tmp = v[l];
      ins->setOperand(l, v[r]);
      ins->setOperand(r, tmp);
      tmp = v[l + 1];
      ins->setOperand(l + 1, v[r + 1]);
      ins->setOperand(r + 1, tmp);
      l += 2;
      r -= 2;
    }
    break;
  }
  case Instruction::OpID::Cmp: {
    exit_ifnot(_refactor_RefactorPartIns, ins->getNumOperand() == 2);
    tmp = v[0];
    ins->removeUseOfOps();
    ins->setOperand(0, v[1]);
    ins->setOperand(1, tmp);
    auto ins_cmp = dynamic_cast<CmpInst *>(ins);
    switch (ins_cmp->getCmpOp()) {
    case CmpInst::CmpOp::GE:
      ins_cmp->setCmpOp(CmpInst::CmpOp::LE);
      break;
    case CmpInst::CmpOp::GT:
      ins_cmp->setCmpOp(CmpInst::CmpOp::LT);
      break;
    case CmpInst::CmpOp::LE:
      ins_cmp->setCmpOp(CmpInst::CmpOp::GE);
      break;
    case CmpInst::CmpOp::LT:
      ins_cmp->setCmpOp(CmpInst::CmpOp::GT);
      break;
    default:
      break;
    }
    break;
  }
  case Instruction::OpID::Sub:
    tmp = v[0];
    ins->removeUseOfOps();
    ins->setOperand(0, v[1]);
    ins->setOperand(1, tmp);
    ins->setOpID(Instruction::OpID::RSub);
    break;
  case Instruction::OpID::Mul:
  case Instruction::OpID::Or:
  case Instruction::OpID::Add:
  case Instruction::OpID::And:
    tmp = v[0];
    ins->removeUseOfOps();
    ins->setOperand(0, v[1]);
    ins->setOperand(1, tmp);
    break;
  default:
    return;
  }
}