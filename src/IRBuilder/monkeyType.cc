#include "syntax_tree.hh"
#include "sysy_driver.hh"

SyntaxCompUnit *MonkeyType(const std::string filename) {
  sysy_driver driver;
  return driver.parse(filename);
}
