#include "PowerArray.h"

// #define DEBUG
std::map<Value *, std::vector<std::pair<int, int>>> array_val;

void PowerArray::run() {
  for (auto func : m_->getFunctions()) {
    for (auto bb : func->getBasicBlocks()) {
      bb_ = bb;
      auto power_array = getPowerArray();

#ifdef DEBUG
      if (power_array == nullptr) {
        std::cerr << "debug in Power" << func->getName() << std::endl;
      }
#endif
      replaceWithShl(power_array);
    }
  }
}

Value *PowerArray::getPowerArray() {
  array_val.clear();
  for (auto instr : bb_->getInstructions()) {
    if (instr->isStore()) {
      auto store_lval = dynamic_cast<StoreInst *>(instr)->getLVal();
      auto store_rval = dynamic_cast<StoreInst *>(instr)->getRVal();
      auto store_lval_instr = dynamic_cast<Instruction *>(store_lval);
      if (store_lval_instr) {
        if (store_lval_instr->isGEP()) {
          auto array = getGepPtr(store_lval_instr);
          auto offset = getGepOffset(store_lval_instr);
          auto const_offset = dynamic_cast<ConstantInt *>(offset);
          auto const_store_rval = dynamic_cast<ConstantInt *>(store_rval);
          if (const_offset && const_store_rval) {
            if (array_val.find(array) != array_val.end()) {
              auto iter = array_val.find(array);
              iter->second.push_back(
                  {const_offset->getValue(), const_store_rval->getValue()});
            } else {
              array_val.insert(
                  {array,
                   {{const_offset->getValue(), const_store_rval->getValue()}}});
            }
          }
        }
      }
    }
  }
  for (auto iter : array_val) {
    std::vector<int> power_array;
    auto max_idx = 0;
    for (auto vector_iter : iter.second) {
      max_idx = (vector_iter.first > max_idx) ? vector_iter.first : max_idx;
    }

    power_array.resize(max_idx + 1, 0);
    for (auto vector_iter : iter.second) {
      power_array[vector_iter.first] = vector_iter.second;
    }
    if (power_array.size() != 31) {
      return nullptr;
    }
    for (int i = 0; i < power_array.size(); i++) {
      if (power_array[i] == 1 << i) {
        if (i == power_array.size() - 1) {
          return iter.first;
        } else {
          continue;
        }
      } else {
        break;
      }
    }
  }
  return nullptr;
}

void PowerArray::replaceWithShl(Value *array) {
  std::vector<Instruction *> wait_delete;
  if (array == nullptr) {
    // std::cerr<<"no bitset"<<std::endl;
    return;
  }
  for (auto use : array->getUseList()) {
    auto use_instr = dynamic_cast<Instruction *>(use.val_);
    if (use_instr) {
      if (use_instr->isGEP()) {
        for (auto gep_use : use_instr->getUseList()) {
          auto gep_use_instr = dynamic_cast<Instruction *>(gep_use.val_);
          if (gep_use_instr->isLoad()) {
            auto builder = new IRBuilder(use_instr->getParent());
            auto shl = BinaryInst::createShl(ConstantInt::get(1, m_),
                                             getGepOffset(use_instr));
            builder->insertInstr(gep_use_instr, shl);
            gep_use_instr->replaceAllUseWith(shl);
#ifdef DEBUG
            std::cerr << "gep: " << use_instr->getName() << std::endl;
            std::cerr << "load: " << gep_use_instr->getName() << std::endl;
            std::cerr << "gepoffset: " << getGepOffset(use_instr)->getName()
                      << std::endl;
#endif
          }
          wait_delete.push_back(gep_use_instr);
        }
      }
      if (use_instr->isLoad()) {
        use_instr->replaceAllUseWith(ConstantInt::get(1, m_));
      }
      wait_delete.push_back(use_instr);
    }
  }
  for (auto instr : wait_delete) {
    auto bb = instr->getParent();
    bb->deleteInstr(instr);
  }
  return;
}

Value *PowerArray::getGepPtr(Instruction *instr) {
  return instr->getOperand(0);
}

Value *PowerArray::getGepOffset(Instruction *instr) {
  return instr->getOperand(1);
}