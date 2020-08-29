#ifndef SYSYC_BASICBLOCK_H
#define SYSYC_BASICBLOCK_H

#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "BaseBlock.h"
#include "Value.h"

class Function;
class Instruction;
class Module;

class BasicBlock : public BaseBlock {
public:
  static BasicBlock *create(Module *m, const std::string &name) {
    return new BasicBlock(m, name);
  }

  // this will auto add this to function's baseblock list
  // don't use this in nested structure
  static BasicBlock *create(Module *m, const std::string &name,
                            Function *func) {
    return new BasicBlock(m, name, func);
  }

  void setParent(Function *parent) { func_ = parent; }

  // return parent, or null if none.
  Function *getParent() const { return func_; }

  Module *getModule() const;

  // TODO(zyh) 待确认
  auto rbegin() { return instr_list_.rbegin(); }
  auto rend() { return instr_list_.rend(); }
  std::list<Instruction *> &getInstructions() { return instr_list_; }

  /// Returns the terminator instruction if the block is well formed or null
  /// if the block is not well formed.
  const Instruction *getTerminator() const;
  Instruction *getTerminator() {
    return const_cast<Instruction *>(
        static_cast<const BasicBlock *>(this)->getTerminator());
  }

  void addInstruction(Instruction *instr);

  void addInstrBegin(Instruction *instr);

  bool empty() { return instr_list_.empty(); }
  // void eraseFromParent() { parent_->Remove(this); }

  int getNumOfInstr() { return instr_list_.size(); }
  std::list<Instruction *> &getInstrList() { return instr_list_; }

  void insertInstr(Instruction *pos, Instruction *insert);
  void deleteInstr(Instruction *instr);

  std::map<Value *, std::set<Value *>> &getBBAssignIn() {
    return reach_assign_in_;
  }
  std::map<Value *, std::set<Value *>> &getBBAssignOut() {
    return reach_assign_out_;
  }

  std::list<BasicBlock *> &getPreBasicBlocks() { return pre_bbs_; }
  std::list<BasicBlock *> &getSuccBasicBlocks() { return succ_bbs_; }

  void addPreBasicBlock(BasicBlock *bb) { pre_bbs_.push_back(bb); }
  void addSuccBasicBlock(BasicBlock *bb) { succ_bbs_.push_back(bb); }

  void removePreBasicBlock(BasicBlock *bb) { pre_bbs_.remove(bb); }
  void removeSuccBasicBlock(BasicBlock *bb) { succ_bbs_.remove(bb); }
  void clearSuccBasicBlock() { succ_bbs_.clear(); }

  void addDom(BasicBlock *bb) { doms_.insert(bb); }
  std::set<BasicBlock *> &getDoms() { return doms_; }
  void setDoms(std::set<BasicBlock *> &doms) {
    doms_.clear();
    doms_.insert(doms.begin(), doms.end());
  }

  BasicBlock *getIDom() { return idom_; }
  void setIDom(BasicBlock *idom) { idom_ = idom; }

  void addDominanceFrontier(BasicBlock *bb) { domFrontier_.insert(bb); }
  std::set<BasicBlock *> &getDominanceFrontier() { return domFrontier_; }
  void setDominanceFrontier(std::set<BasicBlock *> &df) {
    domFrontier_.clear();
    domFrontier_.insert(df.begin(), df.end());
  }

  // successor blocks of this node in dominance tree
  std::set<BasicBlock *> getDomTreeSuccBlocks() { return domTreeSuccBlocks_; }
  void addDomTreeSuccBlock(BasicBlock *bb) { domTreeSuccBlocks_.insert(bb); }

  virtual void print() override;

protected:
  explicit BasicBlock(Module *m, const std::string &name);
  explicit BasicBlock(Module *m, const std::string &name, Function *func);

  std::list<Instruction *> instr_list_;

private:
  std::map<Value *, std::set<Value *>> reach_assign_in_; // data flow
  std::map<Value *, std::set<Value *>> reach_assign_out_;

  std::list<BasicBlock *> pre_bbs_;
  std::list<BasicBlock *> succ_bbs_;

  std::set<BasicBlock *> doms_;        // dominance set
  BasicBlock *idom_ = nullptr;         // immediate dominance
  std::set<BasicBlock *> domFrontier_; // dominance frontier set
  std::set<BasicBlock *> domTreeSuccBlocks_;

  // TODO(zyh) 待确认
  std::unordered_set<Value *> activeIn, activeOut, definedVals;
  std::unordered_map<Value *, BasicBlock *> inheritedVals;
  // bool hasReturn;
  // bool isEntry;
public:
  bool hasRet();
  bool isEntry() { return getName() == "entry"; }
  auto &getActiveIns() { return activeIn; }
  auto &getActiveOuts() { return activeOut; }
  auto &getDefinedVals() { return definedVals; }
  auto &getInheritedVals() { return inheritedVals; }
  void addActiveIn(Value *val) { activeIn.insert(val); }
  void addActiveOut(Value *val) { activeOut.insert(val); }
  void initAuxilliaryValsOfActiveSet(
      std::unordered_set<Value *> *,
      std::unordered_map<BasicBlock *, std::unordered_set<Value *>> *);
  void initDefinedVals(std::unordered_set<Value *> *);
  BasicBlock *intendedBB = nullptr;
};

#endif // SYSYC_BASICBLOCK_H
