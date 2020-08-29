#include "Function.h"
#include "BasicBlock.h"
#include "Module.h"

#define PRINT_DEPTH_(N)                                                        \
  { std::cout << std::string(N, ' '); }

Function::Function(FunctionType *ty, const std::string &name, Module *parent)
    : Value(ty, name), parent_(parent) {
  // num_args_ = ty->getNumParams();
  parent->addFunction(this);
  print_cnt_ = 0;
  buildArgs();
}

Function *Function::create(FunctionType *ty, const std::string &name,
                           Module *parent) {
  return new Function(ty, name, parent);
}

FunctionType *Function::getFunctionType() const {
  return static_cast<FunctionType *>(getType());
}

Type *Function::getResultType() const {
  return getFunctionType()->getResultType();
}

unsigned Function::getNumArgs() const {
  return getFunctionType()->getNumArgs();
}

Module *Function::getParent() const { return parent_; }

void Function::buildArgs() {
  auto *func_ty = getFunctionType();
  unsigned num_args = getNumArgs();
  for (int i = 0; i < num_args; i++) {
    arguments_.push_back(new Argument(func_ty->getArgType(i), "", this, i));
  }
}

void Function::addBasicBlock(BasicBlock *bb) {
  // if (basic_blocks_.empty())
  // {
  //     bb->setName(std::to_string(getNumArgs()));
  // }
  // else
  // {
  //     auto seq = getNumArgs();
  //     for (auto block : basic_blocks_)
  //     {
  //         seq += block->getNumOfInstr();
  //     }
  //     bb->setName(std::to_string(seq));
  // }
  basic_blocks_.push_back(bb);
}

void Function::addBasicBlockAfter(std::list<BasicBlock *>::iterator after_pos,
                                  BasicBlock *bb) {
  ++after_pos;
  basic_blocks_.insert(after_pos, bb);
}

void Function::removeBasicBlock(BasicBlock *bb) {
  // for(auto instr : bb->getInstructions() )
  // {
  //     instr->removeUseOfOps();
  // }

  basic_blocks_.remove(bb);
  for (auto pre : bb->getPreBasicBlocks()) {
    pre->removeSuccBasicBlock(bb);
  }
  for (auto succ : bb->getSuccBasicBlocks()) {
    succ->removePreBasicBlock(bb);
  }
}

void NameBaseBlock(BaseBlock *bb, std::map<Value *, int> &seq,
                   int &print_cnt_) {
  if (seq.find(bb) == seq.end() && bb->getName().empty()) {
    auto seq_num = seq.size() + print_cnt_;
    bb->setName(std::to_string(seq_num));
    seq.insert({bb, seq_num});
  }

  if (bb->isBasicBlock()) {
    auto basicblock = dynamic_cast<BasicBlock *>(bb);
    for (auto instr : basicblock->getInstrList()) {
      if (seq.find(instr) == seq.end() && instr->getName().empty() &&
          (!instr->isVoid())) {
        auto seq_num = seq.size() + print_cnt_;
        instr->setName(std::to_string(seq_num));
        seq.insert({instr, seq_num});
      }
    }
  } else if (bb->isIfBlock()) {
    auto ifbb = dynamic_cast<IfBlock *>(bb);
    for (auto basebb : ifbb->getCondBBs()) {
      NameBaseBlock(basebb, seq, print_cnt_);
    }
    for (auto basebb : ifbb->getIfBodyBBs()) {
      NameBaseBlock(basebb, seq, print_cnt_);
    }
    for (auto basebb : ifbb->getElseBodyBBs()) {
      NameBaseBlock(basebb, seq, print_cnt_);
    }
  } else if (bb->isWhileBlock()) {
    auto whilebb = dynamic_cast<WhileBlock *>(bb);
    for (auto basebb : whilebb->getCondBBs()) {
      NameBaseBlock(basebb, seq, print_cnt_);
    }
    for (auto basebb : whilebb->getBodyBBs()) {
      NameBaseBlock(basebb, seq, print_cnt_);
    }
  }
}

void PrintBaseBlock(BaseBlock *bb, int depth) {
  if (bb->isBasicBlock()) {
    PRINT_DEPTH_(depth);
    auto basicblock = dynamic_cast<BasicBlock *>(bb);
    std::cerr << "<label>" << bb->getName();
    std::cerr << std::endl;
    for (auto instr : basicblock->getInstrList()) {
      PRINT_DEPTH_(depth);

      std::cerr << "    ";
      if (!instr->isVoid()) {
        instr->print();
        std::cerr << " = ";
      }
      switch (instr->getInstrType()) {
      case Instruction::Ret:
        std::cerr << "ret ";
        break;

      case Instruction::Br:
        std::cerr << "Br ";
        if (static_cast<BranchInst *>(instr)->isCmpBr()) {
          switch (static_cast<BranchInst *>(instr)->getCmpOp()) {
          case CmpInst::EQ:
            std::cerr << "EQ ";
            break;
          case CmpInst::NE:
            std::cerr << "NE ";
            break;
          case CmpInst::GT:
            std::cerr << "GT ";
            break;
          case CmpInst::GE:
            std::cerr << "GE ";
            break;
          case CmpInst::LT:
            std::cerr << "LT ";
            break;
          case CmpInst::LE:
            std::cerr << "LE ";
            break;
          default:
            std::cerr << "error in cmp type" << std::endl;
            break;
          }
        }
        break;

      case Instruction::Neg:
        std::cerr << "Neg ";
        break;

      case Instruction::Add:
        std::cerr << "Add ";
        break;

      case Instruction::Sub:
        std::cerr << "Sub ";
        break;

      case Instruction::RSub:
        std::cerr << "RSub ";
        break;

      case Instruction::Mul:
        std::cerr << "Mul ";
        break;

      case Instruction::Div:
        std::cerr << "Div ";
        break;

      case Instruction::Rem:
        std::cerr << "Rem ";
        break;

      case Instruction::AddAddr:
        std::cerr << "AddAddr ";
        break;

      case Instruction::And:
        std::cerr << "And ";
        break;

      case Instruction::Or:
        std::cerr << "Or ";
        break;

      case Instruction::Not:
        std::cerr << "Not ";
        break;

      case Instruction::Alloca:
        std::cerr << "Alloca ";
        if (static_cast<AllocaInst *>(instr)->getInit()) {
          std::cerr << "Init 0 ";
        }

        break;

      case Instruction::Load:
        std::cerr << "Load ";
        break;

      case Instruction::Store:
        std::cerr << "Store ";
        break;

      case Instruction::Shl:
        std::cerr << "Shl ";
        break;

      case Instruction::AShr:
        std::cerr << "AShr ";
        break;

      case Instruction::LShr:
        std::cerr << "LShr ";
        break;

      case Instruction::Cmp:
        std::cerr << "Cmp";
        switch (static_cast<CmpInst *>(instr)->getCmpOp()) {
        case CmpInst::EQ:
          std::cerr << "EQ ";
          break;
        case CmpInst::NE:
          std::cerr << "NE ";
          break;
        case CmpInst::GT:
          std::cerr << "GT ";
          break;
        case CmpInst::GE:
          std::cerr << "GE ";
          break;
        case CmpInst::LT:
          std::cerr << "LT ";
          break;
        case CmpInst::LE:
          std::cerr << "LE ";
          break;
        default:
          std::cerr << "error in cmp type" << std::endl;
          break;
        }
        break;

      case Instruction::Call:
        std::cerr << "Call ";
        std::cerr << static_cast<Function *>(instr->getOperand(0))->getName()
                  << " ";
        break;

      case Instruction::GEP:
        std::cerr << "GEP ";
        break;

      case Instruction::ZExt:
        std::cerr << "ZExt ";
        break;

      case Instruction::PHI:
        std::cerr << "PHI ";
        break;

      case Instruction::MulAdd:
        std::cerr << "MulAdd ";
        break;
      case Instruction::VV:
        std::cerr << "VV" << static_cast<VVInst *>(instr)->getNumInt32() << " ";
        break;

      case Instruction::BIC:
        std::cerr << "BIC ";
        break;

      case Instruction::Break:
        std::cerr << "Break ";
        break;
      case Instruction::Continue:
        std::cerr << "Continue ";
        break;

      default:
        std::cerr << "error instr type";
        break;
      }

      for (auto op : instr->getOperandList()) {
        auto op_instr = dynamic_cast<Instruction *>(op);
        if (op_instr) {
          op_instr->print();
        }

        auto op_const = dynamic_cast<Constant *>(op);
        if (op_const) {
          op_const->print();
        }

        auto op_arg = dynamic_cast<Argument *>(op);
        if (op_arg) {
          op_arg->print();
        }

        auto op_global = dynamic_cast<GlobalVariable *>(op);
        if (op_global) {
          op_global->print();
        }

        auto op_bb = dynamic_cast<BasicBlock *>(op);
        if (op_bb) {
          op_bb->print();
        }
      }
      std::cerr << std::endl;
    }
  } else if (bb->isIfBlock()) {
    PRINT_DEPTH_(depth);
    std::cerr << "IF :" << std::endl;
    auto ifbb = dynamic_cast<IfBlock *>(bb);
    PRINT_DEPTH_(depth);
    std::cerr << "    Cond:" << std::endl;
    for (auto basebb : ifbb->getCondBBs()) {
      PrintBaseBlock(basebb, depth + 4);
    }
    PRINT_DEPTH_(depth);
    std::cerr << "    Then:" << std::endl;
    for (auto basebb : ifbb->getIfBodyBBs()) {
      PrintBaseBlock(basebb, depth + 4);
    }
    PRINT_DEPTH_(depth);
    std::cerr << "    Else:" << std::endl;
    for (auto basebb : ifbb->getElseBodyBBs()) {
      PrintBaseBlock(basebb, depth + 4);
    }
  } else if (bb->isWhileBlock()) {
    PRINT_DEPTH_(depth);
    std::cerr << "While:" << std::endl;
    PRINT_DEPTH_(depth);
    std::cerr << "    Cond:" << std::endl;
    auto whilebb = dynamic_cast<WhileBlock *>(bb);
    for (auto basebb : whilebb->getCondBBs()) {
      PrintBaseBlock(basebb, depth + 4);
    }
    PRINT_DEPTH_(depth);
    std::cerr << "    Body:" << std::endl;
    for (auto basebb : whilebb->getBodyBBs()) {
      PrintBaseBlock(basebb, depth + 4);
    }
  }
}

void Function::HighIRprint() {
  std::map<Value *, int> seq;
  for (auto arg : this->getArgs()) {
    if (seq.find(arg) == seq.end() && arg->getName().empty()) {
      auto seq_num = seq.size() + print_cnt_;
      arg->setName(std::to_string(seq_num));
      seq.insert({arg, seq_num});
    }
  }
  for (auto bb : base_blocks_) {
    if (seq.find(bb) == seq.end() && bb->getName().empty()) {
      auto seq_num = seq.size() + print_cnt_;
      bb->setName(std::to_string(seq_num));
      seq.insert({bb, seq_num});
    }
    NameBaseBlock(bb, seq, print_cnt_);
  }

  std::cerr << "define ";
  this->getResultType()->print();
  std::cerr << "@" << this->getName();
  std::cerr << "(";

  for (auto arg : this->getArgs()) {
    arg->print();
    if (arg->getArgNo() + 1 != this->getNumArgs()) {
      std::cerr << ", ";
    }
  }
  std::cerr << ")" << std::endl;

  for (auto bb : base_blocks_) {
    PrintBaseBlock(bb, 0);
  }

  print_cnt_ += seq.size();
}

void Function::print() {
  if (basic_blocks_.empty()) {
    HighIRprint();
    return;
  }

  std::map<Value *, int> seq;
  for (auto arg : this->getArgs()) {
    if (seq.find(arg) == seq.end() && arg->getName().empty()) {
      auto seq_num = seq.size() + print_cnt_;
      arg->setName(std::to_string(seq_num));
      seq.insert({arg, seq_num});
    }
  }
  for (auto bb : basic_blocks_) {
    if (seq.find(bb) == seq.end() && bb->getName().empty()) {
      auto seq_num = seq.size() + print_cnt_;
      bb->setName(std::to_string(seq_num));
      seq.insert({bb, seq_num});
    }

    for (auto instr : bb->getInstrList()) {
      if (seq.find(instr) == seq.end() && instr->getName().empty() &&
          (!instr->isVoid())) {
        auto seq_num = seq.size() + print_cnt_;
        instr->setName(std::to_string(seq_num));
        seq.insert({instr, seq_num});
      }
    }
  }

  std::cerr << "define ";
  this->getResultType()->print();
  std::cerr << "@" << this->getName();
  std::cerr << "(";

  for (auto arg : this->getArgs()) {
    arg->print();
    if (arg->getArgNo() + 1 != this->getNumArgs()) {
      std::cerr << ", ";
    }
  }
  std::cerr << ")" << std::endl;

  for (auto bb : basic_blocks_) {
    std::cerr << "<label>" << bb->getName();
    std::cerr << "        ;preds: ";
    for (auto pre_bb : bb->getPreBasicBlocks()) {
      std::cerr << "%" << pre_bb->getName() << "    ";
    }

    std::cerr << std::endl;
    for (auto instr : bb->getInstrList()) {
      std::cerr << "    ";
      if (!instr->isVoid()) {
        instr->print();
        std::cerr << " = ";
      }
      switch (instr->getInstrType()) {
      case Instruction::Ret:
        std::cerr << "ret ";
        break;

      case Instruction::Br:
        std::cerr << "Br ";
        if (static_cast<BranchInst *>(instr)->isCmpBr()) {
          switch (static_cast<BranchInst *>(instr)->getCmpOp()) {
          case CmpInst::EQ:
            std::cerr << "EQ ";
            break;
          case CmpInst::NE:
            std::cerr << "NE ";
            break;
          case CmpInst::GT:
            std::cerr << "GT ";
            break;
          case CmpInst::GE:
            std::cerr << "GE ";
            break;
          case CmpInst::LT:
            std::cerr << "LT ";
            break;
          case CmpInst::LE:
            std::cerr << "LE ";
            break;
          default:
            std::cerr << "error in cmp type" << std::endl;
            break;
          }
        }
        break;

      case Instruction::Neg:
        std::cerr << "Neg ";
        break;

      case Instruction::Add:
        std::cerr << "Add ";
        break;

      case Instruction::Sub:
        std::cerr << "Sub ";
        break;

      case Instruction::RSub:
        std::cerr << "RSub ";
        break;

      case Instruction::Mul:
        std::cerr << "Mul ";
        break;

      case Instruction::Div:
        std::cerr << "Div ";
        break;

      case Instruction::Rem:
        std::cerr << "Rem ";
        break;

      case Instruction::AddAddr:
        std::cerr << "AddAddr ";
        break;

      case Instruction::And:
        std::cerr << "And ";
        break;

      case Instruction::Or:
        std::cerr << "Or ";
        break;

      case Instruction::Not:
        std::cerr << "Not ";
        break;

      case Instruction::Alloca:
        std::cerr << "Alloca ";
        if (static_cast<AllocaInst *>(instr)->getInit()) {
          std::cerr << "Init 0 ";
        }

        break;

      case Instruction::Load:
        std::cerr << "Load ";
        break;

      case Instruction::Store:
        std::cerr << "Store ";
        break;

      case Instruction::Shl:
        std::cerr << "Shl ";
        break;

      case Instruction::AShr:
        std::cerr << "AShr ";
        break;

      case Instruction::LShr:
        std::cerr << "LShr ";
        break;

      case Instruction::Cmp:
        std::cerr << "Cmp";
        switch (static_cast<CmpInst *>(instr)->getCmpOp()) {
        case CmpInst::EQ:
          std::cerr << "EQ ";
          break;
        case CmpInst::NE:
          std::cerr << "NE ";
          break;
        case CmpInst::GT:
          std::cerr << "GT ";
          break;
        case CmpInst::GE:
          std::cerr << "GE ";
          break;
        case CmpInst::LT:
          std::cerr << "LT ";
          break;
        case CmpInst::LE:
          std::cerr << "LE ";
          break;
        default:
          std::cerr << "error in cmp type" << std::endl;
          break;
        }
        break;

      case Instruction::Call:
        std::cerr << "Call ";
        std::cerr << static_cast<Function *>(instr->getOperand(0))->getName()
                  << " ";
        break;

      case Instruction::GEP:
        std::cerr << "GEP ";
        break;

      case Instruction::ZExt:
        std::cerr << "ZExt ";
        break;

      case Instruction::PHI:
        std::cerr << "PHI ";
        break;

      case Instruction::MulAdd:
        std::cerr << "MulAdd ";
        break;
      case Instruction::VV:
        std::cerr << "VV" << static_cast<VVInst *>(instr)->getNumInt32() << " ";
        break;

      case Instruction::BIC:
        std::cerr << "BIC ";
        break;

      default:
        std::cerr << "error instr type";
        break;
      }

      for (auto op : instr->getOperandList()) {
        auto op_instr = dynamic_cast<Instruction *>(op);
        if (op_instr) {
          op_instr->print();
        }

        auto op_const = dynamic_cast<Constant *>(op);
        if (op_const) {
          op_const->print();
        }

        auto op_arg = dynamic_cast<Argument *>(op);
        if (op_arg) {
          op_arg->print();
        }

        auto op_global = dynamic_cast<GlobalVariable *>(op);
        if (op_global) {
          op_global->print();
        }

        auto op_bb = dynamic_cast<BasicBlock *>(op);
        if (op_bb) {
          op_bb->print();
        }
      }
      std::cerr << std::endl;
    }
    std::cerr << "        ;succ: ";
    for (auto succ_bb : bb->getSuccBasicBlocks()) {
      std::cerr << "%" << succ_bb->getName() << "    ";
    }
    std::cerr << std::endl;
  }
  print_cnt_ += seq.size();
}

void Argument::print() {
  this->getType()->print();
  std::cerr << " %" << this->getName() << " ";
}

void Function::addBaseBlock(BaseBlock *basebb) {
  base_blocks_.push_back(basebb);
  basebb->setFunction(this);
}