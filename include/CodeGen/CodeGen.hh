#ifndef SYSYC_CODEGEN_H
#define SYSYC_CODEGEN_H

#include <linux/sched.h>
#include <sys/wait.h>

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "GlobalVariable.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "InstructionsGen.hh"
#include "Module.h"
#include "Type.h"
#include "User.h"
#include "Value.h"

const std::string global_vars_label = ".global_vars";
const int arch_version = 8;
const bool enlarge_stack = true;
const int enlarge_stack_size = 256 * (1 << 20); // 256MB

const int op_reg_0 = 12;
const int op_reg_1 = 14; // lr
const int op_reg_2 = vinst_temp_reg.getID();

const std::set<InstGen::Reg> caller_save_regs = {
    InstGen::Reg(0), InstGen::Reg(1),  InstGen::Reg(2),
    InstGen::Reg(3), InstGen::Reg(12), InstGen::Reg(14)};
const std::set<InstGen::Reg> callee_save_regs = {
    InstGen::Reg(4), InstGen::Reg(5), InstGen::Reg(6),  InstGen::Reg(7),
    InstGen::Reg(8), InstGen::Reg(9), InstGen::Reg(10), InstGen::Reg(11)};
const std::set<InstGen::Reg> allocate_regs = {
    InstGen::Reg(0), InstGen::Reg(1), InstGen::Reg(2), InstGen::Reg(3),
    InstGen::Reg(4), InstGen::Reg(5), InstGen::Reg(6), InstGen::Reg(7),
    InstGen::Reg(8), InstGen::Reg(9), InstGen::Reg(10)};
const std::set<InstGen::Reg> temp_regs = {
    InstGen::Reg(op_reg_0), InstGen::Reg(op_reg_1), InstGen::Reg(op_reg_2)};

const int cache_line_bits = 7;
const int cache_line_size = 1 << cache_line_bits;
const int mt_num_threads = 4;

const int L1_cache_size = 32 * (1 << 10); // 32KiB per core
const int L2_cache_size = 1 * (1 << 20);  // 1MB shared

const int thread_id_reg = 10;

const int clone_flag = CLONE_VM | SIGCHLD;

class CodeGen {
private:
  std::shared_ptr<Module> module;
  std::map<Value *, int> register_mapping;
  std::map<Value *, int> stack_mapping;
  std::set<Value *> allocated;
  std::map<Value *, int> GOT;
  std::map<Instruction *, std::set<Value *>> context_active_vars;
  int stack_size;
  int thread_stack_bits;
  int thread_stack_size;
  bool debug;
  double spill_cost_total;
  double color_bonus_total;
  bool mt_enabled;
  std::map<Value *, Value *> in_mt_env;
  Value *cur_inst_mt_env;

public:
  CodeGen(std::shared_ptr<Module> module, bool debug = false) {
    this->module = module;
    this->debug = debug;
  }
  ~CodeGen() {}
  std::string generateModuleCode(std::map<Value *, int> register_mapping);
  std::string generateModuleCode(bool autoAlloc) {
    return generateModuleCode(autoAlloc ? regAlloc()
                                        : std::map<Value *, int>());
  }
  std::string generateFunctionCode(Function *func);
  void findMTVars(Function *func);
  std::string generateFunctionEntryCode(Function *func);
  std::string generateFunctionExitCode(Function *func);
  std::string generateFunctionPostCode(Function *func);
  std::string generateBasicBlockCode(BasicBlock *bb);
  std::string generateInstructionCode(Instruction *inst);
  std::string global(std::string name);
  std::string getLabelName(BasicBlock *bb);
  std::string getLabelName(Function *func, int type);
  std::string generateFunctionCall(Instruction *inst, std::string func_name,
                                   std::vector<Value *> ops, int return_reg = 0,
                                   int sp_ofs = 0);
  std::vector<InstGen::Reg> getAllRegisters(Function *func);
  std::vector<InstGen::Reg> getCallerSaveRegisters(Function *func);
  std::vector<InstGen::Reg> getCalleeSaveRegisters(Function *func);
  void allocateStackSpace(Function *func);
  bool isSameMapping(Value *a, Value *b);
  std::string virtualRegMove(std::vector<Value *> target,
                             std::vector<Value *> source, int sp_ofs = 0);
  std::string virtualRegMove(Value *target, Value *source, int sp_ofs = 0);
  std::string assignToSpecificReg(Value *val, int target, int sp_ofs = 0);
  std::string getFromSpecificReg(Value *val, int source, int sp_ofs = 0);
  std::string generateGOT();
  std::string generateGlobalVarsCode();
  std::string generateInitializerCode(Constant *init);
  std::pair<int, bool> getConstIntVal(Value *val);
  int queryGOT(Value *val);
  std::string comment(std::string s);
  std::map<Value *, int> regAlloc();
  void printIR();
  void printStat();
  InstGen::CmpOp cmpConvert(CmpInst::CmpOp op, bool reverse = false);
  bool is_mt_inside(Function *func);
  bool is_mt_inst(Value *val);
  bool is_mt_start(Value *val);
  bool is_mt_end(Value *val);
  std::string mtstart();
  std::string mtend();
  std::string vv(const InstGen::Reg &len);
};

#endif