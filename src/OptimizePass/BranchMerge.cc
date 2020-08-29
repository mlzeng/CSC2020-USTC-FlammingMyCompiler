#include "BranchMerge.h"

ExpVal *AriExp::computeExpVal() {
  if (isBoolOpID()) {
    return new ExpVal(0, 1);
  } else if (op_ == REM && getRopExp()->isConst()) {
    ExpVal *ret = new ExpVal();
    ret->setLowLimit(0);
    ret->setHighLimit(getRopExp()->getHighLimit() - 1);
    return ret;
  } else {
    return new ExpVal();
  }

  return nullptr;
}

AriExp::AriExp(AriExp *exp1, AriExp *exp2, OpID op) {
  op_ = op;
  ari_exp1_ = exp1;
  ari_exp2_ = exp2;
  val_ = computeExpVal();
}

void BranchMerge::simplifyAlgebra(AriExp *exp) {}

AriExp *BranchMerge::getCondInst(BranchInst *br) {
  AriExp *ret = new AriExp();
  if (br->getNumOperand() != 3) {
    std::cerr << "error getCondInst" << std::endl;
    abort();
  } else {
    ret = getAriExp(br->getOperand(0));
  }
  return ret;
}

AriExp *BranchMerge::getAriExp(Value *op_val) {
  auto const_int = dynamic_cast<ConstantInt *>(op_val);
  auto instr = dynamic_cast<Instruction *>(op_val);
  if (const_int) {
    return new AriExp(const_int->getValue());
  } else if (instr) {
    if (instr->isAdd() || instr->isSub() || instr->isMul() || instr->isDiv() ||
        instr->isRem() || instr->isAnd() || instr->isOr() || instr->isCmp()) {
      AriExp::OpID ari_op;
      switch (instr->getInstrType()) {
      case Instruction::Add:
        ari_op = AriExp::ADD;
        break;
      case Instruction::Sub:
        ari_op = AriExp::SUB;
        break;
      case Instruction::Mul:
        ari_op = AriExp::MUL;
        break;
      case Instruction::Div:
        ari_op = AriExp::DIV;
        break;
      case Instruction::Rem:
        ari_op = AriExp::REM;
        break;
      case Instruction::And:
        ari_op = AriExp::AND;
        break;
      case Instruction::Or:
        ari_op = AriExp::OR;
        break;
      case Instruction::Cmp:
        switch (static_cast<CmpInst *>(instr)->getCmpOp()) {
        case CmpInst::EQ:
          ari_op = AriExp::EQ;
          break;
        case CmpInst::NE:
          ari_op = AriExp::NE;
          break;
        case CmpInst::GT:
          ari_op = AriExp::GT;
          break;
        case CmpInst::GE:
          ari_op = AriExp::GE;
          break;
        case CmpInst::LT:
          ari_op = AriExp::LT;
          break;
        case CmpInst::LE:
          ari_op = AriExp::LE;
          break;
        default:
          std::cerr << "error in getAriExp" << std::endl;
          abort();
          break;
        }
        break;
      default:
        std::cerr << "error in getAriExp" << std::endl;
        abort();
        break;
      }
      auto op1 = getAriExp(instr->getOperand(0));
      auto op2 = getAriExp(instr->getOperand(1));
      if (op1 && op2) {
        return new AriExp(op1, op2, ari_op);
      } else {
        std::cerr << "error in getAriExp" << std::endl;
        abort();
      }
      return nullptr;
    } else {
      return new AriExp(); // load call argue
    }
  }
}