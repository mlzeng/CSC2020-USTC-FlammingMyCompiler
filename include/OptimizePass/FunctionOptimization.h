#include "Analyzer.hh"
#include "BasicBlock.h"
#include "Function.h"
#include "IRBuilder.h"
#include "PassManager.h"

class FunctionOptimization : public Transform {
private:
  Function *func_;
  IRBuilder *builder;

public:
  FunctionOptimization(Module *m) : Transform(m) {
    builder = new IRBuilder(nullptr);
  }
  ~FunctionOptimization() {}
  void run() override;
  void Set();
  void Gnp();
  void Mv();
  void MM();
  void createArgs();
};