#ifndef SYSYC_FUNCTIONINLINE_H
#define SYSYC_FUNCTIONINLINE_H

#include "PassManager.h"
#include <map>
#include <set>

class FunctionInline : public Transform {
public:
  const bool ONLY_INLINE_SMALL_FUNC = false;
  const bool INLINE_BB_NUM_MAX = 4;
  const bool NOT_INLINE_MULTILEVEL_LOOP_FUNC = true;
  const int INLINE_LOOP_LEVEL_MAX = 1;
  const bool INLINE_RECURSIVE = false;

public:
  FunctionInline(Module *m) : Transform(m) {}
  void run() override;
  void inlining(Function *entry_func);
  void inliningRecursive(Function *entry_func);
  void init();
  bool isIgnored(Function *f) { return ignored_.find(f) != ignored_.end(); }
  bool isInlined(Function *f) { return inlined_.find(f) != inlined_.end(); }

  bool canBeInlined(Instruction *instr);
  bool canBeInlinedRecursive(Instruction *instr);
  void removeInlineFunc();

  bool isRecursive(Function *f) {
    return recursive_func_.find(f) != recursive_func_.end();
  }

private:
  void createFuncSucc();
  void detectRecursive();
  void visitFunc(Function *entry, std::set<Function *> &visited);

  std::list<BasicBlock *> copyBasicBlocks(Function *old_func);
  std::list<BasicBlock *> copyBasicBlocks(CallInst *call_inst,
                                          Function *old_func);
  std::list<BasicBlock *> copyBasicBlocks(CallInst *call_inst,
                                          std::list<BasicBlock *> copy_bbs,
                                          Function *old_func);
  BasicBlock *splitBasicBlock(std::list<Instruction *>::iterator call_pos,
                              std::list<Instruction *> &instrs);
  void handleReturns(BasicBlock *return_bb, AllocaInst *ret_val,
                     std::list<BasicBlock *> &func_bbs);
  void handleVoidReturn(BasicBlock *return_bb,
                        std::list<BasicBlock *> &func_bbs);
  void handlePHIReturns(BasicBlock *return_bb, PhiInst *ret_val,
                        std::list<BasicBlock *> &func_bbs);

private:
  std::set<Function *> ignored_;    // ignored functions
  std::set<Function *> inlined_;    // inlined functions
  std::set<Function *> has_inline_; // func that has inlined functions
  std::map<Function *, std::set<Function *>> func_succ_;
  std::set<Function *> recursive_func_;
};

#endif // SYSYC_FUNCTIONINLINE_H
