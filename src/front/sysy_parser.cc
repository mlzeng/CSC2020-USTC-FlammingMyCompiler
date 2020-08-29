// A Bison parser, made by GNU Bison 3.0.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// First part of user declarations.

#line 37 "sysy_parser.cc" // lalr1.cc:404

#ifndef YY_NULLPTR
#if defined __cplusplus && 201103L <= __cplusplus
#define YY_NULLPTR nullptr
#else
#define YY_NULLPTR 0
#endif
#endif

#include "sysy_parser.hh"

// User implementation prologue.

#line 51 "sysy_parser.cc" // lalr1.cc:412
// Unqualified %code blocks.
#line 34 "sysy_parser.yy" // lalr1.cc:413

#include "sysy_driver.hh"
#define yylex driver.lexer.yylex

#line 58 "sysy_parser.cc" // lalr1.cc:413

#ifndef YY_
#if defined YYENABLE_NLS && YYENABLE_NLS
#if ENABLE_NLS
#include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#define YY_(msgid) dgettext("bison-runtime", msgid)
#endif
#endif
#ifndef YY_
#define YY_(msgid) msgid
#endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
#define YYLLOC_DEFAULT(Current, Rhs, N)                                        \
  do                                                                           \
    if (N) {                                                                   \
      (Current).begin = YYRHSLOC(Rhs, 1).begin;                                \
      (Current).end = YYRHSLOC(Rhs, N).end;                                    \
    } else {                                                                   \
      (Current).begin = (Current).end = YYRHSLOC(Rhs, 0).end;                  \
    }                                                                          \
  while (/*CONSTCOND*/ false)
#endif

// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void)(E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
#define YYCDEBUG                                                               \
  if (yydebug_)                                                                \
  (*yycdebug_)

#define YY_SYMBOL_PRINT(Title, Symbol)                                         \
  do {                                                                         \
    if (yydebug_) {                                                            \
      *yycdebug_ << Title << ' ';                                              \
      yy_print_(*yycdebug_, Symbol);                                           \
      *yycdebug_ << std::endl;                                                 \
    }                                                                          \
  } while (false)

#define YY_REDUCE_PRINT(Rule)                                                  \
  do {                                                                         \
    if (yydebug_)                                                              \
      yy_reduce_print_(Rule);                                                  \
  } while (false)

#define YY_STACK_PRINT()                                                       \
  do {                                                                         \
    if (yydebug_)                                                              \
      yystack_print_();                                                        \
  } while (false)

#else // !YYDEBUG

#define YYCDEBUG                                                               \
  if (false)                                                                   \
  std::cerr
#define YY_SYMBOL_PRINT(Title, Symbol) YYUSE(Symbol)
#define YY_REDUCE_PRINT(Rule) static_cast<void>(0)
#define YY_STACK_PRINT() static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok (yyerrstatus_ = 0)
#define yyclearin (yyla.clear())

#define YYACCEPT goto yyacceptlab
#define YYABORT goto yyabortlab
#define YYERROR goto yyerrorlab
#define YYRECOVERING() (!!yyerrstatus_)

namespace yy {
#line 144 "sysy_parser.cc" // lalr1.cc:479

/* Return YYSTR after stripping away unnecessary quotes and
   backslashes, so that it's suitable for yyerror.  The heuristic is
   that double-quoting is unnecessary unless the string contains an
   apostrophe, a comma, or backslash (other than backslash-backslash).
   YYSTR is taken from yytname.  */
std::string sysy_parser::yytnamerr_(const char *yystr) {
  if (*yystr == '"') {
    std::string yyr = "";
    char const *yyp = yystr;

    for (;;)
      switch (*++yyp) {
      case '\'':
      case ',':
        goto do_not_strip_quotes;

      case '\\':
        if (*++yyp != '\\')
          goto do_not_strip_quotes;
        // Fall through.
      default:
        yyr += *yyp;
        break;

      case '"':
        return yyr;
      }
  do_not_strip_quotes:;
  }

  return yystr;
}

/// Build a parser object.
sysy_parser::sysy_parser(sysy_driver &driver_yyarg)
    :
#if YYDEBUG
      yydebug_(false), yycdebug_(&std::cerr),
#endif
      driver(driver_yyarg) {
}

sysy_parser::~sysy_parser() {}

/*---------------.
| Symbol types.  |
`---------------*/

// by_state.
inline sysy_parser::by_state::by_state() : state(empty_state) {}

inline sysy_parser::by_state::by_state(const by_state &other)
    : state(other.state) {}

inline void sysy_parser::by_state::clear() { state = empty_state; }

inline void sysy_parser::by_state::move(by_state &that) {
  state = that.state;
  that.clear();
}

inline sysy_parser::by_state::by_state(state_type s) : state(s) {}

inline sysy_parser::symbol_number_type sysy_parser::by_state::type_get() const {
  if (state == empty_state)
    return empty_symbol;
  else
    return yystos_[state];
}

inline sysy_parser::stack_symbol_type::stack_symbol_type() {}

inline sysy_parser::stack_symbol_type::stack_symbol_type(state_type s,
                                                         symbol_type &that)
    : super_type(s, that.location) {
  switch (that.type_get()) {
  case 85: // AddExp
    value.move<SyntaxAddExp *>(that.value);
    break;

  case 51: // ArrayConstExpList
    value.move<SyntaxArrayConstExpList *>(that.value);
    break;

  case 77: // ArrayExpList
    value.move<SyntaxArrayExpList *>(that.value);
    break;

  case 70: // AssignStmt
    value.move<SyntaxAssignStmt *>(that.value);
    break;

  case 64: // Block
    value.move<SyntaxBlock *>(that.value);
    break;

  case 66: // BlockItem
    value.move<SyntaxBlockItem *>(that.value);
    break;

  case 65: // BlockItemList
    value.move<SyntaxBlockItemList *>(that.value);
    break;

  case 68: // BreakStmt
    value.move<SyntaxBreakStmt *>(that.value);
    break;

  case 81: // Callee
    value.move<SyntaxCallee *>(that.value);
    break;

  case 46: // CompUnit
    value.move<SyntaxCompUnit *>(that.value);
    break;

  case 75: // Cond
    value.move<SyntaxCond *>(that.value);
    break;

  case 48: // ConstDecl
    value.move<SyntaxConstDecl *>(that.value);
    break;

  case 50: // ConstDef
    value.move<SyntaxConstDef *>(that.value);
    break;

  case 49: // ConstDefList
    value.move<SyntaxConstDefList *>(that.value);
    break;

  case 90: // ConstExp
    value.move<SyntaxConstExp *>(that.value);
    break;

  case 52: // ConstInitVal
    value.move<SyntaxConstInitVal *>(that.value);
    break;

  case 53: // ConstInitValList
    value.move<SyntaxConstInitValList *>(that.value);
    break;

  case 69: // ContinueStmt
    value.move<SyntaxContinueStmt *>(that.value);
    break;

  case 47: // DeclDef
    value.move<SyntaxDeclDef *>(that.value);
    break;

  case 87: // EqExp
    value.move<SyntaxEqExp *>(that.value);
    break;

  case 74: // Exp
    value.move<SyntaxExp *>(that.value);
    break;

  case 83: // ExpList
    value.move<SyntaxExpList *>(that.value);
    break;

  case 59: // FuncDef
    value.move<SyntaxFuncDef *>(that.value);
    break;

  case 62: // FuncFParam
    value.move<SyntaxFuncFParam *>(that.value);
    break;

  case 61: // FuncFParamList
    value.move<SyntaxFuncFParamList *>(that.value);
    break;

  case 57: // InitVal
    value.move<SyntaxInitVal *>(that.value);
    break;

  case 58: // InitValList
    value.move<SyntaxInitValList *>(that.value);
    break;

  case 72: // IterationStmt
    value.move<SyntaxIterationStmt *>(that.value);
    break;

  case 88: // LAndExp
    value.move<SyntaxLAndExp *>(that.value);
    break;

  case 89: // LOrExp
    value.move<SyntaxLOrExp *>(that.value);
    break;

  case 76: // LVal
    value.move<SyntaxLVal *>(that.value);
    break;

  case 84: // MulExp
    value.move<SyntaxMulExp *>(that.value);
    break;

  case 79: // Number
    value.move<SyntaxNumber *>(that.value);
    break;

  case 63: // ParamArrayExpList
    value.move<SyntaxParamArrayExpList *>(that.value);
    break;

  case 78: // PrimaryExp
    value.move<SyntaxPrimaryExp *>(that.value);
    break;

  case 86: // RelExp
    value.move<SyntaxRelExp *>(that.value);
    break;

  case 73: // ReturnStmt
    value.move<SyntaxReturnStmt *>(that.value);
    break;

  case 71: // SelectStmt
    value.move<SyntaxSelectStmt *>(that.value);
    break;

  case 67: // Stmt
    value.move<SyntaxStmt *>(that.value);
    break;

  case 80: // UnaryExp
    value.move<SyntaxUnaryExp *>(that.value);
    break;

  case 54: // VarDecl
    value.move<SyntaxVarDecl *>(that.value);
    break;

  case 56: // VarDef
    value.move<SyntaxVarDef *>(that.value);
    break;

  case 55: // VarDefList
    value.move<SyntaxVarDefList *>(that.value);
    break;

  case 31: // NUMBER
    value.move<int>(that.value);
    break;

  case 30: // IDENTIFIER
    value.move<std::string>(that.value);
    break;

  case 60: // DefType
    value.move<type_specifier>(that.value);
    break;

  case 82: // UnaryOp
    value.move<unaryop>(that.value);
    break;

  default:
    break;
  }

  // that is emptied.
  that.type = empty_symbol;
}

inline sysy_parser::stack_symbol_type &
sysy_parser::stack_symbol_type::operator=(const stack_symbol_type &that) {
  state = that.state;
  switch (that.type_get()) {
  case 85: // AddExp
    value.copy<SyntaxAddExp *>(that.value);
    break;

  case 51: // ArrayConstExpList
    value.copy<SyntaxArrayConstExpList *>(that.value);
    break;

  case 77: // ArrayExpList
    value.copy<SyntaxArrayExpList *>(that.value);
    break;

  case 70: // AssignStmt
    value.copy<SyntaxAssignStmt *>(that.value);
    break;

  case 64: // Block
    value.copy<SyntaxBlock *>(that.value);
    break;

  case 66: // BlockItem
    value.copy<SyntaxBlockItem *>(that.value);
    break;

  case 65: // BlockItemList
    value.copy<SyntaxBlockItemList *>(that.value);
    break;

  case 68: // BreakStmt
    value.copy<SyntaxBreakStmt *>(that.value);
    break;

  case 81: // Callee
    value.copy<SyntaxCallee *>(that.value);
    break;

  case 46: // CompUnit
    value.copy<SyntaxCompUnit *>(that.value);
    break;

  case 75: // Cond
    value.copy<SyntaxCond *>(that.value);
    break;

  case 48: // ConstDecl
    value.copy<SyntaxConstDecl *>(that.value);
    break;

  case 50: // ConstDef
    value.copy<SyntaxConstDef *>(that.value);
    break;

  case 49: // ConstDefList
    value.copy<SyntaxConstDefList *>(that.value);
    break;

  case 90: // ConstExp
    value.copy<SyntaxConstExp *>(that.value);
    break;

  case 52: // ConstInitVal
    value.copy<SyntaxConstInitVal *>(that.value);
    break;

  case 53: // ConstInitValList
    value.copy<SyntaxConstInitValList *>(that.value);
    break;

  case 69: // ContinueStmt
    value.copy<SyntaxContinueStmt *>(that.value);
    break;

  case 47: // DeclDef
    value.copy<SyntaxDeclDef *>(that.value);
    break;

  case 87: // EqExp
    value.copy<SyntaxEqExp *>(that.value);
    break;

  case 74: // Exp
    value.copy<SyntaxExp *>(that.value);
    break;

  case 83: // ExpList
    value.copy<SyntaxExpList *>(that.value);
    break;

  case 59: // FuncDef
    value.copy<SyntaxFuncDef *>(that.value);
    break;

  case 62: // FuncFParam
    value.copy<SyntaxFuncFParam *>(that.value);
    break;

  case 61: // FuncFParamList
    value.copy<SyntaxFuncFParamList *>(that.value);
    break;

  case 57: // InitVal
    value.copy<SyntaxInitVal *>(that.value);
    break;

  case 58: // InitValList
    value.copy<SyntaxInitValList *>(that.value);
    break;

  case 72: // IterationStmt
    value.copy<SyntaxIterationStmt *>(that.value);
    break;

  case 88: // LAndExp
    value.copy<SyntaxLAndExp *>(that.value);
    break;

  case 89: // LOrExp
    value.copy<SyntaxLOrExp *>(that.value);
    break;

  case 76: // LVal
    value.copy<SyntaxLVal *>(that.value);
    break;

  case 84: // MulExp
    value.copy<SyntaxMulExp *>(that.value);
    break;

  case 79: // Number
    value.copy<SyntaxNumber *>(that.value);
    break;

  case 63: // ParamArrayExpList
    value.copy<SyntaxParamArrayExpList *>(that.value);
    break;

  case 78: // PrimaryExp
    value.copy<SyntaxPrimaryExp *>(that.value);
    break;

  case 86: // RelExp
    value.copy<SyntaxRelExp *>(that.value);
    break;

  case 73: // ReturnStmt
    value.copy<SyntaxReturnStmt *>(that.value);
    break;

  case 71: // SelectStmt
    value.copy<SyntaxSelectStmt *>(that.value);
    break;

  case 67: // Stmt
    value.copy<SyntaxStmt *>(that.value);
    break;

  case 80: // UnaryExp
    value.copy<SyntaxUnaryExp *>(that.value);
    break;

  case 54: // VarDecl
    value.copy<SyntaxVarDecl *>(that.value);
    break;

  case 56: // VarDef
    value.copy<SyntaxVarDef *>(that.value);
    break;

  case 55: // VarDefList
    value.copy<SyntaxVarDefList *>(that.value);
    break;

  case 31: // NUMBER
    value.copy<int>(that.value);
    break;

  case 30: // IDENTIFIER
    value.copy<std::string>(that.value);
    break;

  case 60: // DefType
    value.copy<type_specifier>(that.value);
    break;

  case 82: // UnaryOp
    value.copy<unaryop>(that.value);
    break;

  default:
    break;
  }

  location = that.location;
  return *this;
}

template <typename Base>
inline void sysy_parser::yy_destroy_(const char *yymsg,
                                     basic_symbol<Base> &yysym) const {
  if (yymsg)
    YY_SYMBOL_PRINT(yymsg, yysym);
}

#if YYDEBUG
template <typename Base>
void sysy_parser::yy_print_(std::ostream &yyo,
                            const basic_symbol<Base> &yysym) const {
  std::ostream &yyoutput = yyo;
  YYUSE(yyoutput);
  symbol_number_type yytype = yysym.type_get();
  // Avoid a (spurious) G++ 4.8 warning about "array subscript is
  // below array bounds".
  if (yysym.empty())
    std::abort();
  yyo << (yytype < yyntokens_ ? "token" : "nterm") << ' ' << yytname_[yytype]
      << " (" << yysym.location << ": ";
  YYUSE(yytype);
  yyo << ')';
}
#endif

inline void sysy_parser::yypush_(const char *m, state_type s,
                                 symbol_type &sym) {
  stack_symbol_type t(s, sym);
  yypush_(m, t);
}

inline void sysy_parser::yypush_(const char *m, stack_symbol_type &s) {
  if (m)
    YY_SYMBOL_PRINT(m, s);
  yystack_.push(s);
}

inline void sysy_parser::yypop_(unsigned int n) { yystack_.pop(n); }

#if YYDEBUG
std::ostream &sysy_parser::debug_stream() const { return *yycdebug_; }

void sysy_parser::set_debug_stream(std::ostream &o) { yycdebug_ = &o; }

sysy_parser::debug_level_type sysy_parser::debug_level() const {
  return yydebug_;
}

void sysy_parser::set_debug_level(debug_level_type l) { yydebug_ = l; }
#endif // YYDEBUG

inline sysy_parser::state_type
sysy_parser::yy_lr_goto_state_(state_type yystate, int yysym) {
  int yyr = yypgoto_[yysym - yyntokens_] + yystate;
  if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
    return yytable_[yyr];
  else
    return yydefgoto_[yysym - yyntokens_];
}

inline bool sysy_parser::yy_pact_value_is_default_(int yyvalue) {
  return yyvalue == yypact_ninf_;
}

inline bool sysy_parser::yy_table_value_is_error_(int yyvalue) {
  return yyvalue == yytable_ninf_;
}

int sysy_parser::parse() {
  // State.
  int yyn;
  /// Length of the RHS of the rule being reduced.
  int yylen = 0;

  // Error handling.
  int yynerrs_ = 0;
  int yyerrstatus_ = 0;

  /// The lookahead symbol.
  symbol_type yyla;

  /// The locations where the error started and ended.
  stack_symbol_type yyerror_range[3];

  /// The return value of parse ().
  int yyresult;

  // FIXME: This shoud be completely indented.  It is not yet to
  // avoid gratuitous conflicts when merging into the master branch.
  try {
    YYCDEBUG << "Starting parse" << std::endl;

// User initialization code.
#line 23 "sysy_parser.yy" // lalr1.cc:741
    {
      // Initialize the initial location.
      yyla.location.begin.filename = yyla.location.end.filename = &driver.file;
    }

#line 794 "sysy_parser.cc" // lalr1.cc:741

    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear();
    yypush_(YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_(yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty()) {
      YYCDEBUG << "Reading a token: ";
      try {
        symbol_type yylookahead(yylex(driver));
        yyla.move(yylookahead);
      } catch (const syntax_error &yyexc) {
        error(yyexc);
        goto yyerrlab1;
      }
    }
    YY_SYMBOL_PRINT("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0) {
      if (yy_table_value_is_error_(yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn]) {
      case 85: // AddExp
        yylhs.value.build<SyntaxAddExp *>();
        break;

      case 51: // ArrayConstExpList
        yylhs.value.build<SyntaxArrayConstExpList *>();
        break;

      case 77: // ArrayExpList
        yylhs.value.build<SyntaxArrayExpList *>();
        break;

      case 70: // AssignStmt
        yylhs.value.build<SyntaxAssignStmt *>();
        break;

      case 64: // Block
        yylhs.value.build<SyntaxBlock *>();
        break;

      case 66: // BlockItem
        yylhs.value.build<SyntaxBlockItem *>();
        break;

      case 65: // BlockItemList
        yylhs.value.build<SyntaxBlockItemList *>();
        break;

      case 68: // BreakStmt
        yylhs.value.build<SyntaxBreakStmt *>();
        break;

      case 81: // Callee
        yylhs.value.build<SyntaxCallee *>();
        break;

      case 46: // CompUnit
        yylhs.value.build<SyntaxCompUnit *>();
        break;

      case 75: // Cond
        yylhs.value.build<SyntaxCond *>();
        break;

      case 48: // ConstDecl
        yylhs.value.build<SyntaxConstDecl *>();
        break;

      case 50: // ConstDef
        yylhs.value.build<SyntaxConstDef *>();
        break;

      case 49: // ConstDefList
        yylhs.value.build<SyntaxConstDefList *>();
        break;

      case 90: // ConstExp
        yylhs.value.build<SyntaxConstExp *>();
        break;

      case 52: // ConstInitVal
        yylhs.value.build<SyntaxConstInitVal *>();
        break;

      case 53: // ConstInitValList
        yylhs.value.build<SyntaxConstInitValList *>();
        break;

      case 69: // ContinueStmt
        yylhs.value.build<SyntaxContinueStmt *>();
        break;

      case 47: // DeclDef
        yylhs.value.build<SyntaxDeclDef *>();
        break;

      case 87: // EqExp
        yylhs.value.build<SyntaxEqExp *>();
        break;

      case 74: // Exp
        yylhs.value.build<SyntaxExp *>();
        break;

      case 83: // ExpList
        yylhs.value.build<SyntaxExpList *>();
        break;

      case 59: // FuncDef
        yylhs.value.build<SyntaxFuncDef *>();
        break;

      case 62: // FuncFParam
        yylhs.value.build<SyntaxFuncFParam *>();
        break;

      case 61: // FuncFParamList
        yylhs.value.build<SyntaxFuncFParamList *>();
        break;

      case 57: // InitVal
        yylhs.value.build<SyntaxInitVal *>();
        break;

      case 58: // InitValList
        yylhs.value.build<SyntaxInitValList *>();
        break;

      case 72: // IterationStmt
        yylhs.value.build<SyntaxIterationStmt *>();
        break;

      case 88: // LAndExp
        yylhs.value.build<SyntaxLAndExp *>();
        break;

      case 89: // LOrExp
        yylhs.value.build<SyntaxLOrExp *>();
        break;

      case 76: // LVal
        yylhs.value.build<SyntaxLVal *>();
        break;

      case 84: // MulExp
        yylhs.value.build<SyntaxMulExp *>();
        break;

      case 79: // Number
        yylhs.value.build<SyntaxNumber *>();
        break;

      case 63: // ParamArrayExpList
        yylhs.value.build<SyntaxParamArrayExpList *>();
        break;

      case 78: // PrimaryExp
        yylhs.value.build<SyntaxPrimaryExp *>();
        break;

      case 86: // RelExp
        yylhs.value.build<SyntaxRelExp *>();
        break;

      case 73: // ReturnStmt
        yylhs.value.build<SyntaxReturnStmt *>();
        break;

      case 71: // SelectStmt
        yylhs.value.build<SyntaxSelectStmt *>();
        break;

      case 67: // Stmt
        yylhs.value.build<SyntaxStmt *>();
        break;

      case 80: // UnaryExp
        yylhs.value.build<SyntaxUnaryExp *>();
        break;

      case 54: // VarDecl
        yylhs.value.build<SyntaxVarDecl *>();
        break;

      case 56: // VarDef
        yylhs.value.build<SyntaxVarDef *>();
        break;

      case 55: // VarDefList
        yylhs.value.build<SyntaxVarDefList *>();
        break;

      case 31: // NUMBER
        yylhs.value.build<int>();
        break;

      case 30: // IDENTIFIER
        yylhs.value.build<std::string>();
        break;

      case 60: // DefType
        yylhs.value.build<type_specifier>();
        break;

      case 82: // UnaryOp
        yylhs.value.build<unaryop>();
        break;

      default:
        break;
      }

      // Compute the default @$.
      {
        slice<stack_symbol_type, stack_type> slice(yystack_, yylen);
        YYLLOC_DEFAULT(yylhs.location, slice, yylen);
      }

      // Perform the reduction.
      YY_REDUCE_PRINT(yyn);
      try {
        switch (yyn) {
        case 2:
#line 112 "sysy_parser.yy" // lalr1.cc:859
        {
          auto printer = new syntax_tree_printer;
          // printer->visit(*$1);
          // std::cout << "test END\n";
          driver.root = yystack_[1].value.as<SyntaxCompUnit *>();
          // std::cout << driver.root << $1;
          // printer->visit(*(driver.root));
          return 0;
        }
#line 1100 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 3:
#line 122 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[1].value.as<SyntaxCompUnit *>()->DeclDefList.push_back(
              std::shared_ptr<SyntaxDeclDef>(
                  yystack_[0].value.as<SyntaxDeclDef *>()));
          yylhs.value.as<SyntaxCompUnit *>() =
              yystack_[1].value.as<SyntaxCompUnit *>();
        }
#line 1109 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 4:
#line 126 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxCompUnit *>() = new SyntaxCompUnit();
          yylhs.value.as<SyntaxCompUnit *>()->DeclDefList.push_back(
              std::shared_ptr<SyntaxDeclDef>(
                  yystack_[0].value.as<SyntaxDeclDef *>()));
        }
#line 1118 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 5:
#line 131 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxDeclDef *>() = new SyntaxDeclDef();
          yylhs.value.as<SyntaxDeclDef *>()->ConstDecl =
              std::shared_ptr<SyntaxConstDecl>(
                  yystack_[0].value.as<SyntaxConstDecl *>());
          yylhs.value.as<SyntaxDeclDef *>()->VarDecl = nullptr;
          yylhs.value.as<SyntaxDeclDef *>()->FuncDef = nullptr;
        }
#line 1129 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 6:
#line 137 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxDeclDef *>() = new SyntaxDeclDef();
          yylhs.value.as<SyntaxDeclDef *>()->ConstDecl = nullptr;
          yylhs.value.as<SyntaxDeclDef *>()->VarDecl =
              std::shared_ptr<SyntaxVarDecl>(
                  yystack_[0].value.as<SyntaxVarDecl *>());
          yylhs.value.as<SyntaxDeclDef *>()->FuncDef = nullptr;
        }
#line 1140 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 7:
#line 143 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxDeclDef *>() = new SyntaxDeclDef();
          yylhs.value.as<SyntaxDeclDef *>()->ConstDecl = nullptr;
          yylhs.value.as<SyntaxDeclDef *>()->VarDecl = nullptr;
          yylhs.value.as<SyntaxDeclDef *>()->FuncDef =
              std::shared_ptr<SyntaxFuncDef>(
                  yystack_[0].value.as<SyntaxFuncDef *>());
        }
#line 1151 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 8:
#line 150 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxConstDecl *>() = new SyntaxConstDecl();
          yylhs.value.as<SyntaxConstDecl *>()->ConstDefList.swap(
              yystack_[1].value.as<SyntaxConstDefList *>()->list);
          yylhs.value.as<SyntaxConstDecl *>()->type = TYPE_INT;
        }
#line 1161 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 9:
#line 156 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[2].value.as<SyntaxConstDefList *>()->list.push_back(
              std::shared_ptr<SyntaxConstDef>(
                  yystack_[0].value.as<SyntaxConstDef *>()));
          yylhs.value.as<SyntaxConstDefList *>() =
              yystack_[2].value.as<SyntaxConstDefList *>();
        }
#line 1170 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 10:
#line 160 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxConstDefList *>() = new SyntaxConstDefList();
          yylhs.value.as<SyntaxConstDefList *>()->list.push_back(
              std::shared_ptr<SyntaxConstDef>(
                  yystack_[0].value.as<SyntaxConstDef *>()));
        }
#line 1179 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 11:
#line 165 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxConstDef *>() = new SyntaxConstDef();
          yylhs.value.as<SyntaxConstDef *>()->ArrayConstExpList.swap(
              yystack_[2].value.as<SyntaxArrayConstExpList *>()->list);
          yylhs.value.as<SyntaxConstDef *>()->id =
              yystack_[3].value.as<std::string>();
          yylhs.value.as<SyntaxConstDef *>()->ConstInitVal =
              std::shared_ptr<SyntaxConstInitVal>(
                  yystack_[0].value.as<SyntaxConstInitVal *>());
        }
#line 1190 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 12:
#line 172 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[3].value.as<SyntaxArrayConstExpList *>()->list.push_back(
              std::shared_ptr<SyntaxConstExp>(
                  yystack_[1].value.as<SyntaxConstExp *>()));
          yylhs.value.as<SyntaxArrayConstExpList *>() =
              yystack_[3].value.as<SyntaxArrayConstExpList *>();
        }
#line 1199 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 13:
#line 176 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxArrayConstExpList *>() =
              new SyntaxArrayConstExpList();
        }
#line 1207 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 14:
#line 180 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxConstInitVal *>() = new SyntaxConstInitVal();
          yylhs.value.as<SyntaxConstInitVal *>()->ConstExp =
              std::shared_ptr<SyntaxConstExp>(
                  yystack_[0].value.as<SyntaxConstExp *>());
        }
#line 1216 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 15:
#line 184 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxConstInitVal *>() = new SyntaxConstInitVal();
          yylhs.value.as<SyntaxConstInitVal *>()->ConstExp = nullptr;
          yylhs.value.as<SyntaxConstInitVal *>()->ConstInitValList.swap(
              yystack_[1].value.as<SyntaxConstInitValList *>()->list);
        }
#line 1226 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 16:
#line 189 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxConstInitVal *>() = new SyntaxConstInitVal();
          yylhs.value.as<SyntaxConstInitVal *>()->ConstExp = nullptr;
        }
#line 1235 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 17:
#line 194 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[2].value.as<SyntaxConstInitValList *>()->list.push_back(
              std::shared_ptr<SyntaxConstInitVal>(
                  yystack_[0].value.as<SyntaxConstInitVal *>()));
          yylhs.value.as<SyntaxConstInitValList *>() =
              yystack_[2].value.as<SyntaxConstInitValList *>();
        }
#line 1244 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 18:
#line 198 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxConstInitValList *>() =
              new SyntaxConstInitValList();
          yylhs.value.as<SyntaxConstInitValList *>()->list.push_back(
              std::shared_ptr<SyntaxConstInitVal>(
                  yystack_[0].value.as<SyntaxConstInitVal *>()));
        }
#line 1253 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 19:
#line 203 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxVarDecl *>() = new SyntaxVarDecl();
          yylhs.value.as<SyntaxVarDecl *>()->VarDefList.swap(
              yystack_[1].value.as<SyntaxVarDefList *>()->list);
          yylhs.value.as<SyntaxVarDecl *>()->type = TYPE_INT;
        }
#line 1263 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 20:
#line 209 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[2].value.as<SyntaxVarDefList *>()->list.push_back(
              std::shared_ptr<SyntaxVarDef>(
                  yystack_[0].value.as<SyntaxVarDef *>()));
          yylhs.value.as<SyntaxVarDefList *>() =
              yystack_[2].value.as<SyntaxVarDefList *>();
        }
#line 1272 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 21:
#line 213 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxVarDefList *>() = new SyntaxVarDefList();
          yylhs.value.as<SyntaxVarDefList *>()->list.push_back(
              std::shared_ptr<SyntaxVarDef>(
                  yystack_[0].value.as<SyntaxVarDef *>()));
        }
#line 1281 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 22:
#line 218 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxVarDef *>() = new SyntaxVarDef();
          yylhs.value.as<SyntaxVarDef *>()->id =
              yystack_[1].value.as<std::string>();
          yylhs.value.as<SyntaxVarDef *>()->ArrayConstExpList.swap(
              yystack_[0].value.as<SyntaxArrayConstExpList *>()->list);
          yylhs.value.as<SyntaxVarDef *>()->InitVal = nullptr;
        }
#line 1292 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 23:
#line 224 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxVarDef *>() = new SyntaxVarDef();
          yylhs.value.as<SyntaxVarDef *>()->id =
              yystack_[3].value.as<std::string>();
          yylhs.value.as<SyntaxVarDef *>()->ArrayConstExpList.swap(
              yystack_[2].value.as<SyntaxArrayConstExpList *>()->list);
          yylhs.value.as<SyntaxVarDef *>()->InitVal =
              std::shared_ptr<SyntaxInitVal>(
                  yystack_[0].value.as<SyntaxInitVal *>());
        }
#line 1303 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 24:
#line 231 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxInitVal *>() = new SyntaxInitVal();
          yylhs.value.as<SyntaxInitVal *>()->Exp =
              std::shared_ptr<SyntaxExp>(yystack_[0].value.as<SyntaxExp *>());
        }
#line 1312 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 25:
#line 235 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxInitVal *>() = new SyntaxInitVal();
          yylhs.value.as<SyntaxInitVal *>()->Exp = nullptr;
        }
#line 1321 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 26:
#line 239 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxInitVal *>() = new SyntaxInitVal();
          yylhs.value.as<SyntaxInitVal *>()->Exp = nullptr;
          yylhs.value.as<SyntaxInitVal *>()->InitValList.swap(
              yystack_[1].value.as<SyntaxInitValList *>()->list);
        }
#line 1331 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 27:
#line 245 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[2].value.as<SyntaxInitValList *>()->list.push_back(
              std::shared_ptr<SyntaxInitVal>(
                  yystack_[0].value.as<SyntaxInitVal *>()));
          yylhs.value.as<SyntaxInitValList *>() =
              yystack_[2].value.as<SyntaxInitValList *>();
        }
#line 1340 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 28:
#line 249 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxInitValList *>() = new SyntaxInitValList();
          yylhs.value.as<SyntaxInitValList *>()->list.push_back(
              std::shared_ptr<SyntaxInitVal>(
                  yystack_[0].value.as<SyntaxInitVal *>()));
        }
#line 1349 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 29:
#line 254 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxFuncDef *>() = new SyntaxFuncDef();
          yylhs.value.as<SyntaxFuncDef *>()->type =
              yystack_[4].value.as<type_specifier>();
          yylhs.value.as<SyntaxFuncDef *>()->id =
              yystack_[3].value.as<std::string>();
          yylhs.value.as<SyntaxFuncDef *>()->Block =
              std::shared_ptr<SyntaxBlock>(
                  yystack_[0].value.as<SyntaxBlock *>());
        }
#line 1360 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 30:
#line 260 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxFuncDef *>() = new SyntaxFuncDef();
          yylhs.value.as<SyntaxFuncDef *>()->type =
              yystack_[5].value.as<type_specifier>();
          yylhs.value.as<SyntaxFuncDef *>()->id =
              yystack_[4].value.as<std::string>();
          yylhs.value.as<SyntaxFuncDef *>()->FuncFParamList.swap(
              yystack_[2].value.as<SyntaxFuncFParamList *>()->list);
          yylhs.value.as<SyntaxFuncDef *>()->Block =
              std::shared_ptr<SyntaxBlock>(
                  yystack_[0].value.as<SyntaxBlock *>());
        }
#line 1372 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 31:
#line 268 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<type_specifier>() = TYPE_VOID;
        }
#line 1378 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 32:
#line 269 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<type_specifier>() = TYPE_INT;
        }
#line 1384 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 33:
#line 271 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[2].value.as<SyntaxFuncFParamList *>()->list.push_back(
              std::shared_ptr<SyntaxFuncFParam>(
                  yystack_[0].value.as<SyntaxFuncFParam *>()));
          yylhs.value.as<SyntaxFuncFParamList *>() =
              yystack_[2].value.as<SyntaxFuncFParamList *>();
        }
#line 1393 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 34:
#line 275 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxFuncFParamList *>() = new SyntaxFuncFParamList();
          yylhs.value.as<SyntaxFuncFParamList *>()->list.push_back(
              std::shared_ptr<SyntaxFuncFParam>(
                  yystack_[0].value.as<SyntaxFuncFParam *>()));
        }
#line 1402 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 35:
#line 280 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxFuncFParam *>() = new SyntaxFuncFParam();
          yylhs.value.as<SyntaxFuncFParam *>()->type = TYPE_INT;
          yylhs.value.as<SyntaxFuncFParam *>()->isarray = 1;
          yylhs.value.as<SyntaxFuncFParam *>()->id =
              yystack_[1].value.as<std::string>();
          yylhs.value.as<SyntaxFuncFParam *>()->ParamArrayExpList.swap(
              yystack_[0].value.as<SyntaxParamArrayExpList *>()->list);
        }
#line 1414 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 36:
#line 287 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxFuncFParam *>() = new SyntaxFuncFParam();
          yylhs.value.as<SyntaxFuncFParam *>()->type = TYPE_INT;
          yylhs.value.as<SyntaxFuncFParam *>()->id =
              yystack_[0].value.as<std::string>();
          yylhs.value.as<SyntaxFuncFParam *>()->isarray = 0;
        }
#line 1425 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 37:
#line 294 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[3].value.as<SyntaxParamArrayExpList *>()->list.push_back(
              std::shared_ptr<SyntaxExp>(yystack_[1].value.as<SyntaxExp *>()));
          yylhs.value.as<SyntaxParamArrayExpList *>() =
              yystack_[3].value.as<SyntaxParamArrayExpList *>();
        }
#line 1434 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 38:
#line 298 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxParamArrayExpList *>() =
              new SyntaxParamArrayExpList();
        }
#line 1442 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 39:
#line 302 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxBlock *>() = new SyntaxBlock();
          yylhs.value.as<SyntaxBlock *>()->BlockItemList.swap(
              yystack_[1].value.as<SyntaxBlockItemList *>()->list);
        }
#line 1451 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 40:
#line 307 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[1].value.as<SyntaxBlockItemList *>()->list.push_back(
              std::shared_ptr<SyntaxBlockItem>(
                  yystack_[0].value.as<SyntaxBlockItem *>()));
          yylhs.value.as<SyntaxBlockItemList *>() =
              yystack_[1].value.as<SyntaxBlockItemList *>();
        }
#line 1460 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 41:
#line 311 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxBlockItemList *>() = new SyntaxBlockItemList();
        }
#line 1468 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 42:
#line 315 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxBlockItem *>() = new SyntaxBlockItem();
          yylhs.value.as<SyntaxBlockItem *>()->ConstDecl =
              std::shared_ptr<SyntaxConstDecl>(
                  yystack_[0].value.as<SyntaxConstDecl *>());
          yylhs.value.as<SyntaxBlockItem *>()->VarDecl = nullptr;
          yylhs.value.as<SyntaxBlockItem *>()->Stmt = nullptr;
        }
#line 1479 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 43:
#line 321 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxBlockItem *>() = new SyntaxBlockItem();
          // std::cout<<"310"<<std::endl;
          yylhs.value.as<SyntaxBlockItem *>()->ConstDecl = nullptr;
          // std::cout<<"312"<<std::endl;
          yylhs.value.as<SyntaxBlockItem *>()->VarDecl =
              std::shared_ptr<SyntaxVarDecl>(
                  yystack_[0].value.as<SyntaxVarDecl *>());
          // std::cout<<"314"<<std::endl;
          yylhs.value.as<SyntaxBlockItem *>()->Stmt = nullptr;
        }
#line 1493 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 44:
#line 330 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxBlockItem *>() = new SyntaxBlockItem();
          yylhs.value.as<SyntaxBlockItem *>()->ConstDecl = nullptr;
          yylhs.value.as<SyntaxBlockItem *>()->VarDecl = nullptr;
          yylhs.value.as<SyntaxBlockItem *>()->Stmt =
              std::shared_ptr<SyntaxStmt>(yystack_[0].value.as<SyntaxStmt *>());
        }
#line 1504 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 45:
#line 337 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxStmt *>() = new SyntaxStmt();
          yylhs.value.as<SyntaxStmt *>()->BreakStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ContinueStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->AssignStmt =
              std::shared_ptr<SyntaxAssignStmt>(
                  yystack_[0].value.as<SyntaxAssignStmt *>());
          yylhs.value.as<SyntaxStmt *>()->Exp = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Block = nullptr;
          yylhs.value.as<SyntaxStmt *>()->SelectStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->IterationStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ReturnStmt = nullptr;
        }
#line 1520 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 46:
#line 348 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxStmt *>() = new SyntaxStmt();
          yylhs.value.as<SyntaxStmt *>()->BreakStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ContinueStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->AssignStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Exp =
              std::shared_ptr<SyntaxExp>(yystack_[1].value.as<SyntaxExp *>());
          yylhs.value.as<SyntaxStmt *>()->Block = nullptr;
          yylhs.value.as<SyntaxStmt *>()->SelectStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->IterationStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ReturnStmt = nullptr;
        }
#line 1536 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 47:
#line 359 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxStmt *>() = new SyntaxStmt();
          yylhs.value.as<SyntaxStmt *>()->BreakStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ContinueStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->AssignStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Exp = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Block = nullptr;
          yylhs.value.as<SyntaxStmt *>()->SelectStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->IterationStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ReturnStmt = nullptr;
        }
#line 1552 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 48:
#line 370 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxStmt *>() = new SyntaxStmt();
          yylhs.value.as<SyntaxStmt *>()->BreakStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ContinueStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->AssignStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Exp = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Block = std::shared_ptr<SyntaxBlock>(
              yystack_[0].value.as<SyntaxBlock *>());
          yylhs.value.as<SyntaxStmt *>()->SelectStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->IterationStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ReturnStmt = nullptr;
        }
#line 1568 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 49:
#line 381 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxStmt *>() = new SyntaxStmt();
          yylhs.value.as<SyntaxStmt *>()->BreakStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ContinueStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->AssignStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Exp = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Block = nullptr;
          yylhs.value.as<SyntaxStmt *>()->SelectStmt =
              std::shared_ptr<SyntaxSelectStmt>(
                  yystack_[0].value.as<SyntaxSelectStmt *>());
          yylhs.value.as<SyntaxStmt *>()->IterationStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ReturnStmt = nullptr;
        }
#line 1584 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 50:
#line 392 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxStmt *>() = new SyntaxStmt();
          yylhs.value.as<SyntaxStmt *>()->BreakStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ContinueStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->AssignStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Exp = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Block = nullptr;
          yylhs.value.as<SyntaxStmt *>()->SelectStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->IterationStmt =
              std::shared_ptr<SyntaxIterationStmt>(
                  yystack_[0].value.as<SyntaxIterationStmt *>());
          yylhs.value.as<SyntaxStmt *>()->ReturnStmt = nullptr;
        }
#line 1600 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 51:
#line 403 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxStmt *>() = new SyntaxStmt();
          yylhs.value.as<SyntaxStmt *>()->BreakStmt =
              std::shared_ptr<SyntaxBreakStmt>(
                  yystack_[0].value.as<SyntaxBreakStmt *>());
          yylhs.value.as<SyntaxStmt *>()->ContinueStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->AssignStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Exp = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Block = nullptr;
          yylhs.value.as<SyntaxStmt *>()->SelectStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->IterationStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ReturnStmt = nullptr;
        }
#line 1616 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 52:
#line 414 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxStmt *>() = new SyntaxStmt();
          yylhs.value.as<SyntaxStmt *>()->BreakStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ContinueStmt =
              std::shared_ptr<SyntaxContinueStmt>(
                  yystack_[0].value.as<SyntaxContinueStmt *>());
          yylhs.value.as<SyntaxStmt *>()->AssignStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Exp = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Block = nullptr;
          yylhs.value.as<SyntaxStmt *>()->SelectStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->IterationStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ReturnStmt = nullptr;
        }
#line 1632 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 53:
#line 425 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxStmt *>() = new SyntaxStmt();
          yylhs.value.as<SyntaxStmt *>()->BreakStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ContinueStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->AssignStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Exp = nullptr;
          yylhs.value.as<SyntaxStmt *>()->Block = nullptr;
          yylhs.value.as<SyntaxStmt *>()->SelectStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->IterationStmt = nullptr;
          yylhs.value.as<SyntaxStmt *>()->ReturnStmt =
              std::shared_ptr<SyntaxReturnStmt>(
                  yystack_[0].value.as<SyntaxReturnStmt *>());
        }
#line 1648 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 54:
#line 437 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxBreakStmt *>() = new SyntaxBreakStmt();
        }
#line 1656 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 55:
#line 441 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxContinueStmt *>() = new SyntaxContinueStmt();
        }
#line 1664 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 56:
#line 445 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxAssignStmt *>() = new SyntaxAssignStmt();
          yylhs.value.as<SyntaxAssignStmt *>()->LVal =
              std::shared_ptr<SyntaxLVal>(yystack_[3].value.as<SyntaxLVal *>());
          yylhs.value.as<SyntaxAssignStmt *>()->Exp =
              std::shared_ptr<SyntaxExp>(yystack_[1].value.as<SyntaxExp *>());
        }
#line 1674 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 57:
#line 451 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxSelectStmt *>() = new SyntaxSelectStmt();
          yylhs.value.as<SyntaxSelectStmt *>()->Cond =
              std::shared_ptr<SyntaxCond>(yystack_[2].value.as<SyntaxCond *>());
          yylhs.value.as<SyntaxSelectStmt *>()->ifStmt =
              std::shared_ptr<SyntaxStmt>(yystack_[0].value.as<SyntaxStmt *>());
        }
#line 1684 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 58:
#line 456 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxSelectStmt *>() = new SyntaxSelectStmt();
          yylhs.value.as<SyntaxSelectStmt *>()->Cond =
              std::shared_ptr<SyntaxCond>(yystack_[4].value.as<SyntaxCond *>());
          yylhs.value.as<SyntaxSelectStmt *>()->ifStmt =
              std::shared_ptr<SyntaxStmt>(yystack_[2].value.as<SyntaxStmt *>());
          yylhs.value.as<SyntaxSelectStmt *>()->elseStmt =
              std::shared_ptr<SyntaxStmt>(yystack_[0].value.as<SyntaxStmt *>());
        }
#line 1695 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 59:
#line 463 "sysy_parser.yy" // lalr1.cc:859
        {
          // std::cout<<"ok"<<std::endl;
          yylhs.value.as<SyntaxIterationStmt *>() = new SyntaxIterationStmt();
          yylhs.value.as<SyntaxIterationStmt *>()->Cond =
              std::shared_ptr<SyntaxCond>(yystack_[2].value.as<SyntaxCond *>());
          yylhs.value.as<SyntaxIterationStmt *>()->Stmt =
              std::shared_ptr<SyntaxStmt>(yystack_[0].value.as<SyntaxStmt *>());
        }
#line 1706 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 60:
#line 470 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxReturnStmt *>() = new SyntaxReturnStmt();
          yylhs.value.as<SyntaxReturnStmt *>()->Exp =
              std::shared_ptr<SyntaxExp>(yystack_[1].value.as<SyntaxExp *>());
        }
#line 1715 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 61:
#line 474 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxReturnStmt *>() = new SyntaxReturnStmt();
          yylhs.value.as<SyntaxReturnStmt *>()->Exp = nullptr;
        }
#line 1724 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 62:
#line 480 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxExp *>() = new SyntaxExp();
          yylhs.value.as<SyntaxExp *>()->AddExp = std::shared_ptr<SyntaxAddExp>(
              yystack_[0].value.as<SyntaxAddExp *>());
        }
#line 1733 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 63:
#line 485 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxCond *>() = new SyntaxCond();
          yylhs.value.as<SyntaxCond *>()->LOrExp =
              std::shared_ptr<SyntaxLOrExp>(
                  yystack_[0].value.as<SyntaxLOrExp *>());
        }
#line 1742 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 64:
#line 490 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxLVal *>() = new SyntaxLVal();
          yylhs.value.as<SyntaxLVal *>()->id =
              yystack_[1].value.as<std::string>();
          yylhs.value.as<SyntaxLVal *>()->ArrayExpList.swap(
              yystack_[0].value.as<SyntaxArrayExpList *>()->list);
        }
#line 1752 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 65:
#line 496 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[3].value.as<SyntaxArrayExpList *>()->list.push_back(
              std::shared_ptr<SyntaxExp>(yystack_[1].value.as<SyntaxExp *>()));
          yylhs.value.as<SyntaxArrayExpList *>() =
              yystack_[3].value.as<SyntaxArrayExpList *>();
        }
#line 1761 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 66:
#line 500 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxArrayExpList *>() = new SyntaxArrayExpList();
        }
#line 1769 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 67:
#line 504 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxPrimaryExp *>() = new SyntaxPrimaryExp();
          yylhs.value.as<SyntaxPrimaryExp *>()->Exp =
              std::shared_ptr<SyntaxExp>(yystack_[1].value.as<SyntaxExp *>());
          yylhs.value.as<SyntaxPrimaryExp *>()->LVal = nullptr;
          yylhs.value.as<SyntaxPrimaryExp *>()->Number = nullptr;
        }
#line 1780 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 68:
#line 510 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxPrimaryExp *>() = new SyntaxPrimaryExp();
          yylhs.value.as<SyntaxPrimaryExp *>()->Exp = nullptr;
          yylhs.value.as<SyntaxPrimaryExp *>()->LVal =
              std::shared_ptr<SyntaxLVal>(yystack_[0].value.as<SyntaxLVal *>());
          yylhs.value.as<SyntaxPrimaryExp *>()->Number = nullptr;
        }
#line 1791 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 69:
#line 516 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxPrimaryExp *>() = new SyntaxPrimaryExp();
          yylhs.value.as<SyntaxPrimaryExp *>()->Exp = nullptr;
          yylhs.value.as<SyntaxPrimaryExp *>()->LVal = nullptr;
          yylhs.value.as<SyntaxPrimaryExp *>()->Number =
              std::shared_ptr<SyntaxNumber>(
                  yystack_[0].value.as<SyntaxNumber *>());
        }
#line 1802 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 70:
#line 523 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxNumber *>() = new SyntaxNumber();
          yylhs.value.as<SyntaxNumber *>()->num = yystack_[0].value.as<int>();
        }
#line 1811 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 71:
#line 528 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxUnaryExp *>() = new SyntaxUnaryExp();
          yylhs.value.as<SyntaxUnaryExp *>()->op = OP_POS;
          yylhs.value.as<SyntaxUnaryExp *>()->PrimaryExp =
              std::shared_ptr<SyntaxPrimaryExp>(
                  yystack_[0].value.as<SyntaxPrimaryExp *>());
          yylhs.value.as<SyntaxUnaryExp *>()->Callee = nullptr;
          yylhs.value.as<SyntaxUnaryExp *>()->UnaryExp = nullptr;
        }
#line 1823 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 72:
#line 535 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxUnaryExp *>() = new SyntaxUnaryExp();
          yylhs.value.as<SyntaxUnaryExp *>()->op = OP_POS;
          yylhs.value.as<SyntaxUnaryExp *>()->PrimaryExp = nullptr;
          yylhs.value.as<SyntaxUnaryExp *>()->Callee =
              std::shared_ptr<SyntaxCallee>(
                  yystack_[0].value.as<SyntaxCallee *>());
          yylhs.value.as<SyntaxUnaryExp *>()->UnaryExp = nullptr;
        }
#line 1835 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 73:
#line 542 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxUnaryExp *>() = new SyntaxUnaryExp();
          yylhs.value.as<SyntaxUnaryExp *>()->op =
              yystack_[1].value.as<unaryop>();
          yylhs.value.as<SyntaxUnaryExp *>()->PrimaryExp = nullptr;
          yylhs.value.as<SyntaxUnaryExp *>()->Callee = nullptr;
          yylhs.value.as<SyntaxUnaryExp *>()->UnaryExp =
              std::shared_ptr<SyntaxUnaryExp>(
                  yystack_[0].value.as<SyntaxUnaryExp *>());
        }
#line 1847 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 74:
#line 550 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxCallee *>() = new SyntaxCallee();
          yylhs.value.as<SyntaxCallee *>()->id =
              yystack_[3].value.as<std::string>();
          yylhs.value.as<SyntaxCallee *>()->ExpList.swap(
              yystack_[1].value.as<SyntaxExpList *>()->list);
        }
#line 1857 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 75:
#line 555 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxCallee *>() = new SyntaxCallee();
          yylhs.value.as<SyntaxCallee *>()->id =
              yystack_[2].value.as<std::string>();
        }
#line 1866 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 76:
#line 560 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<unaryop>() = OP_POS;
        }
#line 1872 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 77:
#line 561 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<unaryop>() = OP_NEG;
        }
#line 1878 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 78:
#line 562 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<unaryop>() = OP_NOT;
        }
#line 1884 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 79:
#line 564 "sysy_parser.yy" // lalr1.cc:859
        {
          yystack_[2].value.as<SyntaxExpList *>()->list.push_back(
              std::shared_ptr<SyntaxExp>(yystack_[0].value.as<SyntaxExp *>()));
          yylhs.value.as<SyntaxExpList *>() =
              yystack_[2].value.as<SyntaxExpList *>();
        }
#line 1893 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 80:
#line 568 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxExpList *>() = new SyntaxExpList();
          yylhs.value.as<SyntaxExpList *>()->list.push_back(
              std::shared_ptr<SyntaxExp>(yystack_[0].value.as<SyntaxExp *>()));
        }
#line 1902 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 81:
#line 573 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxMulExp *>() = new SyntaxMulExp();
          yylhs.value.as<SyntaxMulExp *>()->MulExp = nullptr;
          yylhs.value.as<SyntaxMulExp *>()->UnaryExp =
              std::shared_ptr<SyntaxUnaryExp>(
                  yystack_[0].value.as<SyntaxUnaryExp *>());
        }
#line 1912 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 82:
#line 578 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxMulExp *>() = new SyntaxMulExp();
          yylhs.value.as<SyntaxMulExp *>()->MulExp =
              std::shared_ptr<SyntaxMulExp>(
                  yystack_[2].value.as<SyntaxMulExp *>());
          yylhs.value.as<SyntaxMulExp *>()->UnaryExp =
              std::shared_ptr<SyntaxUnaryExp>(
                  yystack_[0].value.as<SyntaxUnaryExp *>());
          yylhs.value.as<SyntaxMulExp *>()->op = OP_MUL;
        }
#line 1923 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 83:
#line 584 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxMulExp *>() = new SyntaxMulExp();
          yylhs.value.as<SyntaxMulExp *>()->MulExp =
              std::shared_ptr<SyntaxMulExp>(
                  yystack_[2].value.as<SyntaxMulExp *>());
          yylhs.value.as<SyntaxMulExp *>()->UnaryExp =
              std::shared_ptr<SyntaxUnaryExp>(
                  yystack_[0].value.as<SyntaxUnaryExp *>());
          yylhs.value.as<SyntaxMulExp *>()->op = OP_DIV;
        }
#line 1934 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 84:
#line 590 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxMulExp *>() = new SyntaxMulExp();
          yylhs.value.as<SyntaxMulExp *>()->MulExp =
              std::shared_ptr<SyntaxMulExp>(
                  yystack_[2].value.as<SyntaxMulExp *>());
          yylhs.value.as<SyntaxMulExp *>()->UnaryExp =
              std::shared_ptr<SyntaxUnaryExp>(
                  yystack_[0].value.as<SyntaxUnaryExp *>());
          yylhs.value.as<SyntaxMulExp *>()->op = OP_MOD;
        }
#line 1945 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 85:
#line 597 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxAddExp *>() = new SyntaxAddExp();
          yylhs.value.as<SyntaxAddExp *>()->AddExp = nullptr;
          yylhs.value.as<SyntaxAddExp *>()->MulExp =
              std::shared_ptr<SyntaxMulExp>(
                  yystack_[0].value.as<SyntaxMulExp *>());
        }
#line 1955 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 86:
#line 602 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxAddExp *>() = new SyntaxAddExp();
          yylhs.value.as<SyntaxAddExp *>()->AddExp =
              std::shared_ptr<SyntaxAddExp>(
                  yystack_[2].value.as<SyntaxAddExp *>());
          yylhs.value.as<SyntaxAddExp *>()->MulExp =
              std::shared_ptr<SyntaxMulExp>(
                  yystack_[0].value.as<SyntaxMulExp *>());
          yylhs.value.as<SyntaxAddExp *>()->op = OP_PLUS;
        }
#line 1966 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 87:
#line 608 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxAddExp *>() = new SyntaxAddExp();
          yylhs.value.as<SyntaxAddExp *>()->AddExp =
              std::shared_ptr<SyntaxAddExp>(
                  yystack_[2].value.as<SyntaxAddExp *>());
          yylhs.value.as<SyntaxAddExp *>()->MulExp =
              std::shared_ptr<SyntaxMulExp>(
                  yystack_[0].value.as<SyntaxMulExp *>());
          yylhs.value.as<SyntaxAddExp *>()->op = OP_MINUS;
        }
#line 1977 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 88:
#line 615 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxRelExp *>() = new SyntaxRelExp();
          yylhs.value.as<SyntaxRelExp *>()->RelExp = nullptr;
          yylhs.value.as<SyntaxRelExp *>()->AddExp =
              std::shared_ptr<SyntaxAddExp>(
                  yystack_[0].value.as<SyntaxAddExp *>());
        }
#line 1987 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 89:
#line 620 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxRelExp *>() = new SyntaxRelExp();
          yylhs.value.as<SyntaxRelExp *>()->RelExp =
              std::shared_ptr<SyntaxRelExp>(
                  yystack_[2].value.as<SyntaxRelExp *>());
          yylhs.value.as<SyntaxRelExp *>()->AddExp =
              std::shared_ptr<SyntaxAddExp>(
                  yystack_[0].value.as<SyntaxAddExp *>());
          yylhs.value.as<SyntaxRelExp *>()->op = OP_LT;
        }
#line 1998 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 90:
#line 626 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxRelExp *>() = new SyntaxRelExp();
          yylhs.value.as<SyntaxRelExp *>()->RelExp =
              std::shared_ptr<SyntaxRelExp>(
                  yystack_[2].value.as<SyntaxRelExp *>());
          yylhs.value.as<SyntaxRelExp *>()->AddExp =
              std::shared_ptr<SyntaxAddExp>(
                  yystack_[0].value.as<SyntaxAddExp *>());
          yylhs.value.as<SyntaxRelExp *>()->op = OP_GT;
        }
#line 2009 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 91:
#line 632 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxRelExp *>() = new SyntaxRelExp();
          yylhs.value.as<SyntaxRelExp *>()->RelExp =
              std::shared_ptr<SyntaxRelExp>(
                  yystack_[2].value.as<SyntaxRelExp *>());
          yylhs.value.as<SyntaxRelExp *>()->AddExp =
              std::shared_ptr<SyntaxAddExp>(
                  yystack_[0].value.as<SyntaxAddExp *>());
          yylhs.value.as<SyntaxRelExp *>()->op = OP_LTE;
        }
#line 2020 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 92:
#line 638 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxRelExp *>() = new SyntaxRelExp();
          yylhs.value.as<SyntaxRelExp *>()->RelExp =
              std::shared_ptr<SyntaxRelExp>(
                  yystack_[2].value.as<SyntaxRelExp *>());
          yylhs.value.as<SyntaxRelExp *>()->AddExp =
              std::shared_ptr<SyntaxAddExp>(
                  yystack_[0].value.as<SyntaxAddExp *>());
          yylhs.value.as<SyntaxRelExp *>()->op = OP_GTE;
        }
#line 2031 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 93:
#line 645 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxEqExp *>() = new SyntaxEqExp();
          yylhs.value.as<SyntaxEqExp *>()->EqExp = nullptr;
          yylhs.value.as<SyntaxEqExp *>()->RelExp =
              std::shared_ptr<SyntaxRelExp>(
                  yystack_[0].value.as<SyntaxRelExp *>());
        }
#line 2041 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 94:
#line 650 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxEqExp *>() = new SyntaxEqExp();
          yylhs.value.as<SyntaxEqExp *>()->EqExp = std::shared_ptr<SyntaxEqExp>(
              yystack_[2].value.as<SyntaxEqExp *>());
          yylhs.value.as<SyntaxEqExp *>()->RelExp =
              std::shared_ptr<SyntaxRelExp>(
                  yystack_[0].value.as<SyntaxRelExp *>());
          yylhs.value.as<SyntaxEqExp *>()->op = OP_EQ;
        }
#line 2052 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 95:
#line 656 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxEqExp *>() = new SyntaxEqExp();
          yylhs.value.as<SyntaxEqExp *>()->EqExp = std::shared_ptr<SyntaxEqExp>(
              yystack_[2].value.as<SyntaxEqExp *>());
          yylhs.value.as<SyntaxEqExp *>()->RelExp =
              std::shared_ptr<SyntaxRelExp>(
                  yystack_[0].value.as<SyntaxRelExp *>());
          yylhs.value.as<SyntaxEqExp *>()->op = OP_NEQ;
        }
#line 2063 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 96:
#line 663 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxLAndExp *>() = new SyntaxLAndExp();
          yylhs.value.as<SyntaxLAndExp *>()->LAndExp = nullptr;
          yylhs.value.as<SyntaxLAndExp *>()->EqExp =
              std::shared_ptr<SyntaxEqExp>(
                  yystack_[0].value.as<SyntaxEqExp *>());
        }
#line 2073 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 97:
#line 668 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxLAndExp *>() = new SyntaxLAndExp();
          yylhs.value.as<SyntaxLAndExp *>()->LAndExp =
              std::shared_ptr<SyntaxLAndExp>(
                  yystack_[2].value.as<SyntaxLAndExp *>());
          yylhs.value.as<SyntaxLAndExp *>()->EqExp =
              std::shared_ptr<SyntaxEqExp>(
                  yystack_[0].value.as<SyntaxEqExp *>());
          yylhs.value.as<SyntaxLAndExp *>()->op = OP_AND;
        }
#line 2084 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 98:
#line 675 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxLOrExp *>() = new SyntaxLOrExp();
          yylhs.value.as<SyntaxLOrExp *>()->LOrExp = nullptr;
          yylhs.value.as<SyntaxLOrExp *>()->LAndExp =
              std::shared_ptr<SyntaxLAndExp>(
                  yystack_[0].value.as<SyntaxLAndExp *>());
        }
#line 2094 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 99:
#line 680 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxLOrExp *>() = new SyntaxLOrExp();
          yylhs.value.as<SyntaxLOrExp *>()->LOrExp =
              std::shared_ptr<SyntaxLOrExp>(
                  yystack_[2].value.as<SyntaxLOrExp *>());
          yylhs.value.as<SyntaxLOrExp *>()->LAndExp =
              std::shared_ptr<SyntaxLAndExp>(
                  yystack_[0].value.as<SyntaxLAndExp *>());
          yylhs.value.as<SyntaxLOrExp *>()->op = OP_OR;
        }
#line 2105 "sysy_parser.cc" // lalr1.cc:859
        break;

        case 100:
#line 687 "sysy_parser.yy" // lalr1.cc:859
        {
          yylhs.value.as<SyntaxConstExp *>() = new SyntaxConstExp();
          yylhs.value.as<SyntaxConstExp *>()->AddExp =
              std::shared_ptr<SyntaxAddExp>(
                  yystack_[0].value.as<SyntaxAddExp *>());
        }
#line 2114 "sysy_parser.cc" // lalr1.cc:859
        break;

#line 2118 "sysy_parser.cc" // lalr1.cc:859
        default:
          break;
        }
      } catch (const syntax_error &yyexc) {
        error(yyexc);
        YYERROR;
      }
      YY_SYMBOL_PRINT("-> $$ =", yylhs);
      yypop_(yylen);
      yylen = 0;
      YY_STACK_PRINT();

      // Shift the result of the reduction.
      yypush_(YY_NULLPTR, yylhs);
    }
    goto yynewstate;

  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_) {
      ++yynerrs_;
      error(yyla.location, yysyntax_error_(yystack_[0].state, yyla));
    }

    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3) {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      // Return failure if at end of input.
      if (yyla.type_get() == yyeof_)
        YYABORT;
      else if (!yyla.empty()) {
        yy_destroy_("Error: discarding", yyla);
        yyla.clear();
      }
    }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;

  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_(yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3; // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;) {
        yyn = yypact_[yystack_[0].state];
        if (!yy_pact_value_is_default_(yyn)) {
          yyn += yyterror_;
          if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_) {
            yyn = yytable_[yyn];
            if (0 < yyn)
              break;
          }
        }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size() == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_("Error: popping", yystack_[0]);
        yypop_();
        YY_STACK_PRINT();
      }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT(error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyla.empty())
      yy_destroy_("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_(yylen);
    while (1 < yystack_.size()) {
      yy_destroy_("Cleanup: popping", yystack_[0]);
      yypop_();
    }

    return yyresult;
  } catch (...) {
    YYCDEBUG << "Exception caught: cleaning lookahead and stack" << std::endl;
    // Do not try to display the values of the reclaimed symbols,
    // as their printer might throw an exception.
    if (!yyla.empty())
      yy_destroy_(YY_NULLPTR, yyla);

    while (1 < yystack_.size()) {
      yy_destroy_(YY_NULLPTR, yystack_[0]);
      yypop_();
    }
    throw;
  }
}

void sysy_parser::error(const syntax_error &yyexc) {
  error(yyexc.location, yyexc.what());
}

// Generate an error message.
std::string sysy_parser::yysyntax_error_(state_type yystate,
                                         const symbol_type &yyla) const {
  // Number of reported tokens (one for the "unexpected", one per
  // "expected").
  size_t yycount = 0;
  // Its maximum.
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  // Arguments of yyformat.
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yyla) is
       if this state is a consistent state with a default action.
       Thus, detecting the absence of a lookahead is sufficient to
       determine that there is no unexpected or expected token to
       report.  In that case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is
       a consistent state with a default action.  There might have
       been a previous inconsistent state, consistent state with a
       non-default action, or user semantic action that manipulated
       yyla.  (However, yyla is currently not documented for users.)
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state
       merging (from LALR or IELR) and default reductions corrupt the
       expected token list.  However, the list is correct for
       canonical LR with one exception: it will still contain any
       token that will not be accepted due to an error action in a
       later state.
  */
  if (!yyla.empty()) {
    int yytoken = yyla.type_get();
    yyarg[yycount++] = yytname_[yytoken];
    int yyn = yypact_[yystate];
    if (!yy_pact_value_is_default_(yyn)) {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      // Stay within bounds of both yycheck and yytname.
      int yychecklim = yylast_ - yyn + 1;
      int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
      for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_ &&
            !yy_table_value_is_error_(yytable_[yyx + yyn])) {
          if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM) {
            yycount = 1;
            break;
          } else
            yyarg[yycount++] = yytname_[yyx];
        }
    }
  }

  char const *yyformat = YY_NULLPTR;
  switch (yycount) {
#define YYCASE_(N, S)                                                          \
  case N:                                                                      \
    yyformat = S;                                                              \
    break
    YYCASE_(0, YY_("syntax error"));
    YYCASE_(1, YY_("syntax error, unexpected %s"));
    YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
    YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
    YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
    YYCASE_(5,
            YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
  }

  std::string yyres;
  // Argument number.
  size_t yyi = 0;
  for (char const *yyp = yyformat; *yyp; ++yyp)
    if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount) {
      yyres += yytnamerr_(yyarg[yyi++]);
      ++yyp;
    } else
      yyres += *yyp;
  return yyres;
}

const signed char sysy_parser::yypact_ninf_ = -128;

const signed char sysy_parser::yytable_ninf_ = -1;

const short int sysy_parser::yypact_[] = {
    -15,  -128, -128, -1,   19,   31,   -128, -128, -128, -128, 15,   25,
    -128, -128, -128, 45,   83,   -128, -128, 115,  -128, 43,   -8,   -128,
    36,   55,   -128, 25,   58,   52,   39,   -128, 136,  173,  -128, -128,
    150,  -128, -128, -128, 56,   -1,   58,   -128, -128, 173,  72,   73,
    -128, -128, -128, -128, -128, -128, -128, -128, -128, 173,  1,    144,
    144,  75,   122,  -128, -128, 10,   -128, 86,   -128, -128, 96,   -128,
    -128, -3,   155,  105,  -128, 173,  173,  173,  173,  173,  -128, -128,
    -128, 37,   -128, -128, 111,  67,   116,  120,  127,  -128, -128, 36,
    -128, -128, -128, -128, -128, -128, -128, -128, -128, 135,  142,  173,
    -128, 136,  -128, -128, -128, 57,   173,  -128, -128, -128, 1,    1,
    150,  -128, 173,  -128, 143,  173,  -128, -128, -128, 173,  154,  -128,
    173,  -128, 156,  -128, 152,  144,  99,   151,  146,  121,  -128, 165,
    172,  -128, -128, -128, 108,  173,  173,  173,  173,  173,  173,  173,
    173,  108,  -128, 168,  144,  144,  144,  144,  99,   99,   151,  146,
    -128, 108,  -128};

const unsigned char sysy_parser::yydefact_[] = {
    0,  32, 31, 0,  0,  0,  4,   5,  6,  7,  0,  0,  1,  2,  3,  13, 0,  21,
    13, 0,  10, 0,  22, 19, 0,   0,  8,  0,  0,  0,  0,  34, 0,  0,  13, 20,
    0,  9,  41, 29, 36, 0,  0,   76, 77, 0,  0,  66, 70, 78, 23, 24, 68, 71,
    69, 81, 72, 0,  85, 62, 100, 0,  0,  11, 14, 0,  38, 35, 33, 30, 0,  25,
    28, 0,  0,  64, 73, 0,  0,   0,  0,  0,  12, 16, 18, 0,  47, 39, 0,  0,
    0,  0,  0,  42, 43, 0,  48,  40, 44, 51, 52, 45, 49, 50, 53, 0,  68, 0,
    67, 0,  26, 75, 80, 0,  0,   82, 83, 84, 86, 87, 0,  15, 0,  61, 0,  0,
    54, 55, 46, 0,  0,  27, 0,   74, 0,  17, 0,  88, 93, 96, 98, 63, 60, 0,
    0,  37, 79, 65, 0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  56, 57, 89, 91,
    90, 92, 94, 95, 97, 99, 59,  0,  58};

const short int sysy_parser::yypgoto_[] = {
    -128, -128, -128, 184,  128,  -128, 167, 178,  -56, -128, 132,  -128,
    174,  -44,  -128, -128, 2,    -128, 158, -128, -18, -128, -128, -127,
    -128, -128, -128, -128, -128, -128, -28, 76,   -64, -128, -128, -128,
    -26,  -128, -128, -128, 89,   -33,  29,  47,   44,  -128, 175};

const short int sysy_parser::yydefgoto_[] = {
    -1, 4,  5,  6,  7,  19, 20, 22,  63, 85,  8,   16,  17,  50,  73,  9,
    10, 30, 31, 67, 96, 65, 97, 98,  99, 100, 101, 102, 103, 104, 105, 136,
    52, 75, 53, 54, 55, 56, 57, 113, 58, 59,  138, 139, 140, 141, 64};

const unsigned char sysy_parser::yytable_[] = {
    60,  106, 72,  60,  51,  11,  84,  32,  77,  78,  39,  1,   33,  2,   109,
    43,  44,  70,  51,  12,  110, 159, 3,   29,  69,  1,   86,  2,   45,  60,
    168, 76,  38,  87,  13,  88,  1,   89,  2,   90,  47,  48,  170, 29,  79,
    15,  112, 3,   91,  92,  49,  115, 116, 117, 120, 18,  41,  1,   42,  2,
    121, 124, 28,  21,  135, 131, 34,  95,  3,   1,   36,  2,   43,  44,  132,
    33,  133, 43,  44,  130, 38,  51,  40,  123, 106, 45,  134, 60,  66,  137,
    45,  74,  137, 106, 46,  71,  82,  47,  48,  23,  24,  144, 47,  48,  146,
    106, 107, 49,  149, 150, 151, 152, 49,  43,  44,  108, 160, 161, 162, 163,
    137, 137, 137, 137, 86,  114, 45,  43,  44,  122, 38,  26,  27,  88,  125,
    89,  126, 90,  47,  48,  45,  43,  44,  127, 62,  83,  91,  92,  49,  80,
    81,  128, 47,  48,  45,  43,  44,  129, 46,  142, 43,  44,  49,  156, 153,
    154, 47,  48,  45,  118, 119, 148, 62,  45,  111, 145, 49,  147, 43,  44,
    47,  48,  164, 165, 157, 47,  48,  155, 158, 14,  49,  45,  169, 93,  37,
    49,  25,  94,  35,  68,  167, 143, 166, 47,  48,  0,   0,   0,   61,  0,
    0,   0,   0,   49};

const short int sysy_parser::yycheck_[] = {
    33,  65,  46,  36,  32,  3,   62,  15,  7,  8,   28, 26,  20,  28,  17,
    5,   6,   45,  46,  0,   23,  148, 37,  21, 42,  26, 16,  28,  18,  62,
    157, 57,  22,  23,  3,   25,  26,  27,  28, 29,  30, 31,  169, 41,  43,
    30,  74,  37,  38,  39,  40,  77,  78,  79, 17,  30, 17,  26,  19,  28,
    23,  89,  19,  18,  120, 109, 30,  65,  37, 26,  15, 28,  5,   6,   17,
    20,  19,  5,   6,   107, 22,  109, 30,  16, 148, 18, 114, 120, 32,  122,
    18,  18,  125, 157, 22,  23,  21,  30,  31, 16,  17, 129, 30,  31,  132,
    169, 20,  40,  9,   10,  11,  12,  40,  5,  6,   19, 149, 150, 151, 152,
    153, 154, 155, 156, 16,  20,  18,  5,   6,  18,  22, 16,  17,  25,  18,
    27,  16,  29,  30,  31,  18,  5,   6,   16, 22,  23, 38,  39,  40,  5,
    6,   16,  30,  31,  18,  5,   6,   15,  22, 16,  5,  6,   40,  42,  13,
    14,  30,  31,  18,  80,  81,  19,  22,  18, 19,  21, 40,  21,  5,   6,
    30,  31,  153, 154, 19,  30,  31,  41,  16, 5,   40, 18,  24,  65,  27,
    40,  18,  65,  24,  41,  156, 125, 155, 30, 31,  -1, -1,  -1,  33,  -1,
    -1,  -1,  -1,  40};

const unsigned char sysy_parser::yystos_[] = {
    0,  26, 28, 37, 45, 46, 47, 48, 54, 59, 60, 60, 0,  3,  47, 30, 55, 56, 30,
    49, 50, 18, 51, 16, 17, 51, 16, 17, 19, 60, 61, 62, 15, 20, 30, 56, 15, 50,
    22, 64, 30, 17, 19, 5,  6,  18, 22, 30, 31, 40, 57, 74, 76, 78, 79, 80, 81,
    82, 84, 85, 85, 90, 22, 52, 90, 65, 32, 63, 62, 64, 74, 23, 57, 58, 18, 77,
    80, 7,  8,  43, 5,  6,  21, 23, 52, 53, 16, 23, 25, 27, 29, 38, 39, 48, 54,
    60, 64, 66, 67, 68, 69, 70, 71, 72, 73, 74, 76, 20, 19, 17, 23, 19, 74, 83,
    20, 80, 80, 80, 84, 84, 17, 23, 18, 16, 74, 18, 16, 16, 16, 15, 74, 57, 17,
    19, 74, 52, 75, 85, 86, 87, 88, 89, 16, 75, 74, 21, 74, 21, 19, 9,  10, 11,
    12, 13, 14, 41, 42, 19, 16, 67, 85, 85, 85, 85, 86, 86, 87, 88, 67, 24, 67};

const unsigned char sysy_parser::yyr1_[] = {
    0,  44, 45, 46, 46, 47, 47, 47, 48, 49, 49, 50, 51, 51, 52, 52, 52,
    53, 53, 54, 55, 55, 56, 56, 57, 57, 57, 58, 58, 59, 59, 60, 60, 61,
    61, 62, 62, 63, 63, 64, 65, 65, 66, 66, 66, 67, 67, 67, 67, 67, 67,
    67, 67, 67, 68, 69, 70, 71, 71, 72, 73, 73, 74, 75, 76, 77, 77, 78,
    78, 78, 79, 80, 80, 80, 81, 81, 82, 82, 82, 83, 83, 84, 84, 84, 84,
    85, 85, 85, 86, 86, 86, 86, 86, 87, 87, 87, 88, 88, 89, 89, 90};

const unsigned char sysy_parser::yyr2_[] = {
    0, 2, 2, 2, 1, 1, 1, 1, 4, 3, 1, 4, 4, 0, 1, 3, 2, 3, 1, 3, 3,
    1, 2, 4, 1, 2, 3, 3, 1, 5, 6, 1, 1, 3, 1, 3, 2, 4, 1, 3, 2, 0,
    1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 5, 7, 5, 3, 2, 1,
    1, 2, 4, 0, 3, 1, 1, 1, 1, 1, 2, 4, 3, 1, 1, 1, 3, 1, 1, 3, 3,
    3, 1, 3, 3, 1, 3, 3, 3, 3, 1, 3, 3, 1, 3, 1, 3, 1};

// YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
// First, the terminals, then, starting at \a yyntokens_, nonterminals.
const char *const sysy_parser::yytname_[] = {"$end",
                                             "error",
                                             "$undefined",
                                             "END",
                                             "ERROR",
                                             "ADD",
                                             "SUB",
                                             "MUL",
                                             "DIV",
                                             "LT",
                                             "LTE",
                                             "GT",
                                             "GTE",
                                             "EQ",
                                             "NEQ",
                                             "ASSIN",
                                             "SEMICOLON",
                                             "COMMA",
                                             "LPARENTHESE",
                                             "RPARENTHESE",
                                             "LBRACKET",
                                             "RBRACKET",
                                             "LBRACE",
                                             "RBRACE",
                                             "ELSE",
                                             "IF",
                                             "INT",
                                             "RETURN",
                                             "VOID",
                                             "WHILE",
                                             "IDENTIFIER",
                                             "NUMBER",
                                             "ARRAY",
                                             "LETTER",
                                             "EOL",
                                             "COMMENT",
                                             "BLANK",
                                             "CONST",
                                             "BREAK",
                                             "CONTINUE",
                                             "NOT",
                                             "AND",
                                             "OR",
                                             "MOD",
                                             "$accept",
                                             "Begin",
                                             "CompUnit",
                                             "DeclDef",
                                             "ConstDecl",
                                             "ConstDefList",
                                             "ConstDef",
                                             "ArrayConstExpList",
                                             "ConstInitVal",
                                             "ConstInitValList",
                                             "VarDecl",
                                             "VarDefList",
                                             "VarDef",
                                             "InitVal",
                                             "InitValList",
                                             "FuncDef",
                                             "DefType",
                                             "FuncFParamList",
                                             "FuncFParam",
                                             "ParamArrayExpList",
                                             "Block",
                                             "BlockItemList",
                                             "BlockItem",
                                             "Stmt",
                                             "BreakStmt",
                                             "ContinueStmt",
                                             "AssignStmt",
                                             "SelectStmt",
                                             "IterationStmt",
                                             "ReturnStmt",
                                             "Exp",
                                             "Cond",
                                             "LVal",
                                             "ArrayExpList",
                                             "PrimaryExp",
                                             "Number",
                                             "UnaryExp",
                                             "Callee",
                                             "UnaryOp",
                                             "ExpList",
                                             "MulExp",
                                             "AddExp",
                                             "RelExp",
                                             "EqExp",
                                             "LAndExp",
                                             "LOrExp",
                                             "ConstExp",
                                             YY_NULLPTR};

#if YYDEBUG
const unsigned short int sysy_parser::yyrline_[] = {
    0,   112, 112, 122, 126, 131, 137, 143, 150, 156, 160, 165, 172, 176, 180,
    184, 189, 194, 198, 203, 209, 213, 218, 224, 231, 235, 239, 245, 249, 254,
    260, 268, 269, 271, 275, 280, 287, 294, 298, 302, 307, 311, 315, 321, 330,
    337, 348, 359, 370, 381, 392, 403, 414, 425, 437, 441, 445, 451, 456, 463,
    470, 474, 480, 485, 490, 496, 500, 504, 510, 516, 523, 528, 535, 542, 550,
    555, 560, 561, 562, 564, 568, 573, 578, 584, 590, 597, 602, 608, 615, 620,
    626, 632, 638, 645, 650, 656, 663, 668, 675, 680, 687};

// Print the state stack on the debug stream.
void sysy_parser::yystack_print_() {
  *yycdebug_ << "Stack now";
  for (stack_type::const_iterator i = yystack_.begin(), i_end = yystack_.end();
       i != i_end; ++i)
    *yycdebug_ << ' ' << i->state;
  *yycdebug_ << std::endl;
}

// Report on the debug stream that the rule \a yyrule is going to be reduced.
void sysy_parser::yy_reduce_print_(int yyrule) {
  unsigned int yylno = yyrline_[yyrule];
  int yynrhs = yyr2_[yyrule];
  // Print the symbols being reduced, and their result.
  *yycdebug_ << "Reducing stack by rule " << yyrule - 1 << " (line " << yylno
             << "):" << std::endl;
  // The symbols being reduced.
  for (int yyi = 0; yyi < yynrhs; yyi++)
    YY_SYMBOL_PRINT("   $" << yyi + 1 << " =", yystack_[(yynrhs) - (yyi + 1)]);
}
#endif // YYDEBUG

} // namespace yy
#line 2624 "sysy_parser.cc" // lalr1.cc:1167
#line 693 "sysy_parser.yy"  // lalr1.cc:1168

// Register errors to the driver:
void yy::sysy_parser::error(const location_type &l, const std::string &m) {
  driver.error(l, m);
}
