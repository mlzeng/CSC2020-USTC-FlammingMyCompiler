// A Bison parser, made by GNU Bison 3.0.4.

// Skeleton interface for Bison LALR(1) parsers in C++

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

/**
 ** \file sysy_parser.hh
 ** Define the yy::parser class.
 */

// C++ LALR(1) parser skeleton written by Akim Demaille.

#ifndef YY_YY_SYSY_PARSER_HH_INCLUDED
#define YY_YY_SYSY_PARSER_HH_INCLUDED
// //                    "%code requires" blocks.
#line 11 "sysy_parser.yy" // lalr1.cc:377

#include "syntax_tree.hh"
#include <string>
class sysy_driver;

#line 50 "sysy_parser.hh" // lalr1.cc:377

#include "location.hh"
#include "stack.hh"
#include <cassert>
#include <cstdlib> // std::abort
#include <iostream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>
#ifndef YYASSERT
#include <cassert>
#define YYASSERT assert
#endif

#ifndef YY_ATTRIBUTE
#if (defined __GNUC__ &&                                                       \
     (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__))) ||             \
    defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#define YY_ATTRIBUTE(Spec) __attribute__(Spec)
#else
#define YY_ATTRIBUTE(Spec) /* empty */
#endif
#endif

#ifndef YY_ATTRIBUTE_PURE
#define YY_ATTRIBUTE_PURE YY_ATTRIBUTE((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
#define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE((__unused__))
#endif

#if !defined _Noreturn &&                                                      \
    (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
#if defined _MSC_VER && 1200 <= _MSC_VER
#define _Noreturn __declspec(noreturn)
#else
#define _Noreturn YY_ATTRIBUTE((__noreturn__))
#endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if !defined lint || defined __GNUC__
#define YYUSE(E) ((void)(E))
#else
#define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                                    \
  _Pragma("GCC diagnostic push")                                               \
      _Pragma("GCC diagnostic ignored \"-Wuninitialized\"")                    \
          _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
#define YY_IGNORE_MAYBE_UNINITIALIZED_END _Pragma("GCC diagnostic pop")
#else
#define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
#define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* Debug traces.  */
#ifndef YYDEBUG
#define YYDEBUG 1
#endif

namespace yy {
#line 127 "sysy_parser.hh" // lalr1.cc:377

/// A char[S] buffer to store and retrieve objects.
///
/// Sort of a variant, but does not keep track of the nature
/// of the stored data, since that knowledge is available
/// via the current state.
template <size_t S> struct variant {
  /// Type of *this.
  typedef variant<S> self_type;

  /// Empty construction.
  variant() : yytypeid_(YY_NULLPTR) {}

  /// Construct and fill.
  template <typename T> variant(const T &t) : yytypeid_(&typeid(T)) {
    YYASSERT(sizeof(T) <= S);
    new (yyas_<T>()) T(t);
  }

  /// Destruction, allowed only if empty.
  ~variant() { YYASSERT(!yytypeid_); }

  /// Instantiate an empty \a T in here.
  template <typename T> T &build() {
    YYASSERT(!yytypeid_);
    YYASSERT(sizeof(T) <= S);
    yytypeid_ = &typeid(T);
    return *new (yyas_<T>()) T;
  }

  /// Instantiate a \a T in here from \a t.
  template <typename T> T &build(const T &t) {
    YYASSERT(!yytypeid_);
    YYASSERT(sizeof(T) <= S);
    yytypeid_ = &typeid(T);
    return *new (yyas_<T>()) T(t);
  }

  /// Accessor to a built \a T.
  template <typename T> T &as() {
    YYASSERT(*yytypeid_ == typeid(T));
    YYASSERT(sizeof(T) <= S);
    return *yyas_<T>();
  }

  /// Const accessor to a built \a T (for %printer).
  template <typename T> const T &as() const {
    YYASSERT(*yytypeid_ == typeid(T));
    YYASSERT(sizeof(T) <= S);
    return *yyas_<T>();
  }

  /// Swap the content with \a other, of same type.
  ///
  /// Both variants must be built beforehand, because swapping the actual
  /// data requires reading it (with as()), and this is not possible on
  /// unconstructed variants: it would require some dynamic testing, which
  /// should not be the variant's responsability.
  /// Swapping between built and (possibly) non-built is done with
  /// variant::move ().
  template <typename T> void swap(self_type &other) {
    YYASSERT(yytypeid_);
    YYASSERT(*yytypeid_ == *other.yytypeid_);
    std::swap(as<T>(), other.as<T>());
  }

  /// Move the content of \a other to this.
  ///
  /// Destroys \a other.
  template <typename T> void move(self_type &other) {
    build<T>();
    swap<T>(other);
    other.destroy<T>();
  }

  /// Copy the content of \a other to this.
  template <typename T> void copy(const self_type &other) {
    build<T>(other.as<T>());
  }

  /// Destroy the stored \a T.
  template <typename T> void destroy() {
    as<T>().~T();
    yytypeid_ = YY_NULLPTR;
  }

private:
  /// Prohibit blind copies.
  self_type &operator=(const self_type &);
  variant(const self_type &);

  /// Accessor to raw memory as \a T.
  template <typename T> T *yyas_() {
    void *yyp = yybuffer_.yyraw;
    return static_cast<T *>(yyp);
  }

  /// Const accessor to raw memory as \a T.
  template <typename T> const T *yyas_() const {
    const void *yyp = yybuffer_.yyraw;
    return static_cast<const T *>(yyp);
  }

  union {
    /// Strongest alignment constraints.
    long double yyalign_me;
    /// A buffer large enough to store any of the semantic values.
    char yyraw[S];
  } yybuffer_;

  /// Whether the content is built: if defined, the name of the stored type.
  const std::type_info *yytypeid_;
};

/// A Bison parser.
class sysy_parser {
public:
#ifndef YYSTYPE
  /// An auxiliary type to compute the largest semantic type.
  union union_type {
    // AddExp
    char dummy1[sizeof(SyntaxAddExp *)];

    // ArrayConstExpList
    char dummy2[sizeof(SyntaxArrayConstExpList *)];

    // ArrayExpList
    char dummy3[sizeof(SyntaxArrayExpList *)];

    // AssignStmt
    char dummy4[sizeof(SyntaxAssignStmt *)];

    // Block
    char dummy5[sizeof(SyntaxBlock *)];

    // BlockItem
    char dummy6[sizeof(SyntaxBlockItem *)];

    // BlockItemList
    char dummy7[sizeof(SyntaxBlockItemList *)];

    // BreakStmt
    char dummy8[sizeof(SyntaxBreakStmt *)];

    // Callee
    char dummy9[sizeof(SyntaxCallee *)];

    // CompUnit
    char dummy10[sizeof(SyntaxCompUnit *)];

    // Cond
    char dummy11[sizeof(SyntaxCond *)];

    // ConstDecl
    char dummy12[sizeof(SyntaxConstDecl *)];

    // ConstDef
    char dummy13[sizeof(SyntaxConstDef *)];

    // ConstDefList
    char dummy14[sizeof(SyntaxConstDefList *)];

    // ConstExp
    char dummy15[sizeof(SyntaxConstExp *)];

    // ConstInitVal
    char dummy16[sizeof(SyntaxConstInitVal *)];

    // ConstInitValList
    char dummy17[sizeof(SyntaxConstInitValList *)];

    // ContinueStmt
    char dummy18[sizeof(SyntaxContinueStmt *)];

    // DeclDef
    char dummy19[sizeof(SyntaxDeclDef *)];

    // EqExp
    char dummy20[sizeof(SyntaxEqExp *)];

    // Exp
    char dummy21[sizeof(SyntaxExp *)];

    // ExpList
    char dummy22[sizeof(SyntaxExpList *)];

    // FuncDef
    char dummy23[sizeof(SyntaxFuncDef *)];

    // FuncFParam
    char dummy24[sizeof(SyntaxFuncFParam *)];

    // FuncFParamList
    char dummy25[sizeof(SyntaxFuncFParamList *)];

    // InitVal
    char dummy26[sizeof(SyntaxInitVal *)];

    // InitValList
    char dummy27[sizeof(SyntaxInitValList *)];

    // IterationStmt
    char dummy28[sizeof(SyntaxIterationStmt *)];

    // LAndExp
    char dummy29[sizeof(SyntaxLAndExp *)];

    // LOrExp
    char dummy30[sizeof(SyntaxLOrExp *)];

    // LVal
    char dummy31[sizeof(SyntaxLVal *)];

    // MulExp
    char dummy32[sizeof(SyntaxMulExp *)];

    // Number
    char dummy33[sizeof(SyntaxNumber *)];

    // ParamArrayExpList
    char dummy34[sizeof(SyntaxParamArrayExpList *)];

    // PrimaryExp
    char dummy35[sizeof(SyntaxPrimaryExp *)];

    // RelExp
    char dummy36[sizeof(SyntaxRelExp *)];

    // ReturnStmt
    char dummy37[sizeof(SyntaxReturnStmt *)];

    // SelectStmt
    char dummy38[sizeof(SyntaxSelectStmt *)];

    // Stmt
    char dummy39[sizeof(SyntaxStmt *)];

    // UnaryExp
    char dummy40[sizeof(SyntaxUnaryExp *)];

    // VarDecl
    char dummy41[sizeof(SyntaxVarDecl *)];

    // VarDef
    char dummy42[sizeof(SyntaxVarDef *)];

    // VarDefList
    char dummy43[sizeof(SyntaxVarDefList *)];

    // NUMBER
    char dummy44[sizeof(int)];

    // IDENTIFIER
    char dummy45[sizeof(std::string)];

    // DefType
    char dummy46[sizeof(type_specifier)];

    // UnaryOp
    char dummy47[sizeof(unaryop)];
  };

  /// Symbol semantic values.
  typedef variant<sizeof(union_type)> semantic_type;
#else
  typedef YYSTYPE semantic_type;
#endif
  /// Symbol locations.
  typedef location location_type;

  /// Syntax errors thrown from user actions.
  struct syntax_error : std::runtime_error {
    syntax_error(const location_type &l, const std::string &m);
    location_type location;
  };

  /// Tokens.
  struct token {
    enum yytokentype {
      TOK_END = 299,
      TOK_ERROR = 258,
      TOK_ADD = 259,
      TOK_SUB = 260,
      TOK_MUL = 261,
      TOK_DIV = 262,
      TOK_LT = 263,
      TOK_LTE = 264,
      TOK_GT = 265,
      TOK_GTE = 266,
      TOK_EQ = 267,
      TOK_NEQ = 268,
      TOK_ASSIN = 269,
      TOK_SEMICOLON = 270,
      TOK_COMMA = 271,
      TOK_LPARENTHESE = 272,
      TOK_RPARENTHESE = 273,
      TOK_LBRACKET = 274,
      TOK_RBRACKET = 275,
      TOK_LBRACE = 276,
      TOK_RBRACE = 277,
      TOK_ELSE = 278,
      TOK_IF = 279,
      TOK_INT = 280,
      TOK_RETURN = 281,
      TOK_VOID = 282,
      TOK_WHILE = 283,
      TOK_IDENTIFIER = 284,
      TOK_NUMBER = 285,
      TOK_ARRAY = 286,
      TOK_LETTER = 287,
      TOK_EOL = 288,
      TOK_COMMENT = 289,
      TOK_BLANK = 290,
      TOK_CONST = 291,
      TOK_BREAK = 292,
      TOK_CONTINUE = 293,
      TOK_NOT = 294,
      TOK_AND = 295,
      TOK_OR = 296,
      TOK_MOD = 297
    };
  };

  /// (External) token type, as returned by yylex.
  typedef token::yytokentype token_type;

  /// Symbol type: an internal symbol number.
  typedef int symbol_number_type;

  /// The symbol type number to denote an empty symbol.
  enum { empty_symbol = -2 };

  /// Internal symbol number for tokens (subsumed by symbol_number_type).
  typedef unsigned char token_number_type;

  /// A complete symbol.
  ///
  /// Expects its Base type to provide access to the symbol type
  /// via type_get().
  ///
  /// Provide access to semantic value and location.
  template <typename Base> struct basic_symbol : Base {
    /// Alias to Base.
    typedef Base super_type;

    /// Default constructor.
    basic_symbol();

    /// Copy constructor.
    basic_symbol(const basic_symbol &other);

    /// Constructor for valueless symbols, and symbols from each type.

    basic_symbol(typename Base::kind_type t, const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxAddExp *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxArrayConstExpList *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxArrayExpList *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxAssignStmt *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxBlock *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxBlockItem *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxBlockItemList *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxBreakStmt *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxCallee *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxCompUnit *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxCond *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxConstDecl *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxConstDef *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxConstDefList *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxConstExp *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxConstInitVal *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxConstInitValList *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxContinueStmt *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxDeclDef *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxEqExp *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxExp *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxExpList *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxFuncDef *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxFuncFParam *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxFuncFParamList *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxInitVal *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxInitValList *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxIterationStmt *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxLAndExp *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxLOrExp *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxLVal *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxMulExp *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxNumber *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxParamArrayExpList *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxPrimaryExp *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxRelExp *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxReturnStmt *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxSelectStmt *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxStmt *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxUnaryExp *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxVarDecl *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxVarDef *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const SyntaxVarDefList *v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const int v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const std::string v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const type_specifier v,
                 const location_type &l);

    basic_symbol(typename Base::kind_type t, const unaryop v,
                 const location_type &l);

    /// Constructor for symbols with semantic value.
    basic_symbol(typename Base::kind_type t, const semantic_type &v,
                 const location_type &l);

    /// Destroy the symbol.
    ~basic_symbol();

    /// Destroy contents, and record that is empty.
    void clear();

    /// Whether empty.
    bool empty() const;

    /// Destructive move, \a s is emptied into this.
    void move(basic_symbol &s);

    /// The semantic value.
    semantic_type value;

    /// The location.
    location_type location;

  private:
    /// Assignment operator.
    basic_symbol &operator=(const basic_symbol &other);
  };

  /// Type access provider for token (enum) based symbols.
  struct by_type {
    /// Default constructor.
    by_type();

    /// Copy constructor.
    by_type(const by_type &other);

    /// The symbol type as needed by the constructor.
    typedef token_type kind_type;

    /// Constructor from (external) token numbers.
    by_type(kind_type t);

    /// Record that this symbol is empty.
    void clear();

    /// Steal the symbol type from \a that.
    void move(by_type &that);

    /// The (internal) type number (corresponding to \a type).
    /// \a empty when empty.
    symbol_number_type type_get() const;

    /// The token.
    token_type token() const;

    /// The symbol type.
    /// \a empty_symbol when empty.
    /// An int, not token_number_type, to be able to store empty_symbol.
    int type;
  };

  /// "External" symbols: returned by the scanner.
  typedef basic_symbol<by_type> symbol_type;

  // Symbol constructors declarations.
  static inline symbol_type make_END(const location_type &l);

  static inline symbol_type make_ERROR(const location_type &l);

  static inline symbol_type make_ADD(const location_type &l);

  static inline symbol_type make_SUB(const location_type &l);

  static inline symbol_type make_MUL(const location_type &l);

  static inline symbol_type make_DIV(const location_type &l);

  static inline symbol_type make_LT(const location_type &l);

  static inline symbol_type make_LTE(const location_type &l);

  static inline symbol_type make_GT(const location_type &l);

  static inline symbol_type make_GTE(const location_type &l);

  static inline symbol_type make_EQ(const location_type &l);

  static inline symbol_type make_NEQ(const location_type &l);

  static inline symbol_type make_ASSIN(const location_type &l);

  static inline symbol_type make_SEMICOLON(const location_type &l);

  static inline symbol_type make_COMMA(const location_type &l);

  static inline symbol_type make_LPARENTHESE(const location_type &l);

  static inline symbol_type make_RPARENTHESE(const location_type &l);

  static inline symbol_type make_LBRACKET(const location_type &l);

  static inline symbol_type make_RBRACKET(const location_type &l);

  static inline symbol_type make_LBRACE(const location_type &l);

  static inline symbol_type make_RBRACE(const location_type &l);

  static inline symbol_type make_ELSE(const location_type &l);

  static inline symbol_type make_IF(const location_type &l);

  static inline symbol_type make_INT(const location_type &l);

  static inline symbol_type make_RETURN(const location_type &l);

  static inline symbol_type make_VOID(const location_type &l);

  static inline symbol_type make_WHILE(const location_type &l);

  static inline symbol_type make_IDENTIFIER(const std::string &v,
                                            const location_type &l);

  static inline symbol_type make_NUMBER(const int &v, const location_type &l);

  static inline symbol_type make_ARRAY(const location_type &l);

  static inline symbol_type make_LETTER(const location_type &l);

  static inline symbol_type make_EOL(const location_type &l);

  static inline symbol_type make_COMMENT(const location_type &l);

  static inline symbol_type make_BLANK(const location_type &l);

  static inline symbol_type make_CONST(const location_type &l);

  static inline symbol_type make_BREAK(const location_type &l);

  static inline symbol_type make_CONTINUE(const location_type &l);

  static inline symbol_type make_NOT(const location_type &l);

  static inline symbol_type make_AND(const location_type &l);

  static inline symbol_type make_OR(const location_type &l);

  static inline symbol_type make_MOD(const location_type &l);

  /// Build a parser object.
  sysy_parser(sysy_driver &driver_yyarg);
  virtual ~sysy_parser();

  /// Parse.
  /// \returns  0 iff parsing succeeded.
  virtual int parse();

#if YYDEBUG
  /// The current debugging stream.
  std::ostream &debug_stream() const YY_ATTRIBUTE_PURE;
  /// Set the current debugging stream.
  void set_debug_stream(std::ostream &);

  /// Type for debugging levels.
  typedef int debug_level_type;
  /// The current debugging level.
  debug_level_type debug_level() const YY_ATTRIBUTE_PURE;
  /// Set the current debugging level.
  void set_debug_level(debug_level_type l);
#endif

  /// Report a syntax error.
  /// \param loc    where the syntax error is found.
  /// \param msg    a description of the syntax error.
  virtual void error(const location_type &loc, const std::string &msg);

  /// Report a syntax error.
  void error(const syntax_error &err);

private:
  /// This class is not copyable.
  sysy_parser(const sysy_parser &);
  sysy_parser &operator=(const sysy_parser &);

  /// State numbers.
  typedef int state_type;

  /// Generate an error message.
  /// \param yystate   the state where the error occurred.
  /// \param yyla      the lookahead token.
  virtual std::string yysyntax_error_(state_type yystate,
                                      const symbol_type &yyla) const;

  /// Compute post-reduction state.
  /// \param yystate   the current state
  /// \param yysym     the nonterminal to push on the stack
  state_type yy_lr_goto_state_(state_type yystate, int yysym);

  /// Whether the given \c yypact_ value indicates a defaulted state.
  /// \param yyvalue   the value to check
  static bool yy_pact_value_is_default_(int yyvalue);

  /// Whether the given \c yytable_ value indicates a syntax error.
  /// \param yyvalue   the value to check
  static bool yy_table_value_is_error_(int yyvalue);

  static const signed char yypact_ninf_;
  static const signed char yytable_ninf_;

  /// Convert a scanner token number \a t to a symbol number.
  static token_number_type yytranslate_(token_type t);

  // Tables.
  // YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
  // STATE-NUM.
  static const short int yypact_[];

  // YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
  // Performed when YYTABLE does not specify something else to do.  Zero
  // means the default is an error.
  static const unsigned char yydefact_[];

  // YYPGOTO[NTERM-NUM].
  static const short int yypgoto_[];

  // YYDEFGOTO[NTERM-NUM].
  static const short int yydefgoto_[];

  // YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
  // positive, shift that token.  If negative, reduce the rule whose
  // number is the opposite.  If YYTABLE_NINF, syntax error.
  static const unsigned char yytable_[];

  static const short int yycheck_[];

  // YYSTOS[STATE-NUM] -- The (internal number of the) accessing
  // symbol of state STATE-NUM.
  static const unsigned char yystos_[];

  // YYR1[YYN] -- Symbol number of symbol that rule YYN derives.
  static const unsigned char yyr1_[];

  // YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.
  static const unsigned char yyr2_[];

  /// Convert the symbol name \a n to a form suitable for a diagnostic.
  static std::string yytnamerr_(const char *n);

  /// For a symbol, its name in clear.
  static const char *const yytname_[];
#if YYDEBUG
  // YYRLINE[YYN] -- Source line where rule number YYN was defined.
  static const unsigned short int yyrline_[];
  /// Report on the debug stream that the rule \a r is going to be reduced.
  virtual void yy_reduce_print_(int r);
  /// Print the state stack on the debug stream.
  virtual void yystack_print_();

  // Debugging.
  int yydebug_;
  std::ostream *yycdebug_;

  /// \brief Display a symbol type, value and location.
  /// \param yyo    The output stream.
  /// \param yysym  The symbol.
  template <typename Base>
  void yy_print_(std::ostream &yyo, const basic_symbol<Base> &yysym) const;
#endif

  /// \brief Reclaim the memory associated to a symbol.
  /// \param yymsg     Why this token is reclaimed.
  ///                  If null, print nothing.
  /// \param yysym     The symbol.
  template <typename Base>
  void yy_destroy_(const char *yymsg, basic_symbol<Base> &yysym) const;

private:
  /// Type access provider for state based symbols.
  struct by_state {
    /// Default constructor.
    by_state();

    /// The symbol type as needed by the constructor.
    typedef state_type kind_type;

    /// Constructor.
    by_state(kind_type s);

    /// Copy constructor.
    by_state(const by_state &other);

    /// Record that this symbol is empty.
    void clear();

    /// Steal the symbol type from \a that.
    void move(by_state &that);

    /// The (internal) type number (corresponding to \a state).
    /// \a empty_symbol when empty.
    symbol_number_type type_get() const;

    /// The state number used to denote an empty symbol.
    enum { empty_state = -1 };

    /// The state.
    /// \a empty when empty.
    state_type state;
  };

  /// "Internal" symbol: element of the stack.
  struct stack_symbol_type : basic_symbol<by_state> {
    /// Superclass.
    typedef basic_symbol<by_state> super_type;
    /// Construct an empty symbol.
    stack_symbol_type();
    /// Steal the contents from \a sym to build this.
    stack_symbol_type(state_type s, symbol_type &sym);
    /// Assignment, needed by push_back.
    stack_symbol_type &operator=(const stack_symbol_type &that);
  };

  /// Stack type.
  typedef stack<stack_symbol_type> stack_type;

  /// The stack.
  stack_type yystack_;

  /// Push a new state on the stack.
  /// \param m    a debug message to display
  ///             if null, no trace is output.
  /// \param s    the symbol
  /// \warning the contents of \a s.value is stolen.
  void yypush_(const char *m, stack_symbol_type &s);

  /// Push a new look ahead token on the state on the stack.
  /// \param m    a debug message to display
  ///             if null, no trace is output.
  /// \param s    the state
  /// \param sym  the symbol (for its value and location).
  /// \warning the contents of \a s.value is stolen.
  void yypush_(const char *m, state_type s, symbol_type &sym);

  /// Pop \a n symbols the three stacks.
  void yypop_(unsigned int n = 1);

  /// Constants.
  enum {
    yyeof_ = 0,
    yylast_ = 213, ///< Last index in yytable_.
    yynnts_ = 47,  ///< Number of nonterminal symbols.
    yyfinal_ = 12, ///< Termination state number.
    yyterror_ = 1,
    yyerrcode_ = 256,
    yyntokens_ = 44 ///< Number of tokens.
  };

  // User arguments.
  sysy_driver &driver;
};

// Symbol number corresponding to token number t.
inline sysy_parser::token_number_type sysy_parser::yytranslate_(token_type t) {
  static const token_number_type translate_table[] = {
      0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  1,  2,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
      16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
      34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 2,  3};
  const unsigned int user_token_number_max_ = 299;
  const token_number_type undef_token_ = 2;

  if (static_cast<int>(t) <= yyeof_)
    return yyeof_;
  else if (static_cast<unsigned int>(t) <= user_token_number_max_)
    return translate_table[t];
  else
    return undef_token_;
}

inline sysy_parser::syntax_error::syntax_error(const location_type &l,
                                               const std::string &m)
    : std::runtime_error(m), location(l) {}

// basic_symbol.
template <typename Base>
inline sysy_parser::basic_symbol<Base>::basic_symbol() : value() {}

template <typename Base>
inline sysy_parser::basic_symbol<Base>::basic_symbol(const basic_symbol &other)
    : Base(other), value(), location(other.location) {
  switch (other.type_get()) {
  case 85: // AddExp
    value.copy<SyntaxAddExp *>(other.value);
    break;

  case 51: // ArrayConstExpList
    value.copy<SyntaxArrayConstExpList *>(other.value);
    break;

  case 77: // ArrayExpList
    value.copy<SyntaxArrayExpList *>(other.value);
    break;

  case 70: // AssignStmt
    value.copy<SyntaxAssignStmt *>(other.value);
    break;

  case 64: // Block
    value.copy<SyntaxBlock *>(other.value);
    break;

  case 66: // BlockItem
    value.copy<SyntaxBlockItem *>(other.value);
    break;

  case 65: // BlockItemList
    value.copy<SyntaxBlockItemList *>(other.value);
    break;

  case 68: // BreakStmt
    value.copy<SyntaxBreakStmt *>(other.value);
    break;

  case 81: // Callee
    value.copy<SyntaxCallee *>(other.value);
    break;

  case 46: // CompUnit
    value.copy<SyntaxCompUnit *>(other.value);
    break;

  case 75: // Cond
    value.copy<SyntaxCond *>(other.value);
    break;

  case 48: // ConstDecl
    value.copy<SyntaxConstDecl *>(other.value);
    break;

  case 50: // ConstDef
    value.copy<SyntaxConstDef *>(other.value);
    break;

  case 49: // ConstDefList
    value.copy<SyntaxConstDefList *>(other.value);
    break;

  case 90: // ConstExp
    value.copy<SyntaxConstExp *>(other.value);
    break;

  case 52: // ConstInitVal
    value.copy<SyntaxConstInitVal *>(other.value);
    break;

  case 53: // ConstInitValList
    value.copy<SyntaxConstInitValList *>(other.value);
    break;

  case 69: // ContinueStmt
    value.copy<SyntaxContinueStmt *>(other.value);
    break;

  case 47: // DeclDef
    value.copy<SyntaxDeclDef *>(other.value);
    break;

  case 87: // EqExp
    value.copy<SyntaxEqExp *>(other.value);
    break;

  case 74: // Exp
    value.copy<SyntaxExp *>(other.value);
    break;

  case 83: // ExpList
    value.copy<SyntaxExpList *>(other.value);
    break;

  case 59: // FuncDef
    value.copy<SyntaxFuncDef *>(other.value);
    break;

  case 62: // FuncFParam
    value.copy<SyntaxFuncFParam *>(other.value);
    break;

  case 61: // FuncFParamList
    value.copy<SyntaxFuncFParamList *>(other.value);
    break;

  case 57: // InitVal
    value.copy<SyntaxInitVal *>(other.value);
    break;

  case 58: // InitValList
    value.copy<SyntaxInitValList *>(other.value);
    break;

  case 72: // IterationStmt
    value.copy<SyntaxIterationStmt *>(other.value);
    break;

  case 88: // LAndExp
    value.copy<SyntaxLAndExp *>(other.value);
    break;

  case 89: // LOrExp
    value.copy<SyntaxLOrExp *>(other.value);
    break;

  case 76: // LVal
    value.copy<SyntaxLVal *>(other.value);
    break;

  case 84: // MulExp
    value.copy<SyntaxMulExp *>(other.value);
    break;

  case 79: // Number
    value.copy<SyntaxNumber *>(other.value);
    break;

  case 63: // ParamArrayExpList
    value.copy<SyntaxParamArrayExpList *>(other.value);
    break;

  case 78: // PrimaryExp
    value.copy<SyntaxPrimaryExp *>(other.value);
    break;

  case 86: // RelExp
    value.copy<SyntaxRelExp *>(other.value);
    break;

  case 73: // ReturnStmt
    value.copy<SyntaxReturnStmt *>(other.value);
    break;

  case 71: // SelectStmt
    value.copy<SyntaxSelectStmt *>(other.value);
    break;

  case 67: // Stmt
    value.copy<SyntaxStmt *>(other.value);
    break;

  case 80: // UnaryExp
    value.copy<SyntaxUnaryExp *>(other.value);
    break;

  case 54: // VarDecl
    value.copy<SyntaxVarDecl *>(other.value);
    break;

  case 56: // VarDef
    value.copy<SyntaxVarDef *>(other.value);
    break;

  case 55: // VarDefList
    value.copy<SyntaxVarDefList *>(other.value);
    break;

  case 31: // NUMBER
    value.copy<int>(other.value);
    break;

  case 30: // IDENTIFIER
    value.copy<std::string>(other.value);
    break;

  case 60: // DefType
    value.copy<type_specifier>(other.value);
    break;

  case 82: // UnaryOp
    value.copy<unaryop>(other.value);
    break;

  default:
    break;
  }
}

template <typename Base>
inline sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                                     const semantic_type &v,
                                                     const location_type &l)
    : Base(t), value(), location(l) {
  (void)v;
  switch (this->type_get()) {
  case 85: // AddExp
    value.copy<SyntaxAddExp *>(v);
    break;

  case 51: // ArrayConstExpList
    value.copy<SyntaxArrayConstExpList *>(v);
    break;

  case 77: // ArrayExpList
    value.copy<SyntaxArrayExpList *>(v);
    break;

  case 70: // AssignStmt
    value.copy<SyntaxAssignStmt *>(v);
    break;

  case 64: // Block
    value.copy<SyntaxBlock *>(v);
    break;

  case 66: // BlockItem
    value.copy<SyntaxBlockItem *>(v);
    break;

  case 65: // BlockItemList
    value.copy<SyntaxBlockItemList *>(v);
    break;

  case 68: // BreakStmt
    value.copy<SyntaxBreakStmt *>(v);
    break;

  case 81: // Callee
    value.copy<SyntaxCallee *>(v);
    break;

  case 46: // CompUnit
    value.copy<SyntaxCompUnit *>(v);
    break;

  case 75: // Cond
    value.copy<SyntaxCond *>(v);
    break;

  case 48: // ConstDecl
    value.copy<SyntaxConstDecl *>(v);
    break;

  case 50: // ConstDef
    value.copy<SyntaxConstDef *>(v);
    break;

  case 49: // ConstDefList
    value.copy<SyntaxConstDefList *>(v);
    break;

  case 90: // ConstExp
    value.copy<SyntaxConstExp *>(v);
    break;

  case 52: // ConstInitVal
    value.copy<SyntaxConstInitVal *>(v);
    break;

  case 53: // ConstInitValList
    value.copy<SyntaxConstInitValList *>(v);
    break;

  case 69: // ContinueStmt
    value.copy<SyntaxContinueStmt *>(v);
    break;

  case 47: // DeclDef
    value.copy<SyntaxDeclDef *>(v);
    break;

  case 87: // EqExp
    value.copy<SyntaxEqExp *>(v);
    break;

  case 74: // Exp
    value.copy<SyntaxExp *>(v);
    break;

  case 83: // ExpList
    value.copy<SyntaxExpList *>(v);
    break;

  case 59: // FuncDef
    value.copy<SyntaxFuncDef *>(v);
    break;

  case 62: // FuncFParam
    value.copy<SyntaxFuncFParam *>(v);
    break;

  case 61: // FuncFParamList
    value.copy<SyntaxFuncFParamList *>(v);
    break;

  case 57: // InitVal
    value.copy<SyntaxInitVal *>(v);
    break;

  case 58: // InitValList
    value.copy<SyntaxInitValList *>(v);
    break;

  case 72: // IterationStmt
    value.copy<SyntaxIterationStmt *>(v);
    break;

  case 88: // LAndExp
    value.copy<SyntaxLAndExp *>(v);
    break;

  case 89: // LOrExp
    value.copy<SyntaxLOrExp *>(v);
    break;

  case 76: // LVal
    value.copy<SyntaxLVal *>(v);
    break;

  case 84: // MulExp
    value.copy<SyntaxMulExp *>(v);
    break;

  case 79: // Number
    value.copy<SyntaxNumber *>(v);
    break;

  case 63: // ParamArrayExpList
    value.copy<SyntaxParamArrayExpList *>(v);
    break;

  case 78: // PrimaryExp
    value.copy<SyntaxPrimaryExp *>(v);
    break;

  case 86: // RelExp
    value.copy<SyntaxRelExp *>(v);
    break;

  case 73: // ReturnStmt
    value.copy<SyntaxReturnStmt *>(v);
    break;

  case 71: // SelectStmt
    value.copy<SyntaxSelectStmt *>(v);
    break;

  case 67: // Stmt
    value.copy<SyntaxStmt *>(v);
    break;

  case 80: // UnaryExp
    value.copy<SyntaxUnaryExp *>(v);
    break;

  case 54: // VarDecl
    value.copy<SyntaxVarDecl *>(v);
    break;

  case 56: // VarDef
    value.copy<SyntaxVarDef *>(v);
    break;

  case 55: // VarDefList
    value.copy<SyntaxVarDefList *>(v);
    break;

  case 31: // NUMBER
    value.copy<int>(v);
    break;

  case 30: // IDENTIFIER
    value.copy<std::string>(v);
    break;

  case 60: // DefType
    value.copy<type_specifier>(v);
    break;

  case 82: // UnaryOp
    value.copy<unaryop>(v);
    break;

  default:
    break;
  }
}

// Implementation of basic_symbol constructor for each type.

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const location_type &l)
    : Base(t), value(), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxAddExp *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxArrayConstExpList *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxArrayExpList *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxAssignStmt *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxBlock *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxBlockItem *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxBlockItemList *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxBreakStmt *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxCallee *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxCompUnit *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxCond *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxConstDecl *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxConstDef *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxConstDefList *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxConstExp *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxConstInitVal *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxConstInitValList *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxContinueStmt *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxDeclDef *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxEqExp *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxExp *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxExpList *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxFuncDef *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxFuncFParam *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxFuncFParamList *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxInitVal *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxInitValList *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxIterationStmt *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxLAndExp *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxLOrExp *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxLVal *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxMulExp *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxNumber *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxParamArrayExpList *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxPrimaryExp *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxRelExp *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxReturnStmt *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxSelectStmt *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxStmt *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxUnaryExp *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxVarDecl *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxVarDef *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const SyntaxVarDefList *v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const int v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const std::string v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const type_specifier v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
sysy_parser::basic_symbol<Base>::basic_symbol(typename Base::kind_type t,
                                              const unaryop v,
                                              const location_type &l)
    : Base(t), value(v), location(l) {}

template <typename Base>
inline sysy_parser::basic_symbol<Base>::~basic_symbol() {
  clear();
}

template <typename Base> inline void sysy_parser::basic_symbol<Base>::clear() {
  // User destructor.
  symbol_number_type yytype = this->type_get();
  basic_symbol<Base> &yysym = *this;
  (void)yysym;
  switch (yytype) {
  default:
    break;
  }

  // Type destructor.
  switch (yytype) {
  case 85: // AddExp
    value.template destroy<SyntaxAddExp *>();
    break;

  case 51: // ArrayConstExpList
    value.template destroy<SyntaxArrayConstExpList *>();
    break;

  case 77: // ArrayExpList
    value.template destroy<SyntaxArrayExpList *>();
    break;

  case 70: // AssignStmt
    value.template destroy<SyntaxAssignStmt *>();
    break;

  case 64: // Block
    value.template destroy<SyntaxBlock *>();
    break;

  case 66: // BlockItem
    value.template destroy<SyntaxBlockItem *>();
    break;

  case 65: // BlockItemList
    value.template destroy<SyntaxBlockItemList *>();
    break;

  case 68: // BreakStmt
    value.template destroy<SyntaxBreakStmt *>();
    break;

  case 81: // Callee
    value.template destroy<SyntaxCallee *>();
    break;

  case 46: // CompUnit
    value.template destroy<SyntaxCompUnit *>();
    break;

  case 75: // Cond
    value.template destroy<SyntaxCond *>();
    break;

  case 48: // ConstDecl
    value.template destroy<SyntaxConstDecl *>();
    break;

  case 50: // ConstDef
    value.template destroy<SyntaxConstDef *>();
    break;

  case 49: // ConstDefList
    value.template destroy<SyntaxConstDefList *>();
    break;

  case 90: // ConstExp
    value.template destroy<SyntaxConstExp *>();
    break;

  case 52: // ConstInitVal
    value.template destroy<SyntaxConstInitVal *>();
    break;

  case 53: // ConstInitValList
    value.template destroy<SyntaxConstInitValList *>();
    break;

  case 69: // ContinueStmt
    value.template destroy<SyntaxContinueStmt *>();
    break;

  case 47: // DeclDef
    value.template destroy<SyntaxDeclDef *>();
    break;

  case 87: // EqExp
    value.template destroy<SyntaxEqExp *>();
    break;

  case 74: // Exp
    value.template destroy<SyntaxExp *>();
    break;

  case 83: // ExpList
    value.template destroy<SyntaxExpList *>();
    break;

  case 59: // FuncDef
    value.template destroy<SyntaxFuncDef *>();
    break;

  case 62: // FuncFParam
    value.template destroy<SyntaxFuncFParam *>();
    break;

  case 61: // FuncFParamList
    value.template destroy<SyntaxFuncFParamList *>();
    break;

  case 57: // InitVal
    value.template destroy<SyntaxInitVal *>();
    break;

  case 58: // InitValList
    value.template destroy<SyntaxInitValList *>();
    break;

  case 72: // IterationStmt
    value.template destroy<SyntaxIterationStmt *>();
    break;

  case 88: // LAndExp
    value.template destroy<SyntaxLAndExp *>();
    break;

  case 89: // LOrExp
    value.template destroy<SyntaxLOrExp *>();
    break;

  case 76: // LVal
    value.template destroy<SyntaxLVal *>();
    break;

  case 84: // MulExp
    value.template destroy<SyntaxMulExp *>();
    break;

  case 79: // Number
    value.template destroy<SyntaxNumber *>();
    break;

  case 63: // ParamArrayExpList
    value.template destroy<SyntaxParamArrayExpList *>();
    break;

  case 78: // PrimaryExp
    value.template destroy<SyntaxPrimaryExp *>();
    break;

  case 86: // RelExp
    value.template destroy<SyntaxRelExp *>();
    break;

  case 73: // ReturnStmt
    value.template destroy<SyntaxReturnStmt *>();
    break;

  case 71: // SelectStmt
    value.template destroy<SyntaxSelectStmt *>();
    break;

  case 67: // Stmt
    value.template destroy<SyntaxStmt *>();
    break;

  case 80: // UnaryExp
    value.template destroy<SyntaxUnaryExp *>();
    break;

  case 54: // VarDecl
    value.template destroy<SyntaxVarDecl *>();
    break;

  case 56: // VarDef
    value.template destroy<SyntaxVarDef *>();
    break;

  case 55: // VarDefList
    value.template destroy<SyntaxVarDefList *>();
    break;

  case 31: // NUMBER
    value.template destroy<int>();
    break;

  case 30: // IDENTIFIER
    value.template destroy<std::string>();
    break;

  case 60: // DefType
    value.template destroy<type_specifier>();
    break;

  case 82: // UnaryOp
    value.template destroy<unaryop>();
    break;

  default:
    break;
  }

  Base::clear();
}

template <typename Base>
inline bool sysy_parser::basic_symbol<Base>::empty() const {
  return Base::type_get() == empty_symbol;
}

template <typename Base>
inline void sysy_parser::basic_symbol<Base>::move(basic_symbol &s) {
  super_type::move(s);
  switch (this->type_get()) {
  case 85: // AddExp
    value.move<SyntaxAddExp *>(s.value);
    break;

  case 51: // ArrayConstExpList
    value.move<SyntaxArrayConstExpList *>(s.value);
    break;

  case 77: // ArrayExpList
    value.move<SyntaxArrayExpList *>(s.value);
    break;

  case 70: // AssignStmt
    value.move<SyntaxAssignStmt *>(s.value);
    break;

  case 64: // Block
    value.move<SyntaxBlock *>(s.value);
    break;

  case 66: // BlockItem
    value.move<SyntaxBlockItem *>(s.value);
    break;

  case 65: // BlockItemList
    value.move<SyntaxBlockItemList *>(s.value);
    break;

  case 68: // BreakStmt
    value.move<SyntaxBreakStmt *>(s.value);
    break;

  case 81: // Callee
    value.move<SyntaxCallee *>(s.value);
    break;

  case 46: // CompUnit
    value.move<SyntaxCompUnit *>(s.value);
    break;

  case 75: // Cond
    value.move<SyntaxCond *>(s.value);
    break;

  case 48: // ConstDecl
    value.move<SyntaxConstDecl *>(s.value);
    break;

  case 50: // ConstDef
    value.move<SyntaxConstDef *>(s.value);
    break;

  case 49: // ConstDefList
    value.move<SyntaxConstDefList *>(s.value);
    break;

  case 90: // ConstExp
    value.move<SyntaxConstExp *>(s.value);
    break;

  case 52: // ConstInitVal
    value.move<SyntaxConstInitVal *>(s.value);
    break;

  case 53: // ConstInitValList
    value.move<SyntaxConstInitValList *>(s.value);
    break;

  case 69: // ContinueStmt
    value.move<SyntaxContinueStmt *>(s.value);
    break;

  case 47: // DeclDef
    value.move<SyntaxDeclDef *>(s.value);
    break;

  case 87: // EqExp
    value.move<SyntaxEqExp *>(s.value);
    break;

  case 74: // Exp
    value.move<SyntaxExp *>(s.value);
    break;

  case 83: // ExpList
    value.move<SyntaxExpList *>(s.value);
    break;

  case 59: // FuncDef
    value.move<SyntaxFuncDef *>(s.value);
    break;

  case 62: // FuncFParam
    value.move<SyntaxFuncFParam *>(s.value);
    break;

  case 61: // FuncFParamList
    value.move<SyntaxFuncFParamList *>(s.value);
    break;

  case 57: // InitVal
    value.move<SyntaxInitVal *>(s.value);
    break;

  case 58: // InitValList
    value.move<SyntaxInitValList *>(s.value);
    break;

  case 72: // IterationStmt
    value.move<SyntaxIterationStmt *>(s.value);
    break;

  case 88: // LAndExp
    value.move<SyntaxLAndExp *>(s.value);
    break;

  case 89: // LOrExp
    value.move<SyntaxLOrExp *>(s.value);
    break;

  case 76: // LVal
    value.move<SyntaxLVal *>(s.value);
    break;

  case 84: // MulExp
    value.move<SyntaxMulExp *>(s.value);
    break;

  case 79: // Number
    value.move<SyntaxNumber *>(s.value);
    break;

  case 63: // ParamArrayExpList
    value.move<SyntaxParamArrayExpList *>(s.value);
    break;

  case 78: // PrimaryExp
    value.move<SyntaxPrimaryExp *>(s.value);
    break;

  case 86: // RelExp
    value.move<SyntaxRelExp *>(s.value);
    break;

  case 73: // ReturnStmt
    value.move<SyntaxReturnStmt *>(s.value);
    break;

  case 71: // SelectStmt
    value.move<SyntaxSelectStmt *>(s.value);
    break;

  case 67: // Stmt
    value.move<SyntaxStmt *>(s.value);
    break;

  case 80: // UnaryExp
    value.move<SyntaxUnaryExp *>(s.value);
    break;

  case 54: // VarDecl
    value.move<SyntaxVarDecl *>(s.value);
    break;

  case 56: // VarDef
    value.move<SyntaxVarDef *>(s.value);
    break;

  case 55: // VarDefList
    value.move<SyntaxVarDefList *>(s.value);
    break;

  case 31: // NUMBER
    value.move<int>(s.value);
    break;

  case 30: // IDENTIFIER
    value.move<std::string>(s.value);
    break;

  case 60: // DefType
    value.move<type_specifier>(s.value);
    break;

  case 82: // UnaryOp
    value.move<unaryop>(s.value);
    break;

  default:
    break;
  }

  location = s.location;
}

// by_type.
inline sysy_parser::by_type::by_type() : type(empty_symbol) {}

inline sysy_parser::by_type::by_type(const by_type &other) : type(other.type) {}

inline sysy_parser::by_type::by_type(token_type t) : type(yytranslate_(t)) {}

inline void sysy_parser::by_type::clear() { type = empty_symbol; }

inline void sysy_parser::by_type::move(by_type &that) {
  type = that.type;
  that.clear();
}

inline int sysy_parser::by_type::type_get() const { return type; }

inline sysy_parser::token_type sysy_parser::by_type::token() const {
  // YYTOKNUM[NUM] -- (External) token number corresponding to the
  // (internal) symbol number NUM (which must be that of a token).  */
  static const unsigned short int yytoken_number_[] = {
      0,   256, 298, 299, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268,
      269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283,
      284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297};
  return static_cast<token_type>(yytoken_number_[type]);
}
// Implementation of make_symbol for each symbol type.
sysy_parser::symbol_type sysy_parser::make_END(const location_type &l) {
  return symbol_type(token::TOK_END, l);
}

sysy_parser::symbol_type sysy_parser::make_ERROR(const location_type &l) {
  return symbol_type(token::TOK_ERROR, l);
}

sysy_parser::symbol_type sysy_parser::make_ADD(const location_type &l) {
  return symbol_type(token::TOK_ADD, l);
}

sysy_parser::symbol_type sysy_parser::make_SUB(const location_type &l) {
  return symbol_type(token::TOK_SUB, l);
}

sysy_parser::symbol_type sysy_parser::make_MUL(const location_type &l) {
  return symbol_type(token::TOK_MUL, l);
}

sysy_parser::symbol_type sysy_parser::make_DIV(const location_type &l) {
  return symbol_type(token::TOK_DIV, l);
}

sysy_parser::symbol_type sysy_parser::make_LT(const location_type &l) {
  return symbol_type(token::TOK_LT, l);
}

sysy_parser::symbol_type sysy_parser::make_LTE(const location_type &l) {
  return symbol_type(token::TOK_LTE, l);
}

sysy_parser::symbol_type sysy_parser::make_GT(const location_type &l) {
  return symbol_type(token::TOK_GT, l);
}

sysy_parser::symbol_type sysy_parser::make_GTE(const location_type &l) {
  return symbol_type(token::TOK_GTE, l);
}

sysy_parser::symbol_type sysy_parser::make_EQ(const location_type &l) {
  return symbol_type(token::TOK_EQ, l);
}

sysy_parser::symbol_type sysy_parser::make_NEQ(const location_type &l) {
  return symbol_type(token::TOK_NEQ, l);
}

sysy_parser::symbol_type sysy_parser::make_ASSIN(const location_type &l) {
  return symbol_type(token::TOK_ASSIN, l);
}

sysy_parser::symbol_type sysy_parser::make_SEMICOLON(const location_type &l) {
  return symbol_type(token::TOK_SEMICOLON, l);
}

sysy_parser::symbol_type sysy_parser::make_COMMA(const location_type &l) {
  return symbol_type(token::TOK_COMMA, l);
}

sysy_parser::symbol_type sysy_parser::make_LPARENTHESE(const location_type &l) {
  return symbol_type(token::TOK_LPARENTHESE, l);
}

sysy_parser::symbol_type sysy_parser::make_RPARENTHESE(const location_type &l) {
  return symbol_type(token::TOK_RPARENTHESE, l);
}

sysy_parser::symbol_type sysy_parser::make_LBRACKET(const location_type &l) {
  return symbol_type(token::TOK_LBRACKET, l);
}

sysy_parser::symbol_type sysy_parser::make_RBRACKET(const location_type &l) {
  return symbol_type(token::TOK_RBRACKET, l);
}

sysy_parser::symbol_type sysy_parser::make_LBRACE(const location_type &l) {
  return symbol_type(token::TOK_LBRACE, l);
}

sysy_parser::symbol_type sysy_parser::make_RBRACE(const location_type &l) {
  return symbol_type(token::TOK_RBRACE, l);
}

sysy_parser::symbol_type sysy_parser::make_ELSE(const location_type &l) {
  return symbol_type(token::TOK_ELSE, l);
}

sysy_parser::symbol_type sysy_parser::make_IF(const location_type &l) {
  return symbol_type(token::TOK_IF, l);
}

sysy_parser::symbol_type sysy_parser::make_INT(const location_type &l) {
  return symbol_type(token::TOK_INT, l);
}

sysy_parser::symbol_type sysy_parser::make_RETURN(const location_type &l) {
  return symbol_type(token::TOK_RETURN, l);
}

sysy_parser::symbol_type sysy_parser::make_VOID(const location_type &l) {
  return symbol_type(token::TOK_VOID, l);
}

sysy_parser::symbol_type sysy_parser::make_WHILE(const location_type &l) {
  return symbol_type(token::TOK_WHILE, l);
}

sysy_parser::symbol_type sysy_parser::make_IDENTIFIER(const std::string &v,
                                                      const location_type &l) {
  return symbol_type(token::TOK_IDENTIFIER, v, l);
}

sysy_parser::symbol_type sysy_parser::make_NUMBER(const int &v,
                                                  const location_type &l) {
  return symbol_type(token::TOK_NUMBER, v, l);
}

sysy_parser::symbol_type sysy_parser::make_ARRAY(const location_type &l) {
  return symbol_type(token::TOK_ARRAY, l);
}

sysy_parser::symbol_type sysy_parser::make_LETTER(const location_type &l) {
  return symbol_type(token::TOK_LETTER, l);
}

sysy_parser::symbol_type sysy_parser::make_EOL(const location_type &l) {
  return symbol_type(token::TOK_EOL, l);
}

sysy_parser::symbol_type sysy_parser::make_COMMENT(const location_type &l) {
  return symbol_type(token::TOK_COMMENT, l);
}

sysy_parser::symbol_type sysy_parser::make_BLANK(const location_type &l) {
  return symbol_type(token::TOK_BLANK, l);
}

sysy_parser::symbol_type sysy_parser::make_CONST(const location_type &l) {
  return symbol_type(token::TOK_CONST, l);
}

sysy_parser::symbol_type sysy_parser::make_BREAK(const location_type &l) {
  return symbol_type(token::TOK_BREAK, l);
}

sysy_parser::symbol_type sysy_parser::make_CONTINUE(const location_type &l) {
  return symbol_type(token::TOK_CONTINUE, l);
}

sysy_parser::symbol_type sysy_parser::make_NOT(const location_type &l) {
  return symbol_type(token::TOK_NOT, l);
}

sysy_parser::symbol_type sysy_parser::make_AND(const location_type &l) {
  return symbol_type(token::TOK_AND, l);
}

sysy_parser::symbol_type sysy_parser::make_OR(const location_type &l) {
  return symbol_type(token::TOK_OR, l);
}

sysy_parser::symbol_type sysy_parser::make_MOD(const location_type &l) {
  return symbol_type(token::TOK_MOD, l);
}

} // namespace yy
#line 2625 "sysy_parser.hh" // lalr1.cc:377

#endif // !YY_YY_SYSY_PARSER_HH_INCLUDED
