#include "BasicBlock.h"
#include "Function.h"
#include "Instruction.h"
#include "Module.h"
#include "unordered_set"
#include <cassert>
#include <unordered_map>

static void _printTest(Function *func);
// #define DEBUG

bool needRescan;

bool inPrePathOtherThan(
    Value *val, BasicBlock *BB, BasicBlock *except,
    std::unordered_map<BasicBlock *, std::unordered_set<Value *>> *referenced) {
  std::unordered_set<BasicBlock *> accessed{BB};
  std::unordered_set<BasicBlock *> toCheck;

  for (auto arg : BB->getParent()->getArgs()) { // TODO 待优化
    if (val == arg)
      return true;
  }
  for (auto postBB : BB->getSuccBasicBlocks()) {
    toCheck.insert(postBB);
  }
  // DEBUG
  // std::cout << "inPrePathOtherThan: check";
  // val->print();putchar('|');
  while (toCheck.size()) {
    auto item = *toCheck.begin();
    accessed.insert(item);
    if (item->getDefinedVals().find(val) != item->getDefinedVals().end())
      return false;
    if (referenced->at(item).find(val) != referenced->at(item).end())
      return true;
    for (auto tmp : item->getSuccBasicBlocks()) {
      if (accessed.find(tmp) != accessed.end())
        continue;
      toCheck.insert(tmp);
    }
    toCheck.erase(item);
  }

  return false;
}

void BasicBlock::initDefinedVals(std::unordered_set<Value *> *valSet) {
  definedVals.clear();
  for (auto instr : getInstructions()) {
    if (valSet->find(instr) != valSet->end())
      definedVals.insert(instr);
  }
}

void BasicBlock::initAuxilliaryValsOfActiveSet(
    std::unordered_set<Value *> *valSet,
    std::unordered_map<BasicBlock *, std::unordered_set<Value *>>
        *referenced) { // TODO 更改名字
  inheritedVals.clear();
  activeIn.clear();
  activeOut.clear();

  for (auto instr : getInstructions()) { // 先使用，后定义，除了PHI，应该没有

    if (instr->isPHI()) {
      auto vec = instr->getOperands();
      for (int i = 0; i < vec.size(); i += 2) {
        // assert(vec[i]->getType()->isIntegerTy());
        if (valSet->find(vec[i]) != valSet->end()) {
          if (!inPrePathOtherThan(vec[i], this,
                                  static_cast<BasicBlock *>(vec[i + 1]),
                                  referenced))
            inheritedVals.insert(
                {vec[i], static_cast<BasicBlock *>(vec[i + 1])});
          else
            inheritedVals.insert({vec[i], nullptr});
        }
      }
    } else {
      for (auto val : instr->getOperands()) {
        // auto tmp = dynamic_cast<ConstantInt*>(val);
        if (valSet->find(val) != valSet->end() &&
            definedVals.find(val) == definedVals.end())
          inheritedVals.insert({val, nullptr});
      }
    }
  }
  for (auto val : inheritedVals) {
    activeIn.insert(val.first);
  }
}

void initSet(Module *m) {
  for (auto func : m->getFunctions()) {
    if (!func->getBasicBlocks().size())
      continue;

    std::unordered_set<Value *> definedVals;
    std::unordered_map<BasicBlock *, std::unordered_set<Value *>> refrecnced;
    for (auto arg : func->getArgs()) {
      definedVals.insert(arg);
    }
    for (auto BB : func->getBasicBlocks()) {
      refrecnced.insert({BB, std::unordered_set<Value *>()});
      for (auto instr : BB->getInstructions()) {
        if (!instr->isVoid() && !instr->isAlloca()) {
          definedVals.insert(instr);
          refrecnced[BB].insert(instr);
        }
      }
    }
    for (auto BB : func->getBasicBlocks()) {
      refrecnced.insert({BB, std::unordered_set<Value *>()});
      for (auto instr : BB->getInstructions()) {
        if (definedVals.find(instr) != definedVals.end()) {
          refrecnced[BB].insert(instr);
        }
        for (auto val : instr->getOperands()) {
          if (definedVals.find(val) != definedVals.end())
            refrecnced[BB].insert(val);
        }
      }
    }

    for (auto BB : func->getBasicBlocks()) {
      BB->initDefinedVals(&definedVals);
    }

    for (auto BB : func->getBasicBlocks()) {
      BB->initAuxilliaryValsOfActiveSet(&definedVals, &refrecnced);
    }
  }
}

void ActiveVars(Module *m) {
  // std::unordered_map<Value*, BasicBlock*> ;
  initSet(m);

  for (auto func : m->getFunctions()) {
    if (func->getBasicBlocks().size() == 0)
      continue;
    needRescan = true;
#ifdef DEBUG
    std::cout << "name:" << func->getName() << "\n";
#endif
    while (needRescan) {
#ifdef DEBUG
      getchar();
      _printTest(func);
#endif
      needRescan = false;
      for (auto BB : func->getBasicBlocks()) {
        auto &curOut = BB->getActiveOuts();
        for (auto successor : BB->getSuccBasicBlocks()) {
          for (auto inItem : successor->getActiveIns()) {
            auto inheritedVals = successor->getInheritedVals();
            // TODO 改进判断
            if (inheritedVals.find(inItem) != inheritedVals.end() &&
                (inheritedVals[inItem] != BB &&
                 inheritedVals[inItem] != nullptr))
              continue;
            // BB->addActiveOut(inItem);
            curOut.insert(inItem);
#ifdef DEBUG
            std::cout << "Insert:\t";
            inItem->print();
            std::cout << "\tto\t";
            BB->print();
            std::cout << "\tOutSet\n";
#endif
          }
        }
        auto &curIn = BB->getActiveIns();
        auto curDefinedVals = BB->getDefinedVals();
        for (auto outItem : curOut) {
          if (curDefinedVals.find(outItem) == curDefinedVals.end() &&
              curIn.find(outItem) == curIn.end()) {
#ifdef DEBUG
            std::cout << "Insert:\t";
            outItem->print();
            std::cout << "\tto\t";
            BB->print();
            std::cout << "\tInSet\n";
#endif

            // BB->addActiveIn(outItem);
            curIn.insert(outItem);
            needRescan = true;
          }
        }
      }
    }

    // getchar();
    // _printTest(func);
  }
}

static void _printTest(Function *func) {
  std::cout << "\n--------------------------\n";
  for (auto BB : func->getBasicBlocks()) {
    BB->print();
    std::cout << "\nInherited\n";
    for (auto val : BB->getInheritedVals()) {
      val.first->print();
    }
    std::cout << "\ndefinded\n";
    for (auto val : BB->getDefinedVals()) {
      val->print();
    }
    std::cout << "\nIns:\n";
    for (auto val : BB->getActiveIns()) {
      val->print();
    }
    std::cout << "\nOuts\n";
    for (auto val : BB->getActiveOuts()) {
      val->print();
    }
    std::cout << "\n\n";
    // getchar();
  }
}
