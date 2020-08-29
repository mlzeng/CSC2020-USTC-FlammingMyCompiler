//
// Created by cqy on 2020/6/11.
//

#ifndef SYSYC_SYSYBUILDER_HPP
#define SYSYC_SYSYBUILDER_HPP

#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "GlobalVariable.h"
#include "HighBlock.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "Type.h"
#include "User.h"
#include "Value.h"

// #include "Verifier.h"
#include "syntax_tree.hh"
#include <map>

class Scope {
public:
  // enter a new scope
  void enter() {
    inner.push_back({});
    array_param.push_back({});
  }

  // exit a scope
  void exit() {
    inner.pop_back();
    array_param.pop_back();
  }

  bool in_global() { return inner.size() == 1; }

  // push a name to scope
  // return true if successful
  // return false if this name already exits
  bool push(std::string name, Value *val) {
    auto result = inner[inner.size() - 1].insert({name, val});
    return result.second;
  }

  bool push_params(std::string name, Value *val, std::vector<Value *> params) {
    auto result = array_param[array_param.size() - 1].insert({name, params});
    return result.second;
  }

  Value *find(std::string name) {
    for (auto s = inner.rbegin(); s != inner.rend(); s++) {
      auto iter = s->find(name);
      if (iter != s->end()) {
        return iter->second;
      }
    }

    return nullptr;
  }

  Value *find_params(std::string name, std::vector<Value *> &params) {
    // std::cout<<"find_params"<<std::endl;
    for (auto s = array_param.rbegin(); s != array_param.rend(); s++) {
      // std::cout<<"find_params1"<<std::endl;
      auto iter = s->find(name);
      if (iter != s->end()) {
        // std::cout<<"find_params2"<<std::endl;
        params.assign(iter->second.begin(), iter->second.end());
        return iter->second[0];
      }
    }
    // std::cout<<"find_params3"<<std::endl;
    return nullptr;
  }

private:
  std::vector<std::map<std::string, Value *>> inner;
  std::vector<std::map<std::string, std::vector<Value *>>> array_param;
};

class SYSYCBuilder : public syntax_tree_visitor {
public:
  SYSYCBuilder() {
    module = std::shared_ptr<Module>(new Module("SysY code"));
    builder = std::unique_ptr<IRBuilder>(new IRBuilder(nullptr));
    auto TyVoid = Type::getVoidTy(&*module);
    auto TyInt32 = Type::getInt32Ty(&*module);
    auto TyPtr = Type::getInt32PtrTy(&*module);

    auto getint_type = FunctionType::get(TyInt32, {});
    // auto getch_type = FunctionType::get(TyInt32, false);
    auto getint_fun = Function::create(getint_type, "getint", module.get());

    auto getch_fun = Function::create(getint_type, "getch", module.get());

    std::vector<Type *> putint_params;
    putint_params.push_back(TyInt32);
    auto putint_type = FunctionType::get(TyVoid, putint_params);

    auto putint_fun = Function::create(putint_type, "putint", module.get());

    auto putch_fun = Function::create(putint_type, "putch", module.get());

    std::vector<Type *> getarray_params;
    getarray_params.push_back(TyPtr);
    auto getarray_type = FunctionType::get(TyInt32, getarray_params);

    auto getarray_fun =
        Function::create(getarray_type, "getarray", module.get());

    std::vector<Type *> putarray_params;
    putarray_params.push_back(TyInt32);
    putarray_params.push_back(TyPtr);
    auto putarray_type = FunctionType::get(TyInt32, putarray_params);

    auto putarray_fun =
        Function::create(putarray_type, "putarray", module.get());

    std::vector<Type *> starttime_params;
    starttime_params.push_back(TyInt32);

    auto starttime_type = FunctionType::get(TyVoid, starttime_params);

    auto starttime_fun =
        Function::create(starttime_type, "_sysy_starttime", module.get());

    std::vector<Type *> stoptime_params;
    stoptime_params.push_back(TyInt32);
    auto stoptime_type = FunctionType::get(TyVoid, stoptime_params);

    auto stoptime_fun =
        Function::create(stoptime_type, "_sysy_stoptime", module.get());

    auto mtstart_type = FunctionType::get(TyInt32, {});

    auto mtstart_fun =
        Function::create(mtstart_type, "__mtstart", module.get());

    std::vector<Type *> mtend_params;
    mtend_params.push_back(TyInt32);
    auto mtend_type = FunctionType::get(TyVoid, mtend_params);

    auto mtend_fun = Function::create(mtend_type, "__mtend", module.get());

    scope.enter();
    scope.push("getint", getint_fun);
    scope.push("getch", getch_fun);
    scope.push("putint", putint_fun);
    scope.push("putch", putch_fun);
    scope.push("getarray", getarray_fun);
    scope.push("putarray", putarray_fun);
    scope.push("_sysy_starttime", starttime_fun);
    scope.push("_sysy_stoptime", stoptime_fun);

    scope.push("__mtstart", mtstart_fun);
    scope.push("__mtend", mtend_fun);
  }

  void IRprint() { module->print(); }

  void HighIRprint() { module->HighIRprint(); }

  std::shared_ptr<Module> getModule() { return this->module; }

private:
  virtual void visit(SyntaxCompUnit &) override final;
  virtual void visit(SyntaxConstDecl &) override final;
  virtual void visit(SyntaxConstDef &) override final;
  virtual void visit(SyntaxConstInitVal &) override final;
  virtual void visit(SyntaxVarDecl &) override final;
  virtual void visit(SyntaxVarDef &) override final;
  virtual void visit(SyntaxInitVal &) override final;
  virtual void visit(SyntaxFuncDef &) override final;
  virtual void visit(SyntaxFuncFParam &) override final;
  virtual void visit(SyntaxBlock &) override final;
  virtual void visit(SyntaxBreakStmt &) override final;
  virtual void visit(SyntaxContinueStmt &) override final;
  virtual void visit(SyntaxAssignStmt &) override final;
  virtual void visit(SyntaxSelectStmt &) override final;
  virtual void visit(SyntaxIterationStmt &) override final;
  virtual void visit(SyntaxReturnStmt &) override final;
  virtual void visit(SyntaxLVal &) override final;
  virtual void visit(SyntaxPrimaryExp &) override final;
  virtual void visit(SyntaxNumber &) override final;
  virtual void visit(SyntaxUnaryExp &) override final;
  virtual void visit(SyntaxCallee &) override final;
  virtual void visit(SyntaxMulExp &) override final;
  virtual void visit(SyntaxAddExp &) override final;
  virtual void visit(SyntaxRelExp &) override final;
  virtual void visit(SyntaxEqExp &) override final;
  virtual void visit(SyntaxLAndExp &) override final;
  virtual void visit(SyntaxLOrExp &) override final;
  virtual void visit(SyntaxConstExp &) override final;
  std::unique_ptr<IRBuilder> builder;
  Scope scope;
  std::shared_ptr<Module> module;
};
#endif // SYSYC_SYSYBUILDERLLVM_HPP
