#include "GlobalVariableLocal.h"
#include "ReturnVal.h"

#define CONST(num, module) ConstantInt::get(num, module)

void GlobalVariableLocal::createFuncSucc() {
  for (auto f : m_->getFunctions()) {
    for (auto use : f->getUseList()) {
      auto instr = dynamic_cast<CallInst *>(use.val_);
      exit_ifnot(_funcUsedByNonCall_createFuncSucc_FunctionInline, instr);
      auto use_f = instr->getParent()->getParent();
      func_succ_[use_f].insert(f);
    }
  }
}

void GlobalVariableLocal::detectRecursive() {
  auto entry = m_->getMainFunction();
  std::set<Function *> visited;
  visitFunc(entry, visited);
}

void GlobalVariableLocal::visitFunc(Function *entry,
                                    std::set<Function *> &visited) {
  visited.insert(entry);
  for (auto succ : func_succ_[entry]) {
    if (visited.find(succ) != visited.end()) {
      recursive_func_.insert(succ);
    } else {
      visitFunc(succ, visited);
    }
  }
  visited.erase(entry);
}

void GlobalVariableLocal::run() {
  createFuncSucc();
  detectRecursive();
  std::map<GlobalVariable *, std::set<Function *>> var_func_use;
  // generate which func use global var
  for (auto var : m_->getGlobalVariables()) {
    for (auto use : var->getUseList()) {
      auto instr = dynamic_cast<Instruction *>(use.val_);
      exit_ifnot(_ConvInsErr_run_GlobalVarivaleLocal, instr);

      auto use_f = instr->getParent()->getParent();
      var_func_use[var].insert(use_f);
    }
  }
  auto entry = m_->getMainFunction();
  auto &global_vars = m_->getGlobalVariables();
  if (global_vars.size() > 10)
    return;
  for (auto iter = global_vars.begin(); iter != global_vars.end();) {
    auto var = *iter;
    // unused global variable
    if (var_func_use[var].size() == 0) {
      // std::cerr << "Remove global variable " << var->getName() << std::endl;
      iter = global_vars.erase(iter);
      continue;
    }
    // only local global var which has only one func
    else if (var_func_use[var].size() == 1) {
      auto func = *(var_func_use[var].begin());
      if (func == entry &&
          recursive_func_.find(func) == recursive_func_.end()) {
        localGlobalVariable(var, func);
        iter = global_vars.erase(iter);
        continue;
      }
      // std::cerr << "Global variable " << var->getName()
      //     << " local to function " << func->getName() << std::endl;
      // remove it
    } else if (var->getType()->isArrayTy()) {
      for (auto func : var_func_use[var])
        genGEPLocal(var, func);
    }
    ++iter;
  }
}

void GlobalVariableLocal::localGlobalVariable(GlobalVariable *global_var,
                                              Function *func) {
  auto alloca =
      AllocaInst::createAlloca(global_var->getType()->getPointerElementType());
  auto entry = func->getEntryBlock();
  insert_bb_ = entry;
  init_instr_.clear();
  init_instr_.push_back(alloca);
  alloca->setParent(entry);

  if (global_var->getType()->getPointerElementType()->isArrayTy()) {
    alloca->setInit();
    if (global_var->getInit()->getType()->isArrayTy()) {
      genArrayInitStores(alloca, global_var->getInit());
    }
  } else {
    auto ret = genArrayInitStores(alloca, global_var->getInit());
    if (!ret) {
      auto store = StoreInst::createStore(m_, {CONST(0, m_)}, alloca);
      store->setParent(insert_bb_);
      init_instr_.push_back(store);
    }
  }

  auto &entry_insts = insert_bb_->getInstructions();
  entry_insts.insert(entry_insts.begin(), init_instr_.begin(),
                     init_instr_.end());
  global_var->replaceAllUseWith(alloca);
}

bool GlobalVariableLocal::genArrayInitStores(Instruction *ptr,
                                             Constant *init_var) {
  bool is_init = false;
  if (init_var->getType()->isArrayTy()) {
    auto arr = static_cast<ConstantArray *>(init_var);
    for (int i = 0; i < arr->getNumElements(); i++) {
      auto gep = GetElementPtrInst::createGEP(ptr, {CONST(i, m_)});
      gep->setParent(insert_bb_);
      init_instr_.push_back(gep);
      bool ret = genArrayInitStores(gep, arr->getElementValue(i));
      if (!ret) {
        init_instr_.remove(gep);
        gep->removeUseOfOps();
        delete gep->getType();
        delete gep;
      } else {
        is_init = true;
      }
    }
  } else {
    if (static_cast<ConstantInt *>(init_var)->getValue() == 0)
      return false;
    auto store = StoreInst::createStore(m_, init_var, ptr);
    store->setParent(insert_bb_);
    init_instr_.push_back(store);
    is_init = true;
  }
  return is_init;
}

void GlobalVariableLocal::genGEPLocal(GlobalVariable *global_var,
                                      Function *func) {
  auto entry = func->getEntryBlock();
  auto gep = GetElementPtrInst::createGEP(global_var, {CONST(0, m_)});
  auto &entry_insts = insert_bb_->getInstructions();
  auto iter = entry_insts.begin();
  for (; iter != entry_insts.end(); ++iter) {
    if (!(*iter)->isAlloca())
      break;
  }
  entry_insts.insert(++iter, gep);
  gep->setParent(entry);
  for (auto use : global_var->getUseList()) {
    auto inst = dynamic_cast<Instruction *>(use.val_);
    if (inst && inst->getFunction() == func) {
      global_var->removeUse(inst, use.arg_no_);
      inst->setOperand(use.arg_no_, gep);
    }
  }
}
