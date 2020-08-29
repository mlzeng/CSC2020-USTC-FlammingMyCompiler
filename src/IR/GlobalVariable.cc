//
// Created by cqy on 2020/6/29.
//
#include "GlobalVariable.h"

GlobalVariable::GlobalVariable(std::string name, Module *m, Type *ty,
                               bool is_const, Constant *init)
    : User(ty, name, init != nullptr), is_const_(is_const), init_val_(init) {
  m->addGlobalVariable(this);
  if (init) {
    this->setOperand(0, init);
  }
} // global操作数为initval

GlobalVariable *GlobalVariable::create(std::string name, Module *m, Type *ty,
                                       bool is_const,
                                       Constant *init = nullptr) {
  return new GlobalVariable(name, m, PointerType::get(ty), is_const, init);
}

void GlobalVariable::print() {
  this->getType()->print();
  std::cerr << " @" << this->getName() << " ";
}