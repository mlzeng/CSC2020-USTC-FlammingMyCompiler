#include "Vectorization.h"
#include <algorithm>

void Vectorization::run() {
  std::set<BBset_t *> nestest_loop;
  int nestest_level = 1;
  LoopFind lf(m_);
  lf.run();
  for (auto func : m_->getFunctions()) {
    if (func->getUseList().empty())
      continue;
    for (auto bb : func->getBasicBlocks()) {
      int nested_level = 0;
      for (auto bs = lf.getBaseLoop(bb); bs != nullptr;
           bs = lf.getParentLoop(bs)) {
        nested_level++;
      }
      if (nested_level > nestest_level) {
        nestest_level = nested_level;
        nestest_loop.clear();
        nestest_loop.insert(lf.getBaseLoop(bb));
      } else if (nested_level == nestest_level) {
        nestest_loop.insert(lf.getBaseLoop(bb));
      }
    }
  }
  for (auto loop : nestest_loop) {
    if (loop->size() != 2)
      continue;
    bool ignore = false;
    for (auto bb : *loop) {
      for (auto inst : bb->getInstructions()) {
        if (inst->isCall()) {
          ignore = true;
        }
      }
    }
    if (ignore)
      continue;
    auto cond = lf.getLoopCond(loop);
    if (cond->getInstructions().size() != 3)
      continue;
    BasicBlock *body;
    for (auto bb : *loop) {
      if (bb != cond)
        body = bb;
    }
    if (tryVectorizate(cond, body)) {
      vectorizate_loops++;
      // std::cerr << "Vectorizate " << body->getName() << std::endl;
    }
  }
}

// arg: cond: 3 instructions
// which loop can vectorizate:
// 1. cond:
//  %iter_max should be variable(not defined in body) or %iter_max-%iter_init >
//  MIN_LOOP_ITERS instructions:
//      phi %iter <label>body %const <label>outer or phi %const <label>outer
//      %iter <label>body cmp LE/LT %iter %iter_max br cmp <label>body
//      <label>exit
// 2. body:
//
bool Vectorization::tryVectorizate(BasicBlock *cond, BasicBlock *body) {
  CmpInst *cmp = nullptr;
  Instruction *phi = nullptr;

  Instruction *iter = nullptr;
  ConstantInt *iter_init = nullptr;
  ConstantInt *iter_max = nullptr;

  std::unordered_set<Instruction *> body_defined;
  for (auto inst : body->getInstructions())
    body_defined.insert(inst);
  for (auto inst : cond->getInstructions()) {
    // cmp LE/LT %iter %iter_max
    if (inst->isCmp()) {
      if (cmp)
        return false;
      cmp = static_cast<CmpInst *>(inst);
      if (cmp->getCmpOp() != CmpInst::LE && cmp->getCmpOp() != CmpInst::LT) {
        return false;
      }
      auto val = dynamic_cast<ConstantInt *>(cmp->getOperand(1));
      if (val)
        iter_max = val;
      else {
        auto op = dynamic_cast<Instruction *>(cmp->getOperand(1));
        if (op && body_defined.find(op) != body_defined.end())
          return false;
      }
    }
    // phi %iter <label>body %const <label>outer or phi %const <label>outer
    // %iter <label>body
    else if (inst->isPHI()) {
      if (inst->getNumOperand() != 4)
        return false;
      if (phi)
        return false;
      phi = inst;
      for (int i = 0; i < 4; i += 2) {
        auto val = dynamic_cast<Instruction *>(inst->getOperand(i));
        if (val) {
          if (body_defined.find(val) == body_defined.end())
            return false;
          if (dynamic_cast<BasicBlock *>(inst->getOperand(i + 1)) != body)
            return false;
          if (val->isAdd())
            iter = val;
          else
            return false;
        } else {
          auto val = dynamic_cast<ConstantInt *>(inst->getOperand(i));
          if (val)
            iter_init = val;
          else
            return false;
          auto bb = dynamic_cast<BasicBlock *>(inst->getOperand(i + 1));
          if (bb == cond || bb == body)
            return false;
        }
      }
    } else if (inst->isBr()) {
      auto br = static_cast<BranchInst *>(inst);
      if (!br->isCondBr()) {
        return false;
      }
      auto if_true = dynamic_cast<BasicBlock *>(br->getOperand(1));
      if (if_true != body)
        return false;
      auto if_false = dynamic_cast<BasicBlock *>(br->getOperand(2));
      if (if_false == cond || if_false == body)
        return false;
    } else
      return false;
  }
  assert(cmp);
  if (!iter)
    return false;
  if (iter_max) {
    if (iter_init) {
      if (iter_max->getValue() - iter_init->getValue() <= MIN_LOOP_ITERS)
        return false;
    } else if (iter_max->getValue() <= MIN_LOOP_ITERS)
      return false;
  }

  auto iter_num = dynamic_cast<ConstantInt *>(iter->getOperand(1));
  if (iter_num) {
    if (iter_num->getValue() != 1)
      return false;
  }

  std::set<Instruction *> vvs;
  std::set<Instruction *> stores;
  std::set<Instruction *> adds;
  for (auto inst : body->getInstructions()) {
    if (inst->isMul()) {
      for (auto op : inst->getOperands()) {
        auto tmp = dynamic_cast<Instruction *>(op);
        if (!tmp->isLoad())
          return false;
        auto gep = dynamic_cast<Instruction *>(tmp->getOperand(0));
        if (!gep || !gep->isGEP())
          return false;
        if (dynamic_cast<Instruction *>(gep->getOperand(1)) != phi)
          return false;
      }
      if (inst->getUseList().size() != 1)
        return false;
      auto add = dynamic_cast<Instruction *>(inst->getUseList().back().val_);
      if (!add)
        return false;
      adds.insert(add);
      vvs.insert(inst);
    } else if (inst->isStore()) {
      auto add = dynamic_cast<Instruction *>(inst->getOperand(0));
      if (add && add->isAdd() && adds.find(add) != adds.end()) {
        auto gep = dynamic_cast<Instruction *>(inst->getOperand(1));
        if (gep && body_defined.find(gep) != body_defined.end())
          return false;
      } else
        return false;
    }
  }
  for (auto mul : vvs) {
    vectorizate(body, mul, iter);
  }
  return true;
}

void Vectorization::vectorizate(BasicBlock *body, Instruction *mul,
                                Instruction *iter) {
  auto iter_num = dynamic_cast<ConstantInt *>(iter->getOperand(1));
  if (iter_num) {
    if (iter_num->getValue() != 1)
      abort();
    iter_num->setValue(VECTORIZATE_INT32_NUM);
  } else
    abort();
  auto op1 = dynamic_cast<Instruction *>(mul->getOperand(0));
  if (op1) {
    if (op1->isLoad()) {
      op1 = dynamic_cast<Instruction *>(op1->getOperand(0));
    } else
      abort();
  } else
    abort();
  auto op2 = dynamic_cast<Instruction *>(mul->getOperand(1));
  if (op2) {
    if (op2->isLoad()) {
      op2 = dynamic_cast<Instruction *>(op2->getOperand(0));
    } else
      abort();
  } else
    abort();

  auto vv = VVInst::createVV(VECTORIZATE_INT32_NUM, op1, op2);
  body->insertInstr(mul, vv);
  mul->replaceAllUseWith(vv);
  body->deleteInstr(mul);
}