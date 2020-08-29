#ifndef SYSYC_TYPE_H
#define SYSYC_TYPE_H

#include <iostream>
#include <vector>

class Module;
class IntegerType;
class FunctionType;
class ArrayType;
class PointerType;

class Type {
public:
  enum TypeID {
    VoidTyID,     // Void
    LabelTyID,    // Labels, e.g., BasicBlock
    IntegerTyID,  // Integers, include 32 bits and 1 bit
    FunctionTyID, // Functions
    ArrayTyID,    // Arrays
    PointerTyID,  // Pointer
  };

  explicit Type(TypeID tid);
  ~Type() = default;

  TypeID getTypeID() const { return tid_; }

  bool isVoidTy() const { return getTypeID() == VoidTyID; }

  bool isLabelTy() const { return getTypeID() == LabelTyID; }

  bool isIntegerTy() const { return getTypeID() == IntegerTyID; }

  bool isFunctionTy() const { return getTypeID() == FunctionTyID; }

  bool isArrayTy() const { return getTypeID() == ArrayTyID; }

  bool isPointerTy() const { return getTypeID() == PointerTyID; }

  bool isInt1();

  bool isInt32();

  static Type *getVoidTy(Module *m);

  static Type *getLabelTy(Module *m);

  static IntegerType *getInt1Ty(Module *m);

  static IntegerType *getInt32Ty(Module *m);

  static PointerType *getInt32PtrTy(Module *m);

  Type *getPointerElementType();

  void print();
  std::string CommentPrint();

  int getSize(bool extended = true);

  bool eq(Type rhs) {
    if (this->tid_ != rhs.tid_) {
      return false;
    } else if (this->isPointerTy()) {
      return this->getPointerElementType()->eq(*rhs.getPointerElementType());
    } else {
      return true;
    }
  }

private:
  TypeID tid_;
};

class IntegerType : public Type {
public:
  explicit IntegerType(unsigned num_bits);

  static IntegerType *get(unsigned num_bits);

  unsigned getNumBits();

private:
  unsigned num_bits_;
};

class FunctionType : public Type {
public:
  FunctionType(Type *result, std::vector<Type *> params);

  static bool isValidReturnType(Type *ty);
  static bool isValidArgumentType(Type *ty);

  static FunctionType *get(Type *result, std::vector<Type *> params);

  unsigned getNumArgs() const;

  Type *getArgType(unsigned i) const;
  Type *getResultType() const;

private:
  Type *result_;
  std::vector<Type *> args_;
};

class ArrayType : public Type {
public:
  ArrayType(Type *contained, unsigned num_elements);

  static bool isValidElementType(Type *ty);

  static ArrayType *get(Type *contained, unsigned num_elements);

  Type *getElementType() const { return contained_; }
  unsigned getNumOfElements() const { return num_elements_; }
  std::vector<unsigned> getDims() const;

private:
  Type *contained_;       // The element type of the array.
  unsigned num_elements_; // Number of elements in the array.
};

class PointerType : public Type {
public:
  PointerType(Type *contained);
  Type *getElementType() const { return contained_; }

  static PointerType *get(Type *contained);

private:
  Type *contained_; // The element type of the ptr.
};

#endif // SYSYC_TYPE_H