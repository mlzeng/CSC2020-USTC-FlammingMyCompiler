#include "LoopFind.h"
#include "ReturnVal.h"

using BBset_t = std::unordered_set<BasicBlock *>;

// #define DEBUG

void LoopFind::run() {
  std::unordered_set<CFGnode *> reserved;
  std::unordered_set<std::unordered_set<CFGnode *> *> comps;
  std::unordered_set<CFGnode *> nodes;

  for (auto func : m_->getFunctions()) {
    if (func->getBasicBlocks().size() == 0)
      continue;
    exit_ifnot(_NodesEmpty_run_LoopFind, nodes.size() == 0);

    buildCFG(func, nodes);

#ifdef DEBUG
    std::cout << "in func:" + func->getName() + ":\n";
    std::cout << _printCFG(nodes);
    // getchar();
    std::string comment;
    std::string tmpComment;
#endif
    while (stronglyConnComponents(nodes, comps)) {

      if (comps.size() == 0)
        break;
      for (auto set : comps) {
        // std::cout << "Set :\t";
        // for (auto n : *set) {
        //     std::cout << n->BB->getName() + " ";
        // } putchar('\n');getchar();
        auto base = findLoopBase(set, reserved);

#ifdef DEBUG
        tmpComment = "";
        tmpComment += "find Loop\t(";
        for (auto n : *set) {
          tmpComment += n->BB->getName() + " ";
        }
        tmpComment += ") delete " + base->BB->getName() + "\n";
        std::cout << tmpComment;
        comment += tmpComment;
#endif

        // store result
        auto BBset = new BBset_t;
        for (auto n : *set) {
          BBset->insert(n->BB);
        }
        loops.insert(BBset);
        base2Loop.insert({base->BB, BBset});
        loop2Base.insert({BBset, base->BB});
        for (auto BB : *BBset) {
          if (BB2Base.find(BB) == BB2Base.end())
            BB2Base.insert({BB, base->BB});
          else
            BB2Base[BB] = base->BB;
        }

        // prepare for recongnize inner loop
        reserved.insert(base);
        nodes.erase(base);
        for (auto su : base->succs) {
          su->preds.erase(base);
        }
        for (auto pred : base->preds) {
          pred->succs.erase(base);
        }
      }

      // clear
      for (auto set : comps)
        delete set;
      comps.clear();
      for (auto n : nodes) {
        n->index = n->lowlink = -1;
        n->onStack = false;
      }
    }
    // clear
    for (auto n : reserved) {
      nodes.insert(n);
      for (auto pred : n->preds) {
        pred->succs.insert(n);
      }
    }
#ifdef DEBUG
    std::cout << "-------------- Result CFG (" + (func->getName()) +
                     ") ------------\n";
    std::cout << _printCFG(nodes) + "/*\n" + comment + "*/\n";
// getchar();
#endif
    reserved.clear();
    for (auto _n : nodes) {
      delete _n;
    }
    nodes.clear();
  }
}

// https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
bool LoopFind::stronglyConnComponents(
    std::unordered_set<CFGnode *> &nodes,
    std::unordered_set<std::unordered_set<CFGnode *> *> &result) {
  indexCount = 0;
  stack.clear();
  for (auto n : nodes) {
    if (n->index == -1)
      traverse(n, result);
  }
  return result.size() != 0;
}

void LoopFind::traverse(
    CFGnode *n, std::unordered_set<std::unordered_set<CFGnode *> *> &result) {
  n->index = indexCount++;
  n->lowlink = n->index;
  stack.push_back(n);
  n->onStack = true;

  for (auto su : n->succs) {
    if (su->index == -1) {
      traverse(su, result);
      n->lowlink = std::min(su->lowlink, n->lowlink);
    } else if (su->onStack) {
      n->lowlink = std::min(su->index, n->lowlink);
    }
  }

  if (n->index == n->lowlink) {
    auto set = new std::unordered_set<CFGnode *>;
    CFGnode *tmp;
    do {
      tmp = stack.back();
      tmp->onStack = false;
      set->insert(tmp);
      stack.pop_back();
    } while (tmp != n);
    if (set->size() == 1)
      delete set;
    else
      result.insert(set);
  }
}

CFGnode *LoopFind::findLoopBase(std::unordered_set<CFGnode *> *set,
                                std::unordered_set<CFGnode *> &reserved) {
  CFGnode *base = nullptr;
  bool hadBeen = false;
  for (auto n : *set) {
    for (auto pred : n->preds) {
      if (set->find(pred) == set->end()) {
        exit_ifnot(_hadBeen_findLoopBase_LoopFind, !hadBeen);
        base = n;
      }
    }
  }
  if (base != nullptr)
    return base;
  for (auto res : reserved) {
    for (auto succ : res->succs) {
      if (set->find(succ) != set->end()) {
        base = succ;
      }
    }
  }
  exit_ifnot(_baseEmpty_findLoopBase_LoopFind, base != nullptr);
  return base;
}

void LoopFind::buildCFG(Function *func, std::unordered_set<CFGnode *> &result) {
  std::unordered_map<BasicBlock *, CFGnode *> m;
  for (auto BB : func->getBasicBlocks()) {
    auto node = new CFGnode;
    node->BB = BB;
    node->index = node->lowlink = -1;
    node->onStack = false;
    m.insert({BB, node});
    result.insert(node);
  }
  for (auto BB : func->getBasicBlocks()) {
    auto node = m[BB];
    for (auto succ : BB->getSuccBasicBlocks()) {
      node->succs.insert(m[succ]);
    }
    for (auto pred : BB->getPreBasicBlocks()) {
      node->preds.insert(m[pred]);
    }
  }
}

std::string LoopFind::_printCFG(std::unordered_set<CFGnode *> &nodes) {
  std::string nodeDecl;
  std::string eageDecl;
  std::vector<CFGnode *> stack;
  std::unordered_set<CFGnode *> processd;
  std::unordered_map<CFGnode *, int> count;
  // std::vector<std::string> color {"black", "red", "green", "blue", "yellow",
  // "magenta"};
  std::vector<std::string> style{"solid", "dashed", "dotted", "bold"};

  int counter = 0;
  for (auto n : nodes) {
    count.insert({n, counter++});
  }

  for (auto n : nodes) {
    for (auto succ : n->succs) {
      eageDecl += "\t" + std::to_string(count[n]) + "->" +
                  std::to_string(count[succ]) + "\n";
    }
  }

  int styleChoose;
  for (auto n : nodes) {
    styleChoose = 0;
    if (base2Loop.find(n->BB) != base2Loop.end()) {
      styleChoose = 1;
    }
    nodeDecl += "\t" + std::to_string(count[n]) + "[label=\"" +
                n->BB->getName() + "\",style=" + style[styleChoose] + "]\n";
  }
  return "digraph G {\n" + nodeDecl + "\n" + eageDecl + "}\n";
}

BBset_t *LoopFind::getBaseLoop(BasicBlock *BB) {
  if (BB2Base.find(BB) == BB2Base.end())
    return nullptr;
  return base2Loop[BB2Base[BB]];
}

BasicBlock *LoopFind::getLoopCond(BBset_t *set) { return loop2Base[set]; }

BBset_t *LoopFind::getParentLoop(BBset_t *set) {
  auto base = loop2Base[set];
  for (auto pred : base->getPreBasicBlocks()) {
    if (set->find(pred) != set->end())
      continue;
    auto loop = getBaseLoop(pred);
    if (loop == nullptr || loop->find(base) == loop->end())
      return nullptr;
    else {
      return loop;
    }
  }
  std::cerr << "loopBase " + base->getName() + "(" +
                   base->getParent()->getName() + ")" +
                   " doesn't have a entry\n";
  exit_ifnot(_getParentLoop_LoopFind, false && "Loop has no In BB");
  return nullptr;
}

bool LoopFind::isBaseLoop(BBset_t *set) {
  auto base = loop2Base[set];
  for (auto tmp : *set) {
    if (tmp == base)
      continue;
    if (base2Loop.find(tmp) != base2Loop.end())
      return false;
  }
  return true;
}

BasicBlock *LoopFind::getLoopOut(BBset_t *set) {
  std::unordered_set<BasicBlock *> OutBBs;
}

LoopFind::~LoopFind() {
  for (auto set : loops) {
    delete set;
  }
}
