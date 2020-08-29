#pragma once

#ifndef YY_DECL
#define YY_DECL                                                                \
  yy::sysy_parser::symbol_type sysyFlexLexer::yylex(sysy_driver &driver)
#endif

// We need this for yyFlexLexer. If we don't #undef yyFlexLexer, the
// preprocessor chokes on the line `#define yyFlexLexer yyFlexLexer`
// in `FlexLexer.h`:
#undef yyFlexLexer
#include <FlexLexer.h>

// We need this for the yy::calcxx_parser::symbol_type:
#include "sysy_parser.hh"

// We need this for the yy::location type:
#include "location.hh"

class sysyFlexLexer : public yyFlexLexer {
public:
  // Use the superclass's constructor:
  using yyFlexLexer::yyFlexLexer;

  // Provide the interface to `yylex`; `flex` will emit the
  // definition into `calc++-scanner.cc`:
  yy::sysy_parser::symbol_type yylex(sysy_driver &driver);

  // This seems like a reasonable place to put the location object
  // rather than it being static (in the sense of having internal
  // linkage at translation unit scope, not in the sense of being a
  // class variable):
  yy::location loc;
};
