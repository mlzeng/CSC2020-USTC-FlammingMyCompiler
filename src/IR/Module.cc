#include "Module.h"

Module::Module(std::string name) : module_name_(name) {
  void_ty_ = new Type(Type::VoidTyID);
  label_ty_ = new Type(Type::LabelTyID);
  int1_ty_ = new IntegerType(1);
  int32_ty_ = new IntegerType(32);
  int32ptr_ty_ = new PointerType(int32_ty_);
}

Module::~Module() {
  delete void_ty_;
  delete label_ty_;
  delete int1_ty_;
  delete int32_ty_;
  delete int32ptr_ty_;
}

Type *Module::getVoidTy() { return void_ty_; }

Type *Module::getLabelTy() { return label_ty_; }

IntegerType *Module::getInt1Ty() { return int1_ty_; }

IntegerType *Module::getInt32Ty() { return int32_ty_; }

PointerType *Module::getInt32PtrTy() { return int32ptr_ty_; }

void Module::addFunction(Function *f) { function_list_.push_back(f); }

void Module::addGlobalVariable(GlobalVariable *g) { global_list_.push_back(g); }

void Module::print() {
  std::cerr << "; Module name: "
            << "\'" << module_name_ << "\'" << std::endl;
  std::cerr << "source_filename: "
            << "\"" << source_file_name_ << "\"" << std::endl;
  for (auto gobal_var : global_list_) {
    gobal_var->print();
    std::cerr << " = "
              << "global ";
    gobal_var->getInit()->print();
    std::cerr << std::endl;
  }
  for (auto func : function_list_) {
    func->print();
  }
}

void Module::HighIRprint() {
  std::cerr << "; Module name: "
            << "\'" << module_name_ << "\'" << std::endl;
  std::cerr << "source_filename: "
            << "\"" << source_file_name_ << "\"" << std::endl;
  for (auto gobal_var : global_list_) {
    gobal_var->print();
    std::cerr << " = "
              << "global ";
    gobal_var->getInit()->print();
    std::cerr << std::endl;
  }
  for (auto func : function_list_) {
    func->HighIRprint();
  }
}
