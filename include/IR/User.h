#ifndef SYSYC_USER_H
#define SYSYC_USER_H

#include "Value.h"
#include <vector>
// #include <memory>
// class Value;
class User : public Value {
public:
  User(Type *ty, const std::string &name = "", unsigned num_ops = 0);
  ~User() = default;

  std::vector<Value *> &getOperandList();

  // start from 0
  Value *getOperand(unsigned i) const;

  // start from 0, auto add use
  void setOperand(unsigned i, Value *v);

  void addOperand(Value *v);
  void removeOperand(unsigned i);
  void removeOperand(unsigned i, unsigned j);

  unsigned getNumOperand() const;

  // TODO(zyh) 待确认
  std::vector<Value *> &getOperands() { return operands_; }

  virtual void print() override {}

  // remove the use of all operands
  void removeUseOfOps();

  void setNumOps(unsigned num) {
    num_ops_ = num;
    operands_.resize(num, nullptr);
  }
  void clearOps() {
    num_ops_ = 0;
    removeUseOfOps();
    operands_.clear();
  }

private:
  // std::unique_ptr< std::list<Value *> > operands_;   // operands of this
  // value
  std::vector<Value *> operands_; // operands of this value
  unsigned num_ops_;
};

#endif // SYSYC_USER_H