#include "CondSimplify.h"

// #define DEBUG
#define PRINT_DEPTH_(N)                                                        \
  { std::cout << std::string(N, ' '); }

std::map<int, std::string> op_name;
std::map<Exp *, Value *> exp2val;
std::map<Value *, Exp *> val2exp;

void CondSimplify::run() {
  op_name.insert({Exp::Assign, "Assign"});
  op_name.insert({Exp::Add, "Add"});
  op_name.insert({Exp::Sub, "Sub"});
  op_name.insert({Exp::Mul, "Mul"});
  op_name.insert({Exp::Div, "Div"});
  op_name.insert({Exp::Rem, "Rem"});
  op_name.insert({Exp::And, "And"});
  op_name.insert({Exp::Or, "Or"});
  op_name.insert({Exp::GT, "GT"});
  op_name.insert({Exp::GE, "GE"});
  op_name.insert({Exp::LT, "LT"});
  op_name.insert({Exp::LE, "LE"});
  op_name.insert({Exp::EQ, "EQ"});
  op_name.insert({Exp::NE, "NE"});
  for (auto func : m_->getFunctions()) {
    for (auto bb : func->getBasicBlocks()) {
      // bb_ = bb;
      EmliteCond();
    }
  }
}

Exp *CondSimplify::genExp(BranchInst *br) {
  if (br->getNumOperand() != 3) {
    return nullptr;
  } else {
    return Exp::create(br->getOperand(0));
  }
  return nullptr;
}

void CondSimplify::EmliteCond() { ; }

void CondSimplify::BranchMerge() {

  for (auto bb : func_->getBasicBlocks()) {
    BasicBlock *bb0, *bb1, *bb2, *bb3, *bb4;
    if (bb->getSuccBasicBlocks().size() == 2) {
      auto succ1 = bb->getSuccBasicBlocks().front();
      auto succ2 = bb->getSuccBasicBlocks().back();
      if (succ1->getSuccBasicBlocks().size() == 1 &&
          succ2->getSuccBasicBlocks().size() == 2) {
        if (succ1->getSuccBasicBlocks().front() == succ2) {
          auto succ2_succ1 = succ2->getSuccBasicBlocks().front();
          auto succ2_succ2 = succ2->getSuccBasicBlocks().back();
          if (succ2_succ1->getSuccBasicBlocks().size() == 1) {
            if (succ2_succ1->getSuccBasicBlocks().front() == succ2_succ2) {
              bb0 = bb;
              bb1 = succ1;
              bb2 = succ2;
              bb3 = succ2_succ1;
              bb4 = succ2_succ2;
            }
          } else if (succ2_succ2->getSuccBasicBlocks().size() == 1) {
            if (succ2_succ2->getSuccBasicBlocks().front() == succ2_succ1) {
              bb0 = bb;
              bb1 = succ1;
              bb2 = succ2;
              bb3 = succ2_succ2;
              bb4 = succ2_succ1;
            }
          }
        }
      } else if (succ1->getSuccBasicBlocks().size() == 2 &&
                 succ2->getSuccBasicBlocks().size() == 1) {
        if (succ2->getSuccBasicBlocks().front() == succ1) {
          auto succ1_succ1 = succ1->getSuccBasicBlocks().front();
          auto succ1_succ2 = succ1->getSuccBasicBlocks().back();
          if (succ1_succ1->getSuccBasicBlocks().size() == 1) {
            if (succ1_succ1->getSuccBasicBlocks().front() == succ1_succ2) {
              bb0 = bb;
              bb1 = succ2;
              bb2 = succ1;
              bb3 = succ1_succ1;
              bb4 = succ1_succ2;
            }
          } else if (succ1_succ2->getSuccBasicBlocks().size() == 1) {
            if (succ1_succ2->getSuccBasicBlocks().front() == succ1_succ1) {
              bb0 = bb;
              bb1 = succ2;
              bb2 = succ1;
              bb3 = succ1_succ2;
              bb4 = succ1_succ1;
            }
          }
        }
      }
    }
    if (bb0->getTerminator()->isBr()) {
      auto br0 = dynamic_cast<BranchInst *>(bb0->getTerminator());
      auto cond0_exp = genExp(br0);
      if (cond0_exp) {
        auto cond0_simp_exp = ExpSimplify(cond0_exp);
        if (bb2->getTerminator()->isBr()) {
          auto br2 = dynamic_cast<BranchInst *>(bb2->getTerminator());
          auto cond2_exp = genExp(br2);
          if (cond2_exp) {
            auto cond2_simp_exp = ExpSimplify(cond2_exp);
            for (auto instr : bb4->getInstructions()) {
              if (instr->isStore()) {
                Value *load_addr, *additem;
                auto accumulater = Accumulate(dynamic_cast<StoreInst *>(instr));
                load_addr = accumulater.first;
                additem = accumulater.second;
              }
            }
          }
        }
      }
    }
  }
}

Value *CondSimplify::ExpSimplify(Exp *exp) {
  Exp *item1, *item2, *item3;
  if (exp->isAnd()) {
    auto sub_exp1 = exp->getLSubexp();
    auto sub_exp2 = exp->getRSubexp();
    if (sub_exp1->isAnd()) {
      item1 = sub_exp2;
      item2 = sub_exp1->getLSubexp();
      item3 = sub_exp1->getRSubexp();
    } else if (sub_exp2->isAnd()) {
      item1 = sub_exp1;
      item2 = sub_exp2->getLSubexp();
      item3 = sub_exp2->getRSubexp();
    }

    auto env1 = item_2(item1);
    auto env2 = item_2(item2);
    auto env3 = item_2(item3);
    if (env1.first != nullptr && env1.second != nullptr) {
      auto eq1 = item_1(item2);
      auto eq2 = item_1(item3);
      if ((eq1.first == env1.first && eq2.first == env1.second) ||
          (eq1.first == env1.second && eq2.first == env1.second)) {
        if (eq1.second != eq2.second) {
          assert(exp2val.find(item2) != exp2val.end());
          assert(exp2val.find(item3) != exp2val.end());

          auto ret1 = exp2val[item2];
          auto ret2 = exp2val[item3];

          if (item3->getCostDepth() > item2->getCostDepth()) {
            return ret1;
          } else {
            return ret2;
          }
        }
      }
    } else if (env2.first != nullptr && env2.second != nullptr) {
      auto eq1 = item_1(item1);
      auto eq2 = item_1(item3);
      if ((eq1.first == env1.first && eq2.first == env1.second) ||
          (eq1.first == env1.second && eq2.first == env1.second)) {
        if (eq1.second != eq2.second) {
          assert(exp2val.find(item1) != exp2val.end());
          assert(exp2val.find(item3) != exp2val.end());

          auto ret1 = exp2val[item1];
          auto ret2 = exp2val[item3];

          if (item3->getCostDepth() > item1->getCostDepth()) {
            return ret1;
          } else {
            return ret2;
          }
        }
      }
    } else if (env3.first != nullptr && env3.second != nullptr) {
      auto eq1 = item_1(item1);
      auto eq2 = item_1(item2);
      if ((eq1.first == env1.first && eq2.first == env1.second) ||
          (eq1.first == env1.second && eq2.first == env1.second)) {
        if (eq1.second != eq2.second) {
          assert(exp2val.find(item1) != exp2val.end());
          assert(exp2val.find(item2) != exp2val.end());

          auto ret1 = exp2val[item1];
          auto ret2 = exp2val[item2];

          if (item2->getCostDepth() > item1->getCostDepth()) {
            return ret1;
          } else {
            return ret2;
          }
        }
      }
    }
  }
  return nullptr;
}

std::pair<Value *, int> CondSimplify::item_1(Exp *exp) {
  if (exp->isEQ()) {
    auto lhs = exp->getLSubexp();
    auto rhs = exp->getRSubexp();
    if (lhs->isRem()) {
      auto lhs_lhs = lhs->getLSubexp();
      auto lhs_rhs = lhs->getRSubexp();
      if (rhs->isAssign() && lhs_rhs->isAssign()) {
        auto is_eqret = dynamic_cast<ConstantInt *>(rhs->getVal());
        auto is_const2 = dynamic_cast<ConstantInt *>(lhs_rhs->getVal());
        if (is_const2 && is_eqret && is_const2->getValue() == 2 &&
            (is_eqret->getValue() == 0 || is_eqret->getValue() == 1)) {
          if (exp2val.find(lhs_lhs) != exp2val.end()) {
            return {exp2val[lhs_lhs], is_eqret->getValue()};
          } else {
            return {nullptr, is_eqret->getValue()};
          }
        }
      }
    }
  }
  return {nullptr, 0};
}

std::pair<Value *, Value *> CondSimplify::item_2(Exp *exp) {
  if (exp->isNE()) {
    auto lhs = exp->getLSubexp();
    auto rhs = exp->getRSubexp();
    if (lhs->isRem() && rhs->isRem()) {
      auto lhs_lhs = lhs->getLSubexp();
      auto lhs_rhs = lhs->getRSubexp();
      auto rhs_lhs = rhs->getLSubexp();
      auto rhs_rhs = rhs->getRSubexp();
      auto const1 = dynamic_cast<ConstantInt *>(lhs_rhs->getVal());
      auto const2 = dynamic_cast<ConstantInt *>(rhs_rhs->getVal());
      if (const1->getValue() == 2 && const2->getValue() == 2) {
        if (exp2val.find(lhs_lhs) != exp2val.end() &&
            exp2val.find(rhs_lhs) != exp2val.end()) {
          return {exp2val[lhs_lhs], exp2val[rhs_lhs]};
        } else {
          return {nullptr, nullptr};
        }
      }
    }
  }
  return {nullptr, nullptr};
}

// std::pair<Value *, int> CondSimplify::item_3( Exp *exp )
// {
//     ;
// }

std::pair<Value *, Value *> CondSimplify::Accumulate(StoreInst *str) {
  auto add = str->getOperand(0);
  auto store_addr = str->getOperand(1);
  auto add_instr = dynamic_cast<Instruction *>(add);
  if (add_instr) {
    auto add_op1 = add_instr->getOperand(0);
    auto add_op2 = add_instr->getOperand(1);
    auto add_op1_instr = dynamic_cast<Instruction *>(add_op1);
    if (add_op1_instr->isLoad()) {
      auto load_instr = dynamic_cast<LoadInst *>(add_op1_instr);
      auto load_addr = load_instr->getOperand(1);
      if (load_addr == store_addr) {
        return {load_addr, add_op2};
      }
    }
  }
}

bool CondSimplify::AddrCmp(Instruction *addr1, Instruction *addr2) {
  if (addr1->isAlloca() && addr2->isAlloca()) {
    return addr1 == addr2;
  } else if (addr1->isGEP() && addr2->isGEP()) {
    auto addr1_addr = addr1->getOperand(0);
    auto addr1_offset = addr1->getOperand(1);
    auto addr2_addr = addr2->getOperand(0);
    auto addr2_offset = addr2->getOperand(1);

    auto exp_offset1 = Exp::create(addr1_offset);
    auto exp_offset2 = Exp::create(addr2_offset);
    if (dynamic_cast<Instruction *>(addr1_addr) &&
        dynamic_cast<Instruction *>(addr2_addr)) {
      if (AddrCmp(dynamic_cast<Instruction *>(addr1_addr),
                  dynamic_cast<Instruction *>(addr2_addr)) &&
          expCmp(exp_offset1, exp_offset2)) {
        return true;
      }
    }
  } else {
    return false;
  }
  return false;
}

bool CondSimplify::expCmp(Exp *exp1, Exp *exp2) {
  if (exp1->getOp() != exp2->getOp()) {
    return false;
  } else {
    if (exp1->isAssign()) {
      if (exp1->getVal() == exp2->getVal()) {
        return true;
      } else {
        return false;
      }
    } else {
      if (expCmp(exp1->getLSubexp(), exp2->getLSubexp()) &&
          expCmp(exp1->getRSubexp(), exp2->getRSubexp())) {
        return true;
      }
    }
  }
  return false;
}

Exp *Exp::create(Value *root) {
  auto instr = dynamic_cast<Instruction *>(root);
  if (instr) {
    switch (instr->getInstrType()) {
    case Instruction::Add: {
      auto lhs = create(instr->getOperand(0));
      auto rhs = create(instr->getOperand(1));
      if (lhs && rhs) {
        Exp *exp;

        if (val2exp.find(instr) != val2exp.end()) {
          exp = val2exp[instr];
        } else {
          exp = new Exp(lhs, rhs, Add);
          val2exp.insert({instr, exp});
          exp2val.insert({exp, instr});
        }

        return exp;
      }
    } break;
    case Instruction::Sub: {
      auto lhs = create(instr->getOperand(0));
      auto rhs = create(instr->getOperand(1));
      if (lhs && rhs) {
        Exp *exp;

        if (val2exp.find(instr) != val2exp.end()) {
          exp = val2exp[instr];
        } else {
          exp = new Exp(lhs, rhs, Sub);
          val2exp.insert({instr, exp});
          exp2val.insert({exp, instr});
        }

        return exp;
      }
    } break;
    case Instruction::Div: {
      auto lhs = create(instr->getOperand(0));
      auto rhs = create(instr->getOperand(1));
      if (lhs && rhs) {
        Exp *exp;

        if (val2exp.find(instr) != val2exp.end()) {
          exp = val2exp[instr];
        } else {
          exp = new Exp(lhs, rhs, Div);
          val2exp.insert({instr, exp});
          exp2val.insert({exp, instr});
        }

        return exp;
      }
    } break;
    case Instruction::Rem: {
      auto lhs = create(instr->getOperand(0));
      auto rhs = create(instr->getOperand(1));
      if (lhs && rhs) {
        Exp *exp;

        if (val2exp.find(instr) != val2exp.end()) {
          exp = val2exp[instr];
        } else {
          exp = new Exp(lhs, rhs, Rem);
          val2exp.insert({instr, exp});
          exp2val.insert({exp, instr});
        }

        return exp;
      }
    } break;
    case Instruction::And: {
      auto lhs = create(instr->getOperand(0));
      auto rhs = create(instr->getOperand(1));
      if (lhs && rhs) {
        Exp *exp;

        if (val2exp.find(instr) != val2exp.end()) {
          exp = val2exp[instr];
        } else {
          exp = new Exp(lhs, rhs, And);
          val2exp.insert({instr, exp});
          exp2val.insert({exp, instr});
        }

        return exp;
      }
    } break;
    case Instruction::Or: {
      auto lhs = create(instr->getOperand(0));
      auto rhs = create(instr->getOperand(1));
      if (lhs && rhs) {
        Exp *exp;

        if (val2exp.find(instr) != val2exp.end()) {
          exp = val2exp[instr];
        } else {
          exp = new Exp(lhs, rhs, Or);
          val2exp.insert({instr, exp});
          exp2val.insert({exp, instr});
        }

        return exp;
      }
    } break;
    case Instruction::Cmp: {
      auto cmp = dynamic_cast<CmpInst *>(instr);
      auto op1 = cmp->getOperand(0);
      auto op2 = cmp->getOperand(1);
      if (cmp->getCmpOp() == CmpInst::NE) {
        auto op1_int = dynamic_cast<ConstantInt *>(op1);
        auto op2_int = dynamic_cast<ConstantInt *>(op2);
        if (op1_int) {
          if (op1_int->getValue() == 0) {
            return create(op2);
          }
        } else if (op2_int) {
          if (op2_int->getValue() == 0) {
            return create(op1);
          }
        }
      }
      auto lhs = create(op1);
      auto rhs = create(op2);
      Exp::OpID op_id = (cmp->getCmpOp() == CmpInst::GT)
                            ? Exp::GT
                            : (cmp->getCmpOp() == CmpInst::GE)
                                  ? Exp::GE
                                  : (cmp->getCmpOp() == CmpInst::LT)
                                        ? Exp::LT
                                        : (cmp->getCmpOp() == CmpInst::LE)
                                              ? Exp::LE
                                              : (cmp->getCmpOp() == CmpInst::EQ)
                                                    ? Exp::EQ
                                                    : NE;
      if (lhs && rhs) {
        Exp *exp;

        if (val2exp.find(instr) != val2exp.end()) {
          exp = val2exp[instr];
        } else {
          exp = new Exp(lhs, rhs, op_id);
          val2exp.insert({instr, exp});
          exp2val.insert({exp, instr});
        }

        return exp;
      }
    } break;
    case Instruction::ZExt: {
      return create(instr->getOperand(0));
    } break;
    case Instruction::Load: {
      Exp *exp;

      if (val2exp.find(instr) != val2exp.end()) {
        exp = val2exp[instr];
      } else {
        exp = new Exp(instr);
        val2exp.insert({instr, exp});
        exp2val.insert({exp, instr});
      }

      return exp;
    } break;
    default: {
      Exp *exp;

      if (val2exp.find(instr) != val2exp.end()) {
        exp = val2exp[instr];
      } else {
        exp = new Exp(instr);
        val2exp.insert({instr, exp});
        exp2val.insert({exp, instr});
      }

      return exp;
    } break;
    }
  }
  Exp *exp;

  if (val2exp.find(root) != val2exp.end()) {
    exp = val2exp[root];
  } else {
    exp = new Exp(root);
    val2exp.insert({root, exp});
    exp2val.insert({exp, root});
  }

  return exp;
}

void Exp::print() {
  PRINT_DEPTH_(print_depth_)
  std::cerr << "op: " << op_name[op_] << std::endl;
  print_depth_ += 4;
  if (op_ == Assign) {
    PRINT_DEPTH_(print_depth_)
    std::cerr << "%" << val_->getName() << std::endl;
  } else {
    lhs_->setPrintDepth(print_depth_);
    lhs_->print();
    rhs_->setPrintDepth(print_depth_);
    rhs_->print();
  }
  print_depth_ -= 4;
}