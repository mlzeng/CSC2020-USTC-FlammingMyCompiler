#include "Dominators.h"
#include "ReturnVal.h"
#include <algorithm>
#include <string>

void Dominators::run() {
  for (auto f : m_->getFunctions()) {
    if (f->getBasicBlocks().size() == 0)
      continue;
    createReversePostOrder(f);
    createIDom(f);
    createDominanceFrontier(f);
    createDomTreeSucc(f);
    // for debug
    // printIDom(f);
    // printDominanceFrontier(f);
  }
}

void Dominators::createDoms(Function *f) {
  // init
  for (auto bb : f->getBasicBlocks()) {
    bb->addDom(bb);
  }
  // iterate
  bool changed = true;
  std::vector<BasicBlock *> ret(f->getNumBasicBlocks());
  std::vector<BasicBlock *> pre(f->getNumBasicBlocks());
  while (changed) {
    changed = false;
    for (auto bb : f->getBasicBlocks()) {
      auto &bbs = bb->getPreBasicBlocks();
      auto &first = (*bbs.begin())->getDoms();
      pre.insert(pre.begin(), first.begin(), first.end());
      pre.resize(first.size());
      ret.resize(f->getNumBasicBlocks());
      for (auto iter = ++bbs.begin(); iter != bbs.end(); ++iter) {
        auto &now = (*iter)->getDoms();
        auto it = std::set_intersection(pre.begin(), pre.end(), now.begin(),
                                        now.end(), ret.begin());
        ret.resize(it - ret.begin());
        pre.resize(ret.size());
        pre.insert(pre.begin(), ret.begin(), ret.end());
      }
      std::set<BasicBlock *> doms;
      doms.insert(bb);
      doms.insert(pre.begin(), pre.end());
      if (bb->getDoms() != doms) {
        bb->setDoms(doms);
        changed = true;
      }
    }
  }
}

void Dominators::createReversePostOrder(Function *f) {
  reversePostOrder_.clear();
  postOrderID_.clear();
  std::set<BasicBlock *> visited;
  postOrderVisit(f->getEntryBlock(), visited);
  reversePostOrder_.reverse();
}

void Dominators::postOrderVisit(BasicBlock *bb,
                                std::set<BasicBlock *> &visited) {
  visited.insert(bb);
  for (auto b : bb->getSuccBasicBlocks()) {
    if (visited.find(b) == visited.end())
      postOrderVisit(b, visited);
  }
  postOrderID_[bb] = reversePostOrder_.size();
  reversePostOrder_.push_back(bb);
}

void Dominators::createIDom(Function *f) {
  // init
  for (auto bb : f->getBasicBlocks())
    bb->setIDom(nullptr);
  auto root = f->getEntryBlock();
  root->setIDom(root);

  // iterate
  bool changed = true;
  while (changed) {
    changed = false;
    for (auto bb : this->reversePostOrder_) {
      if (bb == root) {
        continue;
      }

      // find one pred which has idom
      BasicBlock *pred = nullptr;
      for (auto p : bb->getPreBasicBlocks()) {
        if (p->getIDom()) {
          pred = p;
          break;
        }
      }
      exit_ifnot(_assertPredFalse_createIDom_Dominators, pred);

      BasicBlock *new_idom = pred;
      for (auto p : bb->getPreBasicBlocks()) {
        if (p == pred)
          continue;
        if (p->getIDom()) {
          new_idom = intersect(p, new_idom);
        }
      }
      if (bb->getIDom() != new_idom) {
        bb->setIDom(new_idom);
        changed = true;
      }
    }
  }
}

// find closest parent of b1 and b2
BasicBlock *Dominators::intersect(BasicBlock *b1, BasicBlock *b2) {
  while (b1 != b2) {
    while (postOrderID_[b1] < postOrderID_[b2]) {
      exit_ifnot(_getIDomFalse_intersect_Dominators, b1->getIDom());
      b1 = b1->getIDom();
    }
    while (postOrderID_[b2] < postOrderID_[b1]) {
      exit_ifnot(_getIDomFalse_intersect_Dominators, b2->getIDom());
      b2 = b2->getIDom();
    }
  }
  return b1;
}

void Dominators::createDominanceFrontier(Function *f) {
  for (auto bb : f->getBasicBlocks()) {
    if (bb->getPreBasicBlocks().size() >= 2) {
      for (auto p : bb->getPreBasicBlocks()) {
        auto runner = p;
        while (runner != bb->getIDom()) {
          runner->addDominanceFrontier(bb);
          runner = runner->getIDom();
        }
      }
    }
  }
}

void Dominators::createDomTreeSucc(Function *f) {
  for (auto bb : f->getBasicBlocks()) {
    auto idom = bb->getIDom();
    // e.g, entry bb
    if (idom != bb) {
      idom->addDomTreeSuccBlock(bb);
    }
  }
}

void Dominators::printIDom(Function *f) {
  int counter = 0;
  std::map<BasicBlock *, std::string> bb_id;
  for (auto bb : f->getBasicBlocks()) {
    if (bb->getName().empty())
      bb_id[bb] = "bb" + std::to_string(counter);
    else
      bb_id[bb] = bb->getName();
    counter++;
  }
  printf("Immediate dominance of function %s:\n", f->getName().c_str());
  for (auto bb : f->getBasicBlocks()) {
    std::string output;
    output = bb_id[bb] + ": ";
    if (bb->getIDom()) {
      output += bb_id[bb->getIDom()];
    } else {
      output += "null";
    }
    printf("%s\n", output.c_str());
  }
}

void Dominators::printDominanceFrontier(Function *f) {
  int counter = 0;
  std::map<BasicBlock *, std::string> bb_id;
  for (auto bb : f->getBasicBlocks()) {
    if (bb->getName().empty())
      bb_id[bb] = "bb" + std::to_string(counter);
    else
      bb_id[bb] = bb->getName();
    counter++;
  }
  printf("Dominance Frontier of function %s:\n", f->getName().c_str());
  for (auto bb : f->getBasicBlocks()) {
    std::string output;
    output = bb_id[bb] + ": ";
    if (bb->getDominanceFrontier().empty()) {
      output += "null";
    } else {
      bool first = true;
      for (auto df : bb->getDominanceFrontier()) {
        if (first) {
          first = false;
        } else {
          output += ", ";
        }
        output += bb_id[df];
      }
    }
    printf("%s\n", output.c_str());
  }
}
