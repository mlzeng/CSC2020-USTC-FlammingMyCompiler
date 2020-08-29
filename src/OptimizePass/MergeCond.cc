#include "MergeCond.h"
#include <algorithm>
// #define DEBUG

void MergeCond::run() {
  for (auto fun : m_->getFunctions()) {
    func_ = fun;
    runFun();
  }
}

void MergeCond::runFun() {
  auto &list = func_->getBaseBlocks();
  for (auto basebb = list.begin(); basebb != list.end(); basebb++) {
    if ((*basebb)->isIfBlock()) {
      if (runIf(dynamic_cast<IfBlock *>(*basebb), MergeCond::FUNCTION)) {
        auto deletebb = basebb;
        basebb--;
        func_->removeBaseBlock(*deletebb);
      }
    } else if ((*basebb)->isWhileBlock()) {
      runWhile(dynamic_cast<WhileBlock *>(*basebb), MergeCond::FUNCTION);
    }
  }
}

bool MergeCond::runIf(IfBlock *ifbb, mother_type ty) {
  if (ifbb->getElseBodyBBs().empty()) {
    bool allif = true;
    for (auto basebb : ifbb->getIfBodyBBs()) {
      if (basebb->isIfBlock()) {
        if (dynamic_cast<IfBlock *>(basebb)->getElseBodyBBs().empty()) {
          continue;
        }
      }
      allif = false;
      break;
    }
    if (allif) {
      auto &list_ifbody = ifbb->getIfBodyBBs();
      for (auto thenbb = list_ifbody.begin(); thenbb != list_ifbody.end();
           thenbb++) {
        assert((*thenbb)->isIfBlock());
        if (runIf(dynamic_cast<IfBlock *>(*thenbb), MergeCond::IF_THEN)) {
          auto deletebb = thenbb;
          thenbb--;
          ifbb->removeIfBodyBB(*deletebb);
        }
      }

      for (auto thenbb : ifbb->getIfBodyBBs()) {
        auto sub_ifbb = dynamic_cast<IfBlock *>(thenbb);
        assert(sub_ifbb && "error in merge cond in runif");
        assert(ifbb->getCondBBs().size() == 1 &&
               sub_ifbb->getCondBBs().size() == 1);
        auto condbb = dynamic_cast<BasicBlock *>(ifbb->getCondBBs().front());
        auto sub_condbb =
            dynamic_cast<BasicBlock *>(sub_ifbb->getCondBBs().front());
        auto cmp = condbb->getInstructions().back();
        auto sub_cmp = sub_condbb->getInstructions().back();

        std::map<Value *, Value *> old2new;
        for (auto instr : condbb->getInstructions()) {
#ifdef DEBUG
          if (instr->getName() == "78") {
            std::cerr << sub_condbb->getName() << std::endl;
          }
#endif
          auto new_instr = instr->copy(sub_condbb);
          old2new[instr] = new_instr;
          // sub_condbb->addInstruction(instr);
        }

        for (auto instr : condbb->getInstructions()) {
          Instruction *new_inst = dynamic_cast<Instruction *>(old2new[instr]);
          if (new_inst) {
            int i = 0;
            for (auto op : instr->getOperands()) {
              Value *new_op;
              if (dynamic_cast<Constant *>(op) ||
                  dynamic_cast<GlobalVariable *>(op) ||
                  dynamic_cast<Function *>(op) ||
                  dynamic_cast<AllocaInst *>(op)) {
                new_op = op;
              } else {
                new_op = old2new[op];
                if (!new_op) {
                  std::cerr << instr->getName() << std::endl;
                  abort();
                }
              }
              new_inst->setOperand(i, new_op);
              i++;
            }
          }
        }

        auto new_and = BinaryInst::createAnd(old2new[cmp], sub_cmp, sub_condbb);
        auto new_zext =
            CastInst::createZExt(new_and, m_->getInt32Ty(), sub_condbb);
        auto new_cmp = CmpInst::createCmp(
            CmpInst::NE, new_zext, ConstantInt::get(0, m_), sub_condbb, m_);
      }

      for (auto thenbb : ifbb->getIfBodyBBs()) {
        assert(thenbb->isIfBlock());
        switch (ty) {
        case MergeCond::FUNCTION: {
          auto &list_0 = ifbb->getFunction()->getBaseBlocks();
          assert(std::find(list_0.begin(), list_0.end(), ifbb) != list_0.end());
          ifbb->getFunction()->insertBaseBlock(
              std::find(list_0.begin(), list_0.end(), ifbb), thenbb);
        } break;
        case MergeCond::IF_THEN: {
          auto father = ifbb->getBaseFather();
          assert(ifbb->isIfBlock());
          auto &list_1 = dynamic_cast<IfBlock *>(father)->getIfBodyBBs();
          assert(std::find(list_1.begin(), list_1.end(), ifbb) != list_1.end());
          dynamic_cast<IfBlock *>(father)->insertIfBodyBB(
              std::find(list_1.begin(), list_1.end(), ifbb), thenbb);
        } break;
        case MergeCond::IF_ELSE: {
          auto father = ifbb->getBaseFather();
          assert(ifbb->isIfBlock());
          auto &list_2 = dynamic_cast<IfBlock *>(father)->getElseBodyBBs();
          assert(std::find(list_2.begin(), list_2.end(), ifbb) != list_2.end());
          dynamic_cast<IfBlock *>(father)->insertElseBodyBB(
              std::find(list_2.begin(), list_2.end(), ifbb), thenbb);
        } break;
        case MergeCond::WHILE_BODY: {
          auto father = ifbb->getBaseFather();
          assert(ifbb->isIfBlock());
          auto &list_3 = dynamic_cast<WhileBlock *>(father)->getBodyBBs();
          assert(std::find(list_3.begin(), list_3.end(), ifbb) != list_3.end());
          dynamic_cast<WhileBlock *>(father)->insertWhileBodyBB(
              std::find(list_3.begin(), list_3.end(), ifbb), thenbb);
        } break;
        default:
          break;
        }
        //维护thenbb的father
      }

      return true;
    } else {
      auto &list = ifbb->getIfBodyBBs();
      for (auto basebb = list.begin(); basebb != list.end(); basebb++) {
        if ((*basebb)->isIfBlock()) {
          if (runIf(dynamic_cast<IfBlock *>(*basebb), MergeCond::IF_THEN)) {
            auto deletebb = basebb;
            basebb--;
            ifbb->removeIfBodyBB(*deletebb);
          }
        } else if ((*basebb)->isWhileBlock()) {
          runWhile(dynamic_cast<WhileBlock *>(*basebb), MergeCond::IF_THEN);
        }
      }
    }

    return false;
  } else {
    auto &list = ifbb->getIfBodyBBs();
    for (auto basebb = list.begin(); basebb != list.end(); basebb++) {
      if ((*basebb)->isIfBlock()) {
        if (runIf(dynamic_cast<IfBlock *>(*basebb), MergeCond::IF_THEN)) {
          auto deletebb = basebb;
          basebb--;
          ifbb->removeIfBodyBB(*deletebb);
        }
      } else if ((*basebb)->isWhileBlock()) {
        runWhile(dynamic_cast<WhileBlock *>(*basebb), MergeCond::IF_THEN);
      }
    }

    auto &list_1 = ifbb->getElseBodyBBs();
    for (auto basebb = list_1.begin(); basebb != list_1.end(); basebb++) {
      if ((*basebb)->isIfBlock()) {
        if (runIf(dynamic_cast<IfBlock *>(*basebb), MergeCond::IF_ELSE)) {
          auto deletebb = basebb;
          basebb--;
          ifbb->removeElseBodyBB(*deletebb);
        }
      } else if ((*basebb)->isWhileBlock()) {
        runWhile(dynamic_cast<WhileBlock *>(*basebb), MergeCond::IF_ELSE);
      }
    }
  }
  return false;
}

void MergeCond::runWhile(WhileBlock *whilebb, mother_type ty) {
  auto &list = whilebb->getBodyBBs();
  for (auto basebb = list.begin(); basebb != list.end(); basebb++) {
    if ((*basebb)->isIfBlock()) {
      if (runIf(dynamic_cast<IfBlock *>(*basebb), MergeCond::WHILE_BODY)) {
        auto deletebb = basebb;
        basebb--;
        whilebb->removeWhileBodyBB(*deletebb);
      }
    } else if ((*basebb)->isWhileBlock()) {
      runWhile(dynamic_cast<WhileBlock *>(*basebb), MergeCond::WHILE_BODY);
    }
  }
}