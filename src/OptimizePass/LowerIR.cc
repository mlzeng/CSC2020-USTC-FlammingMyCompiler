#include "LowerIR.h"
#include "BBConstPropagation.h"
#include "ReturnVal.h"
#include <cmath>

extern bool disable_div_optimization;

void LowerIR::run() {
  BBConstPropagation const_fold(m_);
  for (auto f : m_->getFunctions()) {
    for (auto bb : f->getBasicBlocks()) {
      fuseCmpBr(bb);
      splitGEP(bb);
      if (!disable_div_optimization)
        convertRemToAnd(bb);
      splitRem(bb);
    }
  }
  const_fold.run();
  for (auto f : m_->getFunctions()) {
    for (auto bb : f->getBasicBlocks()) {
      removeUnusedOp(bb);
    }
  }
  for (auto f : m_->getFunctions()) {
    for (auto bb : f->getBasicBlocks()) {
      fuseAddLoadStore(bb);
    }
  }
  for (auto f : m_->getFunctions()) {
    for (auto bb : f->getBasicBlocks()) {
      if (!disable_div_optimization)
        convertMulDivToShift(bb);
      fuseConstShift(bb);
      fuseShiftLoadStore(bb);
    }
  }
  for (auto f : m_->getFunctions()) {
    for (auto bb : f->getBasicBlocks()) {
      fuseMulAdd(bb);
    }
  }
  // add has three ops
  for (auto f : m_->getFunctions()) {
    for (auto bb : f->getBasicBlocks()) {
      fuseShiftArithmetic(bb);
    }
  }
  m_->setIRLevel(Module::LIR);
}

void LowerIR::fuseCmpBr(BasicBlock *bb) {
  auto t = bb->getTerminator();
  exit_ifnot(_fuseCmpBr_LowerIR, t);
  if (t->isBr()) {
    auto br = static_cast<BranchInst *>(t);
    if (br->isCondBr()) {
      auto inst = static_cast<Instruction *>(br->getOperand(0));
      if (inst->isCmp()) {
        auto cmp = static_cast<CmpInst *>(br->getOperand(0));
        if (cmp->getParent() == bb && cmp->getUseList().size() == 1) {
          br->fuseCmpInst();
          bb->deleteInstr(cmp);
          delete cmp;
        }
      }
    }
  }
}

void LowerIR::fuseMulAdd(BasicBlock *bb) {
  auto &insts = bb->getInstructions();
  for (auto iter = insts.begin(); iter != insts.end(); ++iter) {
    auto inst = *iter;
    if (inst->isAdd()) {
      auto op1 = dynamic_cast<Instruction *>(inst->getOperand(0));
      if (op1 && op1->isMul() && (op1->getParent() == bb) &&
          (op1->getUseList().size() == 1)) {
        auto muladd = MulAddInst::createMulAddInst(
            op1->getOperand(0), op1->getOperand(1), inst->getOperand(1));
        iter = insts.erase(iter);
        iter = insts.insert(iter, muladd);
        muladd->setParent(bb);
        bb->deleteInstr(op1);
        inst->replaceAllUseWith(muladd);
        inst->removeUseOfOps();
        continue;
      }
      auto op2 = dynamic_cast<Instruction *>(inst->getOperand(1));
      if (op2 && op2->isMul() && (op2->getParent() == bb) &&
          (op2->getUseList().size() == 1)) {
        auto muladd = MulAddInst::createMulAddInst(
            op2->getOperand(0), op2->getOperand(1), inst->getOperand(0));
        iter = insts.erase(iter);
        iter = insts.insert(iter, muladd);
        muladd->setParent(bb);
        bb->deleteInstr(op2);
        inst->replaceAllUseWith(muladd);
        inst->removeUseOfOps();
      }
    }
  }
}

void LowerIR::splitGEP(BasicBlock *bb) {
  auto &insts = bb->getInstructions();
  for (auto iter = insts.begin(); iter != insts.end();) {
    auto inst = *iter;
    if (inst->isGEP() && (inst->getNumOperand() == 2)) {
      auto size = inst->getType()->getPointerElementType()->getSize();
      auto offset = BinaryInst::createMul(inst->getOperand(1),
                                          ConstantInt::get(size, m_));
      offset->setParent(bb);
      auto addaddr = BinaryInst::createAdd(inst->getOperand(0), offset);
      addaddr->setParent(bb);
      insts.insert(iter, offset);
      insts.insert(iter, addaddr);
      inst->replaceAllUseWith(addaddr);
      inst->removeUseOfOps();
      iter = insts.erase(iter);
    } else if (inst->isGEP()) {
      std::cerr << "GEP have more than 2 operands ???" << std::endl;
      exit_ifnot(_splitGEP_LowerIR, false);
    } else {
      ++iter;
    }
  }
}

// Function to check if x is power of 2
bool isPowerOfTwo(int x) {
  // First x in the below expression is for the case when x is 0
  return x && (!(x & (x - 1)));
}

void LowerIR::convertMulDivToShift(BasicBlock *bb) {
  auto &insts = bb->getInstructions();
  for (auto iter = insts.begin(); iter != insts.end();) {
    auto inst = *iter;
    if (inst->isMul()) {
      auto op1 = dynamic_cast<Constant *>(inst->getOperand(0));
      if (op1) {
        exit_ifnot(_convertMulDivToShift_LowerIR,
                   op1->getType()->isIntegerTy());
        int v = static_cast<ConstantInt *>(op1)->getValue();
        if (isPowerOfTwo(v)) {
          int p = (int)std::ceil(std::log2(v));
          exit_ifnot(_convertMulDivToShift_LowerIR, p >= 1 && p <= 31);
          auto shl = BinaryInst::createShl(inst->getOperand(1),
                                           ConstantInt::get(p, m_));
          inst->replaceAllUseWith(shl);
          inst->removeUseOfOps();
          shl->setParent(bb);
          insts.insert(iter, shl);
          iter = insts.erase(iter);
          continue;
        }
      }
      auto op2 = dynamic_cast<Constant *>(inst->getOperand(1));
      if (op2) {
        exit_ifnot(_convertMulDivToShift_LowerIR,
                   op2->getType()->isIntegerTy());
        int v = static_cast<ConstantInt *>(op2)->getValue();
        if (isPowerOfTwo(v)) {
          int p = (int)std::ceil(std::log2(v));
          auto shl = BinaryInst::createShl(inst->getOperand(0),
                                           ConstantInt::get(p, m_));
          inst->replaceAllUseWith(shl);
          inst->removeUseOfOps();
          shl->setParent(bb);
          insts.insert(iter, shl);
          iter = insts.erase(iter);
          continue;
        }
      }
    } else if (inst->isDiv()) {
      auto op2 = dynamic_cast<Constant *>(inst->getOperand(1));
      if (op2) {
        exit_ifnot(_convertMulDivToShift_LowerIR,
                   op2->getType()->isIntegerTy());
        int v = static_cast<ConstantInt *>(op2)->getValue();
        if (isPowerOfTwo(v)) {
          int p = (int)std::ceil(std::log2(v));
          auto ashr = BinaryInst::createAShr(inst->getOperand(0),
                                             ConstantInt::get(p, m_));
          inst->replaceAllUseWith(ashr);
          inst->removeUseOfOps();
          ashr->setParent(bb);
          insts.insert(iter, ashr);
          iter = insts.erase(iter);
          continue;
        }
      }
    }
    ++iter;
  }
}

void LowerIR::convertRemToAnd(BasicBlock *bb) {
  auto &insts = bb->getInstructions();
  for (auto iter = insts.begin(); iter != insts.end();) {
    auto inst = *iter;
    if (inst->isRem()) {
      auto op2 = dynamic_cast<Constant *>(inst->getOperand(1));
      if (op2) {
        exit_ifnot(_convertRemToAnd_LowerIR, op2->getType()->isIntegerTy());
        int v = static_cast<ConstantInt *>(op2)->getValue();
        if (isPowerOfTwo(v)) {
          int p = v - 1;
          auto tmp = BinaryInst::createAnd(inst->getOperand(0),
                                           ConstantInt::get(p, m_));
          inst->replaceAllUseWith(tmp);
          inst->removeUseOfOps();
          tmp->setParent(bb);
          insts.insert(iter, tmp);
          iter = insts.erase(iter);
          continue;
        }
      }
    }
    ++iter;
  }
}

void LowerIR::fuseAddLoadStore(BasicBlock *bb) {
  for (auto inst : bb->getInstructions()) {
    if (inst->isLoad()) {
      auto load = static_cast<LoadInst *>(inst);
      if (!load->hasOffset()) {
        auto ptr = dynamic_cast<Instruction *>(load->getOperand(0));
        if (ptr && ptr->isAdd()) {
          exit_ifnot(_fuseAddLoadStore_LowerIR,
                     ptr->getOperand(0)->getType()->isPointerTy());
          load->removeUseOfOps(); // DeadCodeEliminate may remove add
          load->setNumOps(2);
          load->setLVal(ptr->getOperand(0));
          load->setOffset(ptr->getOperand(1));
        }
      }
    } else if (inst->isStore()) {
      auto store = static_cast<StoreInst *>(inst);
      if (!store->hasOffset()) {
        auto ptr = dynamic_cast<Instruction *>(store->getLVal());
        if (ptr && ptr->isAdd()) {
          exit_ifnot(_fuseAddLoadStore_LowerIR,
                     ptr->getOperand(0)->getType()->isPointerTy());
          ptr->removeUse(inst, 1); // DeadCodeEliminate may remove add
          store->setNumOps(3);
          store->setLVal(ptr->getOperand(0));
          store->setOffset(ptr->getOperand(1));
        }
      }
    }
  }
}

void LowerIR::fuseConstShift(BasicBlock *bb) {
  for (auto inst : bb->getInstructions()) {
    if (inst->isShl()) {
      auto lval = dynamic_cast<Instruction *>(inst->getOperand(0));
      if (lval && lval->isShl()) {
        auto const_val1 = dynamic_cast<ConstantInt *>(inst->getOperand(1));
        auto const_val2 = dynamic_cast<ConstantInt *>(lval->getOperand(1));
        if (const_val1 && const_val2) {
          inst->removeUseOfOps();
          inst->setOperand(0, lval->getOperand(0));
          inst->setOperand(1, ConstantInt::get(const_val1->getValue() +
                                                   const_val2->getValue(),
                                               m_));
        }
      }
    } else if (inst->isAShr()) {
      auto lval = dynamic_cast<Instruction *>(inst->getOperand(0));
      if (lval && lval->isAShr()) {
        auto const_val1 = dynamic_cast<ConstantInt *>(inst->getOperand(1));
        auto const_val2 = dynamic_cast<ConstantInt *>(lval->getOperand(1));
        if (const_val1 && const_val2) {
          inst->removeUseOfOps();
          inst->setOperand(0, lval->getOperand(0));
          inst->setOperand(1, ConstantInt::get(const_val1->getValue() +
                                                   const_val2->getValue(),
                                               m_));
        }
      }
    }
  }
}

void LowerIR::fuseShiftLoadStore(BasicBlock *bb) {
  auto &insts = bb->getInstructions();
  for (auto iter = insts.begin(); iter != insts.end();) {
    auto inst = *iter;
    if (inst->isLoad()) {
      auto load = static_cast<LoadInst *>(inst);
      if (load->hasOffset()) {
        auto offset = dynamic_cast<Instruction *>(load->getOffset());
        if (offset && offset->isShl()) {
          auto const_val = dynamic_cast<ConstantInt *>(offset->getOperand(1));
          if (const_val) {
            load->setNumOps(3);
            load->setOffset(offset->getOperand(0));
            load->setShift(const_val);
            // insts.remove(offset);
            // offset->removeUseOfOps();
            offset->removeUse(load, 1);
            continue;
          }
        }
      }
    } else if (inst->isStore()) {
      auto store = static_cast<StoreInst *>(inst);
      if (store->hasOffset()) {
        auto offset = dynamic_cast<Instruction *>(store->getOffset());
        if (offset && offset->isShl()) {
          auto const_val = dynamic_cast<ConstantInt *>(offset->getOperand(1));
          if (const_val) {
            store->setNumOps(4);
            store->setOffset(offset->getOperand(0));
            store->setShift(const_val);
            // insts.remove(offset);
            // offset->removeUseOfOps();
            offset->removeUse(store, 2);
            continue;
          }
        }
      }
    }
    ++iter;
  }
}

void LowerIR::removeUnusedOp(BasicBlock *bb) {
  auto &insts = bb->getInstructions();
  for (auto iter = insts.begin(); iter != insts.end();) {
    auto inst = *iter;
    bool remove = false;
    // remove a+0
    if (inst->isAdd()) {
      int i = 0;
      exit_ifnot(_removeUnusedOp_LowerIR, inst->getNumOperand() == 2);
      for (auto op : inst->getOperands()) {
        auto const_val = dynamic_cast<ConstantInt *>(op);
        if (const_val && const_val->getValue() == 0) {
          if (i == 0) {
            if (dynamic_cast<GlobalVariable *>(inst->getOperand(1)))
              break;
            inst->replaceAllUseWith(inst->getOperand(1));
          } else {
            exit_ifnot(_removeUnusedOp_LowerIR, i == 1);
            if (dynamic_cast<GlobalVariable *>(inst->getOperand(0)))
              break;
            inst->replaceAllUseWith(inst->getOperand(0));
          }
          inst->removeUseOfOps();
          iter = insts.erase(iter);
          remove = true;
          break;
        }
        i++;
      }
    }
    // remove a*0, a*1
    else if (inst->isMul()) {
      int i = 0;
      exit_ifnot(_removeUnusedOp_LowerIR, inst->getNumOperand() == 2);
      for (auto op : inst->getOperands()) {
        auto const_val = dynamic_cast<ConstantInt *>(op);
        if (const_val && const_val->getValue() == 0 &&
            const_val->getValue() == 1) {

          if (const_val->getValue() == 0) {
            inst->replaceAllUseWith(ConstantInt::get(0, m_));
          } else {
            if (i == 0) {
              inst->replaceAllUseWith(inst->getOperand(1));
            } else {
              exit_ifnot(_removeUnusedOp_LowerIR, i == 1);
              inst->replaceAllUseWith(inst->getOperand(0));
            }
          }

          inst->removeUseOfOps();
          iter = insts.erase(iter);
          remove = true;
          break;
        }
        i++;
      }
    }
    if (!remove)
      ++iter;
  }
}

void LowerIR::fuseShiftArithmetic(BasicBlock *bb) {
  for (auto inst : bb->getInstructions()) {
    if (inst->isAdd() && inst->getNumOperand() == 2) {
      auto op1 = dynamic_cast<Instruction *>(inst->getOperand(0));
      if (op1 && op1->isShl()) {
        auto const_val = dynamic_cast<ConstantInt *>(op1->getOperand(1));
        if (const_val) {
          inst->removeUseOfOps();
          inst->addOperand(const_val);
          inst->setOperand(0, inst->getOperand(1));
          inst->setOperand(1, op1->getOperand(0));
          continue;
        }
      }
      auto op2 = dynamic_cast<Instruction *>(inst->getOperand(1));
      if (op2 && op2->isShl()) {
        auto const_val = dynamic_cast<ConstantInt *>(op2->getOperand(1));
        if (const_val) {
          inst->removeUseOfOps();
          inst->addOperand(const_val);
          inst->setOperand(0, inst->getOperand(0));
          inst->setOperand(1, op2->getOperand(0));
        }
      }
    } else if (inst->isSub() && inst->isRSub() && inst->isAnd() &&
               inst->isOr() && inst->getNumOperand() == 2) {
      auto op2 = dynamic_cast<Instruction *>(inst->getOperand(1));
      if (op2 && op2->isShl()) {
        auto const_val = dynamic_cast<ConstantInt *>(op2->getOperand(1));
        if (const_val) {
          inst->removeUseOfOps();
          inst->addOperand(const_val);
          inst->setOperand(0, inst->getOperand(0));
          inst->setOperand(1, op2->getOperand(0));
        }
      }
    }
  }
}

void LowerIR::splitRem(BasicBlock *bb) {
  auto &insts = bb->getInstructions();
  for (auto iter = insts.begin(); iter != insts.end();) {
    auto inst = *iter;
    if (inst->isRem()) {
      auto op1 = inst->getOperand(0);
      auto op2 = inst->getOperand(1);
      auto div = BinaryInst::createDiv(op1, op2);
      div->setParent(bb);
      auto mul = BinaryInst::createMul(div, op2);
      mul->setParent(bb);
      auto sub = BinaryInst::createSub(op1, mul);
      sub->setParent(bb);
      insts.insert(iter, div);
      insts.insert(iter, mul);
      insts.insert(iter, sub);
      inst->replaceAllUseWith(sub);
      inst->removeUseOfOps();
      iter = insts.erase(iter);
    } else
      ++iter;
  }
}