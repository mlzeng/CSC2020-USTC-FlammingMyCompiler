#ifndef SYSYC_DOMINATORS_H
#define SYSYC_DOMINATORS_H

#include "BasicBlock.h"
#include "PassManager.h"
#include <list>
#include <map>
#include <set>

class Dominators : public Analysis {
public:
  Dominators(Module *m) : Analysis(m) {}
  void run() override;
  void createDoms(Function *f);
  void createReversePostOrder(Function *f);
  void createIDom(Function *f);
  void createDominanceFrontier(Function *f);
  void createDomTreeSucc(Function *f);

  // for debug
  void printIDom(Function *f);
  void printDominanceFrontier(Function *f);

private:
  void postOrderVisit(BasicBlock *bb, std::set<BasicBlock *> &visited);
  BasicBlock *intersect(BasicBlock *b1, BasicBlock *b2);

  std::list<BasicBlock *> reversePostOrder_;
  std::map<BasicBlock *, int> postOrderID_; // the root has highest ID
};

#endif // SYSYC_DOMINATORS_H
