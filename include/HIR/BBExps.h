#ifndef __BBEXPS_H__
#define __BBEXPS_H__

#include "PassManager.h"

enum OP_hir {
  nop_hir,
  eq_hir, // ==
  ne_hir, // !=
  gt_hir, // >
  ge_hir, // >=
  lt_hir, // <
  le_hir, // <=
  plus_hir,
  minus_hir,
  multiple_hir,
  divide_hir,
  call_hir,
  rem_hir
};

enum OPlevel {
  Nop_level,
  lor_level,
  land_level,
  cmp_level,
  plus_level,
  multiple_level,
  call_level,
};

struct Exp_hir {
  Value *lval;
  OPlevel level;
  std::vector<std::pair<OP_hir, Exp_hir>> _exp;
  Exp_hir(Value *val, OPlevel level) : lval(val), level(level) {}
};
struct BBExps_hir {
  std::vector<std::tuple<Value *, Instruction *, Exp_hir>> exps;
};

class BBExpsAnalysis_hir : public Pass {
  std::unordered_map<BaseBlock *, BBExps_hir> BB2exp;
  bool failed = false;
  int opMap[20][20];
  void analysisExps(BaseBlock *BB);
  void handleLogicalExp(BasicBlock *BB, Value *val);
  void handleAssignment(BasicBlock *BB, Value *val);
  void handleCall(BasicBlock *BB, Value *val);
  void handleRet(BasicBlock *BB, Value *val);
  OP_hir cmpOp2ExpOp(CmpInst::CmpOp op) { return OP_hir(op + 1); }
  OP_hir instrOp2ExpOp(Instruction *instr);
  OP_hir determineOP(OP_hir UpperOp, OP_hir curOp, OPlevel level);
  bool shouldNotBeLayered(OP_hir UpperOp, OP_hir curOp, OPlevel level);
  Exp_hir getExp(Value *val);
  void _initOpMap();
  std::string _printExp(const Exp_hir &exp);
  std::string _ins2string(const Instruction *ins);

public:
  BBExpsAnalysis_hir(Module *m) : Pass(m) { _initOpMap(); }
  auto &getExps(BasicBlock *BB) { return BB2exp[BB]; }
  bool runFailed() { return failed; }
  void run() override;
};

#endif