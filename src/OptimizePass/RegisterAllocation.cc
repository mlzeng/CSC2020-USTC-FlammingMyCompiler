#include "RegisterAllocation.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

const int numReg = 11;
typedef struct IGNode {
  int color;
  std::vector<Value *> vals;
  bool inIG;
  int spillCost;
} node_t;

// typedef std::unordered_map<value*, int> Val2Col_t;
// typedef std::unordered_map<value*, node_t*> Val2Nod_t;
using Val2Nod_t = std::unordered_map<Value *, node_t *>;
using Val2Col_t = std::map<Value *, int>;
class InterfaceGraph;
typedef class InterfaceGraph IG_t;

void _printIG(IG_t *IG);
void _printMap(std::map<Value *, int> *map);
void _printNode(node_t *node);

// #define DEBUG
// #define DEBUG_PHI
class InterfaceGraph {
public:
  Val2Nod_t *val2nod;
  std::vector<std::vector<node_t *> *> G;
  int nodeCount;

  void initIG(class Function *func, Val2Nod_t *Val2Nod) {
    std::unordered_set<node_t *> in;
    nodeCount = 0;
#ifdef DEBUG
    std::cout << "InitIG By Insert:\n";
#endif
    for (auto it = Val2Nod->begin(); it != Val2Nod->end(); it++) {
      if (in.find(it->second) == in.end()) {
#ifdef DEBUG
        std::cout << "Insert\t";
        _printNode(it->second);
        std::cout << "\n";
#endif

        G.push_back(new std::vector<node_t *>{it->second});
        it->second->inIG = true;
        nodeCount++;
        in.insert(it->second);
      }
    }
#ifdef DEBUG
    std::cout << "END Init:\n";
#endif
    val2nod = Val2Nod;
  }
  node_t *popNodeWithDegreeLessThan_k() {
    int count;
    for (auto list : G) {
      if (!list->at(0)->inIG)
        continue;
      count = 0;
      for (auto n : *list) {
        if (n->inIG) {
          count++;
        }
      }

      if (count <= numReg) {
        auto node = list->at(0);
        nodeCount--;
        node->inIG = false;
        return node;
      }
    }
    return nullptr;
  }
  node_t *popLeastCostNode() {
    int cost = INT32_MAX;
    node_t *p = nullptr;
    for (auto list : G) {
      auto node = list->at(0);
      if (node->inIG == true && node->spillCost < cost) {
        p = node;
        cost = node->spillCost;
      }
    }
    assert(p != nullptr);
    p->inIG = false;
    nodeCount--;
    return p;
  }

  int recover(node_t *node) {
    node->inIG = true;
    nodeCount++;
    if (node->color != -1)
      return node->color;

    std::vector<node_t *> *list = nullptr;
    for (auto tmp : G) {
      auto first = tmp->at(0);
      if (first == node) {
        list = tmp;
      }
    }
    assert(list != nullptr);

    int color = 1;
    while (true) {
      bool useThis = true;
      for (auto node : *list) {
        if (node->color == color) {
          useThis = false;
        }
      }
      if (useThis) {
        node->color = color;
        return color;
      } else {
        color++;
      }
    }
  }

  void insertEage(node_t *n, node_t *n1) {
    int count = 2;
    for (auto vec : G) {
      auto first = vec->at(0);
      if (first == n1) {
        auto tmp = n1;
        n1 = n;
        n = tmp;
      } else if (first != n)
        continue;

      if (std::find(vec->begin(), vec->end(), n1) == vec->end())
        vec->push_back(n1);
      if (--count == 0)
        break;
    }
  }

  bool isEmpty() { return nodeCount == 0; }
  node_t *getNode(Value *val) {
    node_t *tmp;
    try {
      tmp = val2nod->at(val); // TODO reference 会不会导致比较失败？
    } catch (std::out_of_range &a) {
      return nullptr;
    }
    return tmp;
  }

  // void degree(node_t *node);
  // Val2Nod_t* val2nod;
  // std::vector<std::list<node_t*>> G;
};
typedef class InterfaceGraph IG_t;

void clearNode(Val2Nod_t &map) {
  std::unordered_set<node_t *> deleted;
  for (auto tmp = map.begin(); tmp != map.end(); tmp++) {
    if (deleted.find(tmp->second) == deleted.end()) {
      deleted.insert(tmp->second);
      delete tmp->second;
    }
  }
}

int spillCost(node_t *node) { return 1; }

void tryMerge(
    node_t *phi, node_t *prePhi,
    std::unordered_map<Value *, std::unordered_set<Value *>> *coexists) {
  // std::cout << "tryMerge : To be implemented\n";//todo
}

void findNodes(
    Val2Nod_t *val2node, Function *func,
    std::unordered_map<Value *, std::unordered_set<Value *>> *coexists,
    std::unordered_set<Value *> *definedSet) {
  // TODO pillCost评估
  int colorCount = 0;
  std::unordered_map<Value *, int> argMap;
  for (auto arg : func->getArgs()) { // TODO isPHI 如果没用就删了
    node_t *node = new node_t;
    node->vals.push_back(arg);
    node->spillCost = spillCost(node);
    if (colorCount < 4)
      argMap.insert({arg, ++colorCount});
    node->color = -1;
    val2node->insert({arg, node});
    assert(colorCount < 8);
  }

  for (auto BB : func->getBasicBlocks()) {
    for (auto instr : BB->getInstructions()) {
      if (definedSet->find(instr) != definedSet->end()) {
        auto node = new node_t;
        node->vals.push_back(instr);
        node->spillCost = spillCost(node);
        node->color = -1;
        val2node->insert({instr, node});
      }
    }
  }

  for (auto BB : func->getBasicBlocks()) {
    for (auto instr : BB->getInstructions()) {
      if (!instr->isPHI())
        continue;
      node_t *node = val2node->at(instr);
      node_t *prePhi = nullptr;
      assert(node->vals.size() > 0);
      if (node->vals.size() != 1) {
        prePhi = node;
        prePhi->vals.erase(
            std::find(prePhi->vals.begin(), prePhi->vals.end(), instr));
        auto n = new node_t;
        n->vals.push_back(instr);
        n->spillCost = spillCost(n);
        n->color = -1;
        val2node->at(instr) = n;
      }

#ifdef DEGUG_PHI
      instr->print();
      std::cout << ":\nTo find Intented Node\n";
      getchar();
#endif
      auto vec = instr->getOperands();
      std::unordered_set<Value *> intentPhiSet;
      for (int i = 0; i < vec.size(); i++) {
        if (definedSet->find(vec[i]) == definedSet->end())
          continue;
        auto node_i = val2node->at(vec[i]);
        for (auto val : node_i->vals)
          intentPhiSet.insert(val);
      }

#ifdef DEGUG_PHI
      std::cout << "erease vals interface at phi instr\n";
      getchar();
#endif
      for (auto val : coexists->at(instr)) {
        auto it = intentPhiSet.find(val);
        if (it != intentPhiSet.end())
          intentPhiSet.erase(it);
      }

#ifdef DEGUG_PHI
      std::cout << "Other erease types\n";
      getchar();
#endif
      std::unordered_set<Value *> removeSet;
      for (auto val : intentPhiSet) {
        const auto &co = coexists->at(val);
        if (co.find(instr) != co.end())
          removeSet.insert(val);
        else { // TODO 可优化，两个相互干扰，可以移除一个
          for (auto co_item : co) {
            if (intentPhiSet.find(co_item) != intentPhiSet.end() &&
                removeSet.find(co_item) == removeSet.end()) {
              removeSet.insert(val);
              break;
            }
          }
        }
      }
#ifdef DEGUG_PHI
      std::cout << "Remove:\t";
      for (auto val : removeSet) {
        val->print();
      }
      std::cout << "from\t";

      for (auto val : intentPhiSet) {
        val->print();
      }
      putchar('\n');
#endif
      for (auto val : removeSet) {
        intentPhiSet.erase(val);
      }

#ifdef DEGUG_PHI
      std::cout << "insert vals to phi\n";
#endif
      node_t *phiNode = val2node->at(instr);
      for (auto val : intentPhiSet) {
        node_t *node = val2node->at(val);
        auto &vec = node->vals;
        vec.erase(std::find(vec.begin(), vec.end(), val));
        assert(std::find(node->vals.begin(), node->vals.end(), val) ==
               node->vals.end());
        phiNode->vals.push_back(val);
        val2node->at(val) = phiNode;
        if (vec.size() == 0) {
          delete node;
        }
      }

#ifdef DEGUG_PHI
      std::cout << "Result node:\n";
      _printNode(phiNode);
      putchar('\n');
#endif
      if (prePhi)
        tryMerge(phiNode, prePhi, coexists);

      // for ( auto arg : func->getArgs()) {
      //     if (std::find(phiNode->vals.begin(), phiNode->vals.end(), arg) !=
      //     phiNode->vals.end()) {
      //         std::cerr << "Find func args in Phi node!!!\n";
      //         abort();
      //     }
      // }
    }
  }

  for (auto arg : func->getArgs()) {
    val2node->at(arg)->color = argMap.at(arg);
  }

#ifdef DEBUG
// for (auto m : *val2node) {
//     auto item = m.second;
//     putchar('(');
//     for (auto val : item->vals) {
//         val->print();
//     }
//     std::cout << ")\t";
// }
// putchar('\n');
#endif
}

// Create Ig
// extern std::unordered_set<value*> *BBout(BasicBlock* BB);

void CreateInterfaceGraph(
    IG_t *IG, Function *func,
    std::unordered_map<Value *, std::unordered_set<Value *>> *coexists) {
#ifdef DEBUG
  std::cout << "---------BUILD IG-------------\n";
#endif
  for (auto it : *coexists) {
    auto n = IG->getNode(it.first);
    assert(n);
    for (const auto co_item : it.second) {
      auto co_node = IG->getNode(co_item);
      assert(co_item && co_node != n);
      IG->insertEage(n, co_node);
    }
  }
#ifdef DEBUG
  _printIG(IG);
  getchar();
#endif
#ifdef DEBUG
  std::cout << "--------END BUILD--------\n";
#endif
}

// ColorIG 自底向上实现
void ColorIG(IG_t *G, Val2Col_t *val2col) {

  std::vector<node_t *> stack;
#ifdef DEBUG
  std::cout << "------------ ColorIG -----------\n";
#endif

  while (!G->isEmpty()) {
    auto node = G->popNodeWithDegreeLessThan_k();
    if (!node)
      node = G->popLeastCostNode();
#ifdef DEBUG
    std::cout << "Poped Node\t";
    _printNode(node);
    putchar('\n');
#endif
    stack.push_back(node);
  }
  // std::cout <<'\n';
  while (!stack.empty()) {
    auto node = stack.back();
    auto color = G->recover(node);
    for (auto val : node->vals) {
      val2col->insert({val, color});
    }
    stack.pop_back();
  }
}

void findCoexists(Function *func,
                  std::unordered_map<Value *, std::unordered_set<Value *>> *map,
                  std::unordered_set<Value *> *definedVals) {

  for (auto arg : func->getArgs()) {
    map->insert({arg, std::unordered_set<Value *>()});
    assert(map->at(arg).size() == 0);
  }

  for (auto BB : func->getBasicBlocks()) {
    std::unordered_set<Value *> liveSet;
    for (auto val : BB->getActiveOuts()) {
      liveSet.insert(val);
    }

    for (auto it = BB->rbegin(); it != BB->rend(); it++) {
      auto instr = *it;

      if (definedVals->find(instr) != definedVals->end()) {
        // auto set = new std::unordered_set<Value*>;
        if (!instr->isPHI()) {
          liveSet.erase(instr);
        }
        map->insert({instr, std::unordered_set<Value *>()});
        for (auto val : liveSet) {
          if (val == instr)
            continue;
          map->at(instr).insert(val);
        }
      }

      if (instr->isPHI())
        continue;
      for (auto val : instr->getOperands()) {
        if (definedVals->find(val) != definedVals->end())
          liveSet.insert(val);
      }
    }
  }
#ifdef DEGUG_PHI
  std::cout << "\n-------------findCoexists--------------\n";
  for (auto it : *map) {
    it.first->print();
    std::cout << " with\t";
    for (auto val : it.second) {
      val->print();
      putchar('\t');
    }
    putchar('\n');
  }
#endif
}

std::map<Value *, int> *RegisterAllocation(Module *module) {
  std::map<Value *, int> *colorTable = new std::map<Value *, int>;
  std::unordered_set<Value *> definedVals;

  for (auto func : module->getFunctions()) { // TODO 重做了一遍active的内容
    if (!func->getBasicBlocks().size())
      continue;

    for (auto arg : func->getArgs()) {
      definedVals.insert(arg);
    }
    for (auto BB : func->getBasicBlocks()) {
      for (auto instr : BB->getInstructions()) {
        if (!instr->isVoid() && !instr->isAlloca()) {
          definedVals.insert(instr);
        }
      }
    }
  }
  for (auto func : module->getFunctions()) {
    if (func->getBasicBlocks().size() == 0)
      continue;
#ifdef DEBUG
    std::cout << "---------------------" << func->getName()
              << "-----------------\n";
    std::cout << "\n\nFindNodes:\n";
#endif

    std::unordered_map<Value *, std::unordered_set<Value *>> coexistedSet;
    findCoexists(func, &coexistedSet, &definedVals);

    Val2Nod_t IGNodeMap;
    findNodes(&IGNodeMap, func, &coexistedSet, &definedVals);

    IG_t IG;
    IG.initIG(func, &IGNodeMap);
    CreateInterfaceGraph(&IG, func, &coexistedSet);

    Val2Col_t val2col;
    ColorIG(&IG, &val2col);

    colorTable->insert(val2col.begin(), val2col.end());

#ifdef DEBUG
    _printMap(colorTable);
    getchar();
#endif

    clearNode(IGNodeMap);
  }

  for (auto tmp : *colorTable) {
    colorTable->at(tmp.first) = tmp.second - 1;
  }

  // DEBUG
  // _printMap(colorTable);
  // bool existdZero=false;
  // for (auto tmp : *colorTable) {
  //     if (tmp.second == 0) {
  //         existdZero = true;
  //     }
  // }
  // assert(existdZero);

  return colorTable;
}

void _printIG(IG_t *IG) {
  std::cout << "IG:\n";
  for (auto list : IG->G) {
    for (auto item : *list) {
      _printNode(item);
    }
    putchar('\n');
  }
}

void _printNode(node_t *node) {
  putchar('(');
  for (auto p : node->vals) {
    p->print();
  }
  std::cout << " " << node->color;
  putchar(')');
}

void _printMap(std::map<Value *, int> *map) {
  std::cout << "ColorMap:\n";
  for (auto p : *map) {
    p.first->print();
    std::cout << ":\t" << p.second << '\n';
  }
}