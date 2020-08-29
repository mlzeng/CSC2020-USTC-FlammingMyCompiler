#ifndef CONTROL_TREE__
#define CONTROL_TREE__

#include "PassManager.h"
#include <unordered_map>

// class Pass;

enum ControlStruct_t {
  _NULL,
  REAL,
  CONTINUE,
  BREAK,
  RETURN,
  BLOCK,
  IFTHEN,
  IFTHENELSE,
  WHILE,
  ENTRY,
  END
};

struct CTnode {
  struct CTnode *p;
  struct CTnode *firstChild;
  struct CTnode *nextSibling;
  int nodeType;
  BasicBlock *BB;
};

using BB2Node_t = std::unordered_map<BasicBlock *, CTnode *>;

class ControlTreeAnalysis {
  Module *m_;

public:
  ControlTreeAnalysis(Module *m) : m_(m) {}
  void run();
};

// class ControlTree {
//     struct CTnode *root;
//     std::unordered_map<BasicBlock*, CTnode*> m;
// public:
//     void ControlTree(Function* func);
//     void ~ControlTree();
// }

#endif