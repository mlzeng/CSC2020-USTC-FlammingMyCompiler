#include "BasicBlock.h"
#include "ConstFlod.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.h"

class BBConstPropagation : public Transform {
private:
  BasicBlock *bb_;
  IRBuilder *builder_;
  ConstFlod *flod_;
  std::map<Value *, std::map<std::string, ConstantInt *>> array_const_def_;
  std::map<Value *, ConstantInt *> global_def_;

public:
  BBConstPropagation(Module *m) : Transform(m) {
    flod_ = new ConstFlod(m);
    builder_ = new IRBuilder(nullptr);
  }
  ~BBConstPropagation() { delete flod_; }

  void run() override;
  void ConstPropagation();
  ConstantInt *getConstVal(Value *v);
  ConstantInt *getDefVal(Value *v);
  ConstantInt *setDefVal(Value *v, ConstantInt *const_val);
};