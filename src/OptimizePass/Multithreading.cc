#include "Multithreading.h"
#include "CodeGen.hh"

void Multithreading::run() {
  finder = std::make_unique<LoopFind>(m_);
  finder->run();
  std::unordered_set<BBset_t *> totest;

  int maxDepth = -1;
  for (auto it = finder->begin(); it != finder->end(); it++) {
    BBset_t *loop = *it;
    if (finder->getLoopEntry(loop)->getParent()->getMultithreading() == false)
      continue;

    if (finder->isBaseLoop(loop) == false)
      continue;
    int depth = 0;
    while (finder->getParentLoop(loop) != nullptr) {
      depth++;
      loop = finder->getParentLoop(loop);
    }

    if (depth < maxDepth || totest.find(loop) != totest.end())
      continue;
    else if (depth > maxDepth) {
      totest.clear();
      maxDepth = depth;
      totest.insert(loop);
    } else {
      totest.insert(loop);
    }
  }

  for (auto loop : totest) {
    if (findAccumulator(loop) == false)
      continue;

    // std::cout << "wrap loop \t" + finder->getLoopCond(loop)->getName() +
    // "\n";
    wrapMultithreading(finder->getLoopCond(loop), loop);
  }
}

bool Multithreading::findAccumulator(BBset_t *loop) {
  // for (auto BB : *loop) {
  //     for (auto ins : BB->getInstructions()) {
  //         if (ins->isCall()) return false;
  //     }
  // }
  auto loopBase = finder->getLoopCond(loop);
  // TODO 移除仅有一个前驱的假设
  if (loopBase->getPreBasicBlocks().size() != 2)
    return false;
  auto br = loopBase->getTerminator();
  comparer = dynamic_cast<Instruction *>(br->getOperand(0));
  if (comparer == nullptr || comparer->isCmp() == false)
    return false;
  // TODO LE之类
  if (static_cast<CmpInst *>(comparer)->getCmpOp() != CmpInst::CmpOp::LT)
    return false;
  // TODO 更多操作类型
  phi = dynamic_cast<Instruction *>(comparer->getOperand(0));
  if (phi == nullptr || phi->isPHI() == false)
    return false;

  end = comparer->getOperand(1);

  // 不允许条件BB里有其他phi
  for (auto ins : loopBase->getInstructions()) {
    if (ins->isPHI() && ins != phi)
      return false;
  }

  auto v = phi->getOperands();
  if (v.size() != 4)
    return false;
  for (unsigned i = 0; i < v.size(); i += 2) {
    auto label = dynamic_cast<BasicBlock *>(v[i + 1]);
    if (loop->find(label) == loop->end()) {
      start = v[i];
    } else {
      accu = dynamic_cast<Instruction *>(v[i]);
    }
  }
  assert(start && accu);

  auto accuVal = dynamic_cast<ConstantInt *>(accu->getOperand(1));

  if (accuVal == nullptr)
    return false;
  accuValue = accuVal->getValue();
  // TODO 不同的迭代方式
  if (accu->getOperand(0) != phi)
    return false;
  return true;
}

void Multithreading::wrapMultithreading(BasicBlock *loopBase, BBset_t *loop) {
  auto func = loopBase->getParent();
  BasicBlock *preHead = BasicBlock::create(
      m_, "MutithreadBegin" + std::to_string(multithreadCount));
  preHead->setParent(func);

  // 查找BB前驱后继 - TOOD 移除只有一个前驱的假设
  BasicBlock *pred_base;
  BasicBlock *post_base;
  for (auto pred : loopBase->getPreBasicBlocks()) {
    if (loop->find(pred) == loop->end())
      pred_base = pred;
  }
  for (auto succ : loopBase->getSuccBasicBlocks()) {
    if (loop->find(succ) == loop->end())
      post_base = succ;
  }

  // 插入mtstart并对BB分块
  auto mtNumber = CallInst::create(m_->getFunction("__mtstart"), {}, preHead);
  auto itersize = BinaryInst::createSub(end, start, preHead);
  auto _l =
      BinaryInst::createDiv(BinaryInst::createMul(itersize, mtNumber, preHead),
                            ConstantInt::get(mt_num_threads, m_), preHead);
  auto _r = BinaryInst::createDiv(
      BinaryInst::createMul(
          itersize,
          BinaryInst::createAdd(mtNumber, ConstantInt::get(1, m_), preHead),
          preHead),
      ConstantInt::get(mt_num_threads, m_), preHead);
  auto iterR = BinaryInst::createAdd(_r, start, preHead);
  auto iterL = BinaryInst::createAdd(
      BinaryInst::createMul(
          BinaryInst::createDiv(
              BinaryInst::createAdd(_l, ConstantInt::get(accuValue - 1, m_),
                                    preHead),
              ConstantInt::get(accuValue, m_), preHead),
          ConstantInt::get(accuValue, m_), preHead),
      start, preHead);
  BranchInst::createBr(loopBase, preHead);
  // replace start & end
  auto &v = phi->getOperands();
  for (unsigned i = 0; i < v.size(); i += 2) {
    if (v[i] == start) {
      v[i]->removeUse(phi, i);
      v[i + 1]->removeUse(phi, i + 1);
      phi->setOperand(i, iterL);
      phi->setOperand(i + 1, preHead);
    }
  }
  // TODO 移除LT使end在右的假设
  // comparer->removeUse(end);
  end->removeUse(comparer, 1);
  comparer->setOperand(1, iterR);

  // insert PreHead
  // TODO 移除仅有一个前驱的假设
  auto pred_br = pred_base->getTerminator();
  auto &v1 = pred_br->getOperands();
  for (unsigned i = 0; i < v1.size(); i++) {
    if (v1[i] == loopBase) {
      // pred_br->removeUse(loopBase);
      loopBase->removeUse(pred_br, i);
      pred_br->setOperand(i, preHead);
      pred_base->removeSuccBasicBlock(loopBase);
      pred_base->addSuccBasicBlock(preHead);
      preHead->addPreBasicBlock(pred_base);
      loopBase->removePreBasicBlock(pred_base);
      break;
    }
  }
  auto &BBs = func->getBasicBlocks();
  auto it = BBs.begin();
  for (; it != BBs.end(); it++) {
    if (*it == pred_base) {
      BBs.insert(++it, preHead);
      break;
    }
  }

  // 插入mtend
  // debug
  // std::cout << "插入mtend\n";
  BasicBlock *postHead = BasicBlock::create(
      m_, "MutithreadEnd" + std::to_string(multithreadCount++));
  postHead->setParent(func);
  CallInst::create(m_->getFunction("__mtend"), {mtNumber}, postHead);
  BranchInst::createBr(post_base, postHead);
  auto br = loopBase->getTerminator();
  auto &v2 = br->getOperands();
  for (unsigned i = 0; i < v2.size(); i++) {
    if (v2[i] == post_base) {
      // br->removeUse(v2[i]);
      v2[i]->removeUse(br, i);
      br->setOperand(i, postHead);
      loopBase->removeSuccBasicBlock(post_base);
      post_base->removePreBasicBlock(loopBase);
      loopBase->addSuccBasicBlock(postHead);
      postHead->addPreBasicBlock(loopBase);
    }
  }
  for (auto ins : post_base->getInstructions()) {
    if (ins->isPHI() == false)
      continue;
    auto &v3 = ins->getOperands();
    for (unsigned i = 0; i < v3.size(); i++) {
      if (v3[i] == loopBase) {
        // ins->removeUse(v3[i]);
        v3[i]->removeUse(ins, i);
        ins->setOperand(i, postHead);
        break;
      }
    }
  }
  auto &BBs1 = func->getBasicBlocks();
  auto it1 = BBs1.begin();
  for (; it1 != BBs1.end(); it1++) {
    if (*it1 == post_base) {
      BBs1.insert(it1, postHead);
      break;
    }
  }
}
