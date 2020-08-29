#ifndef SYSYC_PASSMANAGER_H
#define SYSYC_PASSMANAGER_H

#include "Module.h"
#include <string>
#include <vector>

class Pass {
public:
  Pass(Module *m) : m_(m) {}
  virtual void run() = 0;
  void setName(std::string name) { name_ = name; }
  std::string getName() { return name_; }
  void setPrintIR(bool print_ir) { print_ir_ = print_ir; }
  bool isPrintIR() { return print_ir_; }

protected:
  Module *m_;
  std::string name_;
  bool print_ir_ = false;
};
class Analysis : public Pass {
public:
  Analysis(Module *m) : Pass(m) {}
};

class Transform : public Pass {
public:
  Transform(Module *m) : Pass(m) {}
};

class IRCheck : public Analysis {
public:
  IRCheck(Module *m) : Analysis(m) {}
  ~IRCheck(){};
  void run() override;

private:
  void CheckParent();
  void CheckPhiPosition();
  void CheckRetBrPostion();
  void CheckTerminate();
  void CheckPredSucc();
  void CheckEntry();
  void CheckUseList();
  void CheckOperandExit();
};

class PassManager {
public:
  PassManager(Module *m) : m_(m), ir_check(m) {}

  template <typename PassTy> void addPass(bool print_ir = false) {
    passes_.push_back(new PassTy(m_));
    passes_.back()->setName(typeid(PassTy).name());
    passes_.back()->setPrintIR(print_ir);
    // std::cout << typeid(PassTy).name() << std::endl;
  }

  void run(bool print_ir = false) {
    auto i = 0;
    // check ast
    try {
      ir_check.run();
    } catch (...) {
      std::cerr << "IRCheck ERROR after SYSYCBuilder" << std::endl;
      exit(1);
    }
    for (auto pass : passes_) {
      i++;
      pass->run();
      if (print_ir || pass->isPrintIR()) {
        std::cerr << ">>>>>>>>>>>> After pass " << pass->getName()
                  << " <<<<<<<<<<<<" << std::endl;
        m_->print();
      }
      try {
        ir_check.run();
      } catch (...) {
        std::cerr << "IRCheck ERROR after pass " << pass->getName()
                  << std::endl;
        exit(i * 2);
      }
    }
  }

private:
  std::vector<Pass *> passes_;
  Module *m_;
  IRCheck ir_check;
};

#endif // SYSYC_PASSMANAGER_H