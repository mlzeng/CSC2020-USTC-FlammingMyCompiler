#include "AccumulatePattern.h"

using std::get;
using exp_t = std::tuple<Value *, Instruction *, Exp_hir>;

void AccumulatePattern::run() {
  BB2exps = std::make_unique<BBExpsAnalysis_hir>(m_);
  BB2exps->run();
  if (BB2exps->runFailed())
    return;
  std::vector<BaseBlock *> BBToCheck;
  for (auto func : m_->getFunctions()) {
    if (func->getBaseBlocks().size() == 0)
      continue;
    for (auto BB : func->getBaseBlocks()) {
      if (BB->isWhileBlock())
        BBToCheck.push_back(BB);
      else if (BB->isIfBlock() && nestedLoop(static_cast<IfBlock *>(BB)))
        BBToCheck.push_back(BB);
    }
  }
  std::vector<WhileBlock *> notNestedWhiles;
  while (!BBToCheck.empty()) {
    auto BB = BBToCheck.back();
    BBToCheck.pop_back();
    if (BB->isIfBlock()) {
      for (auto then : static_cast<IfBlock *>(BB)->getIfBodyBBs()) {
        if (then->isWhileBlock())
          BBToCheck.push_back(then);
        else if (then->isIfBlock() && nestedLoop(static_cast<IfBlock *>(then)))
          BBToCheck.push_back(then);
      }
      for (auto elseBB : static_cast<IfBlock *>(BB)->getElseBodyBBs()) {
        if (elseBB->isWhileBlock())
          BBToCheck.push_back(elseBB);
        else if (elseBB->isIfBlock() &&
                 nestedLoop(static_cast<IfBlock *>(elseBB)))
          BBToCheck.push_back(elseBB);
      }
    } else {
      auto whileBB = static_cast<WhileBlock *>(BB);
      auto &bodyBBs = whileBB->getBodyBBs();
      bool hasLoop = false;
      for (auto bodyBB : bodyBBs) {
        if (bodyBB->isWhileBlock()) {
          BBToCheck.push_back(bodyBB);
          hasLoop = true;
        } else if (bodyBB->isIfBlock() &&
                   nestedLoop(static_cast<IfBlock *>(bodyBB))) {
          BBToCheck.push_back(bodyBB);
          hasLoop = true;
        }
      }
      if (!hasLoop)
        notNestedWhiles.push_back(whileBB);
    }
  } // 限制查找的模式均是最内层循环, 但可以有条件BB

  for (auto it = notNestedWhiles.begin(); it != notNestedWhiles.end();) {
    curWhile = *it;
    if (!isSimpleIterPattern()) {
      it++;
      continue;
    } // 限制简单的加1迭代器模式
    if (!hasSimpleAccuPattern()) {
      it++;
      continue;
    } // 含有对数组的累加模式
    moveAccuOut();
    BB2exps->run();
  }
}

bool AccumulatePattern::isSimpleIterPattern() {
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

  // if (curWhile->getBodyBBs().size() != 1) return false;
  std::vector<BasicBlock *> BasicBBs;
  std::vector<IfBlock *> BB2Check;
  for (auto bodyBB : curWhile->getBodyBBs()) {
    if (bodyBB->isBasicBlock()) {
      BasicBBs.push_back(static_cast<BasicBlock *>(bodyBB));
    } else {
      assert(bodyBB->isIfBlock() && "curWhile should not be nested");
      BB2Check.push_back(static_cast<IfBlock *>(bodyBB));
    }
  }
  while (BB2Check.size()) {
    auto ifBB = BB2Check.back();
    BB2Check.pop_back();
    for (auto thenBB : ifBB->getIfBodyBBs()) {
      if (thenBB->isBasicBlock()) {
        BasicBBs.push_back(static_cast<BasicBlock *>(thenBB));
      } else {
        assert(thenBB->isIfBlock() && "curWhile should not be nested");
        BB2Check.push_back(static_cast<IfBlock *>(thenBB));
      }
    }
    for (auto elseBB : ifBB->getElseBodyBBs()) {
      if (elseBB->isBasicBlock()) {
        BasicBBs.push_back(static_cast<BasicBlock *>(elseBB));
      } else {
        assert(elseBB->isIfBlock() && "curWhile should not be nested");
        BB2Check.push_back(static_cast<IfBlock *>(elseBB));
      }
    }
  }

  bool hasPlusOne = false;
  for (auto bodyBB : BasicBBs) { // 限制为+1模式，且不能对迭代器重复赋值
    auto &bodyExps = BB2exps->getExps(bodyBB);
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
      if (get<1>(exp)->isStore()) {
        curVar.insert(get<0>(exp));
      }
    }
  }
  _BasicBBs.clear();
  _BasicBBs = std::move(BasicBBs);
  return true;
}

bool AccumulatePattern::hasSimpleAccuPattern() {
  AccuAddr = nullptr;
  for (auto BB : _BasicBBs) {
    auto &exps = BB2exps->getExps(BB);
    for (auto &exp : exps.exps) {
      if (get<1>(exp)->isStore() == false)
        continue;
      if (AccuAddr != nullptr && isSameAddr(AccuAddr, get<0>(exp)) == false) {
        for (auto &subexp : get<2>(exp)._exp) { // 限制不能存在“伪的”累加
          if (hasNot(subexp.second, AccuAddr) == false)
            return false;
        }
      } else if (AccuAddr == nullptr) {
        Instruction *val = dynamic_cast<Instruction *>(get<0>(exp));
        if (val == nullptr || val->isGEP() == false) {
          continue; // 非数组变量
        } else {
          bool toContinue = false;
          while (val && val->isGEP()) {
            if (isInvariant(val->getOperand(1)) == false) {
              toContinue = true;
              break;
            }
            val = dynamic_cast<Instruction *>(val->getOperand(0));
          }
          if (toContinue)
            continue;
        }
        if (isAssignPlus(exp))
          AccuAddr = get<0>(exp);
      } else {
        if (!isAssignPlus(exp))
          return false;
      }
    }
  }
  if (AccuAddr == nullptr)
    return false;
  return true;
}

bool AccumulatePattern::isInvariant(Value *val) {
  auto ins = dynamic_cast<Instruction *>(val);
  if (ins == nullptr)
    return true;
  if (curVar.find(ins) != curVar.end())
    return false;
  for (auto op : ins->getOperands()) {
    if (!isInvariant(op))
      return false;
  }
  return true;
}

bool AccumulatePattern::isAssignPlus(exp_t &exp) {
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
  return true;
}

bool AccumulatePattern::hasNot(Exp_hir &exp, Value *val) {
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
    if (hasNot(subexp.second, val) == false)
      return false;
  }
  return true;
}

void AccumulatePattern::moveAccuOut() {
  BasicBlock *preBB = getNewPreBasicBlock(curWhile);
  BasicBlock *postBB = getNewPostBasicBlock(curWhile);
  // add int ans = 0
  auto allocAddr = AllocaInst::createAlloca(m_->getInt32Ty(), preBB);
  StoreInst::createStore(ConstantInt::get(0, m_), allocAddr, preBB);

  // add a[i] = a[i] + ans
  std::unordered_map<Value *, Value *> old2New;
  for (auto ins :
       getRelatedInsInOrder(AccuAddr)) { // 依赖于hir不使用BB作为操作数
    auto new_ins = ins->copy(postBB);
    new_ins->setNumOps(0);
    for (auto operand : ins->getOperands()) {
      if (old2New.find(operand) != old2New.end()) {
        new_ins->addOperand(old2New[operand]);
      } else {
        new_ins->addOperand(operand);
      }
    }
    old2New.insert({ins, new_ins});
  }
  auto addr1 = old2New[AccuAddr];
  old2New.clear();
  for (auto ins :
       getRelatedInsInOrder(AccuAddr)) { // 依赖于hir不使用BB作为操作数
    auto new_ins = ins->copy(postBB);
    new_ins->setNumOps(0);
    for (auto operand : ins->getOperands()) {
      if (old2New.find(operand) != old2New.end()) {
        new_ins->addOperand(old2New[operand]);
      } else {
        new_ins->addOperand(operand);
      }
    }
    old2New.insert({ins, new_ins});
  }
  auto addr2 = old2New[AccuAddr]; // 使用自定的顺序
  auto val = LoadInst::createLoad(m_->getInt32Ty(), addr1, postBB);
  auto val_alloc = LoadInst::createLoad(m_->getInt32Ty(), allocAddr, postBB);
  auto res = BinaryInst::createAdd(val, val_alloc, postBB);
  StoreInst::createStore(res, addr2, postBB);

  // replace a[i] with ans
  for (auto BB : _BasicBBs) {
    std::unordered_set<Instruction *> toDelete;
    old2New.clear();
    for (auto ins : BB->getInstructions()) {
      if (ins->isGEP() && isSameAddr(ins, AccuAddr)) {
        for (auto relatedIns : getRelatedInsInOrder(ins))
          toDelete.insert(relatedIns);
        old2New.insert({ins, allocAddr});
      }
    }
    auto &instrs = BB->getInstructions();
    auto it = instrs.begin();
    for (; it != instrs.end();) {
      auto ins = *it;
      if (toDelete.find(ins) != toDelete.end()) {
        it = instrs.erase(it);
      } else {
        for (unsigned i = 0; i < ins->getNumOperand(); i++) {
          if (old2New.find(ins->getOperand(i)) != old2New.end()) {
            ins->setOperand(i, old2New[ins->getOperand(i)]);
          }
        }
        it++;
      }
    }
  }
}

BasicBlock *AccumulatePattern::getNewPreBasicBlock(BaseBlock *BB) {
  auto baseF = BB->getBaseFather();
  auto func = BB->getFunction();
  std::list<BaseBlock *> *BBs = nullptr;
  if (baseF == nullptr) {
    BBs = &func->getBaseBlocks();
  } else if (baseF->isIfBlock()) {
    auto ifBB = static_cast<IfBlock *>(baseF);
    for (auto _BB : ifBB->getIfBodyBBs()) {
      if (_BB == BB) {
        BBs = &ifBB->getIfBodyBBs();
        break;
      }
    }
    if (BBs == nullptr) {
      for (auto _BB : ifBB->getElseBodyBBs()) {
        if (_BB == BB) {
          BBs = &ifBB->getElseBodyBBs();
          break;
        }
      }
    }
  } else {
    BBs = &(static_cast<WhileBlock *>(baseF)->getBodyBBs());
  }
  assert(BBs && "should find the BB List");

  auto newBB = BasicBlock::create(m_, "preBBofAP");
  if (baseF != nullptr)
    newBB->setBaseFather(baseF);
  else
    newBB->setFunction(func);
  auto it = BBs->begin();
  for (; it != BBs->end(); it++) {
    if (*it == BB) {
      BBs->insert(it, newBB);
      break;
    }
  }
  return newBB;
}

BasicBlock *AccumulatePattern::getNewPostBasicBlock(BaseBlock *BB) {
  auto baseF = BB->getBaseFather();
  auto func = BB->getFunction();
  std::list<BaseBlock *> *BBs = nullptr;
  if (baseF == nullptr) {
    BBs = &func->getBaseBlocks();
  } else if (baseF->isIfBlock()) {
    auto ifBB = static_cast<IfBlock *>(baseF);
    for (auto _BB : ifBB->getIfBodyBBs()) {
      if (_BB == BB) {
        BBs = &ifBB->getIfBodyBBs();
        break;
      }
    }
    if (BBs == nullptr) {
      for (auto _BB : ifBB->getElseBodyBBs()) {
        if (_BB == BB) {
          BBs = &ifBB->getElseBodyBBs();
          break;
        }
      }
    }
  } else {
    BBs = &(static_cast<WhileBlock *>(baseF)->getBodyBBs());
  }
  assert(BBs && "should find the BB List");

  auto newBB = BasicBlock::create(m_, "postBBofAP");
  if (baseF != nullptr)
    newBB->setBaseFather(baseF);
  else
    newBB->setFunction(func);
  auto it = BBs->begin();
  for (; it != BBs->end(); it++) {
    if (*it == BB) {
      BBs->insert(++it, newBB);
      break;
    }
  }
  return newBB;
}

bool AccumulatePattern::isSameAddr(Value *val, Value *val2) {

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

bool AccumulatePattern::isSameOffsetPattern(Value *val, Value *val2) {
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

std::vector<Instruction *> AccumulatePattern::getRelatedInsInOrder(Value *val) {
  auto ins = dynamic_cast<Instruction *>(val);
  assert(ins && "expect get a ins Val");
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

bool AccumulatePattern::nestedLoop(IfBlock *ifBB) {
  for (auto BB : ifBB->getIfBodyBBs()) {
    if (BB->isWhileBlock()) {
      return true;
    } else if (BB->isIfBlock() && nestedLoop(static_cast<IfBlock *>(BB))) {
      return true;
    }
  }
  for (auto BB : ifBB->getElseBodyBBs()) {
    if (BB->isWhileBlock()) {
      return true;
    } else if (BB->isIfBlock() && nestedLoop(static_cast<IfBlock *>(BB))) {
      return true;
    }
  }
  return false;
}
