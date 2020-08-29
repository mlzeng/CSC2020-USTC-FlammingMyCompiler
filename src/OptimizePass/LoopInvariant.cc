#include "LoopInvariant.h"

// #undef DEBUG
// #define DEBUG

void LoopInvariant::run() {
  finder = std::make_unique<LoopFind>(m_);
  finder->run();
#ifdef DEBUG
  std::cout << "---------循环不变量外提------------\n";
#endif
  areaCount = 0;
  for (auto it = finder->begin(); it != finder->end(); it++) {
    if (!finder->isBaseLoop(*it))
      continue;
    auto loop = *it;
    do {
#ifdef DEBUG
      std::cout << "inloop:\t";
      for (auto BB : *loop) {
        std::cout << BB->getName() + "\t";
      }
      putchar('\n');
#endif
      invariant.clear();
      findInvariants(loop);
      if (invariant.size())
        moveInvariantsOut(loop);
      loop = finder->getParentLoop(loop);
    } while (loop);
  }
}

void LoopInvariant::findInvariants(BBset_t *loop) {
  std::unordered_set<Value *> needDefinedInLoop;
  std::unordered_set<Value *> exceptLoad;
  for (auto BB : *loop) {
    for (auto ins : BB->getInstructions()) {
      needDefinedInLoop.insert(ins);
    }
  }

  bool change = false;
  do {
    change = false;
    for (auto BB : *loop) {
      for (auto ins_iter = BB->getInstructions().begin();
           ins_iter != BB->getInstructions().end(); ins_iter++) {
        bool allInvariant = true;
        auto ins = *ins_iter;
        if (ins->isPHI() || ins->isCmp() || ins->isBr() || ins->isCall() ||
            ins->isRet() || ins->isAlloca() ||
            // exceptLoad.find(ins) != exceptLoad.end() ||
            needDefinedInLoop.find(ins) == needDefinedInLoop.end())
          continue;
        for (auto val : ins->getOperands()) {
          if (needDefinedInLoop.find(val) != needDefinedInLoop.end()) {
            allInvariant = false;
          }
        }
        if (allInvariant && ins->isLoad()) {
          Value *ptr = ins->getOperand(0);
          while (dynamic_cast<Instruction *>(ptr) &&
                 dynamic_cast<Instruction *>(ptr)->isGEP()) {
            ptr = dynamic_cast<Instruction *>(ptr)->getOperand(0);
          }
          for (auto val2 : needDefinedInLoop) {
            auto ins2 = dynamic_cast<Instruction *>(val2);
            if (!ins2->isStore())
              continue;

            Value *ptr2 = ins2->getOperand(1);
            while (dynamic_cast<Instruction *>(ptr2) &&
                   dynamic_cast<Instruction *>(ptr2)->isGEP()) {
              ptr2 = dynamic_cast<Instruction *>(ptr2)->getOperand(0);
            }
            if (ptr2 == ptr) {
              // exceptLoad.insert(ins);
              allInvariant = false;
              break;
            }
          }
        }
        if (allInvariant) {
          needDefinedInLoop.erase(ins);
          invariant.push_back(std::make_pair(BB, ins_iter));
          change = true;
        }
      }
    }
  } while (change);
#ifdef DEBUG
  std::cout << "find Loop invariant\n";
  for (auto pair : invariant) {
    std::cout << "in BB: " + pair.first->getName() + "\t" +
                     " Val: " + (*pair.second)->getName() + "\n";
  }
#endif
}

void LoopInvariant::moveInvariantsOut(BBset_t *loop) {

  auto loopBase = finder->getLoopCond(loop);

  BasicBlock *newBB =
      BasicBlock::create(m_, "InvariantArea" + std::to_string(areaCount++));
  newBB->setParent(loopBase->getParent());
  // mov invariant to new
  for (auto pair : invariant) {
    Instruction *ins = *pair.second;
    pair.first->getInstructions().erase(pair.second);
    newBB->addInstruction(ins);
  }

  // create br
  BranchInst::createBr(loopBase, newBB);
  // Deal Phi
  std::vector<Value *> labels;
  std::vector<Value *> vals;
  std::vector<Value *> inloopLabels;
  std::vector<Value *> inloopVals;
  for (auto instr : loopBase->getInstructions()) {
    if (!instr->isPHI())
      continue;

    labels.clear();
    vals.clear();
    inloopVals.clear();
    inloopLabels.clear();
    auto v = instr->getOperands();

    for (unsigned i = 0; i < instr->getNumOperand(); i += 2) {
      if (loop->find(dynamic_cast<BasicBlock *>(v[i + 1])) != loop->end()) {
        inloopLabels.push_back(v[i + 1]);
        inloopVals.push_back(v[i]);
      } else {
        labels.push_back(v[i + 1]);
        vals.push_back(v[i]);
      }
    }

    if (labels.size() == 0)
      continue;
    if (labels.size() == 1) {
      for (unsigned i = 0; i < instr->getNumOperand(); i += 2) {
        if (vals[0] != v[i])
          continue;
        instr->setOperand(i + 1, newBB);
        labels[0]->removeUse(instr, i + 1);
      }
      continue;
    }

    auto new_phi = PhiInst::createPhi(v[0]->getType(), newBB);
    // assert(new_phi->getNumOperand() == 0);
    for (unsigned i = 0; i < labels.size(); i++) {
      new_phi->addOperand(vals[i]);
      new_phi->addOperand(labels[i]);
    }
    instr->clearOps();
    for (unsigned i = 0; i < inloopLabels.size(); i++) {
      instr->addOperand(inloopVals[i]);
      instr->addOperand(inloopLabels[i]);
    }
    instr->addOperand(new_phi);
    instr->addOperand(newBB);
    newBB->addInstrBegin(new_phi);
  }

  // Maintain pre & succ
  std::vector<BasicBlock *> toRemove;
  for (auto preBB : loopBase->getPreBasicBlocks()) {
    if (loop->find(preBB) != loop->end() || preBB == newBB)
      continue;
    toRemove.push_back(preBB);
    preBB->removeSuccBasicBlock(loopBase);
    auto end_ins = preBB->getTerminator();
    for (unsigned i = 0; i < end_ins->getNumOperand(); i++) {
      if (end_ins->getOperand(i) == loopBase) {
        end_ins->getOperand(i)->removeUse(end_ins, i);
        end_ins->setOperand(i, newBB);
      }
    }
    preBB->addSuccBasicBlock(newBB);
    newBB->addPreBasicBlock(preBB);
  }
  for (auto BB : toRemove) {
    loopBase->removePreBasicBlock(BB);
  }
  auto func = loopBase->getParent();
  auto &BBs = func->getBasicBlocks();
  auto it = BBs.begin();
  for (; it != BBs.end(); it++) {
    if (*it == loopBase) {
      BBs.insert(it, newBB);
      break;
    }
  }
}
