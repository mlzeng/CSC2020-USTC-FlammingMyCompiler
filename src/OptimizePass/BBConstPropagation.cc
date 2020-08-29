#include "BBConstPropagation.h"
#include "ReturnVal.h"
// #define DEBUG

void BBConstPropagation::run() {
  for (auto func : m_->getFunctions()) {
    for (auto bb : func->getBasicBlocks()) {
      bb_ = bb;
#ifdef DEBUG
      std::cerr << "BBConstPropagation: " << bb_->getName() << std::endl;
#endif
      ConstPropagation();
    }
  }
  for (auto func : m_->getFunctions()) {
    for (auto bb : func->getBasicBlocks()) {
      builder_->SetInsertPoint(bb);
      if (bb->getTerminator()->isBr()) {
        auto br = bb->getTerminator();
        if (dynamic_cast<BranchInst *>(br)->isCondBr()) //一定是条件br
        {
          auto cond = dynamic_cast<ConstantInt *>(br->getOperand(0));
          auto truebb = br->getOperand(1);
          auto falsebb = br->getOperand(2);
          if (cond) {
            if (cond->getValue() == 0) {
              bb->deleteInstr(br);
              for (auto succ_bb : bb->getSuccBasicBlocks()) {
                succ_bb->removePreBasicBlock(bb);
                if (succ_bb != falsebb) {
                  for (auto instr : succ_bb->getInstructions()) {
                    if (instr->isPHI()) {
                      for (int i = 0; i < instr->getNumOperand(); i++) {
                        if (i % 2 == 1) {
                          if (instr->getOperand(i) == bb) {
                            instr->removeOperand(i - 1, i);
                          }
                        }
                      }
                    }
                  }
                }
              }
              builder_->CreateBr(dynamic_cast<BasicBlock *>(falsebb));
              bb->getSuccBasicBlocks().clear();
              bb->addSuccBasicBlock(dynamic_cast<BasicBlock *>(falsebb));
              // dynamic_cast<BasicBlock *>(falsebb)->addPreBasicBlock(bb);
            } else {
              bb->deleteInstr(br);
              for (auto succ_bb : bb->getSuccBasicBlocks()) {
                succ_bb->removePreBasicBlock(bb);
                if (succ_bb != truebb) {
                  for (auto instr : succ_bb->getInstructions()) {
                    if (instr->isPHI()) {
                      for (int i = 0; i < instr->getNumOperand(); i++) {
                        if (i % 2 == 1) {
                          if (instr->getOperand(i) == bb) {
                            instr->removeOperand(i - 1, i);
                          }
                        }
                      }
                    }
                  }
                }
              }
              builder_->CreateBr(dynamic_cast<BasicBlock *>(truebb));
              bb->getSuccBasicBlocks().clear();
              bb->addSuccBasicBlock(dynamic_cast<BasicBlock *>(truebb));
              // dynamic_cast<BasicBlock *>(truebb)->addPreBasicBlock(bb);
            }
          }
        }
      }
    }
  }
}

void BBConstPropagation::ConstPropagation() {
  array_const_def_.clear();
  global_def_.clear();
  std::vector<Instruction *> wait_delete;
  for (auto instr : bb_->getInstructions()) {
#ifdef DEBUG
    std::cerr << "instr: " << instr->getName() << std::endl;
#endif

    if (instr->isAdd() || instr->isSub() || instr->isMul() || instr->isDiv() ||
        instr->isRem() || instr->isAnd() || instr->isOr()) {
      auto v1 = getConstVal(instr->getOperand(0));
      auto v2 = getConstVal(instr->getOperand(1));
      if (v1 && v2) {
        auto flod_const = flod_->compute(instr->getInstrType(), v1, v2);
        for (auto use : instr->getUseList()) {
          dynamic_cast<User *>(use.val_)->setOperand(use.arg_no_, flod_const);
        }
        wait_delete.push_back(instr);
      }
    } else if (instr->isNot() || instr->isNeg()) {
      auto v1 = getConstVal(instr->getOperand(0));
      if (v1) {
        auto flod_const = flod_->compute(instr->getInstrType(), v1);
        for (auto use : instr->getUseList()) {
          dynamic_cast<User *>(use.val_)->setOperand(use.arg_no_, flod_const);
        }
        wait_delete.push_back(instr);
      }
    } else if (instr->isLoad()) {
      auto v1 = getDefVal(instr->getOperand(0));
      if (v1) {
        for (auto use : instr->getUseList()) {
          dynamic_cast<User *>(use.val_)->setOperand(use.arg_no_, v1);
        }
        wait_delete.push_back(instr);
      }
    } else if (instr->isStore()) {
      auto v1 = dynamic_cast<StoreInst *>(instr)->getLVal();
      auto v2 = getConstVal(dynamic_cast<StoreInst *>(instr)->getRVal());
      if (v2) {
        setDefVal(v1, v2);
        // wait_delete.push_back(instr);
      } else {
#ifdef DEBUG
        std::cerr << v1->getName() << std::endl;
#endif

        if (array_const_def_.find(dynamic_cast<User *>(v1)->getOperand(0)) !=
            array_const_def_.end()) {
#ifdef DEBUG
          std::cerr << "delete in const propa" << std::endl;
#endif
          array_const_def_.erase(
              array_const_def_.find(dynamic_cast<User *>(v1)->getOperand(0)));
        } else if (global_def_.find(v1) != global_def_.end()) {
          global_def_.erase(global_def_.find(v1));
        }
      }

    } else if (instr->isCmp()) {
      auto v1 = getConstVal(instr->getOperand(0));
      auto v2 = getConstVal(instr->getOperand(1));
      if (v1 && v2) {
        auto flod_const =
            flod_->compute(dynamic_cast<CmpInst *>(instr)->getCmpOp(), v1, v2);
        for (auto use : instr->getUseList()) {
          dynamic_cast<User *>(use.val_)->setOperand(use.arg_no_, flod_const);
        }
        wait_delete.push_back(instr);
      }
    } else if (instr->isRet() || instr->isCall() || instr->isGEP()) //替换操作数
    {
      ;
    }
  }
  for (auto instr : wait_delete) {
    bb_->deleteInstr(instr);
  }

  return;
}
ConstantInt *BBConstPropagation::getConstVal(Value *v) {
  auto const_v = dynamic_cast<ConstantInt *>(v);
  if (const_v) {
    return const_v;
  } else {
    return nullptr;
  }
}
ConstantInt *BBConstPropagation::getDefVal(Value *v) {
  auto instr_v = dynamic_cast<Instruction *>(v);
  auto global_v = dynamic_cast<GlobalVariable *>(v);
  auto arg_v = dynamic_cast<Argument *>(v);

  if (instr_v != nullptr) {
    if (instr_v->isGEP()) {
      auto gep_v = dynamic_cast<GetElementPtrInst *>(instr_v);
      if (array_const_def_.find(gep_v->getOperand(0)) !=
          array_const_def_.end()) {
        auto iter1 = array_const_def_.find(gep_v->getOperand(0));
        std::vector<int> index;
        for (int i = 1; i < gep_v->getNumOperand(); i++) {
          auto op = gep_v->getOperand(i);
          if (getConstVal(op)) {
            index.push_back(getConstVal(op)->getValue());
          } else {
            return nullptr;
          }
        }

        std::string idx_key;
        for (auto idx : index) {
          idx_key += std::to_string(idx);
          idx_key += "#";
        }

        if (iter1->second.find(idx_key) != iter1->second.end()) {
          auto iter2 = iter1->second.find(idx_key);
          return iter2->second;
        } else {
          return nullptr;
        }
      } else {
        return nullptr;
      }
    }
  } else if (global_v != nullptr) {
    if (global_def_.find(v) != global_def_.end()) {
      auto iter = global_def_.find(v);
      return iter->second;
    } else {
      return nullptr;
    }
  } else if (arg_v != nullptr) {
  } else {
    std::cerr << "error in constpropagation getdefval" << std::endl;
    // exit_ifnot(_CantGetDefVal_getDefVal_BBConstPropagation, false);
    return nullptr;
  }
  return nullptr;
}

ConstantInt *BBConstPropagation::setDefVal(Value *v, ConstantInt *const_val) {
#ifdef DEBUG
  std::cerr << "setdef: " << v->getName() << std::endl;
#endif

  auto instr_v = dynamic_cast<Instruction *>(v);
#ifdef DEBUG
  std::cerr << "setdef: " << instr_v->getName() << std::endl;
#endif

  auto global_v = dynamic_cast<GlobalVariable *>(v);
  if (global_v != nullptr) {
    if (global_def_.find(global_v) != global_def_.end()) {
      auto iter = global_def_.find(global_v);
      iter->second = const_val;
      return iter->second;
    } else {
      global_def_.insert({global_v, const_val});
      return global_def_.find(global_v)->second;
    }
  } else if (instr_v != nullptr) {
    if (instr_v->isGEP()) {
      auto gep_v = dynamic_cast<GetElementPtrInst *>(instr_v);
      std::vector<int> index;
      for (int i = 1; i < gep_v->getNumOperand(); i++) {
        auto op = gep_v->getOperand(i);
        if (getConstVal(op)) {
          index.push_back(getConstVal(op)->getValue());
        } else {
          return nullptr;
        }
      }

      std::string idx_key;
      for (auto idx : index) {
        idx_key += std::to_string(idx);
        idx_key += "#";
      }

      if (array_const_def_.find(gep_v->getOperand(0)) !=
          array_const_def_.end()) {
        auto iter1 = array_const_def_.find(gep_v->getOperand(0));
        if (iter1->second.find(idx_key) != iter1->second.end()) {
          auto iter2 = iter1->second.find(idx_key);
          iter2->second = const_val;
        } else {
          iter1->second.insert({idx_key, const_val});
          return const_val;
        }
      } else {
        std::map<std::string, ConstantInt *> idx_map;
        idx_map.insert({idx_key, const_val});
        array_const_def_.insert({gep_v->getOperand(0), idx_map});
        return idx_map.find(idx_key)->second;
      }
    }
  } else {
    std::cerr << "error in constpropagation setdefval" << std::endl;
    // exit_ifnot(_CantSetDefVal_setDefVal_BBConstPropagation, false);
    return nullptr;
  }
  return nullptr;
}