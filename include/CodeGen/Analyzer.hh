#ifndef SYSYC_ANALYZER_H
#define SYSYC_ANALYZER_H

#include <map>
#include <random>
#include <set>
#include <tuple>
#include <vector>

#include "CodeGen.hh"

namespace Analyzer {

extern std::random_device rd;

enum Functionality { set, gnp, mm, mv, unknown };

const int sample_count = 16;

const bool show_num_steps = false;

class Var {
private:
  int val;

public:
  Var() { this->val = (rd() % 2) ? (rd() % 16) : (rd()); }
  Var(int val) { this->val = val; }
  Var(const Var &rhs) { this->val = rhs.val; }
  Var operator+(const Var &rhs) const { return this->val + rhs.val; }
  Var operator-(const Var &rhs) const { return this->val - rhs.val; }
  Var operator*(const Var &rhs) const { return this->val * rhs.val; }
  Var operator/(const Var &rhs) const { return this->val / rhs.val; }
  Var operator%(const Var &rhs) const { return this->val % rhs.val; }
  Var operator^(const Var &rhs) const { return this->val ^ rhs.val; }
  Var operator|(const Var &rhs) const { return this->val | rhs.val; }
  Var operator&(const Var &rhs) const { return this->val & rhs.val; }
  Var operator<<(const Var &rhs) const { return this->val << rhs.val; }
  Var operator>>(const Var &rhs) const { return this->val >> rhs.val; }
  Var operator<(const Var &rhs) const { return this->val < rhs.val; }
  Var operator>(const Var &rhs) const { return this->val > rhs.val; }
  Var operator<=(const Var &rhs) const { return this->val <= rhs.val; }
  Var operator>=(const Var &rhs) const { return this->val >= rhs.val; }
  Var operator==(const Var &rhs) const { return this->val == rhs.val; }
  Var operator!=(const Var &rhs) const { return this->val != rhs.val; }
  Var operator-() const { return -this->val; }
  Var operator!() const { return !this->val; }
  Var operator~() const { return ~this->val; }
  int getVal() const { return this->val; }
};

class Analyzer {
private:
  Function *func;
  std::map<Instruction *, Instruction *> next_inst;
  Instruction *start_inst;
  int free_addr;
  std::map<int, Var> vmem;
  std::map<Value *, Var> vreg;
  int ret_gas;

public:
  Analyzer(Function *func);
  void clear();
  std::pair<bool, int> run(int gas);
  Functionality analyzeFunctionality();
  bool analyzeMTsafety();
};
}; // namespace Analyzer

#endif