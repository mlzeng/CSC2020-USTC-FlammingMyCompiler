#ifndef _SYNTAX_TREE_HPP_
#define _SYNTAX_TREE_HPP_

#include <iostream>
#include <memory>
#include <vector>

enum type_specifier { TYPE_INT, TYPE_VOID };

enum relop {
  // <=
  OP_LTE,
  // <
  OP_LT,
  // >
  OP_GT,
  // >=
  OP_GTE,
  // ==
  OP_EQ,
  // !=
  OP_NEQ
};

enum addop {
  // +
  OP_PLUS,
  // -
  OP_MINUS
};

enum mulop {
  // *
  OP_MUL,
  // /
  OP_DIV,
  // %
  OP_MOD
};

enum unaryop {
  // +
  OP_POS,
  // -
  OP_NEG,
  // !
  OP_NOT
};

enum logop {
  // &&
  OP_AND,
  // ||
  OP_OR,

};

struct SyntaxTreeNode;
struct SyntaxCompUnit;
struct SyntaxDeclDef;
// struct SyntaxDecl;
struct SyntaxConstDecl;
struct SyntaxConstDefList;
// struct SyntaxBType;
struct SyntaxConstDef;
struct SyntaxArrayConstExpList;
struct SyntaxConstInitVal;
struct SyntaxConstInitValList;
struct SyntaxVarDecl;
struct SyntaxVarDefList;
struct SyntaxVarDef;
struct SyntaxInitVal;
struct SyntaxInitValList;
struct SyntaxFuncDef;
// struct SyntaxFuncType;
struct SyntaxFuncFParamList;
struct SyntaxFuncFParam;
struct SyntaxParamArrayExpList;
struct SyntaxBlock;
struct SyntaxBlockItemList;
struct SyntaxBlockItem;
struct SyntaxStmt;
struct SyntaxAssignStmt;
struct SyntaxSelectStmt;
struct SyntaxBreakStmt;
struct SyntaxContinueStmt;
struct SyntaxIterationStmt;
struct SyntaxReturnStmt;
struct SyntaxExp;
struct SyntaxCond;
struct SyntaxLVal;
struct SyntaxArrayExpList;
struct SyntaxPrimaryExp;
struct SyntaxNumber;
struct SyntaxUnaryExp;
struct SyntaxCallee;
// struct SyntaxUnaryOp;
struct SyntaxExpList;
struct SyntaxMulExp;
struct SyntaxAddExp;
struct SyntaxRelExp;
struct SyntaxEqExp;
struct SyntaxLAndExp;
struct SyntaxLOrExp;
struct SyntaxConstExp;
class syntax_tree_visitor;

class syntax_tree {
public:
  syntax_tree(SyntaxCompUnit *start_node) {
    root = std::shared_ptr<SyntaxCompUnit>(start_node);
  };
  syntax_tree(syntax_tree &&tree) {
    root = tree.root;
    tree.root = nullptr;
  };
  SyntaxCompUnit *get_root() { return root.get(); }
  void run_visitor(syntax_tree_visitor &visitor);

private:
  std::shared_ptr<SyntaxCompUnit> root = nullptr;
};

struct SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) = 0;
};
struct SyntaxCompUnit : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::vector<std::shared_ptr<SyntaxDeclDef>> DeclDefList;
};
struct SyntaxDeclDef : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxConstDecl> ConstDecl;
  std::shared_ptr<SyntaxVarDecl> VarDecl;
  std::shared_ptr<SyntaxFuncDef> FuncDef;
};
struct SyntaxConstDecl : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::vector<std::shared_ptr<SyntaxConstDef>> ConstDefList;
  type_specifier type;
};
struct SyntaxConstDefList {
  std::vector<std::shared_ptr<SyntaxConstDef>> list;
};
// struct SyntaxBType;
struct SyntaxConstDef : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::vector<std::shared_ptr<SyntaxConstExp>> ArrayConstExpList;
  std::shared_ptr<SyntaxConstInitVal> ConstInitVal;
  std::string id;
};
struct SyntaxArrayConstExpList {
  std::vector<std::shared_ptr<SyntaxConstExp>> list;
};
struct SyntaxConstInitVal : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxConstExp> ConstExp;
  std::vector<std::shared_ptr<SyntaxConstInitVal>> ConstInitValList;
  std::vector<int> bounds;
};
struct SyntaxConstInitValList {
  std::vector<std::shared_ptr<SyntaxConstInitVal>> list;
};
struct SyntaxVarDecl : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::vector<std::shared_ptr<SyntaxVarDef>> VarDefList;
  type_specifier type;
};
struct SyntaxVarDefList {
  std::vector<std::shared_ptr<SyntaxVarDef>> list;
};
struct SyntaxVarDef : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::string id;
  std::vector<std::shared_ptr<SyntaxConstExp>> ArrayConstExpList;
  std::shared_ptr<SyntaxInitVal> InitVal;
};
struct SyntaxInitVal : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::vector<std::shared_ptr<SyntaxInitVal>> InitValList;
  std::shared_ptr<SyntaxExp> Exp;
  std::vector<int> bounds;
};
struct SyntaxInitValList {
  std::vector<std::shared_ptr<SyntaxInitVal>> list;
};
struct SyntaxFuncDef : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  type_specifier type;
  std::string id;
  std::vector<std::shared_ptr<SyntaxFuncFParam>> FuncFParamList;
  std::shared_ptr<SyntaxBlock> Block;
};
// struct SyntaxFuncType;
struct SyntaxFuncFParamList {
  std::vector<std::shared_ptr<SyntaxFuncFParam>> list;
};
struct SyntaxFuncFParam : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  type_specifier type;
  std::string id;
  bool isarray;
  std::vector<std::shared_ptr<SyntaxExp>> ParamArrayExpList;
};
struct SyntaxParamArrayExpList {
  std::vector<std::shared_ptr<SyntaxExp>> list;
};
struct SyntaxBlock : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::vector<std::shared_ptr<SyntaxBlockItem>> BlockItemList;
};
struct SyntaxBlockItemList {
  std::vector<std::shared_ptr<SyntaxBlockItem>> list;
};
struct SyntaxBlockItem : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxConstDecl> ConstDecl;
  std::shared_ptr<SyntaxVarDecl> VarDecl;
  std::shared_ptr<SyntaxStmt> Stmt;
};
struct SyntaxStmt : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxBreakStmt> BreakStmt;
  std::shared_ptr<SyntaxContinueStmt> ContinueStmt;
  std::shared_ptr<SyntaxAssignStmt> AssignStmt;
  std::shared_ptr<SyntaxExp> Exp;
  std::shared_ptr<SyntaxBlock> Block;
  std::shared_ptr<SyntaxSelectStmt> SelectStmt;
  std::shared_ptr<SyntaxIterationStmt> IterationStmt;
  std::shared_ptr<SyntaxReturnStmt> ReturnStmt;
};
struct SyntaxBreakStmt : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
};
struct SyntaxContinueStmt : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
};
struct SyntaxAssignStmt : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxLVal> LVal;
  std::shared_ptr<SyntaxExp> Exp;
};
struct SyntaxSelectStmt : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxCond> Cond;
  std::shared_ptr<SyntaxStmt> ifStmt;
  std::shared_ptr<SyntaxStmt> elseStmt;
};
struct SyntaxIterationStmt : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxCond> Cond;
  std::shared_ptr<SyntaxStmt> Stmt;
};
struct SyntaxReturnStmt : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxExp> Exp;
};

struct SyntaxExp : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxAddExp> AddExp;
};
struct SyntaxCond : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxLOrExp> LOrExp;
};
struct SyntaxLVal : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::vector<std::shared_ptr<SyntaxExp>> ArrayExpList;
  std::string id;
};
struct SyntaxArrayExpList {
  std::vector<std::shared_ptr<SyntaxExp>> list;
};
struct SyntaxPrimaryExp : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxExp> Exp;
  std::shared_ptr<SyntaxLVal> LVal;
  std::shared_ptr<SyntaxNumber> Number;
};
struct SyntaxNumber : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  int32_t num;
};
struct SyntaxUnaryExp : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  unaryop op;
  std::shared_ptr<SyntaxPrimaryExp> PrimaryExp;
  std::shared_ptr<SyntaxCallee> Callee;
  std::shared_ptr<SyntaxUnaryExp> UnaryExp;
};
struct SyntaxCallee : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::string id;
  std::vector<std::shared_ptr<SyntaxExp>> ExpList;
};
// struct SyntaxUnaryOp:SyntaxTreeNode{};
struct SyntaxExpList {
  std::vector<std::shared_ptr<SyntaxExp>> list;
};
struct SyntaxMulExp : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  mulop op;
  std::shared_ptr<SyntaxMulExp> MulExp;
  std::shared_ptr<SyntaxUnaryExp> UnaryExp;
};
struct SyntaxAddExp : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  addop op;
  std::shared_ptr<SyntaxAddExp> AddExp;
  std::shared_ptr<SyntaxMulExp> MulExp;
};
struct SyntaxRelExp : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  relop op;
  std::shared_ptr<SyntaxRelExp> RelExp;
  std::shared_ptr<SyntaxAddExp> AddExp;
};
struct SyntaxEqExp : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  relop op;
  std::shared_ptr<SyntaxEqExp> EqExp;
  std::shared_ptr<SyntaxRelExp> RelExp;
};
struct SyntaxLAndExp : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  logop op;
  std::shared_ptr<SyntaxLAndExp> LAndExp;
  std::shared_ptr<SyntaxEqExp> EqExp;
};
struct SyntaxLOrExp : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  logop op;
  std::shared_ptr<SyntaxLOrExp> LOrExp;
  std::shared_ptr<SyntaxLAndExp> LAndExp;
};
struct SyntaxConstExp : SyntaxTreeNode {
  virtual void accept(syntax_tree_visitor &) final;
  std::shared_ptr<SyntaxAddExp> AddExp;
};

class syntax_tree_visitor {
public:
  virtual void visit(SyntaxConstExp &) = 0;
  virtual void visit(SyntaxCompUnit &) = 0;
  virtual void visit(SyntaxConstDecl &) = 0;
  virtual void visit(SyntaxConstDef &) = 0;
  virtual void visit(SyntaxConstInitVal &) = 0;
  virtual void visit(SyntaxVarDecl &) = 0;
  virtual void visit(SyntaxVarDef &) = 0;
  virtual void visit(SyntaxInitVal &) = 0;
  virtual void visit(SyntaxFuncDef &) = 0;
  virtual void visit(SyntaxFuncFParam &) = 0;
  virtual void visit(SyntaxBlock &) = 0;
  virtual void visit(SyntaxBreakStmt &) = 0;
  virtual void visit(SyntaxContinueStmt &) = 0;
  virtual void visit(SyntaxAssignStmt &) = 0;
  virtual void visit(SyntaxSelectStmt &) = 0;
  virtual void visit(SyntaxIterationStmt &) = 0;
  virtual void visit(SyntaxReturnStmt &) = 0;
  virtual void visit(SyntaxLVal &) = 0;
  virtual void visit(SyntaxPrimaryExp &) = 0;
  virtual void visit(SyntaxNumber &) = 0;
  virtual void visit(SyntaxUnaryExp &) = 0;
  virtual void visit(SyntaxCallee &) = 0;
  virtual void visit(SyntaxMulExp &) = 0;
  virtual void visit(SyntaxAddExp &) = 0;
  virtual void visit(SyntaxRelExp &) = 0;
  virtual void visit(SyntaxEqExp &) = 0;
  virtual void visit(SyntaxLAndExp &) = 0;
  virtual void visit(SyntaxLOrExp &) = 0;
};
class syntax_tree_printer : public syntax_tree_visitor {
public:
  void visit(SyntaxCompUnit &) final;
  void visit(SyntaxConstDecl &) final;
  void visit(SyntaxConstDef &) final;
  void visit(SyntaxConstInitVal &) final;
  void visit(SyntaxVarDecl &) final;
  void visit(SyntaxVarDef &) final;
  void visit(SyntaxInitVal &) final;
  void visit(SyntaxFuncDef &) final;
  void visit(SyntaxFuncFParam &) final;
  void visit(SyntaxBlock &) final;
  void visit(SyntaxBreakStmt &) final;
  void visit(SyntaxContinueStmt &) final;
  void visit(SyntaxAssignStmt &) final;
  void visit(SyntaxSelectStmt &) final;
  void visit(SyntaxIterationStmt &) final;
  void visit(SyntaxReturnStmt &) final;
  void visit(SyntaxLVal &) final;
  void visit(SyntaxPrimaryExp &) final;
  void visit(SyntaxNumber &) final;
  void visit(SyntaxUnaryExp &) final;
  void visit(SyntaxCallee &) final;
  void visit(SyntaxMulExp &) final;
  void visit(SyntaxAddExp &) final;
  void visit(SyntaxRelExp &) final;
  void visit(SyntaxEqExp &) final;
  void visit(SyntaxLAndExp &) final;
  void visit(SyntaxLOrExp &) final;
  void visit(SyntaxConstExp &) final;
  void add_depth() { depth += 6; }
  void remove_depth() {
    // std::cerr<<"depth-4"<<std::endl;
    depth -= 6;
  }

private:
  int depth = 0;
};
#endif // _SYNTAX_TREE_HPP_