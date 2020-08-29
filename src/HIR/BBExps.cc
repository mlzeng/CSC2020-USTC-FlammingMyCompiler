#include "BBExps.h"

using std::get;
using std::make_pair;
using std::make_tuple;
using std::pair;
using std::tuple;

void BBExpsAnalysis_hir::run() {
  BB2exp.clear();
  for (auto func : m_->getFunctions()) {
    if (func->getBaseBlocks().size() == 0)
      continue;
    for (auto BB : func->getBaseBlocks()) {
      analysisExps(BB);
      if (failed)
        return;
    }
  }
}

void BBExpsAnalysis_hir::analysisExps(BaseBlock *BB) {
  if (BB->isWhileBlock()) {
    auto whileBB = dynamic_cast<WhileBlock *>(BB);
    for (auto condBB : whileBB->getCondBBs())
      analysisExps(condBB);
    for (auto bodyBB : whileBB->getBodyBBs())
      analysisExps(bodyBB);
  } else if (BB->isIfBlock()) {
    auto ifBB = dynamic_cast<IfBlock *>(BB);
    for (auto condBB : ifBB->getCondBBs())
      analysisExps(condBB);
    for (auto thenBB : ifBB->getIfBodyBBs())
      analysisExps(thenBB);
    for (auto elseBB : ifBB->getElseBodyBBs())
      analysisExps(elseBB);
  } else {
    auto BasicBB = dynamic_cast<BasicBlock *>(BB);
    BB2exp.insert({BB, BBExps_hir()});
    for (auto instr : BasicBB->getInstructions()) {
      if (instr->isCmp()) { // TODO 随cmp的实现改动
        handleLogicalExp(BasicBB, instr);
      } else if (instr->isStore())
        handleAssignment(BasicBB, instr);
      else if (instr->isCall() && instr->isVoid())
        handleCall(BasicBB, instr);
      else if (instr->isRet())
        handleRet(BasicBB, instr);
      else if (instr->isBreak() || instr->isContinue())
        BB2exp[BasicBB].exps.push_back(
            {nullptr, instr, Exp_hir(nullptr, Nop_level)});

      if (failed)
        return;
    }
    // // DEBUG
    // std::cout << "exps in " + BB->getName() + "\n";
    // for (auto exp : BB2exp[BB].exps) {
    //     auto ins = get<1>(exp);
    //     if (ins->isBreak() || ins->isContinue()) {std::cout <<
    //     _ins2string(ins) << "\n";continue;};

    //     if(get<0>(exp) != nullptr) std::cout << "%" + get<0>(exp)->getName();
    //     std::cout << "\t" + _ins2string(ins);
    //     std::cout << "\t" + _printExp(get<2>(exp));
    //     putchar('\n');
    // }
    // // ENDDEBUG
  }
}

void BBExpsAnalysis_hir::handleLogicalExp(BasicBlock *BB, Value *boolean) {
  auto instr = dynamic_cast<Instruction *>(boolean);
  auto &BBExps = BB2exp[BB];
  // assert(boolean && "Constant boolean for br");
  BBExps.exps.push_back({nullptr, instr, getExp(BB->getInstructions().back())});
}

void BBExpsAnalysis_hir::handleAssignment(BasicBlock *BB, Value *val) {
  auto &BBExps = BB2exp[BB];
  auto storeInstr = dynamic_cast<Instruction *>(val);
  BBExps.exps.push_back({storeInstr->getOperand(1), storeInstr,
                         getExp(storeInstr->getOperand(0))});
}

void BBExpsAnalysis_hir::handleCall(BasicBlock *BB, Value *val) {
  auto &BBExps = BB2exp[BB];
  auto callInstr = dynamic_cast<Instruction *>(val);
  if (!(callInstr && "Require Call Instruction")) {
    failed = true;
    return;
  }
  Exp_hir callExp(nullptr, call_level);
  callExp._exp.push_back(
      {nop_hir, Exp_hir(callInstr->getOperand(0), Nop_level)});
  for (unsigned i = 1; i < callInstr->getNumOperand(); i++) {
    callExp._exp.push_back({call_hir, getExp(callInstr->getOperand(1))});
  }
  BBExps.exps.push_back({nullptr, callInstr, std::move(callExp)});
}

void BBExpsAnalysis_hir::handleRet(BasicBlock *BB, Value *val) {
  auto instr = dynamic_cast<Instruction *>(val);
  auto &BBExps = BB2exp[BB];
  // assert(boolean && "Constant boolean for br");
  if (instr->getNumOperand())
    BBExps.exps.push_back({nullptr, instr, getExp(instr->getOperand(0))});
  else
    BBExps.exps.push_back({nullptr, instr, Exp_hir(nullptr, Nop_level)});
}

Exp_hir BBExpsAnalysis_hir::getExp(Value *val) {
  if (!(val && "Can not getExp from nullptr")) {
    failed = true;
    return Exp_hir(nullptr, Nop_level);
  }
  auto instr = dynamic_cast<Instruction *>(val);
  if (instr == nullptr) { // constantInt
    return Exp_hir(val, Nop_level);
  } else if (instr->isLoad()) {
    return Exp_hir(instr->getOperand(0), Nop_level);
  }
  if (instr->isOr()) {
    std::vector<Value *> Exp;
    auto resExp = Exp_hir(nullptr, lor_level);
    Exp.push_back(instr->getOperand(1));
    Exp.push_back(instr->getOperand(0));
    while (!Exp.empty()) {
      auto item = dynamic_cast<Instruction *>(Exp.back());
      Exp.pop_back();
      if (!(item && "Constant Boolean")) {
        failed = true;
        return resExp;
      }
      if (item->isOr()) {
        Exp.push_back(item->getOperand(1));
        Exp.push_back(item->getOperand(0));
      }
      resExp._exp.push_back(std::make_pair(nop_hir, getExp(item)));
    }
    return resExp;
  } else if (instr->isAnd()) {
    std::vector<Value *> Exp;
    auto resExp = Exp_hir(nullptr, land_level);
    Exp.push_back(instr->getOperand(1));
    Exp.push_back(instr->getOperand(0));
    while (!Exp.empty()) {
      auto item = dynamic_cast<Instruction *>(Exp.back());
      Exp.pop_back();
      if (!(item && "Constant Boolean")) {
        failed = true;
        return resExp;
      }
      if (item->isAnd()) {
        Exp.push_back(item->getOperand(1));
        Exp.push_back(item->getOperand(0));
        continue;
      }
      resExp._exp.push_back(std::make_pair(nop_hir, getExp(item)));
    }
    return resExp;
  } else if (instr->isCmp()) {
    auto cmp = static_cast<CmpInst *>(instr);
    auto resExp = Exp_hir(nullptr, cmp_level);
    resExp._exp.push_back(std::make_pair(nop_hir, getExp(cmp->getOperand(0))));
    resExp._exp.push_back(std::make_pair(cmpOp2ExpOp(cmp->getCmpOp()),
                                         getExp(cmp->getOperand(1))));
    return resExp;
  } else if (instr->isAdd() || instr->isSub()) {
    std::vector<std::pair<OP_hir, Value *>> Exp;
    auto resExp = Exp_hir(nullptr, plus_level);
    Exp.push_back(make_pair(instrOp2ExpOp(instr), instr->getOperand(1)));
    Exp.push_back(make_pair(nop_hir, instr->getOperand(0)));
    while (!Exp.empty()) {
      auto val = Exp.back().second;
      auto item = dynamic_cast<Instruction *>(val);
      auto op_hir = Exp.back().first;
      Exp.pop_back();
      // if (!(item && "Constant Boolean");
      if (item == nullptr) {
        resExp._exp.push_back(make_pair(op_hir, Exp_hir(val, Nop_level)));
      } else if (item->isLoad()) {
        resExp._exp.push_back(
            make_pair(op_hir, Exp_hir(item->getOperand(0), Nop_level)));
      } else if (item->isCall()) {
        resExp._exp.push_back(std::make_pair(op_hir, getExp(item)));
      } else if (item->isAdd() || item->isSub()) {
        Exp.push_back(
            make_pair(determineOP(op_hir, instrOp2ExpOp(item), plus_level),
                      item->getOperand(1)));
        Exp.push_back(make_pair(determineOP(op_hir, nop_hir, plus_level),
                                item->getOperand(0)));
      } else {
        resExp._exp.push_back(std::make_pair(op_hir, getExp(item)));
      }
    }
    return resExp;
  } else if (instr->isMul() || instr->isDiv() || instr->isRem()) {
    std::vector<std::pair<OP_hir, Value *>> Exp;
    auto resExp = Exp_hir(nullptr, multiple_level);
    Exp.push_back(make_pair(instrOp2ExpOp(instr), instr->getOperand(1)));
    Exp.push_back(make_pair(nop_hir, instr->getOperand(0)));
    while (!Exp.empty()) {
      auto val = Exp.back().second;
      auto item = dynamic_cast<Instruction *>(val);
      auto op_hir = Exp.back().first;
      Exp.pop_back();
      // assert(item && "Constant Boolean");
      if (item == nullptr) {
        resExp._exp.push_back(make_pair(op_hir, Exp_hir(val, Nop_level)));
      } else if (item->isLoad()) {
        resExp._exp.push_back(
            make_pair(op_hir, Exp_hir(item->getOperand(0), Nop_level)));
      } else if (item->isCall()) {
        resExp._exp.push_back(std::make_pair(op_hir, getExp(item)));
      } else if (shouldNotBeLayered(op_hir, instrOp2ExpOp(item),
                                    multiple_level)) {
        Exp.push_back(
            make_pair(determineOP(op_hir, instrOp2ExpOp(item), multiple_level),
                      item->getOperand(1)));
        Exp.push_back(make_pair(determineOP(op_hir, nop_hir, multiple_level),
                                item->getOperand(0)));
      } else {
        resExp._exp.push_back(std::make_pair(op_hir, getExp(item)));
      }
    }
    return resExp;
  } else if (instr->isCall()) {
    auto resExp = Exp_hir(nullptr, call_level);
    resExp._exp.push_back(
        make_pair(nop_hir, Exp_hir(instr->getOperand(0), Nop_level)));
    for (unsigned i = 1; i < instr->getNumOperand(); i++) {
      auto tmp = dynamic_cast<Instruction *>(instr->getOperand(i));
      if (tmp == nullptr || tmp->isGEP())
        resExp._exp.push_back(
            {call_hir, Exp_hir(instr->getOperand(i), Nop_level)});
      else
        resExp._exp.push_back(
            std::make_pair(call_hir, getExp(instr->getOperand(i))));
    }
    return resExp;
  } else if (instr->isZExt()) {
    return getExp(instr->getOperand(0));
  } else {
    if (!(false && "Unrecognized Exp")) {
      failed = true;
      return Exp_hir(nullptr, Nop_level);
    }
  }
}

OP_hir BBExpsAnalysis_hir::instrOp2ExpOp(Instruction *instr) {
  // using Instruction::OpID;
  switch (instr->getInstrType()) {
  case Instruction::OpID::Add:
    return plus_hir;
  case Instruction::OpID::Sub:
    return minus_hir;
  case Instruction::OpID::Mul:
    return multiple_hir;
  case Instruction::OpID::Div:
    return divide_hir;
  case Instruction::OpID::Rem:
    return rem_hir;
  default:
    if (!(false && "Not support convernsion")) {
      failed = true;
      return plus_hir;
    }
  }
}

OP_hir BBExpsAnalysis_hir::determineOP(OP_hir UpperOp, OP_hir curOp,
                                       OPlevel level) {
  if (opMap[UpperOp][curOp] == -1)
    if (!(false && "not supported conversion in determinOP")) {
      failed = true;
      return nop_hir;
    }
  return static_cast<OP_hir>(opMap[UpperOp][curOp]);
}

bool BBExpsAnalysis_hir::shouldNotBeLayered(OP_hir UpperOp, OP_hir curOp,
                                            OPlevel level) {
  if (!(level == multiple_level && "Only be used in multiple level for now")) {
    failed = true;
    return false;
  }
  // return false;
  if (UpperOp == nop_hir &&
      (curOp == multiple_hir || curOp == divide_hir || curOp == rem_hir))
    return true;
  else if (UpperOp == multiple_hir && curOp == multiple_hir)
    return true;
  else
    return false;
}

void BBExpsAnalysis_hir::_initOpMap() {
  for (int i = 0; i < 20; i++)
    for (int j = 0; j < 20; j++)
      opMap[i][j] = -1;

  for (int i = 0; i < 20; i++)
    opMap[nop_hir][i] = i;
  for (int i = 0; i < 20; i++)
    opMap[i][nop_hir] = i;
  opMap[plus_hir][minus_hir] = minus_hir;
  opMap[plus_hir][plus_hir] = plus_hir;
  opMap[minus_hir][minus_hir] = plus_hir;
  opMap[minus_hir][plus_hir] = minus_hir;
}

const std::vector<std::string> op2str{
    "", "==", "!=", ">", ">=", "<", "<=", "+", "-", "*", "/", " ", "%",
};

const std::vector<std::string> level2str{"", " || ", " && ", "",
                                         "", "",     "",     ""};

std::string BBExpsAnalysis_hir::_printExp(const Exp_hir &exp) {
  std::string res;
  if (exp.level == Nop_level) {
    if (exp.lval == nullptr)
      return "";
    else if (dynamic_cast<ConstantInt *>(exp.lval) == nullptr)
      return "%" + exp.lval->getName();
    else
      return std::to_string(dynamic_cast<ConstantInt *>(exp.lval)->getValue());
  } else {
    res = _printExp(exp._exp[0].second);
    for (unsigned i = 1; i < exp._exp.size(); i++) {
      res += level2str[exp.level] + op2str[exp._exp[i].first] +
             _printExp(exp._exp[i].second);
    }
    return "(" + std::move(res) + ")";
  }
}

std::vector<std::string> insOp2str{
    // // High IR
    "Break",    // Break,
    "Continue", // Continue,
                // // Terminator Instructions
    "Ret",      // Ret,
    "",         // Br,
                // // Standard unary operators
    "",         // Neg,
                // // Standard binary operators
    "",         // Add,
    "",         // Sub,
    "",         // RSub,   // Reverse Subtract
    "",         // Mul,
    "",         // Div,
    "",         // Rem,
    "",         // AddAddr,    // deprecated
                // // Logical operators
    "",         // And,
    "",         // Or,
    "",         // Not,
                // // Memory operators
    "",         // Alloca,
    "",         // Load,
    "=",        // Store,
                // // Shift operators
    "",         // Shl,    // <<
    "",         // AShr,   // arithmetic >>
    "",         // LShr,   // logical >>
                // // Other operators
    "Cmp",      // Cmp,
    "",         // PHI,
    "Call",     // Call,
    "",         // GEP,    // GetElementPtr
    "",         // ZExt,   // zero extend
                // MulAdd, // a*b+c
                // MTSTART,// %thread_id = MTSTART
                // MTEND,  // MTEND %thread_id
                // // NEON SIMD
                // VV,     // sum(vector .* vector)
};

std::string BBExpsAnalysis_hir::_ins2string(const Instruction *ins) {
  return insOp2str[ins->getInstrType()];
}
