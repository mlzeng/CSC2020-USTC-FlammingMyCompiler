#include "Constant.h"
#include "Module.h"

ConstantInt *ConstantInt::get(int val, Module *m) {
  return new ConstantInt(Type::getInt32Ty(m), val);
}

ConstantArray::ConstantArray(ArrayType *ty, const std::vector<Constant *> &val)
    : Constant(ty, "", val.size()) {
  for (int i = 0; i < val.size(); i++)
    setOperand(i, val[i]);
  this->const_array.assign(val.begin(), val.end());
}

Constant *ConstantArray::getElementValue(int index) {
  return this->const_array[index];
}

ConstantArray *ConstantArray::get(ArrayType *ty,
                                  const std::vector<Constant *> &val) {
  return new ConstantArray(ty, val);
}

void Constant::print() {
  if (this->getType()->isArrayTy()) {
    static_cast<ConstantArray *>(this)->print();
  } else {
    static_cast<ConstantInt *>(this)->print();
  }
}

void ConstantInt::print() {
  this->getType()->print();
  std::cerr << " " << this->getValue() << " ";
}

void ConstantArray::print() {
  this->getType()->print();
  std::cerr << " [ ";
  for (auto const_val : const_array) {
    if (const_val->getType()->isArrayTy()) {
      static_cast<ConstantArray *>(const_val)->print();
    } else {
      static_cast<ConstantInt *>(const_val)->print();
    }
  }
  std::cerr << " ] ";
}