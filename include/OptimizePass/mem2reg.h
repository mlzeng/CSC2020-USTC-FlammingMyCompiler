#ifndef SYSYC_MEM2REG_H
#define SYSYC_MEM2REG_H

#include "BasicBlock.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.h"

class Mem2RegPass : public Transform {
private:
  Function *func_;
  IRBuilder *builder;

public:
  Mem2RegPass(Module *m) : Transform(m) {}
  ~Mem2RegPass(){};
  void run() override;
  void GenPhi();
  void ReName(BasicBlock *bb);
  // void DeleteLS();
  //可加一个遍历删除空phi节点
};

#endif