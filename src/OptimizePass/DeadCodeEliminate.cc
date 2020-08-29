#include "DeadCodeEliminate.h"
#include "ReturnVal.h"

void DeadCodeEliminate::run() {
  deleteDeadFunc(m_);
  detectNotSideEffectFunc(m_);
  for (auto func : m_->getFunctions()) {
    if (func != m_->getMainFunction())
      deleteDeadRet(func);
  }
  for (auto func : m_->getFunctions()) {
    deleteDeadInst(func);
    deleteDeadStore(func);
    deleteDeadInst(func);
  }
  // printStats();
}

bool DeadCodeEliminate::isLocalStore(StoreInst *store) {
  auto addr = dynamic_cast<Instruction *>(getFirstAddr(store->getLVal()));
  if (addr && addr->isAlloca())
    return true;
  return false;
}

void DeadCodeEliminate::detectNotSideEffectFunc(Module *m) {
  for (auto func : m->getFunctions()) {
    if (func->getNumBasicBlocks() == 0 || func == m->getMainFunction())
      continue;
    bool side_effect = false;
    for (auto bb : func->getBasicBlocks()) {
      for (auto inst : bb->getInstructions()) {
        if (inst->isStore()) {
          if (m->isMIRSSALevel() &&
              isLocalStore(static_cast<StoreInst *>(inst)))
            continue;
          side_effect = true;
          break;
        } else if (inst->isCall()) {
          side_effect = true;
          break;
        }
      }
    }
    if (side_effect == false)
      notSideEffectFunc.insert(func);
  }
}

bool DeadCodeEliminate::isDeadInstruction(Instruction *inst) {
  if (inst->getUseList().empty()) {
    switch (inst->getInstrType()) {
    case Instruction::Store:
    case Instruction::Call:
    case Instruction::Ret:
    case Instruction::Br:
      return false;
      break;

    default:
      return true;
    }
  }
  return false;
}

bool DeadCodeEliminate::isSideEffectAndCall(Instruction *inst) {
  switch (inst->getInstrType()) {
  case Instruction::Store:
  case Instruction::Call:
  case Instruction::Ret:
  case Instruction::Br:
    return true;
    break;

  default:
    return false;
  }
  return false;
}

bool DeadCodeEliminate::isSideEffect(Instruction *inst) {
  CallInst *call;
  switch (inst->getInstrType()) {
  case Instruction::Ret:
  case Instruction::Br:
  case Instruction::Store:
    return true;
    break;
  case Instruction::Call:
    call = static_cast<CallInst *>(inst);
    return isSideEffectFunc(call->getFunction());
    break;
  default:
    return false;
  }
  return false;
}

void DeadCodeEliminate::deleteDeadFunc(Module *m) {
  func_counter = 0;
  auto &funcs = m->getFunctions();
  for (auto iter = funcs.begin(); iter != funcs.end();) {
    auto func = *iter;
    if (func->getBasicBlocks().empty()) {
      ++iter;
      continue;
    }
    if (func->getUseList().empty() && func != m_->getMainFunction()) {
      func_counter++;
      iter = funcs.erase(iter);
    } else
      ++iter;
  }
}

void DeadCodeEliminate::markUse(Instruction *inst,
                                std::unordered_set<Instruction *> &worklist) {
  if (worklist.find(inst) != worklist.end())
    return;
  worklist.insert(inst);
  for (auto op : inst->getOperands()) {
    auto op_inst = dynamic_cast<Instruction *>(op);
    if (op_inst) {
      markUse(op_inst, worklist);
    }
  }
}

void DeadCodeEliminate::deleteDeadInst(Function *func) {
  std::unordered_set<Instruction *> worklist;
  for (auto bb : func->getBasicBlocks()) {
    for (auto inst : bb->getInstructions()) {
      if (isSideEffect(inst)) {
        markUse(inst, worklist);
      }
    }
  }
  for (auto bb : func->getBasicBlocks()) {
    std::unordered_set<Instruction *> removelist;
    for (auto inst : bb->getInstructions()) {
      if (worklist.find(inst) == worklist.end()) {
        removelist.insert(inst);
      } else
        exit_ifnot(_DeadInsButNotDelete_deleteDeadInst_DeadCodeEliminate,
                   !isDeadInstruction(inst));
    }
    for (auto inst : removelist) {
      bb->deleteInstr(inst);
      inst_counter++;
    }
  }
}

bool DeadCodeEliminate::isEqualStorePtr(StoreInst *store_a,
                                        StoreInst *store_b) {
  if (store_a->getLVal() != store_b->getLVal())
    return false;
  if (store_a->hasOffset() && store_b->hasOffset()) {
    if (store_a->getOffset() != store_b->getOffset())
      return false;
    int shift_a = 0, shift_b = 0;
    if (store_a->hasShift())
      shift_a = dynamic_cast<ConstantInt *>(store_a->getShift())->getValue();
    if (store_b->hasShift())
      shift_b = dynamic_cast<ConstantInt *>(store_b->getShift())->getValue();
    return shift_a == shift_b;
  } else if (store_a->hasOffset() || store_b->hasOffset())
    return false;
  return true;
}

// not precise, equal if the first addr of both arrays is equal
bool DeadCodeEliminate::isEqualStoreLoadPtr(StoreInst *store, LoadInst *load) {
  if (store->getLVal() != load->getLVal()) {
    return getFirstAddr(store->getLVal()) == getFirstAddr(load->getLVal());
  }
  return true;
}

bool DeadCodeEliminate::isStrictEqualStoreLoadPtr(StoreInst *store,
                                                  LoadInst *load) {
  if (store->getLVal() != load->getLVal()) {
    return false;
  }
  if (store->hasOffset() && load->hasOffset()) {
    if (store->getOffset() != load->getOffset())
      return false;
    int shift_a = 0, shift_b = 0;
    if (store->hasShift())
      shift_a = dynamic_cast<ConstantInt *>(store->getShift())->getValue();
    if (load->hasShift())
      shift_b = dynamic_cast<ConstantInt *>(load->getShift())->getValue();
    return shift_a == shift_b;
  } else if (store->hasOffset() || load->hasOffset())
    return false;
  return true;
}

Value *DeadCodeEliminate::getFirstAddr(Value *v) {
  auto inst = dynamic_cast<Instruction *>(v);
  if (inst) {
    if (inst->isAlloca())
      return inst;
    else if (inst->isGEP())
      return getFirstAddr(inst->getOperand(0));
    else if (inst->isLoad())
      return v;
    else {
      for (auto op : inst->getOperands()) {
        if (op->getType()->isPointerTy())
          return getFirstAddr(op);
      }
    }
  }
  return v;
}

void DeadCodeEliminate::deleteDeadStore(Function *func) {
  for (auto bb : func->getBasicBlocks()) {
    std::unordered_set<StoreInst *> pre_stores;
    std::vector<Instruction *> wait_remove;
    for (auto inst : bb->getInstructions()) {
      if (inst->isStore()) {
        auto new_store = static_cast<StoreInst *>(inst);
        StoreInst *pre_store = nullptr;
        for (auto pre : pre_stores) {
          if (isEqualStorePtr(pre, new_store)) {
            wait_remove.push_back(pre);
            pre_store = pre;
            break;
          }
        }
        if (pre_store)
          pre_stores.erase(pre_store);
        pre_stores.insert(new_store);
      } else if (inst->isLoad()) {
        auto load = static_cast<LoadInst *>(inst);
        std::vector<StoreInst *> removes;
        for (auto pre : pre_stores) {
          if (isStrictEqualStoreLoadPtr(pre, load)) {
            exit_ifnot(_deleteDeadStore_DeadCodeEliminate,
                       wait_remove.empty() || load != wait_remove.back());
            load->replaceAllUseWith(pre->getRVal());
            wait_remove.push_back(load);
          } else if (isEqualStoreLoadPtr(pre, load)) {
            removes.push_back(pre);
          }
        }
        for (auto r : removes)
          pre_stores.erase(r);
      } else if (inst->isCall()) {
        auto call = static_cast<CallInst *>(inst);
        if (isSideEffectFunc(call->getFunction())) {
          pre_stores.clear();
        }
      }
    }
    for (auto inst : wait_remove) {
      bb->deleteInstr(inst);
      store_counter++;
    }
  }
}

void DeadCodeEliminate::printStats() {
  std::cerr << "DeadCodeEliminate: remove " << func_counter << " functions, "
            << inst_counter << " instructions, " << store_counter << " stores."
            << std::endl;
}

void DeadCodeEliminate::deleteDeadRet(Function *func) {
  bool flag = true;
  for (auto use : func->getUseList()) {
    if (!use.val_->getUseList().empty())
      flag = false;
  }
  if (flag) {
    for (auto bb : func->getBasicBlocks()) {
      for (auto inst : bb->getInstructions()) {
        if (inst->isRet() && inst->getNumOperand() == 1) {
          inst->removeUseOfOps();
          inst->setOperand(0, ConstantInt::get(0, m_));
        }
      }
    }
  }
}
