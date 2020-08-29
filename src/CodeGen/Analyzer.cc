#include "Analyzer.hh"

namespace Analyzer {

const int fixed_size = 1024 * 1024 * 64;

std::random_device rd;

Analyzer::Analyzer(Function *func) {
  this->func = func;
  Instruction *prev_inst = nullptr;
  for (auto &bb : func->getBasicBlocks()) {
    for (auto &inst : bb->getInstructions()) {
      if (prev_inst) {
        next_inst[prev_inst] = inst;
      } else {
        start_inst = inst;
      }
      prev_inst = inst;
    }
  }
}

void Analyzer::clear() {
  this->free_addr = 0;
  this->vmem.clear();
  this->vreg.clear();
  for (auto &gvar : this->func->getParent()->getGlobalVariables()) {
    this->vreg[gvar] = this->free_addr;
    this->free_addr += fixed_size;
  }
  for (auto &bb : func->getBasicBlocks()) {
    for (auto &inst : bb->getInstructions()) {
      if (inst->getInstrType() == Instruction::Alloca) {
        this->vreg[inst] = this->free_addr;
        this->free_addr += fixed_size;
      }
    }
  }
  for (auto &arg : func->getArgs()) {
    if (arg->getType()->isPointerTy()) {
      this->vreg[arg] = this->free_addr;
      this->free_addr += fixed_size;
    }
  }
}

std::pair<bool, int> Analyzer::run(int gas) {
  Instruction *cur_inst = this->start_inst;
  int cnt = 0;
  while (gas-- > 0) {
    cnt++;
    if (cur_inst == nullptr) {
      return std::make_pair(false, 0);
    }
    std::vector<int> values;
    for (auto &op : cur_inst->getOperands()) {
      auto val_const = dynamic_cast<ConstantInt *>(op);
      auto val_global = dynamic_cast<GlobalVariable *>(op);
      if (val_const) {
        values.push_back(val_const->getValue());
      } else if (val_global) {
        values.push_back(this->vreg[val_global].getVal());
      } else if (!dynamic_cast<BasicBlock *>(op) &&
                 !dynamic_cast<Function *>(op)) { // not a BB or FUNC
        values.push_back(this->vreg[op].getVal());
      }
    }
    if (values.size() >= 3 && (cur_inst->getInstrType() == Instruction::Add ||
                               cur_inst->getInstrType() == Instruction::Sub ||
                               cur_inst->getInstrType() == Instruction::RSub ||
                               cur_inst->getInstrType() == Instruction::And ||
                               cur_inst->getInstrType() == Instruction::Or ||
                               cur_inst->getInstrType() == Instruction::Cmp)) {
      values.at(1) <<= values.at(2); // shift
    }
    bool branched = false;
    switch (cur_inst->getInstrType()) {
    case Instruction::Ret:
      if (show_num_steps) {
        std::cerr << "Static analyzer: finished in " << cnt << " steps!"
                  << std::endl;
      }
      this->ret_gas = gas;
      return std::make_pair(true, values.size() > 0 ? values.at(0) : 0);
      break;
    case Instruction::Alloca:
      if (static_cast<AllocaInst *>(cur_inst)->getInit()) {
        int sz = cur_inst->getType()->getSize();
        int addr = this->vreg[cur_inst].getVal();
        for (int i = 0; i < sz; i += 4) {
          this->vmem[addr + i] = 0;
        }
      }
      break;
    case Instruction::Load:
      this->vreg[cur_inst] =
          this->vmem[values.at(0) +
                     ((values.size() > 1 ? values.at(1) : 0)
                      << (values.size() > 2 ? values.at(2) : 0))];
      break;
    case Instruction::Store:
      this->vmem[values.at(1) + ((values.size() > 2 ? values.at(2) : 0)
                                 << (values.size() > 3 ? values.at(3) : 0))] =
          values.at(0);
      break;
    case Instruction::Call: {
      const auto &ops = cur_inst->getOperands();
      const auto &callee_func = static_cast<Function *>(ops.at(0));
      if (callee_func->getName() != func->getName()) {
        std::cerr << "Static analyzer: call is not supported!" << std::endl;
        return std::make_pair(false, 0);
      } else {
        auto reg_saved = this->vreg;
        {
          // set args
          {
            std::vector<std::pair<Value *, int>> v;
            // find all arg
            int arg_cnt = 0;
            for (auto &arg : cur_inst->getFunction()->getArgs()) {
              v.push_back(std::make_pair(arg, values.at(arg_cnt++)));
            }
            // arg assign
            for (auto &p : v) {
              this->vreg[p.first] = p.second;
            }
          }
        }
        const auto &ret = Analyzer::run(gas - 10);
        if (!ret.first) {
          return std::make_pair(false, 0);
        }
        this->vreg = reg_saved;
        this->vreg[cur_inst] = ret.second;
        gas = this->ret_gas;
      }
    } break;
    case Instruction::MulAdd:
      this->vreg[cur_inst] = values.at(0) * values.at(1) + values.at(2);
      break;
    case Instruction::VV: {
      int S = 0;
      int sz = (values.size() <= 2)
                   ? (static_cast<VVInst *>(cur_inst)->getNumInt32() * 4)
                   : (values.at(2));
      for (int i = 0; i < sz; i += 4) {
        S += (this->vmem[values.at(0) + i] * this->vmem[values.at(1) + i])
                 .getVal();
      }
      this->vreg[cur_inst] = S;
      break;
    }
    case Instruction::Add:
      this->vreg[cur_inst] = values.at(0) + values.at(1);
      break;
    case Instruction::Sub:
      this->vreg[cur_inst] = values.at(0) - values.at(1);
      break;
    case Instruction::RSub:
      this->vreg[cur_inst] = -(values.at(0) - values.at(1));
      break;
    case Instruction::And:
      this->vreg[cur_inst] = values.at(0) & values.at(1);
      break;
    case Instruction::Or:
      this->vreg[cur_inst] = values.at(0) | values.at(1);
      break;
    case Instruction::Shl:
      this->vreg[cur_inst] = values.at(0) << values.at(1);
      break;
    case Instruction::AShr:
      this->vreg[cur_inst] = values.at(0) >> values.at(1);
      break;
    case Instruction::LShr:
      this->vreg[cur_inst] = ((unsigned int)values.at(0)) >> values.at(1);
      break;
    case Instruction::Mul:
      this->vreg[cur_inst] = values.at(0) * values.at(1);
      break;
    case Instruction::Cmp: {
      bool flag;
      switch (static_cast<CmpInst *>(cur_inst)->getCmpOp()) {
      case CmpInst::CmpOp::EQ:
        flag = values.at(0) == values.at(1);
        break;
      case CmpInst::CmpOp::NE:
        flag = values.at(0) != values.at(1);
        break;
      case CmpInst::CmpOp::LT:
        flag = values.at(0) < values.at(1);
        break;
      case CmpInst::CmpOp::GT:
        flag = values.at(0) > values.at(1);
        break;
      case CmpInst::CmpOp::LE:
        flag = values.at(0) <= values.at(1);
        break;
      case CmpInst::CmpOp::GE:
        flag = values.at(0) >= values.at(1);
        break;
      default:
        std::cerr << "???" << std::endl;
        abort();
        break;
      }
      this->vreg[cur_inst] = flag ? 1 : 0;
      break;
    }
    case Instruction::Neg:
      this->vreg[cur_inst] = -values.at(0);
      break;
    case Instruction::Not:
      this->vreg[cur_inst] = !values.at(0);
      break;
    case Instruction::ZExt:
      this->vreg[cur_inst] = !(!(values.at(0)));
      break;
    case Instruction::GEP:
      this->vreg[cur_inst] =
          values.at(0) +
          values.at(1) *
              cur_inst->getType()->getPointerElementType()->getSize();
      break;
    case Instruction::Div:
      if (values.at(1) == 0) {
        std::cerr << "Static analyzer: divided by zero!" << std::endl;
        return std::make_pair(false, 0);
      }
      this->vreg[cur_inst] = values.at(0) / values.at(1);
      break;
    case Instruction::Rem:
      if (values.at(1) == 0) {
        std::cerr << "Static analyzer: divided by zero!" << std::endl;
        return std::make_pair(false, 0);
      }
      this->vreg[cur_inst] = values.at(0) % values.at(1);
      break;
    case Instruction::Br: {
      auto inst_br = static_cast<BranchInst *>(cur_inst);
      BasicBlock *prev_bb = cur_inst->getParent();
      const bool is_cond = inst_br->isCondBr();
      const bool is_cmp = inst_br->isCmpBr();
      const auto &ops = cur_inst->getOperands();
      if (is_cmp) {
        // CmpBr
        bool flag = false;
        switch (inst_br->getCmpOp()) {
        case CmpInst::CmpOp::EQ:
          flag = values.at(0) == values.at(1);
          break;
        case CmpInst::CmpOp::NE:
          flag = values.at(0) != values.at(1);
          break;
        case CmpInst::CmpOp::LT:
          flag = values.at(0) < values.at(1);
          break;
        case CmpInst::CmpOp::GT:
          flag = values.at(0) > values.at(1);
          break;
        case CmpInst::CmpOp::LE:
          flag = values.at(0) <= values.at(1);
          break;
        case CmpInst::CmpOp::GE:
          flag = values.at(0) >= values.at(1);
          break;
        default:
          std::cerr << "???" << std::endl;
          abort();
          break;
        }
        cur_inst = static_cast<BasicBlock *>(ops.at(flag ? 2 : 3))
                       ->getInstructions()
                       .front();
      } else if (is_cond) {
        // CondBr
        cur_inst = static_cast<BasicBlock *>(ops.at(values.at(0) ? 1 : 2))
                       ->getInstructions()
                       .front();
      } else {
        // Br
        cur_inst =
            static_cast<BasicBlock *>(ops.at(0))->getInstructions().front();
      }
      // resolve PHI
      {
        std::vector<std::pair<Value *, int>> v;
        // find all PHI
        for (auto &inst : cur_inst->getParent()->getInstructions()) {
          if (inst->isPHI()) {
            Value *pre_op = nullptr;
            for (auto &op : inst->getOperands()) {
              if (op == prev_bb) {
                auto val_const = dynamic_cast<ConstantInt *>(pre_op);
                auto val_global = dynamic_cast<GlobalVariable *>(pre_op);
                int val = 0;
                if (val_const) {
                  val = val_const->getValue();
                } else if (val_global) {
                  val = this->vreg[val_global].getVal();
                } else {
                  val = this->vreg[pre_op].getVal();
                }
                v.push_back(std::make_pair(inst, val));
              }
              pre_op = op;
            }
          }
        }
        // PHI assign
        for (auto &p : v) {
          this->vreg[p.first] = p.second;
        }
      }
      // set branched
      branched = true;
      break;
    }
    case Instruction::PHI:
      // do nothing here
      break;
    default:
      std::cerr << "Static analyzer: inst not supported" << std::endl;
      return std::make_pair(false, 0);
      break;
    }
    // next inst
    if (!branched) {
      cur_inst = this->next_inst[cur_inst];
    }
    // std::cerr << gas << std::endl;
  }
  std::cerr << "Static analyzer: gas ran out!" << std::endl;
  return std::make_pair(false, 0);
}

Functionality Analyzer::analyzeFunctionality() {
  std::vector<Value *> args;
  for (auto &arg : this->func->getArgs()) {
    args.push_back(arg);
  }
  if (args.size() == 4 && args.at(0)->getType()->isInt32() &&
      args.at(1)->getType()->isPointerTy() &&
      args.at(2)->getType()->isPointerTy() &&
      args.at(3)->getType()->isPointerTy() &&
      args.at(1)->getType()->getPointerElementType()->isInt32() &&
      args.at(2)->getType()->getPointerElementType()->isInt32() &&
      args.at(3)->getType()->getPointerElementType()->isInt32()) {
    int tot = 10;
    bool flag = true;
    int n = 0;
    auto bounds = static_cast<Argument *>(args.at(1))->getArrayBound();
    int l;
    if (bounds.size() < 2) {
      flag = false;
      goto mm_finished;
    }
    l = static_cast<ConstantInt *>(bounds.at(1))->getValue();
    while (tot--) {
      n++;
      clear();
      this->vreg[args.at(0)] = n;
      if (!run(1e6).first) {
        flag = false;
        goto mm_finished;
      }
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          int S = 0;
          for (int k = 0; k < n; k++) {
            S +=
                (this->vmem[this->vreg[args.at(1)].getVal() + (i * l + k) * 4] *
                 this->vmem[this->vreg[args.at(2)].getVal() + (k * l + j) * 4])
                    .getVal();
          }
          int T = this->vmem[this->vreg[args.at(3)].getVal() + (i * l + j) * 4]
                      .getVal();
          if (S != T) {
            flag = false;
            goto mm_finished;
          }
        }
      }
    }
  mm_finished:
    if (flag) {
      return Functionality::mm;
    }
  }
  if (args.size() == 4 && args.at(0)->getType()->isInt32() &&
      args.at(1)->getType()->isPointerTy() &&
      args.at(2)->getType()->isPointerTy() &&
      args.at(3)->getType()->isPointerTy() &&
      args.at(1)->getType()->getPointerElementType()->isInt32() &&
      args.at(2)->getType()->getPointerElementType()->isInt32() &&
      args.at(3)->getType()->getPointerElementType()->isInt32()) {
    int tot = 20;
    bool flag = true;
    int n = 0;
    auto bounds = static_cast<Argument *>(args.at(1))->getArrayBound();
    int l;
    if (bounds.size() < 2) {
      flag = false;
      goto mv_finished;
    }
    l = static_cast<ConstantInt *>(bounds.at(1))->getValue();
    while (tot--) {
      n++;
      clear();
      this->vreg[args.at(0)] = n;
      if (!run(1e5).first) {
        flag = false;
        goto mv_finished;
      }
      for (int i = 0; i < n; i++) {
        int S = 0;
        for (int j = 0; j < n; j++) {
          S += (this->vmem[this->vreg[args.at(1)].getVal() + (i * l + j) * 4] *
                this->vmem[this->vreg[args.at(2)].getVal() + j * 4])
                   .getVal();
        }
        int T = this->vmem[this->vreg[args.at(3)].getVal() + i * 4].getVal();
        if (S != T) {
          flag = false;
          goto mv_finished;
        }
      }
    }
  mv_finished:
    if (flag) {
      return Functionality::mv;
    }
  }
  if (args.size() == 3 && args.at(0)->getType()->isPointerTy() &&
      args.at(0)->getType()->getPointerElementType()->isInt32() &&
      args.at(1)->getType()->isInt32() && args.at(2)->getType()->isInt32()) {
    int tot = 1000;
    int qwq = 30;
    bool flag = true;
    int n = 0;
    while (tot--) {
      n++;
      clear();
      int pos = n / 2;
      int d = n % 2;
      this->vreg[args.at(1)] = pos;
      this->vreg[args.at(2)] = d;
      int ofs = pos / qwq;
      int bbb = pos % qwq;
      int vvv = rd();
      while (vvv < 0) {
        vvv = rd();
      }
      this->vmem[this->vreg[args.at(0)].getVal() + ofs * 4] = vvv;
      int S = this->vmem[this->vreg[args.at(0)].getVal() + ofs * 4].getVal();
      S &= ~(1 << bbb);
      if (d == 1) {
        S |= 1 << bbb;
      }
      if (!run(1e3).first) {
        flag = false;
        goto set_finished;
      }
      int T = this->vmem[this->vreg[args.at(0)].getVal() + ofs * 4].getVal();
      if (S != T) {
        flag = false;
        goto set_finished;
      }
    }
  set_finished:
    if (flag) {
      return Functionality::set;
    }
  }
  if (args.size() == 2 && args.at(0)->getType()->isInt32() &&
      args.at(1)->getType()->isInt32()) {
    int tot = 1000;
    bool flag = true;
    int n = 0;
    while (tot--) {
      n++;
      clear();
      int bbb = n % 16;
      int vvv = rd();
      while (vvv < 0) {
        vvv = rd();
      }
      this->vreg[args.at(0)] = vvv;
      this->vreg[args.at(1)] = bbb;
      int S = bbb > 7 ? 0 : (vvv >> (bbb * 4)) & ((1 << 4) - 1);
      auto p = run(1e3);
      if (!p.first) {
        flag = false;
        goto gnp_finished;
      }
      int T = p.second;
      if (S != T) {
        flag = false;
        goto gnp_finished;
      }
    }
  gnp_finished:
    if (flag) {
      return Functionality::gnp;
    }
  }
  return Functionality::unknown;
}

bool analyzeMTsafety() { return false; }

}; // namespace Analyzer