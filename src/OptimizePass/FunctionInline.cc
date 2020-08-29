#include "FunctionInline.h"
#include "LoopFind.h"
#include "ReturnVal.h"

extern std::string input_path;

void FunctionInline::run() {
  init();
  createFuncSucc();
  detectRecursive();
  auto main = m_->getMainFunction();
  exit_ifnot(_CantGetMain_run_FunctionInline, main);
  inlining(main);
  if (m_->getIRLevel() == Module::MIR_MEM) {
    if (INLINE_RECURSIVE) {
      for (auto func : m_->getFunctions()) {
        if (!isInlined(func) && func != main) {
          inlining(func);
        }
      }
      // for (auto func : recursive_func_) {
      //     inliningRecursive(func);
      // }
    }
  }
  if (m_->isMIRSSALevel()) {
    for (auto func : m_->getFunctions()) {
      if (!isInlined(func) && func != main) {
        inlining(func);
      }
    }
  }
  removeInlineFunc();
}

void FunctionInline::init() {
  // ignored functions
  LoopFind lf(m_);
  if (NOT_INLINE_MULTILEVEL_LOOP_FUNC) {
    lf.run();
  }
  for (auto func : m_->getFunctions()) {
    if (func->getBasicBlocks().empty()) {
      ignored_.insert(func);
    } else if (ONLY_INLINE_SMALL_FUNC) {
      if (func->getBasicBlocks().size() > INLINE_BB_NUM_MAX)
        ignored_.insert(func);
    } else if (NOT_INLINE_MULTILEVEL_LOOP_FUNC) {
      for (auto &bb : func->getBasicBlocks()) {
        int nested_level = 0;
        for (auto bs = lf.getBaseLoop(bb); bs != nullptr;
             bs = lf.getParentLoop(bs)) {
          nested_level++;
          if (nested_level > INLINE_LOOP_LEVEL_MAX) {
            break;
          }
        }
        for (auto inst : bb->getInstructions()) {
          if (inst->isVV() && inst->getNumOperand() == 3)
            nested_level++;
        }
        if (nested_level > INLINE_LOOP_LEVEL_MAX) {
          ignored_.insert(func);
        }
        if (ignored_.find(func) != ignored_.end()) {
          break;
        }
      }
    }
  }
  for (auto func : m_->getFunctions()) {
    if (func->getName() == "__fastmm") {
      ignored_.erase(func);
    }
  }
}

void FunctionInline::createFuncSucc() {
  for (auto f : m_->getFunctions()) {
    if (!isIgnored(f)) {
      for (auto use : f->getUseList()) {
        auto instr = static_cast<CallInst *>(use.val_);
        exit_ifnot(_funcUsedByNonCall_createFuncSucc_FunctionInline, instr);
        auto use_f = instr->getParent()->getParent();
        func_succ_[use_f].insert(f);
      }
    }
  }
}

void FunctionInline::detectRecursive() {
  auto entry = m_->getMainFunction();
  std::set<Function *> visited;
  visitFunc(entry, visited);
}

void FunctionInline::visitFunc(Function *entry, std::set<Function *> &visited) {
  visited.insert(entry);
  for (auto succ : func_succ_[entry]) {
    if (visited.find(succ) != visited.end()) {
      // ignored_.insert(succ);
      recursive_func_.insert(succ);
    } else {
      visitFunc(succ, visited);
    }
  }
  visited.erase(entry);
}

bool FunctionInline::canBeInlined(Instruction *instr) {
  if (instr->getInstrType() == Instruction::Call) {
    auto call = static_cast<CallInst *>(instr);
    auto func = call->getFunction();
    if (m_->isMIRSSALevel()) {
      if (func->getNumBasicBlocks() == 1) {
        if (func->getBasicBlocks().back()->getInstructions().size() <= 5)
          return true;
      }
      return false;
    }
    return !isIgnored(func) && !isRecursive(func);
  }
  return false;
}

bool FunctionInline::canBeInlinedRecursive(Instruction *instr) {
  if (instr->getInstrType() == Instruction::Call) {
    auto call = static_cast<CallInst *>(instr);
    return !isIgnored(call->getFunction());
  }
  return false;
}

void FunctionInline::removeInlineFunc() {
  auto main_func = m_->getMainFunction();
  // 删除inline函数
  for (auto f : inlined_) {
    bool remove = true;
    for (auto use : f->getUseList()) {
      auto instr = static_cast<CallInst *>(use.val_);
      exit_ifnot(_funcUsedByNonCall_createFuncSucc_FunctionInline, instr);
      auto use_f = instr->getParent()->getParent();
      if ((has_inline_.find(use_f) == has_inline_.end()) &&
          (!isInlined(use_f))) {
        remove = false;
      }
    }
    if (remove) {
      m_->removeFunction(f);
      // remove use of global variables
      for (auto bb : f->getBasicBlocks()) {
        for (auto inst : bb->getInstructions()) {
          inst->removeUseOfOps();
        }
      }
      // std::cout << "remove inlined function: " << f->getName() << std::endl;
    }
  }
}

void FunctionInline::inlining(Function *entry_func) {
  auto &entry_func_bbs = entry_func->getBasicBlocks();
  for (auto bbs_iter = entry_func_bbs.begin(); bbs_iter != entry_func_bbs.end();
       ++bbs_iter) {
    BasicBlock *cur_bb = *bbs_iter;
    auto &instrs = cur_bb->getInstructions();
    for (auto instr_iter = instrs.begin(); instr_iter != instrs.end();
         ++instr_iter) {
      auto cur_inst = *instr_iter;
      if (canBeInlined(cur_inst)) {
        // 切分BB
        auto split_bb = splitBasicBlock(instr_iter, instrs);
        ++bbs_iter;
        auto next_bb_iter = entry_func_bbs.insert(bbs_iter, split_bb);
        split_bb->setParent(entry_func);
        // 维护前驱和后继
        for (auto bb : cur_bb->getSuccBasicBlocks()) {
          bb->removePreBasicBlock(cur_bb);
          bb->addPreBasicBlock(split_bb);
          split_bb->addSuccBasicBlock(bb);
        }
        cur_bb->clearSuccBasicBlock();

        // 复制函数的bbs
        CallInst *call_inst = static_cast<CallInst *>(cur_inst);
        exit_ifnot(_NotCallInst_inlining_FunctionInline, call_inst);
        Function *func = call_inst->getFunction();
        std::list<BasicBlock *> inlined_bbs = copyBasicBlocks(call_inst, func);
        // 设置新bb的父亲
        for (auto bb : inlined_bbs) {
          bb->setParent(entry_func);
        }
        // 将新的bb插入函数
        next_bb_iter = entry_func_bbs.insert(next_bb_iter, inlined_bbs.begin(),
                                             inlined_bbs.end());

        // 设置被split的块跳转到新的bb的entry，注意这个指令会自动添加Br到cur_bb
        BranchInst *br = BranchInst::createBr(*(inlined_bbs.begin()), cur_bb);

        // 给返回值alloca空间，并跳转到新的BB
        if (!call_inst->isVoid()) {
          // 生成PHI指令
          if (m_->isMIRSSALevel()) {
            auto phi = PhiInst::createPhi(call_inst->getType(), nullptr);
            handlePHIReturns(split_bb, phi, inlined_bbs);
            if (phi->getNumOperand() == 2)
              call_inst->replaceAllUseWith(phi->getOperand(0));
            else {
              call_inst->replaceAllUseWith(phi);
              split_bb->addInstrBegin(phi);
            }
          } else {
            auto alloca = AllocaInst::createAlloca(call_inst->getType());
            handleReturns(split_bb, alloca, inlined_bbs);
            auto inlined_entry_bb = *(inlined_bbs.begin());
            inlined_entry_bb->addInstrBegin(alloca);
            // alloca->setParent(inlined_entry_bb);
            auto load = LoadInst::createLoad(call_inst->getType(), alloca);
            call_inst->replaceAllUseWith(load);
            split_bb->addInstrBegin(load);
          }
        }
        // 处理void返回值的情况，直接跳转到新的BB
        else {
          handleVoidReturn(split_bb, inlined_bbs);
        }

        inlined_.insert(func);
        has_inline_.insert(entry_func);

        bbs_iter = --next_bb_iter;
        break;
      }
    }
  }
}

// 进行了形参和实参的替换，新bb内的前驱和后继也有维护，bb没有设置parent
std::list<BasicBlock *> FunctionInline::copyBasicBlocks(CallInst *call_inst,
                                                        Function *old_func) {
  std::list<BasicBlock *> bbs;
  std::map<Value *, Value *> old2new;
  // 创建新的BB和instr，并建立旧的Value和新的Value的map（old2new）
  for (auto bb : old_func->getBasicBlocks()) {
    BasicBlock *new_bb = BasicBlock::create(m_, "");
    old2new[bb] = new_bb;
    bbs.push_back(new_bb);
    for (auto instr : bb->getInstructions()) {
      Instruction *new_inst = instr->copy(new_bb);
      exit_ifnot(_copyhasWrong_copyBasicBlocks_FunctionInline,
                 new_inst->getInstrType() == instr->getInstrType());
      old2new[instr] = new_inst;
    }
  }
  // 创建形参和实参的映射
  // 注意实参的ops是从1开始，0是函数func
  int i = 1;
  for (auto arg : old_func->getArgs()) {
    old2new[arg] = call_inst->getOperand(i);
    i++;
  }

  for (auto bb : old_func->getBasicBlocks()) {
    BasicBlock *new_bb = dynamic_cast<BasicBlock *>(old2new[bb]);
    exit_ifnot(_NewBBEmpty_copyBasicBlocks_FunctionInline, new_bb);
    for (auto instr : bb->getInstructions()) {
      Instruction *new_inst = dynamic_cast<Instruction *>(old2new[instr]);
      exit_ifnot(_NewInsEmpty_copyBasicBlocks_FunctionInline, new_inst);
      int i = 0;
      for (auto op : instr->getOperands()) {
        Value *new_op;
        if (dynamic_cast<Constant *>(op) ||
            dynamic_cast<GlobalVariable *>(op) ||
            dynamic_cast<Function *>(op)) {
          new_op = op;
        } else {
          new_op = old2new[op];
        }
        new_inst->setOperand(i, new_op);
        i++;
      }
    }
  }
  for (auto bb : old_func->getBasicBlocks()) {
    BasicBlock *new_bb = dynamic_cast<BasicBlock *>(old2new[bb]);
    exit_ifnot(_NewBBEmpty_copyBasicBlocks_FunctionInline, new_bb);
    for (auto succ : bb->getSuccBasicBlocks()) {
      BasicBlock *new_succ = dynamic_cast<BasicBlock *>(old2new[succ]);
      exit_ifnot(_NewBBEmpty_copyBasicBlocks_FunctionInline, new_succ);
      new_bb->addSuccBasicBlock(new_succ);
      new_succ->addPreBasicBlock(new_bb);
    }
  }
  return bbs;
}

std::list<BasicBlock *> FunctionInline::copyBasicBlocks(Function *old_func) {
  std::list<BasicBlock *> bbs;
  std::map<Value *, Value *> old2new;
  // 创建新的BB和instr，并建立旧的Value和新的Value的map（old2new）
  for (auto bb : old_func->getBasicBlocks()) {
    BasicBlock *new_bb = BasicBlock::create(m_, "");
    old2new[bb] = new_bb;
    bbs.push_back(new_bb);
    for (auto instr : bb->getInstructions()) {
      Instruction *new_inst = instr->copy(new_bb);
      exit_ifnot(_copyhasWrong_copyBasicBlocks_FunctionInline,
                 new_inst->getInstrType() == instr->getInstrType());
      old2new[instr] = new_inst;
    }
  }

  for (auto bb : old_func->getBasicBlocks()) {
    BasicBlock *new_bb = dynamic_cast<BasicBlock *>(old2new[bb]);
    exit_ifnot(_NewBBEmpty_copyBasicBlocks_FunctionInline, new_bb);
    for (auto instr : bb->getInstructions()) {
      Instruction *new_inst = dynamic_cast<Instruction *>(old2new[instr]);
      exit_ifnot(_NewInsEmpty_copyBasicBlocks_FunctionInline, new_inst);
      int i = 0;
      for (auto op : instr->getOperands()) {
        Value *new_op;
        if (dynamic_cast<Constant *>(op) ||
            dynamic_cast<GlobalVariable *>(op) ||
            dynamic_cast<Function *>(op) || dynamic_cast<Argument *>(op)) {
          new_op = op;
        } else {
          new_op = old2new[op];
        }
        new_inst->setOperand(i, new_op);
        i++;
      }
    }
  }
  for (auto bb : old_func->getBasicBlocks()) {
    BasicBlock *new_bb = dynamic_cast<BasicBlock *>(old2new[bb]);
    exit_ifnot(_NewBBEmpty_copyBasicBlocks_FunctionInline, new_bb);
    for (auto succ : bb->getSuccBasicBlocks()) {
      BasicBlock *new_succ = dynamic_cast<BasicBlock *>(old2new[succ]);
      exit_ifnot(_NewBBEmpty_copyBasicBlocks_FunctionInline, new_succ);
      new_bb->addSuccBasicBlock(new_succ);
      new_succ->addPreBasicBlock(new_bb);
    }
  }
  return bbs;
}

std::list<BasicBlock *> FunctionInline::copyBasicBlocks(
    CallInst *call_inst, std::list<BasicBlock *> copy_bbs, Function *old_func) {
  std::list<BasicBlock *> bbs;
  std::map<Value *, Value *> old2new;
  // 创建新的BB和instr，并建立旧的Value和新的Value的map（old2new）
  for (auto bb : copy_bbs) {
    BasicBlock *new_bb = BasicBlock::create(m_, "");
    old2new[bb] = new_bb;
    bbs.push_back(new_bb);
    for (auto instr : bb->getInstructions()) {
      Instruction *new_inst = instr->copy(new_bb);
      exit_ifnot(_copyhasWrong_copyBasicBlocks_FunctionInline,
                 new_inst->getInstrType() == instr->getInstrType());
      old2new[instr] = new_inst;
    }
  }
  // 创建形参和实参的映射
  // 注意实参的ops是从1开始，0是函数func
  int i = 1;
  for (auto arg : old_func->getArgs()) {
    old2new[arg] = call_inst->getOperand(i);
    i++;
  }

  for (auto bb : copy_bbs) {
    BasicBlock *new_bb = dynamic_cast<BasicBlock *>(old2new[bb]);
    exit_ifnot(_NewBBEmpty_copyBasicBlocks_FunctionInline, new_bb);
    for (auto instr : bb->getInstructions()) {
      Instruction *new_inst = dynamic_cast<Instruction *>(old2new[instr]);
      exit_ifnot(_NewInsEmpty_copyBasicBlocks_FunctionInline, new_inst);
      int i = 0;
      for (auto op : instr->getOperands()) {
        Value *new_op;
        if (dynamic_cast<Constant *>(op) ||
            dynamic_cast<GlobalVariable *>(op) ||
            dynamic_cast<Function *>(op)) {
          new_op = op;
        } else {
          new_op = old2new[op];
        }
        new_inst->setOperand(i, new_op);
        i++;
      }
    }
  }
  for (auto bb : copy_bbs) {
    BasicBlock *new_bb = dynamic_cast<BasicBlock *>(old2new[bb]);
    exit_ifnot(_NewBBEmpty_copyBasicBlocks_FunctionInline, new_bb);
    for (auto succ : bb->getSuccBasicBlocks()) {
      BasicBlock *new_succ = dynamic_cast<BasicBlock *>(old2new[succ]);
      exit_ifnot(_NewBBEmpty_copyBasicBlocks_FunctionInline, new_succ);
      new_bb->addSuccBasicBlock(new_succ);
      new_succ->addPreBasicBlock(new_bb);
    }
  }
  return bbs;
}

// 创建新的BB，包含call_pos iter之后的所有指令
BasicBlock *
FunctionInline::splitBasicBlock(std::list<Instruction *>::iterator call_pos,
                                std::list<Instruction *> &instrs) {
  BasicBlock *new_bb = BasicBlock::create(m_, "");
  auto iter = call_pos;
  for (++iter; iter != instrs.end(); ++iter) {
    new_bb->addInstruction(*iter);
    (*iter)->setParent(new_bb);
    int i = 0;
    for (auto use : (*iter)->getUseList()) {
      auto phi = dynamic_cast<Instruction *>(use.val_);
      if (phi && phi->isPHI()) {
        auto bb = dynamic_cast<BasicBlock *>(phi->getOperand(use.arg_no_ + 1));
        exit_ifnot(_ConvBBfalse_splitBasicBlock_FunctionInline, bb);
        if (bb == (*call_pos)->getParent()) {
          bb->removeUse(phi, use.arg_no_ + 1);
          phi->setOperand(use.arg_no_ + 1, new_bb);
        }
      }
    }
  }

  instrs.erase(call_pos, instrs.end());

  return new_bb;
}

// 必须有返回值，将返回值store在ret_ptr，然后创建Br到return_bb
void FunctionInline::handleReturns(BasicBlock *return_bb, AllocaInst *ret_ptr,
                                   std::list<BasicBlock *> &func_bbs) {

  for (auto bb : func_bbs) {
    Instruction *instr = bb->getTerminator();
    exit_ifnot(_CantGetTerminotor_handleReturns_FunctionInline, instr);
    if (instr->getInstrType() == Instruction::Ret) {
      exit_ifnot(_RetHasMoreThanOneOp_handleReturns_FunctionInline,
                 instr->getNumOperand() == 1);
      auto store = StoreInst::createStore(instr->getOperand(0), ret_ptr, bb);
      exit_ifnot(_caeateStoreErr_handleReturns_FunctionInline,
                 store->getParent() == bb);
      exit_ifnot(_caeateStoreErr_handleReturns_FunctionInline,
                 bb->getInstructions().back() == store);
      auto br = BranchInst::createBr(return_bb, bb);
      exit_ifnot(_createBrErr_handleReturns_FunctionInline,
                 bb->getInstructions().back() == br);
      bb->deleteInstr(instr);
      exit_ifnot(_deleteBr_handleReturns_FunctionInline, br->getParent() == bb);
    }
  }
}

// 处理无返回值的情况
void FunctionInline::handleVoidReturn(BasicBlock *return_bb,
                                      std::list<BasicBlock *> &func_bbs) {
  for (auto bb : func_bbs) {
    Instruction *instr = bb->getTerminator();
    exit_ifnot(_CantGetTerminotor_handleReturns_FunctionInline, instr);
    if (instr->getInstrType() == Instruction::Ret) {
      exit_ifnot(_handleVoidReturn_FunctionInline, instr->getNumOperand() == 0);
      auto br = BranchInst::createBr(return_bb, bb);
      exit_ifnot(_handleVoidReturn_FunctionInline, br->getParent() == bb);
      exit_ifnot(_handleVoidReturn_FunctionInline,
                 bb->getInstructions().back() == br);
      bb->deleteInstr(instr);
    }
  }
}

// 必须有返回值，将返回值形成phi，然后创建Br到return_bb
void FunctionInline::handlePHIReturns(BasicBlock *return_bb, PhiInst *ret_phi,
                                      std::list<BasicBlock *> &func_bbs) {
  for (auto bb : func_bbs) {
    Instruction *instr = bb->getTerminator();
    exit_ifnot(_handlePHIReturns_FunctionInline, instr);
    if (instr->getInstrType() == Instruction::Ret) {
      exit_ifnot(_handlePHIReturns_FunctionInline, instr->getNumOperand() == 1);
      ret_phi->addOperand(instr->getOperand(0));
      ret_phi->addOperand(bb);
      auto br = BranchInst::createBr(return_bb, bb);
      bb->deleteInstr(instr);
      exit_ifnot(_handlePHIReturns_FunctionInline, br->getParent() == bb);
    }
  }
}

void FunctionInline::inliningRecursive(Function *entry_func) {
  auto copy_bbs = copyBasicBlocks(entry_func);
  auto &entry_func_bbs = entry_func->getBasicBlocks();
  for (auto bbs_iter = entry_func_bbs.begin(); bbs_iter != entry_func_bbs.end();
       ++bbs_iter) {
    BasicBlock *cur_bb = *bbs_iter;
    auto &instrs = cur_bb->getInstructions();
    for (auto instr_iter = instrs.begin(); instr_iter != instrs.end();
         ++instr_iter) {
      auto cur_inst = *instr_iter;
      if (canBeInlinedRecursive(cur_inst)) {
        // 切分BB
        auto split_bb = splitBasicBlock(instr_iter, instrs);
        ++bbs_iter;
        auto next_bb_iter = entry_func_bbs.insert(bbs_iter, split_bb);
        split_bb->setParent(entry_func);
        // 维护前驱和后继
        for (auto bb : cur_bb->getSuccBasicBlocks()) {
          bb->removePreBasicBlock(cur_bb);
          bb->addPreBasicBlock(split_bb);
          split_bb->addSuccBasicBlock(bb);
        }
        cur_bb->clearSuccBasicBlock();

        // 复制函数的bbs
        CallInst *call_inst = static_cast<CallInst *>(cur_inst);
        exit_ifnot(_NotCallInst_inlining_FunctionInline, call_inst);
        Function *func = call_inst->getFunction();
        std::list<BasicBlock *> inlined_bbs;
        if (func == entry_func) {
          inlined_bbs = copyBasicBlocks(call_inst, copy_bbs, func);
        } else {
          inlined_bbs = copyBasicBlocks(call_inst, func);
        }
        // 设置新bb的父亲
        for (auto bb : inlined_bbs) {
          bb->setParent(entry_func);
        }
        // 将新的bb插入函数
        entry_func_bbs.insert(next_bb_iter, inlined_bbs.begin(),
                              inlined_bbs.end());

        // 设置被split的块跳转到新的bb的entry，注意这个指令会自动添加Br到cur_bb
        BranchInst *br = BranchInst::createBr(*(inlined_bbs.begin()), cur_bb);

        // 给返回值alloca空间，并跳转到新的BB
        if (!call_inst->isVoid()) {
          // 生成PHI指令
          if (m_->isMIRSSALevel()) {
            auto phi = PhiInst::createPhi(call_inst->getType(), nullptr);
            handlePHIReturns(split_bb, phi, inlined_bbs);
            if (phi->getNumOperand() == 2)
              call_inst->replaceAllUseWith(phi->getOperand(0));
            else {
              call_inst->replaceAllUseWith(phi);
              split_bb->addInstrBegin(phi);
            }
          } else {
            auto alloca = AllocaInst::createAlloca(call_inst->getType());
            handleReturns(split_bb, alloca, inlined_bbs);
            auto inlined_entry_bb = *(inlined_bbs.begin());
            inlined_entry_bb->addInstrBegin(alloca);
            // alloca->setParent(inlined_entry_bb);
            auto load = LoadInst::createLoad(call_inst->getType(), alloca);
            call_inst->replaceAllUseWith(load);
            split_bb->addInstrBegin(load);
          }
        }
        // 处理void返回值的情况，直接跳转到新的BB
        else {
          handleVoidReturn(split_bb, inlined_bbs);
        }

        inlined_.insert(func);
        has_inline_.insert(entry_func);

        bbs_iter = --next_bb_iter;
        break;
      }
    }
  }
  for (auto bb : copy_bbs) {
    for (auto inst : bb->getInstructions()) {
      inst->removeUseOfOps();
    }
  }
}
