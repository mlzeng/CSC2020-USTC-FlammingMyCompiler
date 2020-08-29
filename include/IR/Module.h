#ifndef SYSYC_MODULE_H
#define SYSYC_MODULE_H

#include <list>
#include <map>
#include <string>

#include "Function.h"
#include "GlobalVariable.h"
#include "Type.h"
#include "Value.h"

class GlobalVariable;

class Module {
public:
  enum IRLeval { HIR, MIR_MEM, MIR_SSA, LIR };

  explicit Module(std::string name);
  ~Module();
  Type *getVoidTy();
  Type *getLabelTy();
  IntegerType *getInt1Ty();
  IntegerType *getInt32Ty();
  PointerType *getInt32PtrTy();

  void addFunction(Function *f);
  void removeFunction(Function *f) { function_list_.remove(f); }
  void removeGlobalVariable(GlobalVariable *v) { global_list_.remove(v); }
  std::list<Function *> &getFunctions() { return function_list_; }
  std::list<GlobalVariable *> &getGlobalVariables() { return global_list_; }
  std::map<std::string, Value *> &getValueSym() { return value_sym_; }
  std::string getModuleName() { return module_name_; }
  std::string getSourceFileName() { return source_file_name_; }
  void addGlobalVariable(GlobalVariable *g);
  void HighIRprint();
  virtual void print();

  Function *getMainFunction() {
    for (auto f : function_list_) {
      if (f->getName() == "main") {
        return f;
      }
    }
    // assert(! "Can't find main");
  }

  Function *getFunction(std::string name) {
    for (auto f : function_list_) {
      if (f->getName() == name) {
        return f;
      }
    }
    exit(_getFunction_Function);
  }

  void setIRLevel(IRLeval level) { ir_level_ = level; }
  IRLeval getIRLevel() { return ir_level_; }
  bool isMIRSSALevel() { return ir_level_ == MIR_SSA; }

private:
  std::list<GlobalVariable *>
      global_list_;                     // The Global Variables in the module
  std::list<Function *> function_list_; // The Functions in the module
  std::map<std::string, Value *> value_sym_; // Symbol table for values

  std::string module_name_;      // Human readable identifier for the module
  std::string source_file_name_; // Original source file name for module, for
                                 // test and debug

  IRLeval ir_level_ = HIR;

private:
  IntegerType *int1_ty_;
  IntegerType *int32_ty_;
  Type *label_ty_;
  Type *void_ty_;
  PointerType *int32ptr_ty_;
};

#endif // SYSYC_MODULE_H