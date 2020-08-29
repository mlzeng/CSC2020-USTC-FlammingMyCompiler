//
// Created by cqy on 2020/6/29.
//

#ifndef SYSYC_GLOBALVARIABLE_H
#define SYSYC_GLOBALVARIABLE_H

#include "Constant.h"
#include "Module.h"
#include "User.h"

class GlobalVariable : public User {
private:
  bool is_const_ : true;
  Constant *init_val_;
  GlobalVariable(std::string name, Module *m, Type *ty, bool is_const,
                 Constant *init = nullptr);

public:
  static GlobalVariable *create(std::string name, Module *m, Type *ty,
                                bool is_const, Constant *init);

  Constant *getInit() { return init_val_; }
  void print();
};
#endif // SYSYC_GLOBALVARIABLE_H
