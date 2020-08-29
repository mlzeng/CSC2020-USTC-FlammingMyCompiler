#include "SYSYCBuilder.h"
#include <iostream>

#define CONST(num) ConstantInt::get(num, &*module)

#define _IRBUILDER_ERROR_(str)                                                 \
  {                                                                            \
    std::cerr << "Error in IRbuilder-> " << str << std::endl;                  \
    std::abort();                                                              \
  }

// type
Type *TyInt32;
Type *TyVoid;
Type *TyInt32Ptr;
Type *TyInt1;

// store temporary value
Value *tmp_val = nullptr;
// whether require lvalue
bool require_address = false;
// function that is being built
Function *cur_fun = nullptr;
// detect scope pre-enter (for elegance only)
bool pre_enter_scope = false;
//
std::vector<Value *> array_init;
//
// std::vector<BasicBlock*> iter_expr,iter_cont;
enum CurBaseListType { WHILE_COND, WHILE_BODY, IF_COND, IF_THEN, IF_ELSE };

std::vector<CurBaseListType> BL_types;

std::vector<BaseBlock *> base_layer;
// BaseBlock *cur_base = nullptr;
// BaseBlock *tmp_base;
// TODO(zyh) 待确认
// std::vector<BasicBlock*> intendedBB {nullptr};

int tmp_int = 0;
bool use_int = false;
bool in_global_init = false;

std::list<BaseBlock *> &getCurBaseList(CurBaseListType ty) {
  auto cur_base = base_layer[base_layer.size() - 1];
  switch (ty) {
  case WHILE_COND:
    assert(cur_base->isWhileBlock());
    return dynamic_cast<WhileBlock *>(cur_base)->getCondBBs();
    break;

  case WHILE_BODY:
    assert(cur_base->isWhileBlock());
    return dynamic_cast<WhileBlock *>(cur_base)->getBodyBBs();
    break;

  case IF_COND:
    assert(cur_base->isIfBlock());
    return dynamic_cast<IfBlock *>(cur_base)->getCondBBs();
    break;

  case IF_THEN:
    assert(cur_base->isIfBlock());
    return dynamic_cast<IfBlock *>(cur_base)->getIfBodyBBs();
    break;

  case IF_ELSE:
    assert(cur_base->isIfBlock());
    return dynamic_cast<IfBlock *>(cur_base)->getElseBodyBBs();
    break;

  default:
    break;
  }
  std::list<BaseBlock *> ret;
  return ret;
}

Constant *ToConstArray(std::vector<int32_t> &array_bounds,
                       std::vector<Value *> &array_init) {
  auto cur_bnd = array_bounds[0];
  std::vector<int32_t> bounds;
  bounds.assign(array_bounds.begin() + 1, array_bounds.end());
  std::vector<Constant *> init_list;

  int d_length = array_init.size() / cur_bnd;
  if (d_length == 1) {
    for (int i = 0; i < cur_bnd; i++) {
      init_list.push_back(static_cast<Constant *>(array_init[i]));
    }
  } else {
    for (int i = 0; i < cur_bnd; i++) {
      std::vector<Value *> init;
      init.assign(array_init.begin() + d_length * i,
                  array_init.begin() + d_length * (i + 1));
      auto const_array = ToConstArray(bounds, init);
      init_list.push_back(const_array);
    }
  }
  Type *TyArray = TyInt32;
  for (int i = (array_bounds.size() - 1); i >= 0; i--) {
    TyArray = ArrayType::get(TyArray, static_cast<uint64_t>(array_bounds[i]));
  }
  return ConstantArray::get(static_cast<ArrayType *>(TyArray), init_list);
}

void SYSYCBuilder::visit(SyntaxCompUnit &node) {
  TyInt32 = Type::getInt32Ty(&*module);
  TyVoid = Type::getVoidTy(&*module);
  TyInt32Ptr = Type::getInt32PtrTy(&*module);
  TyInt1 = Type::getInt1Ty(&*module);
  // std::cout<<TyInt32->getTypeID()<<std::endl;
  for (auto DeclDef : node.DeclDefList) {
    DeclDef->accept(*this);
  }
}

void SYSYCBuilder::visit(SyntaxConstDecl &node) {
  for (auto p : node.ConstDefList) {
    p->accept(*this);
  }
}

void SYSYCBuilder::visit(SyntaxConstDef &node) {
  if (node.ArrayConstExpList.size() == 0) {
    if (node.ConstInitVal != nullptr) {
      node.ConstInitVal->accept(*this);
      scope.push(node.id, tmp_val);
    } else {
      _IRBUILDER_ERROR_("ConstDef")
    }
  } else {
    Type *TyArray = TyInt32;
    std::vector<int32_t> array_bounds;
    for (int i = 0; i < node.ArrayConstExpList.size(); i++) {
      auto array_const_exp = node.ArrayConstExpList[i];
      array_const_exp->accept(*this);
      auto dim_value = static_cast<ConstantInt *>(tmp_val)->getValue();
      array_bounds.push_back(dim_value);
    }
    for (int i = (array_bounds.size() - 1); i >= 0; i--) {
      TyArray = ArrayType::get(TyArray, array_bounds[i]);
    }
    if (scope.in_global()) {
      if (node.ConstInitVal != nullptr) {
        node.ConstInitVal->bounds.assign(array_bounds.begin(),
                                         array_bounds.end());
        node.ConstInitVal->accept(*this);
        auto initializer = ToConstArray(array_bounds, array_init);
        auto var = GlobalVariable::create(node.id, &*module, TyArray, false,
                                          initializer);
        scope.push(node.id, var);
      } else {
        auto var =
            GlobalVariable::create(node.id, &*module, TyArray, false, CONST(0));
        scope.push(node.id, var);
      }
    } else { // local var
      auto array_alloc = builder->CreateAlloca(TyArray);
      scope.push(node.id, array_alloc);
      if (node.ConstInitVal != nullptr) {
        array_alloc->setInit();
        node.ConstInitVal->bounds.assign(array_bounds.begin(),
                                         array_bounds.end());
        node.ConstInitVal->accept(*this);

        auto Ptr = builder->CreateGEP(array_alloc, {CONST(0)});
        for (int i = 1; i < node.ConstInitVal->bounds.size(); i++) {
          Ptr = builder->CreateGEP(Ptr, {CONST(0)});
        }
        for (int i = 0; i < array_init.size(); i++) {
          auto const_zero = dynamic_cast<ConstantInt *>(array_init[i]);
          if (const_zero) {
            if (const_zero->getValue() == 0) {
              continue;
            }
          }
          if (i != 0) {
            auto p = builder->CreateGEP(Ptr, {CONST(i)});
            builder->CreateStore(array_init[i], p);
          } else {
            builder->CreateStore(array_init[i], Ptr);
          }
        }
      }
    } // get alloc
  }
}

void SYSYCBuilder::visit(SyntaxConstInitVal &node) {
  if (node.ConstExp != nullptr && node.bounds.size() == 0) {
    node.ConstExp->accept(*this);
  } else {
    auto cur_bnd = node.bounds[0];
    auto d_length = 1;
    for (int i = 1; i < node.bounds.size(); i++) {
      d_length *= node.bounds[i];
    }
    std::vector<Value *> init_list;
    for (int i = 0; i < node.ConstInitValList.size(); i++) {
      auto init_val = node.ConstInitValList[i];
      if (init_val->ConstExp != nullptr) {
        init_val->ConstExp->accept(*this);
        init_list.push_back(tmp_val);
      } else {
        auto pos = init_list.size();
        for (int j = 0; j < (d_length - (pos % d_length)) % d_length; j++) {
          init_list.push_back(CONST(0));
        }
        init_val->bounds.assign(node.bounds.begin() + 1, node.bounds.end());
        init_val->accept(*this);
        init_list.insert(init_list.end(), array_init.begin(), array_init.end());
      }
    }
    for (int i = init_list.size(); i < d_length * cur_bnd; i++) {
      init_list.push_back(CONST(0));
    }
    array_init.assign(init_list.begin(), init_list.end());
  }
}

void SYSYCBuilder::visit(SyntaxVarDecl &node) {
  for (auto p : node.VarDefList) {
    p->accept(*this);
  }
}

void SYSYCBuilder::visit(SyntaxVarDef &node) {
  if (node.ArrayConstExpList.size() == 0) {
    if (scope.in_global()) {
      if (node.InitVal != nullptr) {
        in_global_init = true;
        node.InitVal->accept(*this);
        in_global_init = false;
        auto initializer = static_cast<Constant *>(tmp_val);
        auto var = GlobalVariable::create(node.id, &*module, TyInt32, false,
                                          initializer);
        scope.push(node.id, var);
      } else {
        auto var =
            GlobalVariable::create(node.id, &*module, TyInt32, false, CONST(0));
        scope.push(node.id, var);
      }
    } else {
      auto val_alloc = builder->CreateAlloca(TyInt32);
      scope.push(node.id, val_alloc);
      if (node.InitVal != nullptr) { // assign
        node.InitVal->accept(*this);
        builder->CreateStore(tmp_val, val_alloc);
      }
    }
  } else {
    Type *TyArray = TyInt32;
    std::vector<int32_t> array_bounds;
    for (int i = 0; i < node.ArrayConstExpList.size(); i++) {
      auto array_const_exp = node.ArrayConstExpList[i];
      array_const_exp->accept(*this);
      auto dim_value = static_cast<ConstantInt *>(tmp_val)->getValue();
      array_bounds.push_back(dim_value);
    }
    for (int i = (array_bounds.size() - 1); i >= 0; i--) {
      TyArray = ArrayType::get(TyArray, static_cast<uint64_t>(array_bounds[i]));
    } // get array type
    if (scope.in_global()) {
      if (node.InitVal != nullptr) {
        node.InitVal->bounds.assign(array_bounds.begin(), array_bounds.end());
        in_global_init = true;
        // std::cout<<"ok"<<std::endl;
        node.InitVal->accept(*this);
        in_global_init = false;
        auto initializer = ToConstArray(array_bounds, array_init);
        auto var = GlobalVariable::create(node.id, &*module, TyArray, false,
                                          initializer);
        scope.push(node.id, var);
      } else {
        auto var =
            GlobalVariable::create(node.id, &*module, TyArray, false, CONST(0));
        scope.push(node.id, var);
      }
    } else { // local var
      auto array_alloc = builder->CreateAlloca(TyArray);
      scope.push(node.id, array_alloc);
      if (node.InitVal != nullptr) {
        array_alloc->setInit();
        node.InitVal->bounds.assign(array_bounds.begin(), array_bounds.end());
        node.InitVal->accept(*this);

        auto Ptr = builder->CreateGEP(array_alloc, {CONST(0)});
        for (int i = 1; i < node.InitVal->bounds.size(); i++) {
          Ptr = builder->CreateGEP(Ptr, {CONST(0)});
        }
        for (int i = 0; i < array_init.size(); i++) {
          auto const_zero = dynamic_cast<ConstantInt *>(array_init[i]);
          if (const_zero) {
            if (const_zero->getValue() == 0) {
              continue;
            }
          }
          if (i != 0) {
            auto p = builder->CreateGEP(Ptr, {CONST(i)});
            builder->CreateStore(array_init[i], p);
          } else {
            builder->CreateStore(array_init[i], Ptr);
          }
        }
      }
    } // get alloc
  }
}

void SYSYCBuilder::visit(SyntaxInitVal &node) {
  if (node.Exp != nullptr && node.bounds.size() == 0) {
    if (in_global_init) {
      use_int = true;
      node.Exp->accept(*this);
      // std::cout<<tmp_int<<std::endl;
      tmp_val = CONST(tmp_int);
      use_int = false;
    } else {
      node.Exp->accept(*this);
    }
  } else {
    auto cur_bnd = node.bounds[0];
    auto d_length = 1;
    for (int i = 1; i < node.bounds.size(); i++) {
      d_length *= node.bounds[i];
    }
    std::vector<Value *> init_list;
    for (int i = 0; i < node.InitValList.size(); i++) {
      auto init_val = node.InitValList[i];
      if (init_val->Exp != nullptr) {
        if (in_global_init) {
          use_int = true;
          init_val->Exp->accept(*this);
          // std::cout<<tmp_int<<std::endl;
          tmp_val = CONST(tmp_int);
          use_int = false;
        } else {
          init_val->Exp->accept(*this);
        }
        init_list.push_back(tmp_val);
      } else {
        auto pos = init_list.size();
        for (int j = 0; j < (d_length - (pos % d_length)) % d_length; j++) {
          init_list.push_back(CONST(0));
        }
        init_val->bounds.assign(node.bounds.begin() + 1, node.bounds.end());
        init_val->accept(*this);
        init_list.insert(init_list.end(), array_init.begin(), array_init.end());
      }
    }
    for (int i = init_list.size(); i < d_length * cur_bnd; i++) {
      init_list.push_back(CONST(0));
    }
    array_init.assign(init_list.begin(), init_list.end());
  }
}

void SYSYCBuilder::visit(SyntaxFuncDef &node) {
  FunctionType *fun_type;
  Type *ret_type;
  std::vector<Type *> param_types;
  if (node.type == TYPE_INT) {
    ret_type = TyInt32;
  } else {
    ret_type = TyVoid;
  }
  for (auto &param : node.FuncFParamList) {
    if (param->isarray) {
      param_types.push_back(TyInt32Ptr);
    } else {
      param_types.push_back(TyInt32);
    }
  }
  fun_type = FunctionType::get(ret_type, param_types);
  auto fun = Function::create(fun_type, node.id, module.get());
  scope.push(node.id, fun);
  cur_fun = fun;
  auto funBB = BasicBlock::create(&*module, "entry", fun);
  builder->SetInsertPoint(funBB);
  scope.enter();
  pre_enter_scope = true;
  std::vector<Argument *> args;
  for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++) {
    args.push_back(*arg);
  }
  for (int i = 0; i < node.FuncFParamList.size(); ++i) {
    if (node.FuncFParamList[i]->isarray) {
      auto array_alloc = builder->CreateAlloca(TyInt32Ptr);
      builder->CreateStore(static_cast<Value *>(args[i]), array_alloc);
      std::vector<Value *> array_params;
      array_params.push_back(CONST(0));
      for (auto array_param : node.FuncFParamList[i]->ParamArrayExpList) {
        array_param->accept(*this);
        array_params.push_back(tmp_val);
      }
      scope.push(node.FuncFParamList[i]->id, array_alloc);
      scope.push_params(node.FuncFParamList[i]->id, array_alloc, array_params);
      args[i]->setArrayBound(array_params);
    } else {
      auto alloc = builder->CreateAlloca(TyInt32);
      builder->CreateStore(args[i], alloc);
      scope.push(node.FuncFParamList[i]->id, alloc);
    }
  }
  node.Block->accept(*this);
  // if (builder->GetInsertBlock()->getTerminator() == nullptr) {
  //     if (cur_fun->getResultType() == TyVoid) {
  //         builder->CreateVoidRet();
  //     }
  // else if (builder->GetInsertBlock()->empty()) {
  //     auto parent = builder->GetInsertBlock()->getParent();
  //     parent->removeBasicBlock(builder->GetInsertBlock());
  // }
  // }
  scope.exit();
}

void SYSYCBuilder::visit(SyntaxFuncFParam &node) {}

void SYSYCBuilder::visit(SyntaxBlock &node) {
  bool need_exit_scope = !pre_enter_scope;
  if (pre_enter_scope) {
    pre_enter_scope = false;
  } else {
    scope.enter();
  }

  for (auto &blockitem : node.BlockItemList) {
    if (blockitem->ConstDecl || blockitem->VarDecl) {
      if (base_layer.size() == 0) {
        auto allocaBB = BasicBlock::create(&*module, "", cur_fun);
        builder->SetInsertPoint(allocaBB);
        blockitem->accept(*this);
      } else {
        auto allocaBB = BasicBlock::create(&*module, "");
        builder->SetInsertPoint(allocaBB);
        auto &cur_base_list = getCurBaseList(BL_types[BL_types.size() - 1]);
        cur_base_list.push_back(allocaBB);
        auto cur_base = base_layer[base_layer.size() - 1];
        allocaBB->setBaseFather(cur_base);
        blockitem->accept(*this);
      }
    } else if (blockitem->Stmt) {
      if (blockitem->Stmt->AssignStmt || blockitem->Stmt->BreakStmt ||
          blockitem->Stmt->ContinueStmt || blockitem->Stmt->Exp ||
          blockitem->Stmt->ReturnStmt) {
        if (base_layer.size() == 0) {
          if (builder->GetInsertBlock()->getBaseFather() != nullptr) {
            auto baseBB = BasicBlock::create(&*module, "", cur_fun);
            builder->SetInsertPoint(baseBB);
          }
          blockitem->accept(*this);
        } else {
          auto baseBB = BasicBlock::create(&*module, "");
          builder->SetInsertPoint(baseBB);
          auto &cur_base_list = getCurBaseList(BL_types[BL_types.size() - 1]);
          cur_base_list.push_back(baseBB);
          auto cur_base = base_layer[base_layer.size() - 1];
          baseBB->setBaseFather(cur_base);
          blockitem->accept(*this);
        }
      } else {
        blockitem->accept(*this);
      }
    }
    // if (builder->GetInsertBlock()->getTerminator() != nullptr)
    // break;
  }

  if (need_exit_scope) {
    scope.exit();
  }
}

void SYSYCBuilder::visit(SyntaxBreakStmt &node) {

  builder->CreateBreak(module.get());
  // auto cur_iter = iter_cont[iter_expr.size()-1];
  // builder->CreateBr(cur_iter);
  // builder->GetInsertBlock()->intendedBB = intendedBB.back();
}

void SYSYCBuilder::visit(SyntaxContinueStmt &node) {

  builder->CreateContinue(module.get());
  // auto cur_iter = iter_expr[iter_expr.size()-1];
  // builder->CreateBr(cur_iter);
  // builder->GetInsertBlock()->intendedBB = intendedBB.back();
}

void SYSYCBuilder::visit(SyntaxAssignStmt &node) {
  node.LVal->accept(*this);
  auto lval = tmp_val;
  node.Exp->accept(*this);
  auto rval = tmp_val;
  builder->CreateStore(rval, lval);
}

void SYSYCBuilder::visit(SyntaxSelectStmt &node) {
  IfBlock *ifBB;
  if (base_layer.size() == 0) {
    ifBB = IfBlock::create(module.get(), "", cur_fun);
  } else {
    ifBB = IfBlock::create(module.get(), "");
    auto &cur_base_list = getCurBaseList(BL_types[BL_types.size() - 1]);
    cur_base_list.push_back(ifBB);
    auto cur_base = base_layer[base_layer.size() - 1];
    ifBB->setBaseFather(cur_base);
  }

  base_layer.push_back(ifBB);

  auto exprBB = BasicBlock::create(&*module, "");
  builder->SetInsertPoint(exprBB);
  ifBB->addCondBB(exprBB);
  node.Cond->accept(*this);

  if (node.ifStmt->Block || node.ifStmt->SelectStmt ||
      node.ifStmt->IterationStmt) {
    BL_types.push_back(IF_THEN);
    node.ifStmt->accept(*this);
    BL_types.pop_back();
  } else if (node.ifStmt->AssignStmt || node.ifStmt->BreakStmt ||
             node.ifStmt->ContinueStmt || node.ifStmt->Exp ||
             node.ifStmt->ReturnStmt) {
    auto trueBB = BasicBlock::create(&*module, "");
    builder->SetInsertPoint(trueBB);
    ifBB->addIfBodyBB(trueBB);
    node.ifStmt->accept(*this);
  }

  if (node.elseStmt != nullptr) {
    if (node.elseStmt->Block || node.elseStmt->SelectStmt ||
        node.elseStmt->IterationStmt) {
      BL_types.push_back(IF_ELSE);
      node.elseStmt->accept(*this);
      BL_types.pop_back();
    } else if (node.elseStmt->AssignStmt || node.elseStmt->BreakStmt ||
               node.elseStmt->ContinueStmt || node.elseStmt->Exp ||
               node.elseStmt->ReturnStmt) {
      auto falseBB = BasicBlock::create(&*module, "");
      builder->SetInsertPoint(falseBB);
      ifBB->addElseBodyBB(falseBB);
      node.elseStmt->accept(*this);
    }
  }

  base_layer.pop_back();

  // node.Cond->accept(*this);
  // auto cond_val = tmp_val;
  // auto trueBB = BasicBlock::create(&*module, "", cur_fun);
  // auto falseBB = BasicBlock::create(&*module, "", cur_fun);
  // auto contBB = BasicBlock::create(&*module, "", cur_fun);
  // if (node.elseStmt == nullptr) {
  //     builder->CreateCondBr(cond_val, trueBB, contBB);
  // } else {
  //     builder->CreateCondBr(cond_val, trueBB, falseBB);
  // }

  // builder->SetInsertPoint(trueBB);
  // // TODO(zyh) 待确认
  // // intendedBB.push_back(contBB);

  // node.ifStmt->accept(*this);
  // if (builder->GetInsertBlock()->getTerminator() == nullptr)
  //     builder->CreateBr(contBB);
  // // TODO(zyh) 待确认
  // // intendedBB.pop_back();

  // if (node.elseStmt == nullptr) {
  //     auto parent = falseBB->getParent();
  //     parent->removeBasicBlock(falseBB);
  // } else {
  //     builder->SetInsertPoint(falseBB);
  //     // TODO(zyh) 待确认
  //     // intendedBB.push_back(contBB);

  //     node.elseStmt->accept(*this);
  //     if (builder->GetInsertBlock()->getTerminator() == nullptr)
  //     builder->CreateBr(contBB);

  //     // intendedBB.pop_back();
  // }
  // builder->SetInsertPoint(contBB);
}

void SYSYCBuilder::visit(SyntaxIterationStmt &node) {

  WhileBlock *whileBB;
  if (base_layer.size() == 0) {
    whileBB = WhileBlock::create(module.get(), "", cur_fun);
  } else {
    whileBB = WhileBlock::create(module.get(), "");
    auto &cur_base_list = getCurBaseList(BL_types[BL_types.size() - 1]);
    cur_base_list.push_back(whileBB);
    auto cur_base = base_layer[base_layer.size() - 1];
    whileBB->setBaseFather(cur_base);
  }

  base_layer.push_back(whileBB);

  auto exprBB = BasicBlock::create(&*module, "");
  builder->SetInsertPoint(exprBB);

  whileBB->addCondBB(exprBB);
  node.Cond->accept(*this);

  if (node.Stmt->Block || node.Stmt->SelectStmt || node.Stmt->IterationStmt) {
    BL_types.push_back(WHILE_BODY);
    node.Stmt->accept(*this);
    BL_types.pop_back();
  } else if (node.Stmt->AssignStmt || node.Stmt->BreakStmt ||
             node.Stmt->ContinueStmt || node.Stmt->Exp ||
             node.Stmt->ReturnStmt) {
    auto trueBB = BasicBlock::create(&*module, "");
    builder->SetInsertPoint(trueBB);
    whileBB->addBodyBB(trueBB);
    node.Stmt->accept(*this);
  }

  base_layer.pop_back();

  // if (builder->GetInsertBlock()->getTerminator() == nullptr)
  //     builder->CreateBr(exprBB);

  // iter_expr.push_back(exprBB);

  // auto cond_val = tmp_val;
  // auto trueBB = BasicBlock::create(&*module, "");
  // auto contBB = BasicBlock::create(&*module, "");

  // iter_cont.push_back(contBB);
  // builder->CreateCondBr(cond_val, trueBB, contBB);
  // builder->SetInsertPoint(trueBB);
  // TODO(zyh) 待确认
  // intendedBB.push_back(exprBB);

  // if (builder->GetInsertBlock()->getTerminator() == nullptr)
  //     builder->CreateBr(exprBB);

  // iter_expr.pop_back();
  // iter_cont.pop_back();
  // TODO(zyh) 待确认
  // intendedBB.pop_back();
  // builder->SetInsertPoint(contBB);
}

void SYSYCBuilder::visit(SyntaxReturnStmt &node) {
  if (node.Exp == nullptr) {
    builder->CreateVoidRet();
  } else {
    node.Exp->accept(*this);
    builder->CreateRet(tmp_val);
  }
  // builder->GetInsertBlock()->intendedBB = intendedBB.back();
}

void SYSYCBuilder::visit(SyntaxLVal &node) {
  auto var = scope.find(node.id);
  if (var->getType()->isIntegerTy()) { // constant
    tmp_val = var;
    return;
  }
  // std::cout<<var->getType()->getTypeID()<<std::endl;
  auto is_int = var->getType()->getPointerElementType()->isIntegerTy();
  auto is_ptr = var->getType()->getPointerElementType()->isPointerTy();
  if (node.ArrayExpList.size() == 0) {
    if (is_int) {
      tmp_val = scope.find(node.id);
    } else if (is_ptr) {
      tmp_val = builder->CreateLoad(var);
    } else {
      tmp_val = builder->CreateGEP(var, {CONST(0)});
    }
  } else {
    Value *tmp_ptr;
    if (is_int) {
      tmp_ptr = var;
      for (auto exp : node.ArrayExpList) {
        exp->accept(*this);
        tmp_ptr = builder->CreateGEP(tmp_ptr, {tmp_val});
      }
    } else if (is_ptr) {
      std::vector<Value *> array_params;
      scope.find_params(node.id, array_params);
      tmp_ptr = builder->CreateLoad(var); // array_load
      for (int i = 0; i < node.ArrayExpList.size(); i++) {
        node.ArrayExpList[i]->accept(*this);
        auto val = tmp_val;
        for (int j = i + 1; j < array_params.size(); j++) {
          val = builder->CreateMul(val, array_params[j]);
        }
        tmp_ptr = builder->CreateGEP(tmp_ptr, {val});
      }
    } else {
      tmp_ptr = var;
      for (auto exp : node.ArrayExpList) {
        exp->accept(*this);
        tmp_ptr = builder->CreateGEP(tmp_ptr, {tmp_val});
      }
    }
    tmp_val = tmp_ptr;
  }
}

void SYSYCBuilder::visit(SyntaxPrimaryExp &node) {
  if (use_int) {
    if (node.Exp) {
      node.Exp->accept(*this);
    } else if (node.LVal) {
      node.LVal->accept(*this);
      tmp_int = static_cast<ConstantInt *>(tmp_val)->getValue();
    } else if (node.Number) {
      node.Number->accept(*this);
    }
    return;
  }

  if (node.Exp) {
    node.Exp->accept(*this);
  } else if (node.LVal) {
    if (require_address) {
      require_address = false;
      node.LVal->accept(*this);
      while (!tmp_val->getType()->getPointerElementType()->isIntegerTy()) {
        tmp_val = builder->CreateGEP(tmp_val, {CONST(0)});
      }
    } else {
      node.LVal->accept(*this);
      if (tmp_val->getType()->isIntegerTy()) {
        return;
      }
      tmp_val = builder->CreateLoad(tmp_val);
    }
  } else if (node.Number) {
    node.Number->accept(*this);
  }
  return;
}

void SYSYCBuilder::visit(SyntaxNumber &node) {
  if (use_int) {
    tmp_int = node.num;
    return;
  }
  tmp_val = CONST(node.num);
}

void SYSYCBuilder::visit(SyntaxUnaryExp &node) {
  if (use_int) {
    int val;
    if (node.PrimaryExp) {
      node.PrimaryExp->accept(*this);
      val = tmp_int;
    } else if (node.UnaryExp) {
      node.UnaryExp->accept(*this);
      val = tmp_int;
    } else {
      _IRBUILDER_ERROR_("Function call in ConstExp!");
    }
    switch (node.op) {
    case OP_NEG:
      tmp_int = 0 - val;
      break;
    case OP_NOT:
      tmp_int = val != 0;
      _IRBUILDER_ERROR_("NOT operation in ConstExp!")
      break;
    }
    return;
  }

  Value *val;
  if (node.PrimaryExp) {
    node.PrimaryExp->accept(*this);
    val = tmp_val;
  } else if (node.UnaryExp) {
    node.UnaryExp->accept(*this);
    val = tmp_val;
  } else {
    node.Callee->accept(*this);
    val = tmp_val;
  }
  switch (node.op) {
  case OP_NEG:
    val = builder->CreateSub(CONST(0), val);
    break;
  case OP_NOT:
    val = builder->CreateCmpEQ(val, CONST(0), &*module);
    // val = builder->CreateZext(val, TyInt32);
    break;
  }
  tmp_val = val;
}

void SYSYCBuilder::visit(SyntaxCallee &node) {
  auto fun = scope.find(node.id);
  if (fun == nullptr) {
    exit(120);
  }
  std::vector<Value *> args;
  for (int i = 0; i < node.ExpList.size(); i++) {
    auto arg = node.ExpList[i];
    auto arg_type =
        static_cast<Function *>(fun)->getFunctionType()->getArgType(i);
    if (arg_type->isIntegerTy()) {
      require_address = false;
    } else {
      require_address = true;
    }
    arg->accept(*this);
    require_address = false;
    args.push_back(tmp_val);
  }
  tmp_val = builder->CreateCall(static_cast<Function *>(fun), args);
}

void SYSYCBuilder::visit(SyntaxMulExp &node) {
  if (node.MulExp == nullptr) {
    node.UnaryExp->accept(*this);
  } else {
    if (use_int) {
      node.MulExp->accept(*this);
      auto l_val = tmp_int;
      node.UnaryExp->accept(*this);
      auto r_val = tmp_int;
      switch (node.op) {
      case OP_MUL:
        tmp_int = l_val * r_val;
        break;
      case OP_DIV:
        tmp_int = l_val / r_val;
        break;
      case OP_MOD:
        tmp_int = l_val % r_val;
        break;
      }
      return;
    }

    node.MulExp->accept(*this);
    auto l_val = tmp_val;
    node.UnaryExp->accept(*this);
    auto r_val = tmp_val;

    if (l_val->getType()->isInt1()) {
      l_val = builder->CreateZext(l_val, TyInt32);
    }

    if (r_val->getType()->isInt1()) {
      r_val = builder->CreateZext(r_val, TyInt32);
    }

    switch (node.op) {
    case OP_MUL:
      tmp_val = builder->CreateMul(l_val, r_val);
      break;
    case OP_DIV:
      tmp_val = builder->CreateDiv(l_val, r_val);
      break;
    case OP_MOD:
      tmp_val = builder->CreateRem(l_val, r_val);
      break;
    }
  }
}

void SYSYCBuilder::visit(SyntaxAddExp &node) {
  if (node.AddExp == nullptr) {
    node.MulExp->accept(*this);
  } else {

    if (use_int) {
      node.AddExp->accept(*this);
      auto l_val = tmp_int;
      node.MulExp->accept(*this);
      auto r_val = tmp_int;
      switch (node.op) {
      case OP_PLUS:
        tmp_int = l_val + r_val;
        break;
      case OP_MINUS:
        tmp_int = l_val - r_val;
        break;
      }
      return;
    }

    node.AddExp->accept(*this);
    auto l_val = tmp_val;
    node.MulExp->accept(*this);
    auto r_val = tmp_val;

    if (l_val->getType()->isInt1()) {
      l_val = builder->CreateZext(l_val, TyInt32);
    }

    if (r_val->getType()->isInt1()) {
      r_val = builder->CreateZext(r_val, TyInt32);
    }

    switch (node.op) {
    case OP_PLUS:
      tmp_val = builder->CreateAdd(l_val, r_val);
      break;
    case OP_MINUS:
      tmp_val = builder->CreateSub(l_val, r_val);
      break;
    }
  }
}

void SYSYCBuilder::visit(SyntaxRelExp &node) {
  Value *logicalVal;
  if (node.RelExp == nullptr) {
    node.AddExp->accept(*this);
  } else {
    node.RelExp->accept(*this);
    auto lval = tmp_val;
    node.AddExp->accept(*this);
    auto rval = tmp_val;

    if (lval->getType()->isInt1()) {
      lval = builder->CreateZext(lval, TyInt32);
    }

    if (rval->getType()->isInt1()) {
      rval = builder->CreateZext(rval, TyInt32);
    }

    switch (node.op) {
    case OP_LTE:
      logicalVal = builder->CreateCmpLE(lval, rval, &*module);
      break;
    case OP_LT:
      logicalVal = builder->CreateCmpLT(lval, rval, &*module);
      break;
    case OP_GT:
      logicalVal = builder->CreateCmpGT(lval, rval, &*module);
      break;
    case OP_GTE:
      logicalVal = builder->CreateCmpGE(lval, rval, &*module);
      break;
    }
    tmp_val = logicalVal;
    // tmp_val = builder->CreateZext(logicalVal, TyInt32);
  }
}

void SYSYCBuilder::visit(SyntaxEqExp &node) {
  if (node.EqExp == nullptr) {
    node.RelExp->accept(*this);
  } else {
    node.EqExp->accept(*this);
    auto lval = tmp_val;
    node.RelExp->accept(*this);
    auto rval = tmp_val;
    if (lval->getType()->isInt1()) {
      lval = builder->CreateZext(lval, TyInt32);
    }

    if (rval->getType()->isInt1()) {
      rval = builder->CreateZext(rval, TyInt32);
    }

    switch (node.op) {
    case OP_EQ:
      tmp_val = builder->CreateCmpEQ(lval, rval, &*module);
      break;
    case OP_NEQ:
      tmp_val = builder->CreateCmpNE(lval, rval, &*module);
      break;
    }
    // tmp_val = builder->CreateZext(tmp_val, TyInt32);
  }
}

void SYSYCBuilder::visit(SyntaxLAndExp &node) {
  if (node.LAndExp == nullptr) {
    node.EqExp->accept(*this);
    // tmp_val = builder->CreateCmpNE(tmp_val, CONST(0), &*module);
  } else {
    node.LAndExp->accept(*this);
    auto lval = tmp_val;
    node.EqExp->accept(*this);
    auto rval = tmp_val;
    // builder->CreateCmpNE(tmp_val, CONST(0), &*module);
    if (lval->getType()->isInt1()) {
      lval = builder->CreateZext(lval, TyInt32);
    }

    if (rval->getType()->isInt1()) {
      rval = builder->CreateZext(rval, TyInt32);
    }

    tmp_val = builder->CreateAnd(lval, rval);
  }
}

void SYSYCBuilder::visit(SyntaxLOrExp &node) {
  if (node.LOrExp == nullptr) {
    node.LAndExp->accept(*this);
    if (tmp_val->getType()->isInt32()) {
      tmp_val = builder->CreateCmpNE(tmp_val, CONST(0), &*module);
    }
  } else {
    node.LOrExp->accept(*this);
    auto lval = tmp_val;
    node.LAndExp->accept(*this);
    auto rval = tmp_val;
    if (lval->getType()->isInt1()) {
      lval = builder->CreateZext(lval, TyInt32);
    }

    if (rval->getType()->isInt1()) {
      rval = builder->CreateZext(rval, TyInt32);
    }

    tmp_val = builder->CreateOr(lval, rval);
    if (tmp_val->getType()->isInt32()) {
      tmp_val = builder->CreateCmpNE(tmp_val, CONST(0), &*module);
    }
  }
}

void SYSYCBuilder::visit(SyntaxConstExp &node) {
  use_int = true;
  node.AddExp->accept(*this);
  tmp_val = CONST(tmp_int);
  use_int = false;
}