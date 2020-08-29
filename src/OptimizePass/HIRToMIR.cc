#include "HIRToMIR.h"
#include "HighBlock.h"
#include "ReturnVal.h"

void HIRToMIR::run() {
  for (auto func : m_->getFunctions()) {
    auto basebbs = func->getBaseBlocks();
    auto &basicbbs = func->getBasicBlocks();
    BasicBlock *next_bb = nullptr;
    for (auto iter = basebbs.rbegin(); iter != basebbs.rend(); ++iter) {
      auto basebb = *iter;
      next_bb = genBaseBlock(basebb, next_bb, nullptr, nullptr, func);
    }
    func->getBaseBlocks().clear();
  }
  m_->setIRLevel(Module::MIR_MEM);
}

BasicBlock *HIRToMIR::genBaseBlock(BaseBlock *basebb, BasicBlock *next_bb,
                                   BasicBlock *while_entry,
                                   BasicBlock *while_exit, Function *func) {
  auto &basicbbs = func->getBasicBlocks();
  if (basebb->isBasicBlock()) {
    auto this_bb = dynamic_cast<BasicBlock *>(basebb);
    if (next_bb == nullptr && this_bb->getTerminator() == nullptr &&
        basicbbs.empty()) {
      if (func->getResultType()->isIntegerTy())
        auto ret = ReturnInst::createRet(ConstantInt::get(0, m_), this_bb);
      else
        auto ret = ReturnInst::createVoidRet(this_bb);
    } else if (this_bb->getTerminator() == nullptr) {
      auto inst = this_bb->getInstructions().back();
      BasicBlock *target = next_bb;
      if (inst) {
        if (inst->isBreak()) {
          exit_ifnot(_whileExit_genBaseBlock_HIRTOMIR, while_exit);
          target = while_exit;
          this_bb->deleteInstr(inst);
        } else if (inst->isContinue()) {
          exit_ifnot(_whileEntry_genBaseBlock_HIRTOMIR, while_entry);
          target = while_entry;
          this_bb->deleteInstr(inst);
        }
      }
      auto br = BranchInst::createBr(target, this_bb);
    }
    basicbbs.push_front(this_bb);
    return this_bb;
  } else if (basebb->isIfBlock()) {
    auto ifblock = dynamic_cast<IfBlock *>(basebb);
    BasicBlock *if_true = nullptr;
    BasicBlock *if_false = nullptr;
    auto else_body = ifblock->getElseBodyBBs();
    auto then_body = ifblock->getIfBodyBBs();
    auto if_cond = ifblock->getCondBBs();

    if (next_bb == nullptr) {
      auto bb = BasicBlock::create(m_, "");
      bb->setFunction(func);
      func->addBasicBlock(bb);
      if (func->getResultType()->isIntegerTy())
        auto ret = ReturnInst::createRet(ConstantInt::get(0, m_), bb);
      else
        auto ret = ReturnInst::createVoidRet(bb);
      next_bb = bb;
    }

    BasicBlock *cur_next = next_bb;
    for (auto iter = else_body.rbegin(); iter != else_body.rend(); ++iter) {
      cur_next = genBaseBlock(*iter, cur_next, while_entry, while_exit, func);
    }
    if_false = cur_next;

    cur_next = next_bb;
    for (auto iter = then_body.rbegin(); iter != then_body.rend(); ++iter) {
      cur_next = genBaseBlock(*iter, cur_next, while_entry, while_exit, func);
    }
    if_true = cur_next;

    exit_ifnot(_condSizeMoreThanOne_genBaseBlock_HIRTOMIR, if_cond.size() == 1);
    auto cond_bb = dynamic_cast<BasicBlock *>(if_cond.front());
    exit_ifnot(_CondBegin_genBaseBlock_HIRTOMIR, cond_bb);
    auto cond = cond_bb->getInstructions().back();
    auto br = BranchInst::createCondBr(cond, if_true, if_false, cond_bb);
    basicbbs.push_front(cond_bb);
    return cond_bb;
  } else if (basebb->isWhileBlock()) {
    auto while_block = dynamic_cast<WhileBlock *>(basebb);
    auto while_body = while_block->getBodyBBs();
    auto while_cond = while_block->getCondBBs();

    if (next_bb == nullptr) {
      auto bb = BasicBlock::create(m_, "");
      bb->setFunction(func);
      func->addBasicBlock(bb);
      if (func->getResultType()->isIntegerTy())
        auto ret = ReturnInst::createRet(ConstantInt::get(0, m_), bb);
      else
        auto ret = ReturnInst::createVoidRet(bb);
      next_bb = bb;
    }

    BasicBlock *if_true = nullptr;
    BasicBlock *if_false = next_bb;

    exit_ifnot(_condSizeMoreThanOne_genBaseBlock_HIRTOMIR,
               while_cond.size() == 1);
    auto cond_bb = dynamic_cast<BasicBlock *>(while_cond.front());
    exit_ifnot(_CondBegin_genBaseBlock_HIRTOMIR, cond_bb);

    BasicBlock *cur_next = cond_bb;
    for (auto iter = while_body.rbegin(); iter != while_body.rend(); ++iter) {
      cur_next = genBaseBlock(*iter, cur_next, cond_bb, next_bb, func);
    }
    if_true = cur_next;

    auto cond = cond_bb->getInstructions().back();
    auto br = BranchInst::createCondBr(cond, if_true, if_false, cond_bb);
    basicbbs.push_front(cond_bb);
    return cond_bb;
  } else {
    std::cerr << "Unknown baseblock" << std::endl;
    exit_ifnot(_UnoknowBaseBB_genBaseBlock_HIRTOMIR, false);
  }
  return nullptr;
}