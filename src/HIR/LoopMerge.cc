#include "LoopMerge.h"

using std::get;

void LoopMerge::run() { // TODO 支持更复杂的merge模式
  BB2exps = std::make_unique<BBExpsAnalysis_hir>(m_);
  BB2exps->run();
  if (BB2exps->runFailed())
    return;
  std::vector<std::list<BaseBlock *> *> BBsToCheck;
  std::vector<std::list<BaseBlock *> *> stack;
  for (auto func : m_->getFunctions()) {
    if (func->getBaseBlocks().size() == 0)
      continue;
    auto &BBs = func->getBaseBlocks();
    stack.push_back(&BBs);
  }
  while (!stack.empty()) {
    std::list<BaseBlock *> *BBs_ptr = stack.back();
    stack.pop_back();
    int BBwhileCount = 0;
    for (auto BB : *BBs_ptr) {
      if (BB->isIfBlock()) {
        auto cond = static_cast<IfBlock *>(BB);
        stack.push_back(&cond->getIfBodyBBs());
        stack.push_back(&cond->getElseBodyBBs());
      } else if (BB->isWhileBlock()) {
        BBwhileCount++;
        if (BBwhileCount == 2) {
          BBsToCheck.push_back(BBs_ptr);
        }
        auto whileBB = static_cast<WhileBlock *>(BB);
        stack.push_back(&whileBB->getBodyBBs());
      }
    }
  }
  while (!BBsToCheck.empty()) {
    auto BBs_ptr = BBsToCheck.back();
    BBsToCheck.pop_back();
    auto iter = BBs_ptr->begin();
    BaseBlock *lastBB = nullptr;
    BaseBlock *lastlastBB = nullptr;
    for (; iter != BBs_ptr->end(); iter++) {
      auto BB = *iter;
      auto last = dynamic_cast<BasicBlock *>(lastBB);
      if (!BB->isWhileBlock() || last == nullptr) {
        // 限制BB前必须有初始化BB
        lastlastBB = lastBB;
        lastBB = BB;
        continue;
      }
      curWhile = static_cast<WhileBlock *>(BB);
      if (!isSimpleAccuPat() || !checkSetCurStart(last)) {
        resetLastPat();
        lastlastBB = lastBB;
        lastBB = BB;
        continue;
      }
      if (!mergeAble() || BB2exps->getExps(last).exps.size() != 1 ||
          lastlastBB != lastWhile) {
        setLastPatternFromCur(); // 限制curwhile的前驱只有初始化
                                 // 限制两个while相邻
        lastlastBB = lastBB;
        lastBB = BB;
        continue;
      }
      merge();
      BBsToCheck.push_back(BBs_ptr);
      // std::cout << "HereIsALoopMerge\n";
      break;
    }
  }
}

bool LoopMerge::isSimpleAccuPat() {
  if (curWhile->getCondBBs().size() != 1)
    return false; // 限制LoopBody没有其它分支或循环
  auto &condExps = BB2exps->getExps(
      static_cast<BasicBlock *>(*curWhile->getCondBBs().begin()));
  if (condExps.exps.size() != 1)
    return false; // TODO 短路在hir的情况
  auto &cmpExp = condExps.exps[0];
  if (!std::get<1>(cmpExp)->isCmp())
    return false;
  auto cmpIns = static_cast<CmpInst *>(std::get<1>(cmpExp));
  if (cmpIns->getCmpOp() != CmpInst::CmpOp::LT)
    return false;
  curIter = std::get<2>(cmpExp)._exp[0].second.lval;
  curEndVal = std::get<2>(cmpExp)._exp[1].second.lval;
  if (curIter == nullptr || curEndVal == nullptr)
    return false; // 限制迭代的判断不是复杂的表达式
  if (dynamic_cast<Instruction *>(curIter) &&
      dynamic_cast<Instruction *>(curIter)->isGEP())
    return false; // 限制迭代器不能为数组

  if (curWhile->getBodyBBs().size() != 1)
    return false;
  auto bodyBB = dynamic_cast<BasicBlock *>(*curWhile->getBodyBBs().begin());
  if (bodyBB == nullptr)
    return false;
  auto &bodyExps = BB2exps->getExps(bodyBB);
  bool hasPlusOne = false;
  for (auto exp : bodyExps.exps) {
    if (get<1>(exp)->isStore() && isSameAddr(curIter, get<0>(exp))) {
      auto &rexp = get<2>(exp);
      if (rexp.level != plus_level)
        return false;
      if (rexp._exp.size() != 2)
        return false;
      if (rexp._exp[1].first != plus_hir)
        return false;
      auto &firstOp = rexp._exp[0].second;
      auto &secOp = rexp._exp[1].second;
      if (firstOp.level != Nop_level || secOp.level != Nop_level)
        return false;
      if (dynamic_cast<ConstantInt *>(firstOp.lval) == nullptr &&
          dynamic_cast<ConstantInt *>(secOp.lval) == nullptr)
        return false;
      if (dynamic_cast<ConstantInt *>(firstOp.lval)) {
        auto tmp = dynamic_cast<ConstantInt *>(firstOp.lval);
        if (tmp->getValue() != 1)
          return false;
        if (!isSameAddr(secOp.lval, curIter))
          return false;
        if (hasPlusOne)
          return false;
        hasPlusOne = true;
      } else {
        auto tmp = dynamic_cast<ConstantInt *>(secOp.lval);
        if (tmp->getValue() != 1)
          return false;
        if (!isSameAddr(firstOp.lval, curIter))
          return false;
        if (hasPlusOne)
          return false;
        hasPlusOne = true;
      }
    }
  }
  for (auto exp : bodyExps.exps) {
    if (get<1>(exp)->isStore() && !isSameAddr(curIter, get<0>(exp))) {
      curVar.insert(get<0>(exp));
    }
  }
  return true;
}

bool LoopMerge::checkSetCurStart(BasicBlock *BB) {
  if (BB == nullptr)
    return false;
  auto exp = BB2exps->getExps(BB).exps.back();
  if (get<0>(exp) == nullptr)
    return false;
  if (!isSameAddr(get<0>(exp), curIter))
    return false; // 限制loop前一定有迭代器的初始化
  if (!get<1>(exp)->isStore())
    return false;
  auto rval = get<2>(exp);
  if (rval.level != Nop_level)
    return false; // 限制初始值不是复杂的表达式
  curStartVal = rval.lval;
  return true;
}

bool LoopMerge::mergeAble() {
  if (lastWhile == nullptr)
    return false;
  if (!isSameAddr(curIter, lastIter))
    return false;
  if (dynamic_cast<ConstantInt *>(curStartVal) ||
      dynamic_cast<ConstantInt *>(lastStartVal)) {
    auto cur = dynamic_cast<ConstantInt *>(curStartVal);
    auto lst = dynamic_cast<ConstantInt *>(lastStartVal);
    if (cur == nullptr || lst == nullptr) { // 没有考虑复杂的常数表达式
      return false;
    } else if (cur->getValue() != lst->getValue()) {
      return false;
    }
  } else if (!(isSameAddr(curStartVal, lastStartVal) &&
               isInvariant(curStartVal, curWhile) &&
               isInvariant(lastStartVal, lastWhile)))
    return false; // 限制开始和结束值为循环不变量

  if (dynamic_cast<ConstantInt *>(curEndVal) ||
      dynamic_cast<ConstantInt *>(lastEndVal)) {
    auto cur = dynamic_cast<ConstantInt *>(curEndVal);
    auto lst = dynamic_cast<ConstantInt *>(lastEndVal);
    if (cur == nullptr || lst == nullptr) { // 没有考虑复杂的常数表达式
      return false;
    } else if (cur->getValue() != lst->getValue()) {
      return false;
    }
  } else if (!(isSameAddr(curEndVal, lastEndVal) &&
               isInvariant(curEndVal, curWhile) &&
               isInvariant(lastEndVal, lastWhile)))
    return false;

  if (!isSameAccuPattern())
    return false; //  仅识别简单的+=模式
  return true;
}

bool LoopMerge::isSameAccuPattern() {
  auto &lastWhileBody = lastWhile->getBodyBBs();
  auto &curWhileBody = curWhile->getBodyBBs();
  if (lastWhileBody.size() != 1 || curWhileBody.size() != 1)
    return false; // 限制合并的BBs只有一个body
  BasicBlock *lastBodyBB = dynamic_cast<BasicBlock *>((*lastWhileBody.begin()));
  BasicBlock *curBodyBB = dynamic_cast<BasicBlock *>((*curWhileBody.begin()));
  if (allAccu(lastBodyBB) && allAccu(curBodyBB)) {
    for (auto val : lastVar) {
      bool isSameAccu = false;
      for (auto accuVar : curVar) {
        if (isSameAddr(accuVar, val))
          isSameAccu = true;
      }
      if (isSameAccu)
        continue;
      for (auto instr : curBodyBB->getInstructions()) {
        if (instr->isLoad() && isSameAddr(val, instr->getOperand(0)))
          return false;
      }
    }
    for (auto val : curVar) {
      bool isSameAccu = false;
      for (auto accuVar : lastVar) {
        if (isSameAddr(accuVar, val))
          isSameAccu = true;
      }
      if (isSameAccu)
        continue;
      for (auto instr : lastBodyBB->getInstructions()) {
        if (instr->isLoad() && isSameAddr(val, instr->getOperand(0)))
          return false;
      }
    }
    return true;
  }
  return false;
}

bool LoopMerge::allAccu(BasicBlock *BB) {
  auto &exps = BB2exps->getExps(BB);
  for (auto &exp : exps.exps) {
    if (get<1>(exp)->isStore() == false)
      return false;
    auto assignment = get<0>(exp);
    bool once = false;
    auto &rexp = get<2>(exp);
    if (rexp.level != plus_level)
      return false;
    for (auto &subexp : rexp._exp) {
      if ((subexp.first == nop_hir || subexp.first == plus_hir) &&
          subexp.second.level == Nop_level) {
        if (dynamic_cast<ConstantInt *>(subexp.second.lval))
          continue;
        if (isSameAddr(assignment, subexp.second.lval)) {
          if (once)
            return false;
          once = true;
        }
      } else if (hasNot(subexp.second, assignment) == false) {
        return false;
      }
    }
    if (!once)
      return false;
  }
  return true;
}

bool LoopMerge::hasNot(Exp_hir &exp, Value *val) {
  if (!(val && "Not considered"))
    return false;
  if (!(dynamic_cast<ConstantInt *>(val) == nullptr && "Not consider constant"))
    return false;
  if (exp.lval != nullptr) {
    if (dynamic_cast<ConstantInt *>(exp.lval))
      return true;
    if (isSameAddr(exp.lval, val))
      return false;
    else
      return true;
  }
  for (auto &subexp : exp._exp) {
    if (!hasNot(subexp.second, val))
      return false;
  }
  return true;
}

void LoopMerge::merge() {
  auto &lastWhileBody = lastWhile->getBodyBBs();
  auto &curWhileBody = curWhile->getBodyBBs();
  assert(lastWhileBody.size() == 1 && curWhileBody.size() == 1 &&
         "Should be ensured copy a simple BB");
  BasicBlock *fromBB = dynamic_cast<BasicBlock *>((*lastWhileBody.begin()));
  BasicBlock *toBB = dynamic_cast<BasicBlock *>((*curWhileBody.begin()));
  assert(fromBB && "while Body has a not-BasicBlock type");
  assert(toBB && "while Body has a not-BasicBlock type");
  auto baseFather = curWhile->getBaseFather();

  std::list<BaseBlock *> *BBlist;
  if (baseFather == nullptr) {
    BBlist = &curWhile->getFunction()->getBaseBlocks();
  } else {
    if (baseFather->isWhileBlock())
      BBlist = &static_cast<WhileBlock *>(baseFather)->getBodyBBs();
    else
      assert(false && "TODO : support getList in If-else");
  }

  auto iter = toBB->getInstructions().begin();
  auto iter_insertPiont = toBB->getInstructions().end();
  for (; iter != toBB->getInstructions().end(); iter++) {
    Instruction *ins = *iter;
    if (ins->isLoad() && isSameAddr(ins->getOperand(0), curIter)) {
      iter_insertPiont = iter;
      break;
    }
  }
  assert(iter_insertPiont != toBB->getInstructions().end() &&
         "Can't find related insert point");

  auto &list2Insert = toBB->getInstructions();
  auto &lastExps = BB2exps->getExps(fromBB);
  for (auto exp : lastExps.exps) {
    if (get<0>(exp) != nullptr && isSameAddr(get<0>(exp), lastIter)) {
      iter_insertPiont = toBB->getInstructions().end();
      continue;
    }
    for (auto instru2copy : getRelatedIns(get<1>(exp))) {
      // clearFormerInformation(instru2copy);
      instru2copy->setParent(toBB);
      insertAsNewBefore(instru2copy, iter_insertPiont, list2Insert);
    }
  }

  for (auto iter = BBlist->begin(); iter != BBlist->end(); iter++) {
    if (*iter == lastWhile) {
      iter = BBlist->erase(iter); // remove last BB
      BBlist->erase(iter);        // remove curBB entry;
      break;
    }
  }
}

void LoopMerge::clearFormerInformation(Instruction *ins) {}

void LoopMerge::insertAsNewBefore(Instruction *ins,
                                  std::_List_iterator<Instruction *> pos,
                                  std::list<Instruction *> &lst) {
  lst.insert(pos, ins);
}

std::list<BaseBlock *> &LoopMerge::getList(BaseBlock *curWhile) {
  auto baseFather = curWhile->getBaseFather();
  if (baseFather == nullptr) {
    return curWhile->getFunction()->getBaseBlocks();
  } else {
    if (baseFather->isWhileBlock())
      return static_cast<WhileBlock *>(baseFather)->getBodyBBs();
    else
      assert(false && "TODO : support getList in If-else");
  }
}

bool LoopMerge::isInvariant(Value *val, BaseBlock *block) {
  if (block == curWhile && curVar.find(val) == curVar.end()) {
    return true;
  } else if (block == lastWhile && lastVar.find(val) == lastVar.end()) {
    return true;
  }
  return false;
}

bool LoopMerge::isSameAddr(Value *val, Value *val2) {
  assert(val->getType()->isPointerTy() && val2->getType()->isPointerTy());
  auto ins1 = dynamic_cast<Instruction *>(val);
  auto ins2 = dynamic_cast<Instruction *>(val2);
  if (ins1 == nullptr) { // 非数组的函数参数或全局变量
    return val == val2;
  } else if (ins1->isAlloca()) { // 非数组的局部变量
    return val == val2;
  } else if (ins1->isLoad()) {
    if (!ins2->isLoad())
      return false;
    return isSameAddr(ins1->getOperand(0), ins2->getOperand(0));
  } else if (ins1->isGEP()) {
    if (!ins2->isGEP())
      return false;
    return isSameAddr(ins1->getOperand(0), ins2->getOperand(0)) &&
           isSameOffsetPattern(ins1->getOperand(1), ins2->getOperand(1));
  }
}

bool LoopMerge::isSameOffsetPattern(Value *val, Value *val2) {
  auto ins1 = dynamic_cast<Instruction *>(val);
  auto ins2 = dynamic_cast<Instruction *>(val2);
  if (ins1 == nullptr) {
    if (ins2 != nullptr)
      return false;
    auto _val = dynamic_cast<ConstantInt *>(val);
    auto _val2 = dynamic_cast<ConstantInt *>(val2);
    if (_val && _val2 && _val->getValue() == _val2->getValue()) {
      return true;
    } else if (val == val2) {
      return true;
    }
    return false;
  } else if (ins2 == nullptr) {
    return false;
  } else if (ins1->isLoad()) { // TODO support variant pattern
    if (!ins2->isLoad())
      return false;
    return isSameAddr(ins1->getOperand(0), ins2->getOperand(0));
  } else if (ins1->isGEP()) {
    return isSameAddr(ins1->getOperand(0), ins2->getOperand(0)) &&
           isSameOffsetPattern(ins1->getOperand(1), ins2->getOperand(1));
  } else if (ins1->isCall()) {
    return false;
  } else if (ins1->isAdd() || ins1->isMul()) {
    if (ins2->getInstrType() != ins1->getInstrType())
      return false;
    auto ins1op0 = ins1->getOperand(0);
    auto ins1op1 = ins1->getOperand(1);
    auto ins2op0 = ins2->getOperand(0);
    auto ins2op1 = ins2->getOperand(1);
    return (isSameOffsetPattern(ins1op0, ins2op0) &&
            isSameOffsetPattern(ins1op1, ins2op1)) ||
           (isSameOffsetPattern(ins1op0, ins2op1) &&
            isSameOffsetPattern(ins1op1, ins2op0));
  } else if (ins1->isSub() || ins1->isDiv() || ins1->isRem()) {
    if (ins2->getInstrType() != ins1->getInstrType())
      return false;
    auto ins1op0 = ins1->getOperand(0);
    auto ins1op1 = ins1->getOperand(1);
    auto ins2op0 = ins2->getOperand(0);
    auto ins2op1 = ins2->getOperand(1);
    return (isSameOffsetPattern(ins1op0, ins2op0) &&
            isSameOffsetPattern(ins1op1, ins2op1));
  } else {
    assert(false && "Not supported index pattern");
  }
}

std::vector<Instruction *> LoopMerge::getRelatedIns(Instruction *ins) {
  std::vector<Instruction *> res;
  std::unordered_set<Instruction *> res_unordered;
  auto BB = ins->getParent();
  std::vector<Instruction *> toCheck{ins};
  while (!toCheck.empty()) {
    Instruction *ins_tmp = toCheck.back();
    toCheck.pop_back();
    res_unordered.insert(ins_tmp);
    for (auto op : ins_tmp->getOperands()) {
      Instruction *op_ins = dynamic_cast<Instruction *>(op);
      if (op_ins == nullptr || op_ins->isAlloca())
        continue;
      toCheck.push_back(op_ins); // TODO 确认一个不会出现共用子表达式
    }
  }
  for (auto ins2 : BB->getInstructions()) {
    if (res_unordered.find(ins2) != res_unordered.end())
      res.push_back(ins2);
  }
  return res;
}
