#include "ConstLoopExpansion.h"

#define ITERBOUND 40

void ConstLoopExpansion::run() {
  LoopFind finder(m_);
  finder.run();
  // debug
  // std::cout << "loop finder:\n";
  // find loop to be expanded
  std::vector<std::pair<BasicBlock *, BasicBlock *>> toCheck;
  for (auto it = finder.begin(); it != finder.end(); it++) {
    auto loopSet = *it;
    // DEBUG
    // for (auto BB : *loopSet) {
    //     std::cout << BB->getName() + " ";
    // }
    // putchar('\n');
    if (!finder.isBaseLoop(loopSet) || loopSet->size() != 2)
      continue;
    auto cond = finder.getLoopCond(loopSet);
    BasicBlock *loopBody;
    for (auto BB : *loopSet) {
      if (BB != cond)
        loopBody = BB;
    }
    if (loopBody->getSuccBasicBlocks().size() != 1)
      continue;
    toCheck.push_back(std::make_pair(cond, loopBody));
  }
  for (auto pair : toCheck) {
    // debug
    // std::cout << "pair to check" + pair.first->getName() +
    // pair.second->getName() + " \n"; make sure pass correct args
    bool check = false;
    for (auto su_cond : pair.first->getSuccBasicBlocks()) {
      if (su_cond == pair.second)
        check = true;
    }
    assert(check = true);
    assert(*pair.second->getSuccBasicBlocks().begin() == pair.first);
    bool hasCall = false;
    for (auto ins : pair.second->getInstructions()) {
      if (dynamic_cast<CallInst *>(ins))
        hasCall = true;
    }
    if (hasCall)
      continue;
    tryExpand(pair.first, pair.second);
  }
}

void ConstLoopExpansion::tryExpand(BasicBlock *cond, BasicBlock *loop) {
  // debug
  // std::cout << "try expansion\n";
  // PHI
  int phiCount = 0;
  for (auto instr : cond->getInstructions()) {
    if (instr->isPHI())
      phiCount++;
  }
  if (phiCount != 1)
    return;

  // Iterator, comparer
  int start, end;
  Instruction *br = cond->getTerminator();
  Instruction *iter;
  auto phi = *cond->getInstructions().begin();
  assert(phi->isPHI() && "Cond BB does not start with phi");
  // DEBUG
  // std::cout << "Find PHI" << phi->getName() << "\n";
  auto v = phi->getOperands();
  if (v.size() != 4)
    return;
  if (dynamic_cast<ConstantInt *>(v[2]) == nullptr)
    return;
  assert(v[3] != loop && "Inheriant const from loop by phi");
  start = dynamic_cast<ConstantInt *>(v[2])->getValue();
  iter = dynamic_cast<Instruction *>(v[0]);
  auto comparer = dynamic_cast<Instruction *>(br->getOperand(0));
  // if (comparer->isCmp()) {comparer = static_cast<CmpInst*}
  if (!comparer->isCmp())
    return;
  if (!dynamic_cast<ConstantInt *>(comparer->getOperand(1)))
    return;
  end = dynamic_cast<ConstantInt *>(comparer->getOperand(1))->getValue();
  // 目前支持简单的终结关系——没有表达式
  // 例如 a-1 < 0 需要额外考虑
  if (comparer->getOperand(0) != phi)
    return;
  // debug
  // std::cout << "Find Iter " + iter->getName() + " Comparer " +
  // comparer->getName() + "\n"; Iter-Pattrtn
  int iterPattern = -1; // 目前支持 +：0
  int accumulator;

  if (iter->getNumOperand() != 2 ||
      !dynamic_cast<ConstantInt *>(iter->getOperand(1)))
    return;
  accumulator = dynamic_cast<ConstantInt *>(iter->getOperand(1))->getValue();

  if (iter->isAdd()) {
    iterPattern = 0;
  } else {
    return;
    iterPattern = 1;
    accumulator = -accumulator;
  }

  if ((end - start) / accumulator > ITERBOUND)
    return;
  if (iterPattern == -1)
    return;
  assert(iterPattern == 0);
  // deBUG
  // std::cout << "find accumulator:\t" + std::to_string(accumulator) + "\n";
  switch (static_cast<CmpInst *>(comparer)->getCmpOp()) {
  case CmpInst::LT:
    break;
  default:
    return;
  }
  // TODO 解除这个限制
  // for (auto ins : loop->getInstructions()) {
  //     for (auto op : ins->getOperands()) {
  //         if (ins != iter && op == phi) return;
  //     }
  // }
  // DEBUG
  // std::cout << "expand " + cond->getName() + " and " + loop->getName() +
  // "\n"; expand
  std::unordered_set<Instruction *> ignored_ins{phi, comparer, br};
  BasicBlock *outBB;
  for (auto BB : cond->getSuccBasicBlocks()) {
    if (BB != loop)
      outBB = BB;
  }
  BasicBlock *new_bb = BasicBlock::create(m_, "");
  Function *func = cond->getParent();
  new_bb->setParent(func);
  std::unordered_map<Value *, Instruction *> old2new;
  // for (auto ins : cond->getInstructions()) {
  //     ins->removeUseOfOps();
  // }
  while (start < end) {
    old2new.clear();
    for (auto ins : cond->getInstructions()) {
      if (ignored_ins.find(ins) != ignored_ins.end())
        continue;
      Instruction *new_ins = ins->copy(new_bb);
      old2new.insert({ins, new_ins});
    }
    for (auto ins : cond->getInstructions()) {
      if (ignored_ins.find(ins) != ignored_ins.end())
        continue;
      for (unsigned i = 0; i < ins->getNumOperand(); i++) {
        auto op = ins->getOperand(i);
        if (op == phi) {
          old2new[ins]->setOperand(i, ConstantInt::get(start, m_));
        } else if (old2new.find(op) != old2new.end()) {
          old2new[ins]->setOperand(i, old2new[op]);
        } else {
          old2new[ins]->setOperand(i, op);
        }
        assert(op != comparer);
      }
    }
    for (auto ins : loop->getInstructions()) {
      if (ins == iter || ins->isBr())
        continue;
      Instruction *new_ins = ins->copy(new_bb);
      old2new.insert({ins, new_ins});
    }
    for (auto ins : loop->getInstructions()) {
      if (ins == iter) {
        start += accumulator;
        continue;
      } else if (ins->isBr()) {
        continue;
      }

      for (unsigned i = 0; i < ins->getNumOperand(); i++) {
        auto op = ins->getOperand(i);
        if (op == iter || op == phi) {
          old2new[ins]->setOperand(i, ConstantInt::get(start, m_));
        } else if (old2new.find(op) != old2new.end()) {
          old2new[ins]->setOperand(i, old2new[op]);
        } else {
          old2new[ins]->setOperand(i, op);
        }
        // assert(op != phi);
      }
    }
  }
  old2new.clear();
  std::unordered_map<Value *, Value *> old2new_;
  old2new_.insert({cond, new_bb});
  for (auto ins : cond->getInstructions()) {
    if (ignored_ins.find(ins) != ignored_ins.end())
      continue;
    Instruction *new_ins = ins->copy(new_bb);
    old2new_.insert({ins, new_ins});
  }
  for (auto ins : cond->getInstructions()) {
    if (ignored_ins.find(ins) != ignored_ins.end())
      continue;
    for (unsigned i = 0; i < ins->getNumOperand(); i++) {
      auto op = ins->getOperand(i);
      if (op == phi) {
        dynamic_cast<Instruction *>(old2new_[ins])
            ->setOperand(i, ConstantInt::get(start, m_));
      } else if (old2new_.find(op) != old2new_.end()) {
        dynamic_cast<Instruction *>(old2new_[ins])->setOperand(i, old2new[op]);
      } else {
        dynamic_cast<Instruction *>(old2new_[ins])->setOperand(i, op);
      }
      assert(op != comparer);
    }
  }
  old2new_.insert({phi, ConstantInt::get(start, m_)});
  for (auto ins : outBB->getInstructions()) {
    for (unsigned i = 0; i < ins->getNumOperand(); i++) {
      auto op = ins->getOperand(i);
      if (old2new_.find(op) != old2new_.end()) {
        ins->setOperand(i, old2new_[op]);
      }
    }
  }
  // Br 指令及前驱后继维护
  BranchInst::createBr(outBB, new_bb);
  for (auto pred_cond : cond->getPreBasicBlocks()) {
    if (pred_cond == loop)
      continue;
    auto b = pred_cond->getTerminator();
    for (unsigned i = 0; i < b->getNumOperand(); i++) {
      if (b->getOperand(i) == cond) {
        b->setOperand(i, new_bb);
        pred_cond->addSuccBasicBlock(new_bb);
        pred_cond->removeSuccBasicBlock(cond);
        new_bb->addPreBasicBlock(pred_cond);
      }
    }
  }
  outBB->removePreBasicBlock(cond);

  // func BB列表维护
  auto &BBs = func->getBasicBlocks();
  for (auto ins : cond->getInstructions()) {
    ins->removeUseOfOps();
  }
  for (auto ins : loop->getInstructions()) {
    ins->removeUseOfOps();
  }
  for (auto it = BBs.begin(); it != BBs.end();) {
    if (*it == cond) {
      it = BBs.erase(it);
      BBs.insert(it, new_bb);
    } else if (*it == loop) {
      it = BBs.erase(it);
    } else {
      it++;
    }
  }
}
