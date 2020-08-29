#include "syntax_tree.hh"
#include <iostream>
#include <string>

// #define DEBUG

#define _SYNTAX_TREE_NODE_ERROR_                                               \
  std::cerr << "Abort due to node cast error." << std::endl;                   \
  std::abort();

/*
更改情况
*/

void syntax_tree::run_visitor(syntax_tree_visitor &visitor) {
  root->accept(visitor);
}

// void SyntaxCompUnit::accept(syntax_tree_visitor &visitor)
// {visitor.visit(*this);}
void SyntaxDeclDef::accept(syntax_tree_visitor &visitor) {
#ifdef DEBUG
  std::cout << "SyntaxDeclDef" << std::endl;
#endif
  if (this->ConstDecl) {
    this->ConstDecl->accept(visitor);
    return;
  } else if (this->FuncDef) {
    this->FuncDef->accept(visitor);
    return;
  } else if (this->VarDecl) {
    this->VarDecl->accept(visitor);
    return;
  }
  _SYNTAX_TREE_NODE_ERROR_
}

// void SyntaxConstDecl::accept(syntax_tree_visitor &visitor)
// {visitor.visit(*this);} void SyntaxConstDef::accept(syntax_tree_visitor
// &visitor) {visitor.visit(*this);} void
// SyntaxConstInitVal::accept(syntax_tree_visitor &visitor)
// {visitor.visit(*this);} void SyntaxVarDecl::accept(syntax_tree_visitor
// &visitor) {visitor.visit(*this);} void
// SyntaxVarDef::accept(syntax_tree_visitor &visitor) {visitor.vist(*this);}
// void SyntaxInitVal::accept(syntax_tree_visitor &visitor)
// {visitor.visit(*this);} void SyntaxFuncDef::accept(syntax_tree_visitor
// &visitor) {visitor.visit(*this);} void
// SyntaxFuncFParam::accept(syntax_tree_visitor &visitor)
// {visitor.visit(*this);} void SyntaxBlock::accept(syntax_tree_visitor
// &visitor) {visitor.visit(*this);}

void SyntaxBlockItem::accept(syntax_tree_visitor &visitor) {
#ifdef DEBUG
  std::cout << "SyntaxBlockItem" << std::endl;
#endif
  if (this->ConstDecl) {
    this->ConstDecl->accept(visitor);
    return;
  } else if (this->VarDecl) {
    this->VarDecl->accept(visitor);
    return;
  } else if (this->Stmt) {
    this->Stmt->accept(visitor);
    return;
  }
  _SYNTAX_TREE_NODE_ERROR_
}

void SyntaxStmt::accept(syntax_tree_visitor &visitor) {
#ifdef DEBUG
  std::cout << "SyntaxStmt" << std::endl;
#endif
  if (this->AssignStmt) {
    this->AssignStmt->accept(visitor);
    return;
  } else if (this->Block) {
    this->Block->accept(visitor);
    return;
  } else if (this->BreakStmt) {
    this->BreakStmt->accept(visitor);
    return;
  } else if (this->ContinueStmt) {
    this->ContinueStmt->accept(visitor);
    return;
  } else if (this->Exp) {
    this->Exp->accept(visitor);
    return;
  } else if (this->IterationStmt) {
    this->IterationStmt->accept(visitor);
    return;
  } else if (this->ReturnStmt) {
    this->ReturnStmt->accept(visitor);
    return;
  } else if (this->SelectStmt) {
    this->SelectStmt->accept(visitor);
    return;
  }
  _SYNTAX_TREE_NODE_ERROR_
}

// void SyntaxBreakStmt::accept(syntax_tree_visitor &visitor)
// {visitor.visit(*this);} void SyntaxContinueStmt::accept(syntax_tree_visitor
// &visitor) {visitor.visit(*this);} void
// SyntaxAssignStmt::accept(syntax_tree_visitor &visitor)
// {visitor.visit(*this);} void SyntaxSelectStmt::accept(syntax_tree_visitor
// &visitor){visitor.visit(*this);} void
// SyntaxIterationStmt::accept(syntax_tree_visitor
// &visitor){visitor.visit(*this);} void
// SyntaxReturnStmt::accept(syntax_tree_visitor &visitor){visitor.visit(*this);}
void SyntaxExp::accept(syntax_tree_visitor &visitor) {
#ifdef DEBUG
  std::cout << "SyntaxExp" << std::endl;
#endif
  this->AddExp->accept(visitor);
}
void SyntaxCond::accept(syntax_tree_visitor &visitor) {
#ifdef DEBUG
  std::cout << "SyntaxCond" << std::endl;
#endif
  this->LOrExp->accept(visitor);
}
// void SyntaxLVal::accept(syntax_tree_visitor &visitor){visitor.visit(*this);}
void SyntaxPrimaryExp::accept(syntax_tree_visitor &visitor) {
  // #ifdef DEBUG
  // std::cout << "SyntaxPrimaryExp" << std::endl;
  // #endif
  visitor.visit(*this);
  // if (this->Exp) {
  //     this->Exp->accept(visitor);
  //     return;
  // } else if (this->LVal) {
  //     this->LVal->accept(visitor);
  //     return;
  // } else if (this->Number) {
  //     this->Number->accept(visitor);
  //     return;
  // }

  // _SYNTAX_TREE_NODE_ERROR_
}
// void SyntaxNumber::accept(syntax_tree_visitor
// &visitor){visitor.visit(*this);} void
// SyntaxUnaryExp::accept(syntax_tree_visitor &visitor){visitor.visit(*this);}
// void SyntaxCallee::accept(syntax_tree_visitor
// &visitor){visitor.visit(*this);} void
// SyntaxMulExp::accept(syntax_tree_visitor &visitor){visitor.visit(*this);}
// void SyntaxAddExp::accept(syntax_tree_visitor
// &visitor){visitor.visit(*this);} void
// SyntaxRelExp::accept(syntax_tree_visitor &visitor){visitor.visit(*this);}
// void SyntaxEqExp::accept(syntax_tree_visitor &visitor){visitor.visit(*this);}
// void SyntaxLAndExp::accept(syntax_tree_visitor
// &visitor){visitor.visit(*this);} void
// SyntaxLOrExp::accept(syntax_tree_visitor &visitor){visitor.visit(*this);}
void SyntaxConstExp::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxCompUnit::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxConstDecl::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxConstDef::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxConstInitVal::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxVarDecl::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxVarDef::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxInitVal::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxFuncDef::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxFuncFParam::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxBlock::accept(syntax_tree_visitor &visitor) { visitor.visit(*this); }
void SyntaxBreakStmt::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxContinueStmt::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxAssignStmt::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxSelectStmt::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxIterationStmt::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxReturnStmt::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxLVal::accept(syntax_tree_visitor &visitor) { visitor.visit(*this); }
void SyntaxNumber::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxUnaryExp::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxCallee::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxMulExp::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxAddExp::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxRelExp::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxEqExp::accept(syntax_tree_visitor &visitor) { visitor.visit(*this); }
void SyntaxLAndExp::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}
void SyntaxLOrExp::accept(syntax_tree_visitor &visitor) {
  visitor.visit(*this);
}

#define _DEBUG_PRINT_N_(N)                                                     \
  { std::cout << std::string(N, '-'); }

#define _TYPE_(t) (((t) == TYPE_INT) ? "int" : "void")

// TODO(zyh) 删除printer

void syntax_tree_printer::visit(SyntaxConstExp &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "const_exp" << std::endl;
  add_depth();
  node.AddExp->accept(*this);
  remove_depth();
}

void syntax_tree_printer::visit(SyntaxCompUnit &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "CompUnit" << std::endl;
  add_depth();
  for (auto decl : node.DeclDefList) {
    decl->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxConstDecl &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "ConstDecl" << _TYPE_(node.type) << std::endl;
  add_depth();
  for (auto p : node.ConstDefList) {
    p->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxConstDef &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "ConstDef:" << '\t' << node.id << '\n';
  add_depth();
  for (auto p : node.ArrayConstExpList) {
    p->accept(*this);
  }
  if (node.ConstInitVal) {
    node.ConstInitVal->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxConstInitVal &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "ConstInitVal" << std::endl;
  add_depth();
  if (node.ConstExp) {
    node.ConstExp->accept(*this);
  } else {
    for (auto p : node.ConstInitValList) {
      p->accept(*this);
    }
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxVarDecl &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "VarDecl:\t" << _TYPE_(node.type) << std::endl;
  add_depth();
  for (auto p : node.VarDefList) {
    p->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxVarDef &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "VarDef:\t" << node.id << std::endl;
  add_depth();
  for (auto p : node.ArrayConstExpList) {
    p->accept(*this);
  }
  if (node.InitVal) {
    node.InitVal->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxInitVal &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "ConstInitVal" << std::endl;
  add_depth();
  if (node.Exp) {
    node.Exp->accept(*this);
  } else {
    for (auto p : node.InitValList) {
      p->accept(*this);
    }
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxFuncDef &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "FuncDef:\t" << _TYPE_(node.type) << node.id << std::endl;
  add_depth();
  for (auto p : node.FuncFParamList) {
    p->accept(*this);
  }
  if (node.Block) {
    node.Block->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxFuncFParam &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "FuncFParam" << node.type << node.id
            << "\tisarray:" << node.isarray << '\n';
  add_depth();
  for (auto p : node.ParamArrayExpList) {
    p->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxBlock &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "Block" << std::endl;
  add_depth();
  for (auto p : node.BlockItemList) {
    p->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxBreakStmt &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "break;" << std::endl;
}
void syntax_tree_printer::visit(SyntaxContinueStmt &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "continue;\n";
}
void syntax_tree_printer::visit(SyntaxAssignStmt &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "AssignStmt" << std::endl;
  add_depth();
  node.LVal->accept(*this);
  node.Exp->accept(*this);
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxSelectStmt &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "SelectStmt" << std::endl;
  add_depth();
  node.Cond->accept(*this);
  node.ifStmt->accept(*this);
  if (node.elseStmt) {
    node.elseStmt->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxIterationStmt &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "WhileLoop" << std::endl;
  add_depth();
  if (node.Cond) {
    node.Cond->accept(*this);
  }
  node.Stmt->accept(*this);
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxReturnStmt &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "Return" << std::endl;
  add_depth();
  if (node.Exp) {
    node.Exp->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxLVal &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "Lval:\t" << node.id << std::endl;
  add_depth();
  for (auto p : node.ArrayExpList) {
    p->accept(*this);
  }
  remove_depth();
}

void syntax_tree_printer::visit(SyntaxPrimaryExp &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "PrimaryExp:\t" << std::endl;
  add_depth();
  if (node.Exp) {
    node.Exp->accept(*this);
  } else if (node.LVal) {
    node.LVal->accept(*this);
  } else if (node.Number) {
    node.Number->accept(*this);
  }
  remove_depth();
  // std::cerr<<"PrimaryExp"<<std::endl;
}

void syntax_tree_printer::visit(SyntaxNumber &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "Number:\t" << node.num << '\n';
}
void syntax_tree_printer::visit(SyntaxUnaryExp &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "UnaryExp: " << node.op << std::endl;
  add_depth();
  if (node.PrimaryExp) {
    node.PrimaryExp->accept(*this);
  } else if (node.Callee) {
    node.Callee->accept(*this);
  } else {
    // _DEBUG_PRINT_N_(depth);
    // std::cout << node.op<<std::endl;
    node.UnaryExp->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxCallee &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "Call:\t" << node.id << std::endl;
  add_depth();
  for (auto p : node.ExpList) {
    p->accept(*this);
  }
  remove_depth();
}
void syntax_tree_printer::visit(SyntaxMulExp &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "MulExp:\t" << node.op << std::endl;
  add_depth();
  if (node.MulExp) {
    node.MulExp->accept(*this);
  }
  if (node.UnaryExp) {
    node.UnaryExp->accept(*this);
  }
  remove_depth();
  // std::cerr<<"MulExp"<<std::endl;
}
void syntax_tree_printer::visit(SyntaxAddExp &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "AddExp:\t" << node.op << std::endl;
  add_depth();
  if (node.AddExp) {
    node.AddExp->accept(*this);
  }
  if (node.MulExp) {
    node.MulExp->accept(*this);
  }
  remove_depth();
  // std::cerr<<"AddExp"<<std::endl;
}
void syntax_tree_printer::visit(SyntaxRelExp &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "RelExp:\t" << node.op << std::endl;
  add_depth();
  if (node.RelExp) {
    node.RelExp->accept(*this);
  }
  if (node.AddExp) {
    node.AddExp->accept(*this);
  }
  remove_depth();
  // std::cerr<<"RelExp"<<std::endl;
}
void syntax_tree_printer::visit(SyntaxEqExp &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "EqExp:\t" << node.op << std::endl;
  add_depth();
  if (node.EqExp) {
    node.EqExp->accept(*this);
  }
  if (node.RelExp) {
    node.RelExp->accept(*this);
  }
  remove_depth();
  // std::cerr<<"EqExp"<<std::endl;
}
void syntax_tree_printer::visit(SyntaxLAndExp &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "LogicalAndExp:\t" << node.op << std::endl;
  add_depth();
  if (node.LAndExp) {
    node.LAndExp->accept(*this);
  }
  if (node.EqExp) {
    node.EqExp->accept(*this);
  }
  remove_depth();
  // std::cerr<<"LogicalAndExp"<<std::endl;
}
void syntax_tree_printer::visit(SyntaxLOrExp &node) {
  _DEBUG_PRINT_N_(depth);
  std::cout << "LogicalOrExp:\t" << node.op << std::endl;
  add_depth();
  if (node.LOrExp) {
    node.LOrExp->accept(*this);
  }
  if (node.LAndExp) {
    node.LAndExp->accept(*this);
  }
  remove_depth();
  // std::cerr<<"LogicalOrExp"<<std::endl;
}