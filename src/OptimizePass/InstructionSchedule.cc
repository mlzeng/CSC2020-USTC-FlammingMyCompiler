#include "InstructionSchedule.h"
#include "ReturnVal.h"

bool isAtEnd(Instruction *inst) {
  return inst->isTerminator() || inst->isCall();
}

void InstructionSchedule::run() {
  for (auto f : m_->getFunctions()) {
    if (f->getBasicBlocks().empty())
      continue;
    for (auto bb : f->getBasicBlocks()) {
      auto &insts = bb->getInstructions();
      std::list<Instruction *> results;
      auto pre_call = insts.begin();
      for (auto iter = insts.begin(); iter != insts.end();) {
        auto inst = *iter;
        if (isAtEnd(inst)) {
          std::list<Instruction *> tmp(pre_call, ++iter);
          createDAG(tmp);
          genPrePriority();
          auto rets = schedule(tmp);
          results.insert(results.end(), rets.begin(), rets.end());
          pre_call = iter;
        } else {
          ++iter;
        }
      }
      assertEqual(results, insts);
      bb->getInstructions() = results;
    }
  }
}

InstructionSchedule::Node::Node() {
  delay = -1;
  cycle = -1;
  delay_prio = -1;
  inst = nullptr;
}

InstructionSchedule::Node::Node(Instruction *i) {
  delay = -1;
  cycle = -1;
  delay_prio = -1;
  inst = i;
}

bool InstructionSchedule::notScheduleInst(Instruction *inst) {
  return inst->isPHI() || inst->isAlloca();
}

void InstructionSchedule::initNode(NodePtr node) {
  if (notScheduleInst(node->inst)) {
    node->delay = 0;
    node->unit = Unit::N;
    return;
  }
  switch (node->inst->getInstrType()) {
  case Instruction::Ret:
  case Instruction::Br:
    node->delay = 1;
    node->unit = Unit::B;
    break;
  case Instruction::Call:
    node->delay = 1; // a simple method, maybe need improve
    node->unit = Unit::B;
    break;
  case Instruction::Add:
  case Instruction::Sub:
  case Instruction::RSub:
  case Instruction::Cmp:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::ZExt: // TODO: may need delete
    node->delay = 1;
    node->unit = Unit::I;
    break;
  case Instruction::Shl:
  case Instruction::AShr:
  case Instruction::LShr:
    node->delay = 1;
    node->unit = Unit::I;
    break;
  case Instruction::Mul:
    node->delay = 3;
    node->unit = Unit::M;
    break;
  case Instruction::MulAdd:
    node->delay = 4; // or just 3 ?
    node->unit = Unit::M;
    break;
  case Instruction::Div:
    node->delay = 8; // 4-12
    node->unit = Unit::M;
    break;
  case Instruction::Load:
    node->delay = 4; // when hit in L1
    node->unit = Unit::L;
    break;
  case Instruction::Store:
    node->delay = 1;
    node->unit = Unit::S;
    break;
  case Instruction::Rem: // TODO: split rem
    node->delay = 8;
    node->unit = Unit::M;
    break;
  default:
    std::cerr << "Instruction not supported by InstructionSchedule"
              << std::endl;
    std::cerr << node->inst->CommentPrint() << std::endl;
    abort();
    break;
  }
}

void InstructionSchedule::createDAG(std::list<Instruction *> &insts) {
  succs_.clear();
  pres_.clear();
  inst2node_.clear();
  // 创建在该BB定义的inst
  for (auto inst : insts) {
    inst2node_[inst] = std::make_shared<Node>(inst);
    initNode(inst2node_[inst]);
  }
  // 创建前后向DAG
  std::list<NodePtr> loads;
  std::list<NodePtr> stores;
  for (auto inst : insts) {
    auto node = inst2node_[inst];
    if (inst->isLoad()) {
      loads.push_back(node);
      pres_[node].insert(stores.begin(), stores.end());
      for (auto n : stores) {
        succs_[n].insert(node);
      }
    } else if (inst->isStore()) {
      pres_[node].insert(stores.begin(), stores.end());
      for (auto n : stores) {
        succs_[n].insert(node);
      }

      stores.push_back(node);
      pres_[node].insert(loads.begin(), loads.end());
      for (auto l : loads) {
        succs_[l].insert(node);
      }
    }
  }
  for (auto inst : insts) {
    auto node = inst2node_[inst];
    if (pres_.find(node) == pres_.end()) {
      pres_[node] = std::set<NodePtr>();
      succs_[node] = std::set<NodePtr>();
    }
    for (auto op : inst->getOperands()) {
      auto tmp = dynamic_cast<Instruction *>(op);
      if (tmp && isDefinedInBB(tmp)) {
        pres_[node].insert(inst2node_[tmp]);
        succs_[inst2node_[tmp]].insert(node);
      }
    }
  }
}

void InstructionSchedule::genPrePriority() {
  for (auto pre : pres_) {
    if (pre.second.empty()) {
      pre.first->delay_prio = 0;
      assignPrePriority(pre.first);
    }
  }
}

void InstructionSchedule::assignPrePriority(NodePtr node) {
  for (auto pre : pres_[node]) {
    pre->delay_prio = std::max(pre->delay_prio, node->delay_prio + pre->delay);
  }
  for (auto pre : pres_[node]) {
    assignPrePriority(pre);
  }
}

std::list<Instruction *>
InstructionSchedule::schedule(std::list<Instruction *> &insts) {
  std::map<Unit, int> cur_unit = AVAILABLE_UNIT;
  std::map<Unit, NodePtrQueue> ready;
  std::list<NodePtr> running;
  std::list<Instruction *> result_insts;

  // for phi and alloca
  for (auto inst : insts) {
    if (notScheduleInst(inst)) {
      result_insts.push_back(inst);
      auto node = inst2node_[inst];
      for (auto succ : succs_[node])
        pres_[succ].erase(node);
      pres_.erase(pres_.find(node));
    }
  }

  for (auto iter = pres_.begin(); iter != pres_.end();) {
    auto pre = *iter;
    exit_ifnot(_notSecheduleInst_schedule_InstructionSchedule,
               !notScheduleInst(pre.first->inst));
    if (pre.second.empty()) {
      ready[pre.first->unit].push(pre.first);
      iter = pres_.erase(iter);
    } else {
      ++iter;
    }
  }
  int cycle = 1;
  while (!ready.empty() || !running.empty()) {
    for (auto iter = running.begin(); iter != running.end();) {
      auto node = *iter;
      if (node->cycle + node->delay < cycle) {
        cur_unit[node->unit]++;
        // br, ret, call are in the end
        if (!isAtEnd(node->inst))
          result_insts.push_back(node->inst);
        iter = running.erase(iter);
        for (auto succ : succs_[node]) {
          pres_[succ].erase(node);
          if (pres_[succ].empty()) {
            ready[succ->unit].push(succ);
            pres_.erase(pres_.find(succ));
          }
        }
      } else
        ++iter;
    }

    for (auto &unit : cur_unit) {
      while (unit.second > 0 && ready.find(unit.first) != ready.end()) {
        exit_ifnot(_ReadyEmpty_schedule_InstructionSchedule,
                   !ready[unit.first].empty());
        auto node = ready[unit.first].top();
        exit_ifnot(_Eq_schedule_InstructionSchedule, node->unit == unit.first);
        node->cycle = cycle;
        running.push_back(node);
        ready[unit.first].pop();
        if (ready[unit.first].empty()) {
          ready.erase(ready.find(unit.first));
        }
        unit.second--;
      }
    }

    cycle += 1;
  }
  result_insts.push_back(insts.back());
  if (DEBUG) {
    std::cerr << ">>>>>>>>>> before scheduling <<<<<<<<<<<" << std::endl;
    for (auto inst : insts) {
      std::cerr << inst->CommentPrint() << std::endl;
    }
    std::cerr << ">>>>>>>>>> after scheduling <<<<<<<<<<<" << std::endl;
    for (auto inst : result_insts) {
      std::cerr << inst->CommentPrint() << std::endl;
    }
  }
  assertEqual(result_insts, insts);
  return result_insts;
}

void InstructionSchedule::assertEqual(std::list<Instruction *> &a,
                                      std::list<Instruction *> &b) {
  std::unordered_set<Instruction *> a_s(a.begin(), a.end());
  std::unordered_set<Instruction *> b_s(b.begin(), b.end());

  exit_ifnot(_assertEqual_InstructionSchedule, a_s == b_s);
}
