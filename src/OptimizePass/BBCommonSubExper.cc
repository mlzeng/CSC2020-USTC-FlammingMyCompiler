#include "BBCommonSubExper.h"
#include <tuple>
// #define DEBUG

std::map<std::tuple<Value *, Value *, Instruction::OpID>,
         std::pair<int, Value *>>
    AEB; // string包含两个操作数的地址，int代表pos, value * 代表新变量。
std::map<std::tuple<Value *, int, Instruction::OpID>, std::pair<int, Value *>>
    AEB_rconst;
std::map<std::tuple<int, Value *, Instruction::OpID>, std::pair<int, Value *>>
    AEB_lconst;

std::map<std::tuple<Value *, Value *, int, Instruction::OpID>, Value *>
    load_val;
std::map<std::tuple<Value *, Value *, Instruction::OpID>, Value *> gep_val;
std::map<std::tuple<Value *, int, Instruction::OpID>, Value *> gep_val_const;

std::map<std::pair<std::string, Arglist>, Value *> call_val;

void BBCommonSubExper::run() {
  getEliminatableCall();
  for (auto func : m_->getFunctions()) {
    for (auto bb : func->getBasicBlocks()) {
      AEB.clear();
      AEB_rconst.clear();
      AEB_lconst.clear();
      load_val.clear();
      gep_val.clear();
      gep_val_const.clear();
      call_val.clear();
      if (bb->getPreBasicBlocks().size() == 1) {
        bb_ = bb->getPreBasicBlocks().front();
        CommonSubExperElimination();
        bb_ = bb;
        CommonSubExperElimination();
      } else {
        bb_ = bb;
        CommonSubExperElimination();
      }
    }
  }
}

void BBCommonSubExper::CommonSubExperElimination() {
  auto pos = 0;
  std::vector<Instruction *> wait_delete;
  for (auto instr : bb_->getInstructions()) {
    if (instr->isBinary()) {
      auto lhs = getBinaryLop(instr);
      auto rhs = getBinaryRop(instr);

      auto lhs_const = dynamic_cast<ConstantInt *>(lhs);
      auto rhs_const = dynamic_cast<ConstantInt *>(rhs);

      if (rhs_const) {
        if (AEB_rconst.find({lhs, rhs_const->getValue(),
                             instr->getInstrType()}) != AEB_rconst.end()) {
          auto iter = AEB_rconst.find(
              {lhs, rhs_const->getValue(), instr->getInstrType()});
          instr->replaceAllUseWith(iter->second.second);
          wait_delete.push_back(instr);
        } else {
          AEB_rconst.insert(
              {{lhs, rhs_const->getValue(), instr->getInstrType()},
               {pos, instr}});
        }
      } else if (lhs_const) {
        if (AEB_lconst.find({lhs_const->getValue(), rhs,
                             instr->getInstrType()}) != AEB_lconst.end()) {
          auto iter = AEB_lconst.find(
              {lhs_const->getValue(), rhs, instr->getInstrType()});
          instr->replaceAllUseWith(iter->second.second);
          wait_delete.push_back(instr);
        } else {
          AEB_lconst.insert(
              {{lhs_const->getValue(), rhs, instr->getInstrType()},
               {pos, instr}});
        }
      } else {
        if (AEB.find({lhs, rhs, instr->getInstrType()}) != AEB.end()) {
          auto iter = AEB.find({lhs, rhs, instr->getInstrType()});
          instr->replaceAllUseWith(iter->second.second);
          wait_delete.push_back(instr);
        } else {
          AEB.insert({{lhs, rhs, instr->getInstrType()}, {pos, instr}});
        }
      }
    } else if (instr->isGEP()) {
      auto ptr = getGepPtr(instr);
      auto offset = getGepOffset(instr);
      auto const_offset = dynamic_cast<ConstantInt *>(offset);
      if (const_offset) {
        if (gep_val_const.find(
                {ptr, const_offset->getValue(), instr->getInstrType()}) !=
            gep_val_const.end()) {
          auto iter = gep_val_const.find(
              {ptr, const_offset->getValue(), instr->getInstrType()});
          instr->replaceAllUseWith(iter->second);
          wait_delete.push_back(instr);
        } else {
          gep_val_const.insert(
              {{ptr, const_offset->getValue(), instr->getInstrType()}, instr});
        }
      } else {
        if (gep_val.find({ptr, offset, instr->getInstrType()}) !=
            gep_val.end()) {
          auto iter = gep_val.find({ptr, offset, instr->getInstrType()});
          instr->replaceAllUseWith(iter->second);
          wait_delete.push_back(instr);
        } else {
          gep_val.insert({{ptr, offset, instr->getInstrType()}, instr});
        }
      }
    } else if (instr->isLoad()) {
      auto ptr = getLoadPtr(instr);
      auto offset = getLoadOffset(instr);
      auto offset_shl = getLoadOffsetShl(instr);
      if (load_val.find({ptr, offset, offset_shl, instr->getInstrType()}) !=
          load_val.end()) {
        auto iter =
            load_val.find({ptr, offset, offset_shl, instr->getInstrType()});
        instr->replaceAllUseWith(iter->second);
        wait_delete.push_back(instr);
      } else {
        load_val.insert(
            {{ptr, offset, offset_shl, instr->getInstrType()}, instr});
      }
    } else if (instr->isStore()) {
      std::vector<std::pair<
          const std::tuple<Value *, Value *, int, Instruction::OpID>, Value *>>
          wait_delete_map_iter;
      auto ptr = dynamic_cast<StoreInst *>(instr)->getLVal();
      for (auto iter : load_val) {
        if (std::get<0>(iter.first) == ptr) {
          wait_delete_map_iter.push_back(iter);
        }
      }
      for (auto iter : wait_delete_map_iter) {
        load_val.erase(load_val.find(iter.first));
      }
    } else if (instr->isCall()) {
      auto func_id = getFuncID(dynamic_cast<CallInst *>(instr));
      auto args = getCallArgs(dynamic_cast<CallInst *>(instr));
      if (eliminatableCall(func_id) && !instr->isVoid()) {
#ifdef DEBUG
        std::cerr << "elimi id :" << std::endl;
        std::cerr << func_id << std::endl;
#endif

        if (call_val.find({func_id, *args}) != call_val.end()) {
          auto iter = call_val.find({func_id, *args});
          instr->replaceAllUseWith(iter->second);
          wait_delete.push_back(instr);
        } else {
          call_val.insert({{func_id, *args}, instr});
        }
      }
    }
    pos++;
  }
  for (auto instr : wait_delete) {
    bb_->deleteInstr(instr);
  }
}

Value *BBCommonSubExper::getBinaryLop(Instruction *instr) {
  return instr->getOperand(0);
}

Value *BBCommonSubExper::getBinaryRop(Instruction *instr) {
  return instr->getOperand(1);
}

Value *BBCommonSubExper::getLoadPtr(Instruction *instr) {
  return instr->getOperand(0);
}

Value *BBCommonSubExper::getLoadOffset(Instruction *instr) {
  if (instr->getNumOperand() >= 2) {
    return instr->getOperand(1);
  }
  return nullptr;
}

int BBCommonSubExper::getLoadOffsetShl(Instruction *instr) {
  if (instr->getNumOperand() == 3) {
    return dynamic_cast<ConstantInt *>(instr->getOperand(2))->getValue();
  }
  return 0;
}

Value *BBCommonSubExper::getGepPtr(Instruction *instr) {
  return instr->getOperand(0);
}

Value *BBCommonSubExper::getGepOffset(Instruction *instr) {
  return instr->getOperand(1);
}

Arglist *BBCommonSubExper::getCallArgs(CallInst *instr) {
  auto args = new Arglist();
  for (int i = 1; i < instr->getNumOperand(); i++) {
    args->args_.push_back(instr->getOperand(i));
  }
  return args;
}

std::string BBCommonSubExper::getFuncID(CallInst *instr) {
  return instr->getFunction()->getName();
}

bool BBCommonSubExper::eliminatableCall(std::string func_id) {
  if (eliminatable_call_.find(func_id) != eliminatable_call_.end()) {
    return true;
  } else {
    return false;
  }
}

void BBCommonSubExper::getEliminatableCall() {
  for (auto func : m_->getFunctions()) {
#ifdef DEBUG
    std::cerr << func->getName() << std::endl;
#endif

    auto elimi = true;

    if (func->getNumBasicBlocks() == 0) {
      elimi = false;
    }

    for (auto bb : func->getBasicBlocks()) {
      for (auto instr : bb->getInstructions()) {
        if (instr->isStore()) {
          elimi = false;
        }
      }
    }

    if (elimi) {
#ifdef DEBUG
      std::cerr << func->getName() << std::endl;
#endif
      eliminatable_call_.insert(func->getName());
    }
  }
  return;
}

const bool Arglist::operator<(const Arglist &a) const {
  if (this->args_.size() < a.args_.size()) {
    return true;
  } else if (this->args_.size() == a.args_.size()) {
    for (int i = 0; i < this->args_.size(); i++) {
      auto c1 = dynamic_cast<ConstantInt *>(this->args_[i]);
      auto c2 = dynamic_cast<ConstantInt *>(a.args_[i]);
      if (c1 && c2) {
        if (c1->getValue() == c2->getValue()) {
          continue;
        } else {
          return c1->getValue() < c2->getValue();
        }
      } else {
        if (this->args_[i] == a.args_[i]) {
          continue;
        } else {
          return this->args_[i] < a.args_[i];
        }
      }
    }
  }
  return false;
}

const bool Arglist::operator>(const Arglist &a) const {
  if (this->args_.size() > a.args_.size()) {
    return true;
  } else if (this->args_.size() == a.args_.size()) {
    for (int i = 0; i < this->args_.size(); i++) {
      auto c1 = dynamic_cast<ConstantInt *>(this->args_[i]);
      auto c2 = dynamic_cast<ConstantInt *>(a.args_[i]);
      if (c1 && c2) {
        if (c1->getValue() == c2->getValue()) {
          continue;
        } else {
          return c1->getValue() < c2->getValue();
        }
      } else {
        if (this->args_[i] == a.args_[i]) {
          continue;
        } else {
          return this->args_[i] < a.args_[i];
        }
      }
    }
  } else {
    return false;
  }
  return false;
}

const bool Arglist::operator==(const Arglist &a) const {
  if (this->args_.size() == a.args_.size()) {
    for (int i = 0; i < this->args_.size(); i++) {
      if (i < a.args_.size()) {
        auto c1 = dynamic_cast<ConstantInt *>(this->args_[i]);
        auto c2 = dynamic_cast<ConstantInt *>(a.args_[i]);
        if (c1 && c2) {
          if (c1->getValue() == c2->getValue()) {
            continue;
          } else {
            return false;
          }
        } else {
          if (this->args_[i] == a.args_[i]) {
            continue;
          } else {
            return false;
          }
        }
      }
    }
  } else {
    return false;
  }
  return true;
}