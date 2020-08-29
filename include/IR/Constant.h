//
// Created by cqy on 2020/6/29.
//

#ifndef SYSYC_CONSTANT_H
#define SYSYC_CONSTANT_H
#include "Type.h"
#include "User.h"
#include "Value.h"

// class User;
class Constant : public User {
private:
  // int value;
public:
  Constant(Type *ty, const std::string &name = "", unsigned num_ops = 0)
      : User(ty, name, num_ops) {}
  ~Constant() = default;

  virtual void print() override;
};

class ConstantInt : public Constant {
private:
  int value_;
  ConstantInt(Type *ty, int val) : Constant(ty, "", 0), value_(val) {}

public:
  static int getValue(ConstantInt *const_val) { return const_val->value_; }
  int getValue() { return value_; }
  void setValue(int val) { value_ = val; }
  static ConstantInt *get(int val, Module *m);
  virtual void print() override;
};

class ConstantArray : public Constant {
private:
  std::vector<Constant *> const_array;

  ConstantArray(ArrayType *ty, const std::vector<Constant *> &val);

public:
  ~ConstantArray() = default;

  Constant *getElementValue(int index);

  unsigned getNumElements() const { return const_array.size(); }

  static ConstantArray *get(ArrayType *ty, const std::vector<Constant *> &val);

  virtual void print() override;
};

#endif // SYSYC_CONSTANT_H
