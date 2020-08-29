#ifndef SYSYC_INSTRUCTIONSCHEDULE_H
#define SYSYC_INSTRUCTIONSCHEDULE_H

#include "Instruction.h"
#include "PassManager.h"
#include <memory>
#include <queue>

class InstructionSchedule : public Transform {
public:
  const bool DEBUG = false;

public:
  InstructionSchedule(Module *m) : Transform(m) {}
  void run() override;
  void createDAG(std::list<Instruction *> &insts);
  void genPrePriority();
  std::list<Instruction *> schedule(std::list<Instruction *> &insts);
  bool notScheduleInst(Instruction *inst);
  void assertEqual(std::list<Instruction *> &a, std::list<Instruction *> &b);

public:
  using OpID = Instruction::OpID;
  enum Unit {
    B, // Branch μops
    I, // Integer ALU μops
    M, // Integer shift-ALU, multiply, divide, CRC and
       // sum-of-absolute-differences μops
    L, // Load and register transfer μops
    S, // Store and special memory μops
    N, // op which are not considered during scheduling, e.g., alloca, phi
  };
  const std::map<Unit, int> AVAILABLE_UNIT = {
      {B, 1}, {I, 2}, {M, 1}, {L, 1}, {S, 1}};
  struct Node {
    Instruction *inst;
    int delay;
    int cycle;
    int delay_prio;
    Unit unit;
    OpID id;
    std::list<Node *> succ;
    Node();
    Node(Instruction *i);
  };
  using NodePtr = std::shared_ptr<Node>;
  struct NodePtrCmp {
    bool operator()(NodePtr a, NodePtr b) {
      return a->delay_prio < b->delay_prio;
    }
  };
  using NodePtrQueue =
      std::priority_queue<NodePtr, std::vector<NodePtr>, NodePtrCmp>;

private:
  bool isDefinedInBB(Instruction *inst) {
    return inst2node_.find(inst) != inst2node_.end();
  }
  void initNode(NodePtr node);
  void assignPrePriority(NodePtr node);

private:
  std::map<NodePtr, std::set<NodePtr>>
      succs_; // 后继，使用者集合，使用key的节点
  std::map<NodePtr, std::set<NodePtr>> pres_; // 前驱，操作数集合，key依赖的节点
  std::map<Instruction *, NodePtr> inst2node_;
};

#endif // SYSYC_INSTRUCTIONSCHEDULE_H
