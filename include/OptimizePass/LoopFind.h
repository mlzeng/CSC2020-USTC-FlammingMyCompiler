#ifndef __LOOOFIND_H__
#define __LOOOFIND_H__

#include "BasicBlock.h"
#include "PassManager.h"
#include <string>
#include <unordered_set>
#include <vector>

struct CFGnode {
  std::unordered_set<CFGnode *> succs;
  std::unordered_set<CFGnode *> preds;
  BasicBlock *BB;
  int index;
  int lowlink;
  int onStack;
};

using BBset_t = std::unordered_set<BasicBlock *>;

class LoopFind : public Pass {
  std::unordered_set<BBset_t *> loops;
  std::unordered_map<BasicBlock *, BBset_t *> base2Loop;
  std::unordered_map<BBset_t *, BasicBlock *> loop2Base;
  std::unordered_map<BasicBlock *, BasicBlock *>
      BB2Base; // 默认映射最低层次的Loop

  int indexCount;
  std::vector<CFGnode *> stack;

  void buildCFG(Function *func, std::unordered_set<CFGnode *> &);
  CFGnode *findLoopBase(std::unordered_set<CFGnode *> *,
                        std::unordered_set<CFGnode *> &);
  bool
  stronglyConnComponents(std::unordered_set<CFGnode *> &,
                         std::unordered_set<std::unordered_set<CFGnode *> *> &);
  void traverse(CFGnode *,
                std::unordered_set<std::unordered_set<CFGnode *> *> &);
  std::string _printCFG(std::unordered_set<CFGnode *> &);

public:
  explicit LoopFind(Module *m) : Pass(m) {}
  LoopFind(const LoopFind &) = delete;
  LoopFind &operator=(const LoopFind &) = delete;
  ~LoopFind();
  BBset_t *getBaseLoop(BasicBlock *BB);
  BasicBlock *getLoopCond(BBset_t *set);
  BBset_t *getParentLoop(BBset_t *set);
  BasicBlock *getLoopOut(BBset_t *set);
  BasicBlock *getLoopEntry(BBset_t *set) { return getLoopCond(set); }
  bool isBaseLoop(BBset_t *);
  auto begin() { return loops.begin(); }
  auto end() { return loops.end(); }
  void run() override;
  /*
  LOOPFIND 调用方法 :
      1. 对一个BB： getBaseLoop,getParentLoop 以嵌套层次向前
      2. 对一个循环set, getLoopCond得到条件BB，通过集合直接遍历
      3. 遍历循环: begin,end遍历，若需要识别嵌套结构，可以调用isBaseLoop识
          别，使用（1）中方法查找，但需要小心在begin，end的重复遍历
      4.
  */
};

#endif //__LOOOFIND_H__
