#ifndef __LOOPINVARIANT_H__
#define __LOOPINVARIANT_H__
#include "LoopFind.h"
#include "PassManager.h"
#include <memory>
#include <unordered_set>
#include <vector>

class LoopInvariant : public Pass {
  std::vector<
      std::pair<BasicBlock *, std::__cxx11::list<Instruction *>::iterator>>
      invariant;
  // TODO 待确认
  std::unique_ptr<LoopFind> finder;
  int areaCount;
  // std::unordered_set<Value*> definedInLoop;
  void findInvariants(BBset_t *);
  void buildPrehead();
  void appendTo();
  void moveInvariantsOut(BBset_t *);
  bool definedOut();          // TODO
  bool definedIn_invariant(); // TODO

  BasicBlock *buildBB();

public:
  LoopInvariant(Module *m) : Pass(m) {}
  void run() override;
};

#endif
