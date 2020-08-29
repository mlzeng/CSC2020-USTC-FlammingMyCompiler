#include "CodeGen.hh"

#include <algorithm>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "InstructionsGen.hh"
#include "ReturnVal.h"

extern bool disable_div_optimization; // for debug

std::string
CodeGen::generateModuleCode(std::map<Value *, int> register_mapping) {
  std::string asm_code;
  this->in_mt_env.clear();
  this->register_mapping = register_mapping;
  asm_code += InstGen::spaces + ".arch armv" + std::to_string(arch_version) +
              "-a" + InstGen::newline;
  asm_code += InstGen::spaces + ".file" + " " + "\"" +
              this->module->getSourceFileName() + "\"" + InstGen::newline;
  asm_code += InstGen::spaces + ".text" + InstGen::newline;
  for (auto &func : this->module->getFunctions()) {
    if (func->getBasicBlocks().size()) {
      asm_code += CodeGen::global(func->getName());
    }
  }
  asm_code += InstGen::spaces + ".arm" + InstGen::newline;
  asm_code += InstGen::spaces + ".fpu neon" + InstGen::newline;
  asm_code += CodeGen::generateGOT();
  for (auto &func : this->module->getFunctions()) {
    if (func->getBasicBlocks().size()) {
      asm_code += CodeGen::generateFunctionCode(func);
    }
  }
  asm_code += CodeGen::mtstart();
  asm_code += CodeGen::mtend();
  asm_code += InstGen::spaces + ".data" + InstGen::newline;
  asm_code += CodeGen::generateGlobalVarsCode();
  return asm_code;
}

std::string CodeGen::global(std::string name) {
  return InstGen::spaces + ".global" + " " + name + InstGen::newline;
}

std::string CodeGen::generateFunctionCode(Function *func) {
  std::string asm_code;
  int counter = 0;
  CodeGen::findMTVars(func);
  CodeGen::allocateStackSpace(func);
  for (auto &bb : func->getBasicBlocks()) {
    if (bb->getName().empty()) {
      bb->setName(std::to_string(counter++));
    }
  }
  asm_code += func->getName() + ":" + InstGen::newline;
  asm_code += CodeGen::comment("thread_stack_size=" +
                               std::to_string(this->thread_stack_size));
  asm_code +=
      CodeGen::comment("stack_size=" + std::to_string(this->stack_size));
  asm_code += generateFunctionEntryCode(func);
  for (auto &bb : func->getBasicBlocks()) {
    asm_code += getLabelName(bb) + ":" + InstGen::newline;
    asm_code += generateBasicBlockCode(bb);
  }
  asm_code += generateFunctionPostCode(func);
  return asm_code;
}

void CodeGen::findMTVars(Function *func) {
  for (auto &bb : func->getBasicBlocks()) {
    for (auto &inst : bb->getInstructions()) {
      if (CodeGen::is_mt_start(inst)) {
        std::queue<Instruction *> Q;
        Q.push(inst);
        bool flag = false;
        while (!Q.empty()) {
          Instruction *cur_inst = Q.front();
          Q.pop();
          if (flag && CodeGen::is_mt_start(cur_inst)) {
            std::cerr << "MTSTART followed by MTSTART!!!" << std::endl;
            abort();
          }
          if (flag && CodeGen::is_mt_end(cur_inst)) {
            continue;
          }
          if (flag) {
            if (this->in_mt_env[cur_inst] != nullptr) {
              if (this->in_mt_env[cur_inst] != inst) {
                std::cerr << "MTSTART conflict!!!" << std::endl;
                abort();
              } else {
                // visited -> no expand
                continue;
              }
            } else {
              this->in_mt_env[cur_inst] = inst;
            }
          }
          flag = true;
          // expand
          auto cur_bb = cur_inst->getParent();
          if (cur_inst->isTerminator()) {
            for (auto &next_bb : cur_bb->getSuccBasicBlocks()) {
              Q.push(next_bb->getInstructions().front());
            }
          } else {
            bool found = false;
            for (auto &i : cur_bb->getInstructions()) {
              if (found) {
                Q.push(i);
                break;
              }
              if (i == cur_inst) {
                found = true;
              }
            }
          }
        }
      }
    }
  }
}

std::string CodeGen::generateFunctionEntryCode(Function *func) {
  std::string asm_code;
  this->cur_inst_mt_env = nullptr;
  asm_code += CodeGen::getLabelName(func, 0) + ":" + InstGen::newline;
  asm_code += CodeGen::comment("function preprocess");
  auto save_registers = CodeGen::getCalleeSaveRegisters(func);
  save_registers.push_back(InstGen::lr);
  std::sort(save_registers.begin(), save_registers.end());
  // large stack allocate
  if (func->getName() == "main" && enlarge_stack) {
    const bool set_zero = false;
    asm_code += CodeGen::comment("enlarge stack");
    // do not change lr (r14)
    asm_code += InstGen::push(save_registers);
    asm_code += InstGen::setValue(InstGen::Reg(0),
                                  InstGen::Constant(enlarge_stack_size));
    asm_code += InstGen::setValue(InstGen::Reg(1), InstGen::Constant(1));
    asm_code += InstGen::bl(set_zero ? "calloc" : "malloc");
    asm_code += InstGen::instConst(InstGen::add, InstGen::Reg(op_reg_0),
                                   InstGen::Reg(0),
                                   InstGen::Constant(enlarge_stack_size));
    asm_code += InstGen::pop(save_registers);
    asm_code += InstGen::mov(InstGen::Reg(op_reg_2), InstGen::sp);
    asm_code += InstGen::mov(InstGen::sp, InstGen::Reg(op_reg_0));
    asm_code += InstGen::push({InstGen::Reg(op_reg_2)});
  }
  // save callee-save registers and lr
  asm_code += CodeGen::comment("save callee-save registers and lr");
  asm_code += InstGen::push(save_registers);
  // set thread id reg
  if (this->mt_enabled) {
    asm_code += CodeGen::comment("set thread id register to zero");
    asm_code += InstGen::mov(InstGen::Reg(thread_id_reg), InstGen::Constant(0));
  }
  // allocate stack space and process function args
  asm_code += CodeGen::comment("allocate stack space");
  asm_code += InstGen::instConst(InstGen::sub, InstGen::sp, InstGen::sp,
                                 InstGen::Constant(this->stack_size));
  asm_code += CodeGen::comment("process function args");
  int cnt = 0;
  std::vector<Value *> source, target;
  Type dummy_type(Type::IntegerTyID);
  std::vector<Value> dummys(func->getArgs().size(), Value(&dummy_type));
  for (auto &arg : func->getArgs()) {
    bool extended = false;
    auto sizeof_val = arg->getType()->getSize(extended);
    sizeof_val = ((sizeof_val + 3) / 4) * 4;
    exit_ifnot(_generateFunctionEntryCode_CodeGen, sizeof_val == 4);
    auto dummy = &dummys.at(cnt);
    if (cnt >= 4) {
      int offset = stack_size + (save_registers.size() + cnt - 4) * 4;
      this->register_mapping.erase(dummy);
      this->stack_mapping[dummy] = offset;
      source.push_back(dummy);
      target.push_back(arg);
    } else {
      this->register_mapping[dummy] = cnt;
      this->stack_mapping.erase(dummy);
      source.push_back(dummy);
      target.push_back(arg);
    }
    cnt++;
  }
  asm_code += CodeGen::virtualRegMove(target, source);
  return asm_code;
}

std::string CodeGen::generateFunctionExitCode(Function *func) {
  std::string asm_code;
  asm_code += CodeGen::comment("function return");
  // reclaim stack space
  asm_code += CodeGen::comment("reclaim stack space");
  asm_code += InstGen::instConst(InstGen::add, InstGen::sp, InstGen::sp,
                                 InstGen::Constant(this->stack_size));
  // restore callee-save registers and pc
  asm_code += CodeGen::comment("restore callee-save registers and pc");
  auto save_registers = CodeGen::getCalleeSaveRegisters(func);
  save_registers.push_back(InstGen::pc);
  std::sort(save_registers.begin(), save_registers.end());
  asm_code += InstGen::pop(save_registers);
  // enlarged stack reclaim
  if (func->getName() == "main" && enlarge_stack) {
    asm_code += CodeGen::comment("enlarged stack reclaim");
    asm_code += InstGen::pop({InstGen::Reg(op_reg_0)});
    asm_code += InstGen::mov(InstGen::sp, InstGen::Reg(op_reg_0));
  }
  return asm_code;
}

std::string CodeGen::generateFunctionPostCode(Function *func) {
  std::string asm_code;
  this->cur_inst_mt_env = nullptr;
  asm_code += CodeGen::getLabelName(func, 1) + ":" + InstGen::newline;
  asm_code += CodeGen::comment("function postcode");
  return asm_code;
}

std::string CodeGen::getLabelName(BasicBlock *bb) {
  return "." + bb->getParent()->getName() + "_" + bb->getName();
}

std::string CodeGen::getLabelName(Function *func, int type) {
  const std::vector<std::string> name_list = {"pre", "post"};
  return "." + func->getName() + "_" + name_list.at(type);
}

std::string CodeGen::generateBasicBlockCode(BasicBlock *bb) {
  std::string asm_code;
  for (auto &inst : bb->getInstructions()) {
    asm_code += CodeGen::generateInstructionCode(inst);
  }
  return asm_code;
}

std::string CodeGen::generateInstructionCode(Instruction *inst) {
  std::string asm_code;
  this->cur_inst_mt_env = in_mt_env[inst];
  auto &ops = inst->getOperands();
  asm_code += CodeGen::comment(inst->CommentPrint());
  if (inst->getInstrType() == Instruction::Ret) {
    if (!ops.empty()) {
      asm_code += CodeGen::assignToSpecificReg(ops.at(0), 0); // ABI
    }
    asm_code += CodeGen::generateFunctionExitCode(inst->getFunction());
  } else if (inst->getInstrType() == Instruction::Alloca) {
    if (this->register_mapping.count(inst)) {
      auto offset = stack_mapping.at(inst);
      int target = this->register_mapping.at(inst);
      asm_code += InstGen::instConst(InstGen::add, InstGen::Reg(target),
                                     InstGen::sp, InstGen::Constant(offset));
    }
    bool need_init = static_cast<AllocaInst *>(inst)->getInit();
    int init_val = 0;
    if (need_init) {
      const int init_threshold = 0;
      int sz = inst->getType()->getSize();
      if (sz > init_threshold * 4) {
        Type integer_type(Type::IntegerTyID);
        std::vector<Value *> args = {
            inst, ConstantInt::get(init_val, this->module.get()),
            ConstantInt::get(sz, this->module.get())};
        asm_code += CodeGen::generateFunctionCall(inst, "memset", args, -1);
      } else {
        asm_code += InstGen::mov(InstGen::Reg(op_reg_0), InstGen::Constant(0));
        asm_code += CodeGen::assignToSpecificReg(inst, op_reg_1);
        for (int i = 0; i < sz; i += 4) {
          asm_code +=
              InstGen::spaces + "str" + " " + InstGen::Reg(op_reg_0).getName() +
              ", " + "[" + InstGen::Reg(op_reg_1).getName() + ", " +
              InstGen::Constant(4).getName() + "]" + "!" + InstGen::newline;
        }
      }
    }
  } else if (inst->getInstrType() == Instruction::Load) {
    int alu_op0 = this->register_mapping.count(ops.at(0))
                      ? this->register_mapping.at(ops.at(0))
                      : op_reg_0;
    int alu_ret = this->register_mapping.count(inst)
                      ? this->register_mapping.at(inst)
                      : op_reg_1;
    asm_code += CodeGen::assignToSpecificReg(ops.at(0), alu_op0);
    if (ops.size() >= 2) {
      ConstantInt *op1_const = dynamic_cast<ConstantInt *>(ops.at(1));
      int shift = 0;
      if (ops.size() >= 3) {
        ConstantInt *op2_const = dynamic_cast<ConstantInt *>(ops.at(2));
        exit_ifnot(_op2Const_generateInstructionCode_CodeGen,
                   op2_const != nullptr);
        shift = op2_const->getValue();
        exit_ifnot(_shift_generateInstructionCode_CodeGen,
                   0 <= shift && shift <= 31);
      }
      if (op1_const) {
        asm_code +=
            InstGen::load(InstGen::Reg(alu_ret),
                          InstGen::Addr(InstGen::Reg(alu_op0),
                                        op1_const->getValue() << shift));
      } else {
        int alu_op1 = this->register_mapping.count(ops.at(1))
                          ? this->register_mapping.at(ops.at(1))
                          : op_reg_1;
        asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
        asm_code +=
            InstGen::ldr(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                         InstGen::Reg(alu_op1), InstGen::Constant(shift));
      }
    } else {
      asm_code += InstGen::load(InstGen::Reg(alu_ret),
                                InstGen::Addr(InstGen::Reg(alu_op0), 0));
    }
    asm_code += CodeGen::getFromSpecificReg(inst, alu_ret);
  } else if (inst->getInstrType() == Instruction::Store) {
    int alu_op0 = this->register_mapping.count(ops.at(0))
                      ? this->register_mapping.at(ops.at(0))
                      : op_reg_0;
    int alu_op1 = this->register_mapping.count(ops.at(1))
                      ? this->register_mapping.at(ops.at(1))
                      : op_reg_1;
    asm_code += CodeGen::assignToSpecificReg(ops.at(0), alu_op0);
    asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
    if (ops.size() >= 3) {
      ConstantInt *op2_const = dynamic_cast<ConstantInt *>(ops.at(2));
      int shift = 0;
      if (ops.size() >= 4) {
        ConstantInt *op3_const = dynamic_cast<ConstantInt *>(ops.at(3));
        exit_ifnot(_op3Const_generateInstructionCode_CodeGen,
                   op3_const != nullptr);
        shift = op3_const->getValue();
        exit_ifnot(_shift3_generateInstructionCode_CodeGen,
                   0 <= shift && shift <= 31);
      }
      if (op2_const) {
        asm_code +=
            InstGen::store(InstGen::Reg(alu_op0),
                           InstGen::Addr(InstGen::Reg(alu_op1),
                                         op2_const->getValue() << shift));
      } else {
        int alu_op2 = this->register_mapping.count(ops.at(2))
                          ? this->register_mapping.at(ops.at(2))
                          : op_reg_2;
        asm_code += CodeGen::assignToSpecificReg(ops.at(2), alu_op2);
        asm_code +=
            InstGen::str(InstGen::Reg(alu_op0), InstGen::Reg(alu_op1),
                         InstGen::Reg(alu_op2), InstGen::Constant(shift));
      }
    } else {
      asm_code += InstGen::str(InstGen::Reg(alu_op0),
                               InstGen::Addr(InstGen::Reg(alu_op1), 0));
    }
  } else if (inst->getInstrType() == Instruction::Call) {
    if (inst->getOperands().front()->getName() == "__mtstart") {
      asm_code += InstGen::bl("__mtstart");
    } else if (inst->getOperands().front()->getName() == "__mtend") {
      asm_code += InstGen::bl("__mtend");
    } else {
      std::string func_name = ops.at(0)->getName();
      std::vector<Value *> args(ops.begin() + 1, ops.end());
      asm_code += generateFunctionCall(inst, func_name, args);
    }
  } else if (inst->getInstrType() == Instruction::MulAdd) {
    int alu_op0 = this->register_mapping.count(ops.at(0))
                      ? this->register_mapping.at(ops.at(0))
                      : op_reg_0;
    int alu_op1 = this->register_mapping.count(ops.at(1))
                      ? this->register_mapping.at(ops.at(1))
                      : op_reg_1;
    int alu_op2 = this->register_mapping.count(ops.at(2))
                      ? this->register_mapping.at(ops.at(2))
                      : op_reg_2;
    int alu_ret = this->register_mapping.count(inst)
                      ? this->register_mapping.at(inst)
                      : op_reg_0;
    asm_code += CodeGen::assignToSpecificReg(ops.at(0), alu_op0);
    asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
    asm_code += CodeGen::assignToSpecificReg(ops.at(2), alu_op2);
    asm_code += InstGen::mla(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                             InstGen::Reg(alu_op1), InstGen::Reg(alu_op2));
    asm_code += CodeGen::getFromSpecificReg(inst, alu_ret);
  } else if (inst->getInstrType() == Instruction::VV ||
             inst->getInstrType() == Instruction::Add ||
             inst->getInstrType() == Instruction::Sub ||
             inst->getInstrType() == Instruction::RSub ||
             inst->getInstrType() == Instruction::And ||
             inst->getInstrType() == Instruction::Shl ||
             inst->getInstrType() == Instruction::AShr ||
             inst->getInstrType() == Instruction::LShr ||
             inst->getInstrType() == Instruction::Or ||
             inst->getInstrType() == Instruction::Mul ||
             inst->getInstrType() == Instruction::Cmp ||
             inst->getInstrType() == Instruction::Neg ||
             inst->getInstrType() == Instruction::Not ||
             inst->getInstrType() == Instruction::ZExt ||
             (inst->getInstrType() == Instruction::GEP &&
              inst->getOperands().size() == 2) ||
             inst->getInstrType() == Instruction::Div ||
             inst->getInstrType() == Instruction::Rem) {
    ConstantInt *op1_const =
        (ops.size() >= 2) ? (dynamic_cast<ConstantInt *>(ops.at(1))) : nullptr;
    int alu_op0 = this->register_mapping.count(ops.at(0))
                      ? this->register_mapping.at(ops.at(0))
                      : op_reg_0;
    int alu_op1 = -1;
    if (ops.size() >= 2) {
      alu_op1 = this->register_mapping.count(ops.at(1))
                    ? this->register_mapping.at(ops.at(1))
                    : op_reg_1;
    }
    int alu_ret = this->register_mapping.count(inst)
                      ? this->register_mapping.at(inst)
                      : op_reg_0;
    // flexible operand2
    int shift = 0;
    if (ops.size() >= 3 && (inst->getInstrType() == Instruction::Add ||
                            inst->getInstrType() == Instruction::Sub ||
                            inst->getInstrType() == Instruction::RSub ||
                            inst->getInstrType() == Instruction::And ||
                            inst->getInstrType() == Instruction::Or ||
                            inst->getInstrType() == Instruction::Cmp)) {
      ConstantInt *op2_const = dynamic_cast<ConstantInt *>(ops.at(2));
      assert(op2_const != nullptr);
      shift = op2_const->getValue();
      assert(0 <= shift && shift <= 31);
    }
    // must not change value of alu_op0 alu_op1
    asm_code += CodeGen::assignToSpecificReg(ops.at(0), alu_op0);
    if (inst->getInstrType() == Instruction::Cmp) {
      InstGen::CmpOp asmCmpOp =
          CodeGen::cmpConvert(static_cast<CmpInst *>(inst)->getCmpOp(), false);
      if (op1_const) {
        asm_code += InstGen::instConst(
            InstGen::cmp, InstGen::Reg(alu_op0),
            InstGen::Constant(op1_const->getValue() << shift));
      } else {
        asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
        asm_code += InstGen::cmp(InstGen::Reg(alu_op0),
                                 InstGen::RegShift(alu_op1, shift));
      }
      asm_code += InstGen::mov(InstGen::Reg(alu_ret), InstGen::Constant(0));
      asm_code +=
          InstGen::mov(InstGen::Reg(alu_ret), InstGen::Constant(1), asmCmpOp);
    } else {
      bool flag = false;
      switch (inst->getInstrType()) {
      case Instruction::VV:
        assert(ops.size() == 2 || ops.size() == 3);
        if (ops.size() == 2) {
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
          asm_code +=
              InstGen::vvmul(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                             InstGen::Reg(alu_op1),
                             static_cast<VVInst *>(inst)->getNumInt32());
        } else {
          if (alu_op0 != op_reg_0) {
            asm_code += CodeGen::assignToSpecificReg(ops.at(0), op_reg_0);
          }
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), op_reg_1);
          asm_code += CodeGen::assignToSpecificReg(ops.at(2), op_reg_2);
          asm_code += CodeGen::vv(InstGen::Reg(alu_ret));
        }
        break;
      case Instruction::Add:
        if (op1_const) {
          asm_code += InstGen::instConst(
              InstGen::add, InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
              InstGen::Constant(op1_const->getValue() << shift));
        } else {
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
          asm_code += InstGen::add(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                   InstGen::RegShift(alu_op1, shift));
        }
        break;
      case Instruction::Sub:
        if (op1_const) {
          asm_code += InstGen::instConst(
              InstGen::sub, InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
              InstGen::Constant(op1_const->getValue() << shift));
        } else {
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
          asm_code += InstGen::sub(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                   InstGen::RegShift(alu_op1, shift));
        }
        break;
      case Instruction::RSub:
        if (op1_const) {
          asm_code += InstGen::instConst(
              InstGen::rsb, InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
              InstGen::Constant(op1_const->getValue() << shift));
        } else {
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
          asm_code += InstGen::rsb(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                   InstGen::RegShift(alu_op1, shift));
        }
        break;
      case Instruction::Shl:
        if (op1_const) {
          asm_code += InstGen::instConst(
              InstGen::lsl, InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
              InstGen::Constant(op1_const->getValue()));
        } else {
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
          asm_code += InstGen::lsl(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                   InstGen::Reg(alu_op1));
        }
        break;
      case Instruction::AShr:
        if (op1_const) {
          asm_code += InstGen::instConst(
              InstGen::asr, InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
              InstGen::Constant(op1_const->getValue()));
        } else {
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
          asm_code += InstGen::asr(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                   InstGen::Reg(alu_op1));
        }
        break;
      case Instruction::LShr:
        if (op1_const) {
          asm_code += InstGen::instConst(
              InstGen::lsr, InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
              InstGen::Constant(op1_const->getValue()));
        } else {
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
          asm_code += InstGen::lsr(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                   InstGen::Reg(alu_op1));
        }
        break;
      case Instruction::And:
        if (op1_const) {
          asm_code += InstGen::instConst(
              InstGen::and_, InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
              InstGen::Constant(op1_const->getValue() << shift));
        } else {
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
          asm_code +=
              InstGen::and_(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                            InstGen::RegShift(alu_op1, shift));
        }
        break;
      case Instruction::Or:
        if (op1_const) {
          asm_code += InstGen::instConst(
              InstGen::orr, InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
              InstGen::Constant(op1_const->getValue() << shift));
        } else {
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
          asm_code += InstGen::orr(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                   InstGen::RegShift(alu_op1, shift));
        }
        break;
      case Instruction::Mul:
        if (!op1_const || disable_div_optimization) {
          asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
          asm_code += InstGen::mul(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                   InstGen::Reg(alu_op1));
        } else {
          const int mp = op1_const->getValue();
          // add sub rsb lsl inst * 1
          {
            for (int inst_1 = 0; inst_1 < 4; inst_1++) {
              for (int lsl_1 = 0; lsl_1 < 32; lsl_1++) {
                int x = 1;
                int y;
                if (inst_1 == 0) {
                  y = (x + (x << lsl_1));
                }
                if (inst_1 == 1) {
                  y = (x - (x << lsl_1));
                }
                if (inst_1 == 2) {
                  y = ((x << lsl_1) - x);
                }
                if (inst_1 == 3) {
                  y = (x << lsl_1);
                }
                if (y == mp) {
                  if (inst_1 == 0) {
                    asm_code += InstGen::add(InstGen::Reg(alu_ret),
                                             InstGen::Reg(alu_op0),
                                             InstGen::RegShift(alu_op0, lsl_1));
                  }
                  if (inst_1 == 1) {
                    asm_code += InstGen::sub(InstGen::Reg(alu_ret),
                                             InstGen::Reg(alu_op0),
                                             InstGen::RegShift(alu_op0, lsl_1));
                  }
                  if (inst_1 == 2) {
                    asm_code += InstGen::rsb(InstGen::Reg(alu_ret),
                                             InstGen::Reg(alu_op0),
                                             InstGen::RegShift(alu_op0, lsl_1));
                  }
                  if (inst_1 == 3) {
                    asm_code += InstGen::lsl(InstGen::Reg(alu_ret),
                                             InstGen::Reg(alu_op0),
                                             InstGen::Constant(lsl_1));
                  }
                  goto mul_end;
                }
              }
            }
          }
          // add sub rsb lsl inst * 2
          {
            for (int inst_1 = 0; inst_1 < 4; inst_1++) {
              for (int lsl_1 = 0; lsl_1 < 32; lsl_1++) {
                for (int inst_2 = 0; inst_2 < 4; inst_2++) {
                  for (int lsl_2 = 0; lsl_2 < 32; lsl_2++) {
                    for (int i2o1 = 0; i2o1 < 2; i2o1++) {
                      for (int i2o2 = 0; i2o2 < 2; i2o2++) {
                        int x = 1;
                        int y;
                        int z;
                        if (inst_1 == 0) {
                          y = (x + (x << lsl_1));
                        }
                        if (inst_1 == 1) {
                          y = (x - (x << lsl_1));
                        }
                        if (inst_1 == 2) {
                          y = ((x << lsl_1) - x);
                        }
                        if (inst_1 == 3) {
                          y = (x << lsl_1);
                        }
                        int o1 = i2o1 == 0 ? x : y;
                        int o2 = i2o2 == 0 ? x : y;
                        if (inst_2 == 0) {
                          z = (o1 + (o2 << lsl_2));
                        }
                        if (inst_2 == 1) {
                          z = (o1 - (o2 << lsl_2));
                        }
                        if (inst_2 == 2) {
                          z = ((o2 << lsl_2) - o1);
                        }
                        if (inst_2 == 3) {
                          z = (o1 << lsl_2);
                        }
                        if (z == mp) {
                          if (inst_1 == 0) {
                            asm_code += InstGen::add(
                                InstGen::Reg(op_reg_2), InstGen::Reg(alu_op0),
                                InstGen::RegShift(alu_op0, lsl_1));
                          }
                          if (inst_1 == 1) {
                            asm_code += InstGen::sub(
                                InstGen::Reg(op_reg_2), InstGen::Reg(alu_op0),
                                InstGen::RegShift(alu_op0, lsl_1));
                          }
                          if (inst_1 == 2) {
                            asm_code += InstGen::rsb(
                                InstGen::Reg(op_reg_2), InstGen::Reg(alu_op0),
                                InstGen::RegShift(alu_op0, lsl_1));
                          }
                          if (inst_1 == 3) {
                            asm_code += InstGen::lsl(InstGen::Reg(op_reg_2),
                                                     InstGen::Reg(alu_op0),
                                                     InstGen::Constant(lsl_1));
                          }
                          int r1 = i2o1 == 0 ? alu_op0 : op_reg_2;
                          int r2 = i2o2 == 0 ? alu_op0 : op_reg_2;
                          if (inst_2 == 0) {
                            asm_code += InstGen::add(
                                InstGen::Reg(alu_ret), InstGen::Reg(r1),
                                InstGen::RegShift(r2, lsl_2));
                          }
                          if (inst_2 == 1) {
                            asm_code += InstGen::sub(
                                InstGen::Reg(alu_ret), InstGen::Reg(r1),
                                InstGen::RegShift(r2, lsl_2));
                          }
                          if (inst_2 == 2) {
                            asm_code += InstGen::rsb(
                                InstGen::Reg(alu_ret), InstGen::Reg(r1),
                                InstGen::RegShift(r2, lsl_2));
                          }
                          if (inst_2 == 3) {
                            asm_code += InstGen::lsl(InstGen::Reg(alu_ret),
                                                     InstGen::Reg(r1),
                                                     InstGen::Constant(lsl_2));
                          }
                          goto mul_end;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          // fallback to mul
          {
            asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
            asm_code +=
                InstGen::mul(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                             InstGen::Reg(alu_op1));
            goto mul_end;
          }
        }
      mul_end:
        break;
      case Instruction::Div:
        if (op1_const && !disable_div_optimization) {
          asm_code +=
              InstGen::divConst(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                InstGen::Constant(op1_const->getValue()));
        } else {
          if (arch_version >= 8) {
            asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
            asm_code +=
                InstGen::sdiv(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                              InstGen::Reg(alu_op1));
          } else {
            asm_code +=
                CodeGen::generateFunctionCall(inst, "__aeabi_idiv", ops, 0);
          }
        }
        break;
      case Instruction::Rem:
        asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
        if (arch_version >= 8) {
          asm_code +=
              InstGen::sdiv(InstGen::Reg(op_reg_2), InstGen::Reg(alu_op0),
                            InstGen::Reg(alu_op1));
          asm_code +=
              InstGen::mul(InstGen::Reg(op_reg_2), InstGen::Reg(op_reg_2),
                           InstGen::Reg(alu_op1));
          asm_code += InstGen::sub(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                   InstGen::Reg(op_reg_2));
        } else {
          asm_code +=
              CodeGen::generateFunctionCall(inst, "__aeabi_idivmod", ops, 1);
        }
        break;
      case Instruction::Neg:
        asm_code += InstGen::rsb(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                 InstGen::Constant(0));
        break;
      case Instruction::Not:
        if (inst->getType()->isInt1()) {
          asm_code += InstGen::eor(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                   InstGen::Constant(1));
        } else {
          asm_code +=
              InstGen::mvn(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0));
        }
        break;
      case Instruction::ZExt:
        asm_code += InstGen::mov(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0));
        break;
      case Instruction::GEP:
        asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
        asm_code += InstGen::setValue(
            InstGen::Reg(op_reg_2),
            InstGen::Constant(
                inst->getType()->getPointerElementType()->getSize()));
        asm_code += InstGen::mul(InstGen::Reg(op_reg_2), InstGen::Reg(op_reg_2),
                                 InstGen::Reg(alu_op1));
        asm_code += InstGen::add(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                                 InstGen::Reg(op_reg_2));
        break;
      default:
        std::cerr << "???" << std::endl;
        abort();
        break;
      }
    }
    asm_code += CodeGen::getFromSpecificReg(inst, alu_ret);
  } else if (CodeGen::is_mt_inst(inst)) {
    std::cerr << "Please use Call __mtstart / __mtend instead" << std::endl;
    abort();
  } else if (inst->getInstrType() == Instruction::Br) {
    auto inst_br = dynamic_cast<BranchInst *>(inst);
    const bool is_cond = inst_br->isCondBr();
    const bool is_cmp = inst_br->isCmpBr();
    const int i_start = is_cmp ? 2 : (is_cond ? 1 : 0);
    const int i_end = is_cmp ? 3 : (is_cond ? 2 : 0);
    bool swap_branch = false;
    std::map<int, bool> need_resolve;
    std::map<int, bool> need_jump;
    BasicBlock *bb_cur = inst->getParent();
    // need phi resolve?
    {
      int branch_cnt = 0;
      BasicBlock *succ_bb = nullptr;
      bool flag = false;
      for (auto &for_bb : inst->getFunction()->getBasicBlocks()) {
        if (flag) {
          succ_bb = for_bb;
          break;
        }
        if (for_bb == bb_cur) {
          flag = true;
        }
      }
      for (int i = i_start; i <= i_end; i++) {
        std::vector<std::pair<Value *, Value *>> phis;
        auto bb_next = static_cast<BasicBlock *>(ops.at(i));
        for (auto &inst_phi : bb_next->getInstructions()) {
          if (inst_phi->isPHI()) {
            Value *pre_op = nullptr;
            for (auto &op_phi : inst_phi->getOperands()) {
              if (dynamic_cast<BasicBlock *>(op_phi) &&
                  dynamic_cast<BasicBlock *>(op_phi)->getTerminator() == inst) {
                phis.push_back({inst_phi, pre_op});
              }
              pre_op = op_phi;
            }
          }
        }
        std::vector<Value *> target, source;
        for (auto &p : phis) {
          target.push_back(p.first);
          source.push_back(p.second);
        }
        branch_cnt++;
        need_resolve[branch_cnt] =
            (!CodeGen::virtualRegMove(target, source).empty());
        need_jump[branch_cnt] = (succ_bb != bb_next);
      }
    }
    // swap?
    {
      if ((is_cmp || is_cond) &&
          ((!need_resolve[1] && !need_resolve[2] && !need_jump[2]) ||
           (!need_resolve[1] && need_resolve[2]) ||
           (need_resolve[1] && need_resolve[2] && !need_jump[1]))) {
        std::swap(ops[i_start], ops[i_end]);
        std::swap(need_resolve[1], need_resolve[2]);
        std::swap(need_jump[1], need_jump[2]);
        swap_branch = true;
      }
    }
    // translate Br inst
    std::map<int, bool> elim;
    if (is_cmp) {
      ConstantInt *op1_const = (ops.size() >= 2)
                                   ? (dynamic_cast<ConstantInt *>(ops.at(1)))
                                   : nullptr;
      int alu_op0 = this->register_mapping.count(ops.at(0))
                        ? this->register_mapping.at(ops.at(0))
                        : op_reg_0;
      int alu_op1 = this->register_mapping.count(ops.at(1))
                        ? this->register_mapping.at(ops.at(1))
                        : op_reg_1;
      asm_code += CodeGen::assignToSpecificReg(ops.at(0), alu_op0);
      InstGen::CmpOp asmCmpOp =
          CodeGen::cmpConvert(inst_br->getCmpOp(), true ^ swap_branch);
      if (op1_const) {
        asm_code +=
            InstGen::instConst(InstGen::cmp, InstGen::Reg(alu_op0),
                               InstGen::Constant(op1_const->getValue()));
      } else {
        asm_code += CodeGen::assignToSpecificReg(ops.at(1), alu_op1);
        asm_code += InstGen::cmp(InstGen::Reg(alu_op0), InstGen::Reg(alu_op1));
      }
      if (need_resolve[2]) {
        asm_code +=
            InstGen::b(InstGen::Label(CodeGen::getLabelName(bb_cur) +
                                          "_branch_" + std::to_string(2),
                                      0),
                       asmCmpOp);
      } else {
        auto bb_next = static_cast<BasicBlock *>(ops.at(i_end));
        asm_code += InstGen::b(
            InstGen::Label(CodeGen::getLabelName(bb_next), 0), asmCmpOp);
        elim[2] = true;
      }
    } else if (is_cond) {
      InstGen::CmpOp asmCmpOp =
          CodeGen::cmpConvert(CmpInst::CmpOp::EQ, false ^ swap_branch);
      int alu_op0 = this->register_mapping.count(ops.at(0))
                        ? this->register_mapping.at(ops.at(0))
                        : op_reg_0;
      asm_code += CodeGen::assignToSpecificReg(ops.at(0), alu_op0);
      asm_code += InstGen::cmp(InstGen::Reg(alu_op0), InstGen::Constant(0));
      if (need_resolve[2]) {
        asm_code +=
            InstGen::b(InstGen::Label(CodeGen::getLabelName(bb_cur) +
                                          "_branch_" + std::to_string(2),
                                      0),
                       asmCmpOp);
      } else {
        auto bb_next = static_cast<BasicBlock *>(ops.at(i_end));
        asm_code += InstGen::b(
            InstGen::Label(CodeGen::getLabelName(bb_next), 0), asmCmpOp);
        elim[2] = true;
      }
    }
    // for each branch
    {
      int branch_cnt = 0;
      for (int i = i_start; i <= i_end; i++) {
        std::vector<std::pair<Value *, Value *>> phis;
        auto bb_next = static_cast<BasicBlock *>(ops.at(i));
        for (auto &inst_phi : bb_next->getInstructions()) {
          if (inst_phi->isPHI()) {
            Value *pre_op = nullptr;
            for (auto &op_phi : inst_phi->getOperands()) {
              if (dynamic_cast<BasicBlock *>(op_phi) &&
                  dynamic_cast<BasicBlock *>(op_phi)->getTerminator() == inst) {
                phis.push_back({inst_phi, pre_op});
              }
              pre_op = op_phi;
            }
          }
        }
        asm_code += CodeGen::getLabelName(bb_cur) + "_branch_" +
                    std::to_string(++branch_cnt) + ":" + InstGen::newline;
        std::vector<Value *> target, source;
        for (auto &p : phis) {
          asm_code +=
              CodeGen::comment("%" + p.first->getName() + " <= " +
                               (dynamic_cast<Constant *>(p.second) ? "" : "%") +
                               p.second->getName());
          target.push_back(p.first);
          source.push_back(p.second);
        }
        if (need_resolve[branch_cnt]) {
          CodeGen::comment("PHI resolve code");
          asm_code += CodeGen::virtualRegMove(target, source);
        }
        if ((branch_cnt == 1 && !need_jump[1] && !need_resolve[2] &&
             (!need_jump[2] || elim[2])) ||
            (branch_cnt == 2 && (!need_jump[2] || elim[2]))) {
          asm_code += CodeGen::comment("branch instruction eliminated");
        } else {
          asm_code +=
              InstGen::b(InstGen::Label(CodeGen::getLabelName(bb_next), 0),
                         InstGen::CmpOp::NOP);
        }
      }
    }
  } else if (inst->getInstrType() == Instruction::PHI) {
  } else if (inst->getInstrType() == Instruction::AddAddr) {
    std::cerr << "IR instruction AddAddr is deprecated" << std::endl;
    abort();
  } else if (inst->getInstrType() == Instruction::BIC) {
    std::cerr << "IR instruction BIC is deprecated" << std::endl;
    abort();
    assert(this->register_mapping.count(ops.at(2)));
    int alu_op0 = this->register_mapping.count(ops.at(0))
                      ? this->register_mapping.at(ops.at(0))
                      : op_reg_0;
    int alu_op1 = this->register_mapping.count(ops.at(1))
                      ? this->register_mapping.at(ops.at(1))
                      : op_reg_1;
    int alu_op2 = this->register_mapping.count(ops.at(2))
                      ? this->register_mapping.at(ops.at(2))
                      : op_reg_2;
    int alu_ret = this->register_mapping.count(inst)
                      ? this->register_mapping.at(inst)
                      : op_reg_0;
    asm_code += assignToSpecificReg(ops.at(0), alu_op0);
    asm_code += assignToSpecificReg(ops.at(1), alu_op1);
    asm_code += assignToSpecificReg(ops.at(2), alu_op2);
    asm_code += InstGen::bic(InstGen::Reg(alu_ret), InstGen::Reg(alu_op0),
                             InstGen::Reg(alu_op1), InstGen::Reg(alu_op2));
    asm_code += getFromSpecificReg(inst, alu_ret);
  } else {
    std::cerr << "Cannot translate this function:" << std::endl;
    inst->getFunction()->print();
    std::cerr << std::endl;
    std::cerr << "Cannot translate this instruction:" << std::endl;
    inst->print();
    std::cerr << std::endl;
    abort();
  }
  return asm_code;
}

std::string CodeGen::generateFunctionCall(Instruction *inst,
                                          std::string func_name,
                                          std::vector<Value *> ops,
                                          int return_reg, int sp_ofs) {
  std::string asm_code;
  assert(-1 <= return_reg && return_reg <= 3);
  auto in_func = inst->getFunction();
  auto save_registers = CodeGen::getCallerSaveRegisters(in_func);
  if (this->context_active_vars.count(inst)) {
    // std::cerr << "Using ABI optimization" << std::endl;
    std::set<int> regs_set;
    for (auto &v : this->context_active_vars.at(inst)) {
      if (this->register_mapping.count(v)) {
        regs_set.insert(this->register_mapping.at(v));
      }
    }
    save_registers.clear();
    for (auto &r : regs_set) {
      save_registers.push_back(InstGen::Reg(r));
    }
  }
  bool has_return_value = (return_reg >= 0) && (inst->getType()->getSize() > 0);
  // do not save returned register
  if (has_return_value && this->register_mapping.count(inst)) {
    auto returned_reg = InstGen::Reg(this->register_mapping.at(inst));
    decltype(save_registers) new_save_registers;
    for (auto &reg : save_registers) {
      if (reg.getID() != returned_reg.getID()) {
        new_save_registers.push_back(reg);
      }
    }
    save_registers = new_save_registers;
  }
  std::sort(save_registers.begin(), save_registers.end());
  if (!save_registers.empty()) {
    asm_code += InstGen::push(save_registers);
  }
  int saved_regs_size = save_registers.size() * 4;
  int total_args_size = 0;
  // prepare args
  {
    std::vector<Value *> source, target;
    Type dummy_type(Type::IntegerTyID);
    std::vector<Value> dummys(ops.size(), Value(&dummy_type));
    // args 0 1 2 3
    {
      for (int i = 0; i < std::min(ops.size(), (size_t)4); i++) {
        auto dummy = &dummys.at(i);
        this->register_mapping[dummy] = i;
        this->stack_mapping.erase(dummy);
        target.push_back(dummy);
        source.push_back(ops.at(i));
      }
    }
    // args 4+
    {
      for (int i = ops.size() - 1; i >= 4; i--) {
        total_args_size += 4;
        auto dummy = &dummys.at(i);
        this->register_mapping.erase(dummy);
        this->stack_mapping[dummy] = -(total_args_size + saved_regs_size);
        target.push_back(dummy);
        source.push_back(ops.at(i));
      }
    }
    asm_code +=
        CodeGen::virtualRegMove(target, source, saved_regs_size + sp_ofs);
  }
  asm_code += InstGen::instConst(InstGen::sub, InstGen::sp, InstGen::sp,
                                 InstGen::Constant(total_args_size));
  asm_code += InstGen::bl(func_name);
  asm_code += InstGen::instConst(InstGen::add, InstGen::sp, InstGen::sp,
                                 InstGen::Constant(total_args_size));
  if (has_return_value) {
    asm_code +=
        CodeGen::getFromSpecificReg(inst, return_reg, saved_regs_size + sp_ofs);
  }
  if (!save_registers.empty()) {
    asm_code += InstGen::pop(save_registers);
  }
  return asm_code;
}

std::vector<InstGen::Reg> CodeGen::getAllRegisters(Function *func) {
  std::set<InstGen::Reg> registers;
  for (auto &arg : func->getArgs()) {
    if (this->register_mapping.count(arg) &&
        this->register_mapping.at(arg) <= InstGen::max_reg_id) {
      registers.insert(InstGen::Reg(this->register_mapping.at(arg)));
    }
  }
  for (auto &bb : func->getBasicBlocks()) {
    for (auto &inst : bb->getInstructions()) {
      if (this->register_mapping.count(inst) &&
          this->register_mapping.at(inst) <= InstGen::max_reg_id) {
        registers.insert(InstGen::Reg(this->register_mapping.at(inst)));
      }
    }
  }
  for (auto &reg : temp_regs) { // used as temp regs
    registers.insert(reg);
  }
  return std::vector<InstGen::Reg>(registers.begin(), registers.end());
}

std::vector<InstGen::Reg> CodeGen::getCalleeSaveRegisters(Function *func) {
  std::set<InstGen::Reg> registers;
  for (auto &reg : CodeGen::getAllRegisters(func)) {
    if (callee_save_regs.count(reg)) {
      registers.insert(reg);
    }
  }
  return std::vector<InstGen::Reg>(registers.begin(), registers.end());
}

std::vector<InstGen::Reg> CodeGen::getCallerSaveRegisters(Function *func) {
  std::set<InstGen::Reg> registers;
  for (auto &reg : CodeGen::getAllRegisters(func)) {
    if (caller_save_regs.count(reg) && !temp_regs.count(reg)) {
      registers.insert(reg);
    }
  }
  return std::vector<InstGen::Reg>(registers.begin(), registers.end());
}

void CodeGen::allocateStackSpace(Function *func) {
  this->stack_size = 0;
  this->thread_stack_bits = 0;
  this->thread_stack_size = 0;
  this->stack_mapping.clear();
  this->mt_enabled = CodeGen::is_mt_inside(func);
  if (this->mt_enabled) {
    // set thread id reg
    for (auto &bb : func->getBasicBlocks()) {
      for (auto &inst : bb->getInstructions()) {
        if (CodeGen::is_mt_start(inst)) {
          this->register_mapping[inst] = thread_id_reg;
        }
        if (CodeGen::is_mt_end(inst)) {
          if (inst->isCall()) {
            this->register_mapping[inst->getOperands().at(1)] = thread_id_reg;
          }
        }
      }
    }
  }
  for (auto &arg : func->getArgs()) {
    bool extended = false;
    auto sizeof_val = arg->getType()->getSize(extended);
    sizeof_val = ((sizeof_val + 3) / 4) * 4;
    assert(sizeof_val == 4);
    if (!register_mapping.count(arg)) {
      stack_mapping[arg] = this->stack_size;
      this->stack_size += sizeof_val;
    }
  }
  // non alloca space and non alloca pointer
  for (auto &bb : func->getBasicBlocks()) {
    for (auto &inst : bb->getInstructions()) {
      if (this->register_mapping.count(inst)) {
        int map_reg_id = this->register_mapping.at(inst);
        if (map_reg_id > InstGen::max_reg_id) {
          register_mapping.erase(inst);
        }
        if (!allocate_regs.count(InstGen::Reg(map_reg_id))) {
          std::cerr << "Reg " << map_reg_id << " should not be allocated"
                    << std::endl;
          abort();
        }
      }
      if (this->register_mapping.count(inst)) {
        continue;
      }
      if (this->stack_mapping.count(inst)) {
        continue;
      }
      if (inst->isAlloca()) {
        continue;
      }
      bool extended = false;
      auto sizeof_val = inst->getType()->getSize(extended);
      sizeof_val = ((sizeof_val + 3) / 4) * 4;
      if (sizeof_val > 0) {
        this->stack_mapping[inst] = this->stack_size;
        this->stack_size += sizeof_val;
      }
    }
  }
  this->thread_stack_bits = cache_line_bits;
  while ((1 << this->thread_stack_bits) < this->stack_size) {
    this->thread_stack_bits++;
  }
  this->thread_stack_size = 1 << this->thread_stack_bits;
  if (this->mt_enabled) {
    this->stack_size = this->thread_stack_size * mt_num_threads;
  } else {
    this->stack_size = this->thread_stack_size;
  }
  // alloca space
  for (auto &bb : func->getBasicBlocks()) {
    for (auto &inst : bb->getInstructions()) {
      if (!inst->isAlloca()) {
        continue;
      }
      bool extended = true;
      this->allocated.insert(inst);
      auto sizeof_val = inst->getType()->getSize(extended);
      sizeof_val = ((sizeof_val + 3) / 4) * 4;
      if (sizeof_val > 0) {
        this->stack_mapping[inst] = this->stack_size;
        this->stack_size += sizeof_val;
      }
    }
  }
}

bool CodeGen::isSameMapping(Value *a, Value *b) {
  if (this->register_mapping.count(a) && this->register_mapping.count(b)) {
    return this->register_mapping.at(a) == this->register_mapping.at(b);
  }
  if (this->stack_mapping.count(a) && this->stack_mapping.count(b)) {
    return this->stack_mapping.at(a) == this->stack_mapping.at(b);
  }
  return false;
}

std::string CodeGen::virtualRegMove(std::vector<Value *> target,
                                    std::vector<Value *> source, int sp_ofs) {
  std::string asm_code;
  assert(target.size() == source.size());
  int sz = target.size();
  std::list<std::pair<Value *, Value *>> L;
  for (int i = 0; i < sz; i++) {
    L.push_back({target.at(i), source.at(i)});
  }
  for (auto it = L.begin(); it != L.end(); it++) {
    for (auto it2 = L.begin(); it2 != L.end(); it2++) {
      if (it2 != it && CodeGen::isSameMapping(it2->first, it->first)) {
        std::cerr << "virtualRegMove race condition" << std::endl;
        abort();
      }
    }
  }
  Value *tg_val = nullptr;
  while (!L.empty()) {
    bool flag = true;
    for (auto it = L.begin(); it != L.end(); it++) {
      bool ok = true;
      for (auto it2 = L.begin(); it2 != L.end(); it2++) {
        if (it2 != it && CodeGen::isSameMapping(it2->second, it->first)) {
          ok = false;
        }
      }
      if (ok) {
        asm_code += CodeGen::virtualRegMove(it->first, it->second, sp_ofs);
        L.erase(it);
        flag = false;
        break;
      }
    }
    if (flag) {
      if (tg_val != nullptr) {
        asm_code += CodeGen::getFromSpecificReg(tg_val, op_reg_0, sp_ofs);
      }
      auto it = L.begin();
      asm_code += CodeGen::assignToSpecificReg(it->second, op_reg_0, sp_ofs);
      tg_val = it->first;
      L.erase(it);
    }
  }
  if (tg_val != nullptr) {
    asm_code += CodeGen::getFromSpecificReg(tg_val, op_reg_0, sp_ofs);
  }
  return asm_code;
}

std::string CodeGen::virtualRegMove(Value *target, Value *source, int sp_ofs) {
  std::string asm_code;
  if (CodeGen::isSameMapping(target, source)) {
    return asm_code;
  }
  int alu_op0 = this->register_mapping.count(target)
                    ? this->register_mapping.at(target)
                    : op_reg_0;
  asm_code += CodeGen::assignToSpecificReg(source, alu_op0, sp_ofs);
  asm_code += CodeGen::getFromSpecificReg(target, alu_op0, sp_ofs);
  return asm_code;
}

std::string CodeGen::assignToSpecificReg(Value *val, int target, int sp_ofs) {
  std::string asm_code;
  auto val_const = dynamic_cast<ConstantInt *>(val);
  auto val_global = dynamic_cast<GlobalVariable *>(val);
  if (val_const) {
    int imm = val_const->getValue();
    asm_code += InstGen::setValue(InstGen::Reg(target), InstGen::Constant(imm));
  } else if (val_global) { // need optimization
    asm_code += InstGen::getAddress(
        InstGen::Reg(op_reg_2),
        InstGen::Label(global_vars_label, CodeGen::queryGOT(val_global) * 4));
    asm_code += InstGen::ldr(InstGen::Reg(target),
                             InstGen::Addr(InstGen::Reg(op_reg_2), 0));
  } else if (register_mapping.count(val) &&
             register_mapping.at(val) <= InstGen::max_reg_id) {
    auto source = register_mapping.at(val);
    asm_code += InstGen::mov(InstGen::Reg(target), InstGen::Reg(source));
  } else if (allocated.count(val)) {
    auto offset = stack_mapping.at(val) + sp_ofs;
    asm_code += InstGen::instConst(InstGen::add, InstGen::Reg(target),
                                   InstGen::sp, InstGen::Constant(offset));
  } else if (stack_mapping.count(val)) {
    auto offset = stack_mapping.at(val) + sp_ofs;
    if ((this->cur_inst_mt_env != nullptr) &&
        (this->in_mt_env[val] == this->cur_inst_mt_env)) {
      asm_code +=
          InstGen::setValue(InstGen::Reg(op_reg_2), InstGen::Constant(offset));
      asm_code += InstGen::add(
          InstGen::Reg(op_reg_2), InstGen::Reg(op_reg_2),
          InstGen::RegShift(thread_id_reg, this->thread_stack_bits));
      asm_code += InstGen::ldr(InstGen::Reg(target), InstGen::sp,
                               InstGen::Reg(op_reg_2));
    } else {
      asm_code += InstGen::load(InstGen::Reg(target),
                                InstGen::Addr(InstGen::sp, offset));
    }
  } else {
    std::cerr << "Function assignToSpecificReg exception!" << std::endl;
    abort();
  }
  return asm_code;
}

std::string CodeGen::getFromSpecificReg(Value *val, int source, int sp_ofs) {
  exit_ifnot(_getFromSpecificReg_CodeGen, source != op_reg_2);
  std::string asm_code;
  if (register_mapping.count(val) &&
      register_mapping.at(val) <= InstGen::max_reg_id) {
    auto target = register_mapping.at(val);
    asm_code += InstGen::mov(InstGen::Reg(target), InstGen::Reg(source));
  } else if (stack_mapping.count(val)) {
    auto offset = stack_mapping.at(val) + sp_ofs;
    if ((this->cur_inst_mt_env != nullptr)) {
      asm_code +=
          InstGen::setValue(InstGen::Reg(op_reg_2), InstGen::Constant(offset));
      asm_code += InstGen::add(
          InstGen::Reg(op_reg_2), InstGen::Reg(op_reg_2),
          InstGen::RegShift(thread_id_reg, this->thread_stack_bits));
      asm_code += InstGen::str(InstGen::Reg(source), InstGen::sp,
                               InstGen::Reg(op_reg_2));
    } else {
      asm_code += InstGen::store(InstGen::Reg(source),
                                 InstGen::Addr(InstGen::sp, offset));
    }
  } else {
    std::cerr << "Function getFromSpecificReg exception!" << std::endl;
    abort();
  }
  return asm_code;
}

std::string CodeGen::generateGOT() {
  std::string asm_code;
  this->GOT.clear();
  for (auto &global_var : this->module->getGlobalVariables()) {
    int count = this->GOT.size();
    if (!GOT.count(global_var)) {
      this->GOT[global_var] = count;
    }
  }
  std::vector<Value *> vecGOT;
  vecGOT.resize(this->GOT.size());
  for (auto &i : GOT) {
    vecGOT[i.second] = i.first;
  }
  for (auto &i : vecGOT) {
    asm_code +=
        InstGen::spaces + ".global" + " " + i->getName() + InstGen::newline;
  }
  asm_code += global_vars_label + ":" + InstGen::newline;
  for (auto &i : vecGOT) {
    asm_code +=
        InstGen::spaces + ".long" + " " + i->getName() + InstGen::newline;
  }
  return asm_code;
}

int CodeGen::queryGOT(Value *val) { return this->GOT.at(val); }

std::string CodeGen::generateGlobalVarsCode() {
  std::string asm_code;
  for (auto &global_var : this->module->getGlobalVariables()) {
    asm_code += global_var->getName() + ":" + InstGen::newline;
    if (!global_var->getType()->getPointerElementType()->eq(
            *global_var->getOperands().at(0)->getType())) {
      asm_code += InstGen::spaces + ".zero" + " " +
                  std::to_string(global_var->getType()->getSize()) +
                  InstGen::newline;
    } else {
      asm_code += CodeGen::generateInitializerCode(
          static_cast<Constant *>(global_var->getOperands().at(0)));
    }
  }
  return asm_code;
}

std::string CodeGen::generateInitializerCode(Constant *init) {
  std::string asm_code;
  auto array_init = dynamic_cast<ConstantArray *>(init);
  if (array_init) {
    auto length =
        static_cast<ArrayType *>(array_init->getType())->getNumOfElements();
    for (int i = 0; i < length; i++) {
      asm_code +=
          CodeGen::generateInitializerCode(array_init->getElementValue(i));
    }
  } else {
    auto val = CodeGen::getConstIntVal(init);
    if (!val.second) {
      std::cerr << "Function generateInitializerCode exception!" << std::endl;
      abort();
    }
    asm_code += InstGen::spaces + ".long" + " " + std::to_string(val.first) +
                InstGen::newline;
  }
  return asm_code;
}

std::pair<int, bool> CodeGen::getConstIntVal(Value *val) { // disabled
  auto const_val = dynamic_cast<ConstantInt *>(val);
  auto inst_val = dynamic_cast<Instruction *>(val);
  if (const_val) {
    return std::make_pair(const_val->getValue(), true);
  } else if (inst_val && false) {
    auto op_list = inst_val->getOperands();
    if (dynamic_cast<BinaryInst *>(val)) {
      auto val_0 = CodeGen::getConstIntVal(op_list.at(0));
      auto val_1 = CodeGen::getConstIntVal(op_list.at(1));
      if (val_0.second && val_1.second) {
        int ret = 0;
        bool flag = true;
        switch (inst_val->getInstrType()) {
        case Instruction::Add:
          ret = val_0.first + val_1.first;
          break;
        case Instruction::Sub:
          ret = val_0.first - val_1.first;
          break;
        case Instruction::And:
          ret = val_0.first & val_1.first;
          break;
        case Instruction::Or:
          ret = val_0.first | val_1.first;
          break;
        case Instruction::Mul:
          ret = val_0.first * val_1.first;
          break;
        case Instruction::Div:
          ret = val_0.first / val_1.first;
          break;
        case Instruction::Rem:
          ret = val_0.first % val_1.first;
          break;
        default:
          flag = false;
          break;
        }
        return std::make_pair(ret, flag);
      } else {
        return std::make_pair(0, false);
      }
    }
    if (dynamic_cast<UnaryInst *>(val)) {
      auto val_0 = CodeGen::getConstIntVal(op_list.at(0));
      if (val_0.second) {
        int ret = 0;
        bool flag = true;
        switch (inst_val->getInstrType()) {
        case Instruction::Not:
          ret = !val_0.first;
          break;
        case Instruction::Neg:
          ret = -val_0.first;
          break;
        default:
          flag = false;
          break;
        }
        return std::make_pair(ret, flag);
      } else {
        return std::make_pair(0, false);
      }
    }
  }
  std::cerr << "Function getConstIntVal exception!" << std::endl;
  abort();
}

std::string CodeGen::comment(std::string s) {
  std::string asm_code;
  asm_code += InstGen::spaces + "@ " + s + InstGen::newline;
  return asm_code;
}

InstGen::CmpOp CodeGen::cmpConvert(CmpInst::CmpOp myCmpOp, bool reverse) {
  InstGen::CmpOp asmCmpOp;
  if (!reverse) {
    switch (myCmpOp) {
    case CmpInst::CmpOp::EQ:
      asmCmpOp = InstGen::CmpOp::EQ;
      break;
    case CmpInst::CmpOp::NE:
      asmCmpOp = InstGen::CmpOp::NE;
      break;
    case CmpInst::CmpOp::GT:
      asmCmpOp = InstGen::CmpOp::GT;
      break;
    case CmpInst::CmpOp::GE:
      asmCmpOp = InstGen::CmpOp::GE;
      break;
    case CmpInst::CmpOp::LT:
      asmCmpOp = InstGen::CmpOp::LT;
      break;
    case CmpInst::CmpOp::LE:
      asmCmpOp = InstGen::CmpOp::LE;
      break;
    default:
      std::cerr << "CmpOp type not valid" << std::endl;
      abort();
    }
  } else {
    switch (myCmpOp) {
    case CmpInst::CmpOp::EQ:
      asmCmpOp = InstGen::CmpOp::NE;
      break;
    case CmpInst::CmpOp::NE:
      asmCmpOp = InstGen::CmpOp::EQ;
      break;
    case CmpInst::CmpOp::GT:
      asmCmpOp = InstGen::CmpOp::LE;
      break;
    case CmpInst::CmpOp::GE:
      asmCmpOp = InstGen::CmpOp::LT;
      break;
    case CmpInst::CmpOp::LT:
      asmCmpOp = InstGen::CmpOp::GE;
      break;
    case CmpInst::CmpOp::LE:
      asmCmpOp = InstGen::CmpOp::GT;
      break;
    default:
      std::cerr << "CmpOp type not valid" << std::endl;
      abort();
    }
  }
  return asmCmpOp;
}

bool CodeGen::is_mt_inside(Function *func) {
  for (auto &bb : func->getBasicBlocks()) {
    for (auto &inst : bb->getInstructions()) {
      if (inst->getInstrType() == Instruction::MTSTART ||
          (inst->getInstrType() == Instruction::Call &&
           inst->getOperands().front()->getName() == "__mtstart")) {
        return true;
      }
      if (inst->getInstrType() == Instruction::MTEND ||
          (inst->getInstrType() == Instruction::Call &&
           inst->getOperands().front()->getName() == "__mtend")) {
        return true;
      }
    }
  }
  return false;
}

bool CodeGen::is_mt_inst(Value *val) {
  if (CodeGen::is_mt_start(val)) {
    return true;
  }
  if (CodeGen::is_mt_end(val)) {
    return true;
  }
  return false;
}

bool CodeGen::is_mt_start(Value *val) {
  auto inst = dynamic_cast<Instruction *>(val);
  if (inst && (inst->getInstrType() == Instruction::MTSTART ||
               (inst->getInstrType() == Instruction::Call &&
                inst->getOperands().at(0)->getName() == "__mtstart"))) {
    return true;
  }
  return false;
}

bool CodeGen::is_mt_end(Value *val) {
  auto inst = dynamic_cast<Instruction *>(val);
  if (inst && (inst->getInstrType() == Instruction::MTEND ||
               (inst->getInstrType() == Instruction::Call &&
                inst->getOperands().at(0)->getName() == "__mtend"))) {
    return true;
  }
  return false;
}

std::string CodeGen::mtstart() { // tid = __mtstart();
  std::string asm_code;
  std::string my_name = "__mtstart";
  asm_code += my_name + ":" + InstGen::newline;
  asm_code += InstGen::instConst(InstGen::sub, InstGen::sp, InstGen::sp,
                                 InstGen::Constant(1 << 24));
  asm_code += InstGen::push(
      {InstGen::Reg(0), InstGen::Reg(1), InstGen::Reg(2), InstGen::Reg(3)});
  asm_code += InstGen::mov(InstGen::Reg(3), InstGen::Reg(7));
  asm_code += InstGen::mov(InstGen::Reg(2), InstGen::Constant(mt_num_threads));
  asm_code += "." + my_name + "_1:" + InstGen::newline;
  asm_code +=
      InstGen::sub(InstGen::Reg(2), InstGen::Reg(2), InstGen::Constant(1));
  asm_code += InstGen::cmp(InstGen::Reg(2), InstGen::Constant(0));
  asm_code +=
      InstGen::b(InstGen::Label("." + my_name + "_2"), InstGen::CmpOp::EQ);
  // syscall clone start
  asm_code += InstGen::setValue(InstGen::Reg(7), InstGen::Constant(120));
  asm_code += InstGen::setValue(InstGen::Reg(0), InstGen::Constant(clone_flag));
  asm_code += InstGen::mov(InstGen::Reg(1), InstGen::sp);
  asm_code += InstGen::swi(InstGen::Constant(0));
  // syscall clone end
  asm_code += InstGen::cmp(InstGen::Reg(0), InstGen::Constant(0));
  asm_code +=
      InstGen::b(InstGen::Label("." + my_name + "_1"), InstGen::CmpOp::NE);
  asm_code += "." + my_name + "_2:" + InstGen::newline;
  // return
  asm_code += InstGen::mov(InstGen::Reg(thread_id_reg), InstGen::Reg(2));
  asm_code += InstGen::mov(InstGen::Reg(7), InstGen::Reg(3));
  asm_code += InstGen::pop(
      {InstGen::Reg(0), InstGen::Reg(1), InstGen::Reg(2), InstGen::Reg(3)});
  asm_code += InstGen::instConst(InstGen::add, InstGen::sp, InstGen::sp,
                                 InstGen::Constant(1 << 24));
  asm_code += InstGen::mov(InstGen::pc, InstGen::lr);
  return asm_code;
}

std::string CodeGen::mtend() {    // __mtend(tid);
  const int saved_args_size = 64; // avoid overwriting pushed args in __mtstart
  std::string asm_code;
  std::string my_name = "__mtend";
  asm_code += my_name + ":" + InstGen::newline;
  asm_code += InstGen::cmp(InstGen::Reg(thread_id_reg), InstGen::Constant(0));
  asm_code +=
      InstGen::b(InstGen::Label("." + my_name + "_2"), InstGen::CmpOp::EQ);
  asm_code += "." + my_name + "_1:" + InstGen::newline;
  // syscall exit start
  asm_code += InstGen::setValue(InstGen::Reg(7), InstGen::Constant(1));
  asm_code += InstGen::swi(InstGen::Constant(0));
  // syscall exit end
  asm_code += "." + my_name + "_2:" + InstGen::newline;
  asm_code += InstGen::push(
      {InstGen::Reg(0), InstGen::Reg(1), InstGen::Reg(2), InstGen::Reg(3)});
  asm_code += InstGen::mov(InstGen::Reg(1), InstGen::Constant(mt_num_threads));
  asm_code += "." + my_name + "_3:" + InstGen::newline;
  asm_code +=
      InstGen::sub(InstGen::Reg(1), InstGen::Reg(1), InstGen::Constant(1));
  asm_code += InstGen::cmp(InstGen::Reg(1), InstGen::Constant(0));
  asm_code +=
      InstGen::b(InstGen::Label("." + my_name + "_4"), InstGen::CmpOp::EQ);
  asm_code += InstGen::push({InstGen::Reg(1), InstGen::lr});
  asm_code += InstGen::sub(InstGen::sp, InstGen::sp, InstGen::Constant(4));
  asm_code += InstGen::mov(InstGen::Reg(0), InstGen::sp);
  asm_code += InstGen::bl("wait");
  asm_code += InstGen::add(InstGen::sp, InstGen::sp, InstGen::Constant(4));
  asm_code += InstGen::pop({InstGen::Reg(1), InstGen::lr});
  asm_code += InstGen::b(InstGen::Label("." + my_name + "_3"));
  asm_code += "." + my_name + "_4:" + InstGen::newline;
  asm_code += InstGen::pop(
      {InstGen::Reg(0), InstGen::Reg(1), InstGen::Reg(2), InstGen::Reg(3)});
  asm_code += InstGen::mov(InstGen::Reg(thread_id_reg), InstGen::Constant(0));
  asm_code += InstGen::mov(InstGen::pc, InstGen::lr);
  return asm_code;
}

// addr: op_reg_0 op_reg_1  len: op_reg_2
std::string CodeGen::vv(const InstGen::Reg &ret) {
  static int counter;
  std::string asm_code;
  std::string my_name = "__vv_neon_" + std::to_string(counter++);
  asm_code += InstGen::add(InstGen::Reg(op_reg_2), InstGen::Reg(op_reg_0),
                           InstGen::RegShift(op_reg_2, 2));
  asm_code +=
      InstGen::spaces + "vsub.i32 d0, d0, d0" + InstGen::newline; // set zero
  asm_code += my_name + ":" + InstGen::newline;
  asm_code += InstGen::spaces + "vld2.32 {d24-d27}, [" +
              InstGen::Reg(op_reg_0).getName() + "]!" + InstGen::newline;
  asm_code += InstGen::spaces + "vld2.32 {d20-d23}, [" +
              InstGen::Reg(op_reg_1).getName() + "]!" + InstGen::newline;
  asm_code += InstGen::spaces + "vmul.i32 q8, q12, q10" + InstGen::newline;
  asm_code += InstGen::spaces + "vmla.i32 q8, q13, q11" + InstGen::newline;
  asm_code += InstGen::spaces + "vadd.i32 d16, d17, d16" + InstGen::newline;
  asm_code += InstGen::spaces + "vpadd.i32 d16, d16, d16" + InstGen::newline;
  asm_code += InstGen::spaces + "vadd.i32 d0, d0, d16" + InstGen::newline;
  asm_code += InstGen::cmp(InstGen::Reg(op_reg_0), InstGen::Reg(op_reg_2));
  asm_code += InstGen::b(InstGen::Label(my_name), InstGen::CmpOp::LT);
  asm_code += InstGen::spaces + "vmov.32 " + InstGen::Reg(ret).getName() +
              ", d0[0]" + InstGen::newline; // get sum
  return asm_code;
}