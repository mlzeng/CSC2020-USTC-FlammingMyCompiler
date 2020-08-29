#include "User.h"
#include "ReturnVal.h"
#include <cassert>

User::User(Type *ty, const std::string &name, unsigned num_ops)
    : Value(ty, name), num_ops_(num_ops) {
  // if (num_ops_ > 0)
  //   operands_.reset(new std::list<Value *>());
  operands_.resize(num_ops_, nullptr);
}

std::vector<Value *> &User::getOperandList() { return operands_; }

Value *User::getOperand(unsigned i) const { return operands_[i]; }

void User::setOperand(unsigned i, Value *v) {
  exit_ifnot(_OutOfIndex_setOperand_User, i < num_ops_ && i >= 0 &&
                                              i < operands_.size() &&
                                              "setOperand out of index");
  // assert(operands_[i] == nullptr && "ith operand is not null");
  operands_[i] = v;
  v->addUse(this, i);
}

unsigned User::getNumOperand() const { return num_ops_; }

void User::addOperand(Value *v) {
  operands_.push_back(v);
  v->addUse(this, num_ops_);
  num_ops_++;
}

void User::removeOperand(unsigned i) {
  removeUseOfOps();
  num_ops_ = 0;
  std::vector<Value *> item;
  item.swap(operands_);
  operands_.clear();
  for (int j = 0; j < item.size(); j++) {
    if (i == j) {
      continue;
    } else {
      addOperand(item[j]);
    }
  }
}

void User::removeOperand(unsigned i, unsigned j) {
  removeUseOfOps();
  num_ops_ = 0;
  std::vector<Value *> item;
  item.swap(operands_);
  operands_.clear();
  for (int k = 0; k < item.size(); k++) {
    if (i == k || j == k) {
      continue;
    } else {
      addOperand(item[k]);
    }
  }
}

void User::removeUseOfOps() {
  int i = 0;
  for (auto op : operands_) {
    op->removeUse(this, i);
    i++;
  }
}