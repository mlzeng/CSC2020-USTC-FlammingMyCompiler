#include "BasicBlock.h"
#include "Function.h"
#include "Module.h"
#include "ReturnVal.h"
#include <cassert>

BasicBlock::BasicBlock(Module *m, const std::string &name)
    : BaseBlock(BaseBlock::Basic, m, name) {
  // assert(parent && "currently parent should not be nullptr");
  // if (func)
  //     func->addBasicBlock(this);
}

BasicBlock::BasicBlock(Module *m, const std::string &name, Function *func)
    : BaseBlock(BaseBlock::Basic, m, name, func) {}

Module *BasicBlock::getModule() const { return getParent()->getParent(); }

void BasicBlock::addInstruction(Instruction *instr) {
  // auto seq = atoi(this->getName().c_str());
  // seq += instr_list_.size();
  // instr->setName(std::to_string(seq));
  instr_list_.push_back(instr);
  instr->setParent(this);
}

void BasicBlock::addInstrBegin(Instruction *instr) {
  instr_list_.push_front(instr);
  instr->setParent(this);
}

void BasicBlock::insertInstr(Instruction *pos, Instruction *insert) {
  insert->setParent(pos->getParent());
  for (auto instr = instr_list_.begin(); instr != instr_list_.end(); instr++) {
    if (*instr == pos) {
      instr_list_.insert(instr, insert);
      return;
    }
  }
  // abort();
  exit_ifnot(_PosNotFind_insertInstr_BasicBlock, false);
}

void BasicBlock::deleteInstr(Instruction *instr) {
  instr_list_.remove(instr);
  instr->removeUseOfOps();
}

const Instruction *BasicBlock::getTerminator() const {
  if (instr_list_.empty()) {
    return nullptr;
  }
  switch (instr_list_.back()->getInstrType()) {
  case Instruction::Ret:
    return instr_list_.back();
    break;

  case Instruction::Br:
    return instr_list_.back();
    break;

  default:
    return nullptr;
    break;
  }
  return nullptr;
}

void BasicBlock::print() {
  this->getType()->print();
  std::cerr << " %" << this->getName() << " ";
  return;
}

bool BasicBlock::hasRet() { return getTerminator()->isRet(); }