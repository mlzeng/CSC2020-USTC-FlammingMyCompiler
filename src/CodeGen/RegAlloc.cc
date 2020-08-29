#include <algorithm>
#include <cmath>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "CodeGen.hh"
#include "InstructionsGen.hh"
#include "LoopFind.h"

std::map<Value *, int> CodeGen::regAlloc() {
  this->spill_cost_total = 0;
  this->color_bonus_total = 0;
  this->context_active_vars.clear();
  const double store_cost = 8;
  const double load_cost = 16;
  const double alloca_cost = 2;
  const double mov_cost = 1;
  const double loop_scale = 100;
  std::map<Value *, int> mapping;
  LoopFind lf(this->module.get());
  lf.run();
  for (auto &func : this->module->getFunctions()) {
    std::map<Value *, std::set<Value *>> IG;
    std::map<Value *, double> spill_cost;
    std::map<Value *, std::map<Value *, double>> phi_bonus;
    std::map<Value *, std::map<int, double>> abi_bonus;
    std::map<Value *, double> weight;
    std::map<BasicBlock *, std::set<Value *>> live_in, live_out;
    std::set<Value *> values;
    bool mt_inside = CodeGen::is_mt_inside(func);
    // not a declaration
    if (func->getBasicBlocks().empty()) {
      continue;
    }
    // find all vars
    for (auto &args : func->getArgs()) {
      values.insert(args);
    }
    for (auto &bb : func->getBasicBlocks()) {
      for (auto &inst : bb->getInstructions()) {
        if (inst->getType()->getSize() > 0) {
          values.insert(inst);
        }
      }
    }
    // calc live in
    {
      for (auto &v : values) {
        std::queue<BasicBlock *> Q;
        if (!dynamic_cast<Instruction *>(v)) {
          live_in[func->getEntryBlock()].insert(v);
          Q.push(func->getEntryBlock());
          live_in[Q.front()].insert(v);
        } else {
          auto bb = dynamic_cast<Instruction *>(v)->getParent();
          for (auto &succ_bb : bb->getSuccBasicBlocks()) {
            Q.push(succ_bb);
            live_in[succ_bb].insert(v);
          }
        }
        auto banned = nullptr; // no banned
        while (!Q.empty()) {
          auto x = Q.front();
          Q.pop();
          for (auto &succ_bb : x->getSuccBasicBlocks()) {
            if (succ_bb != banned && !live_in[succ_bb].count(v)) {
              live_in[succ_bb].insert(v);
              Q.push(succ_bb);
            }
          }
        }
      }
    }
    // calc live out
    {
      for (auto &bb : func->getBasicBlocks()) {
        for (auto &inst : bb->getInstructions()) {
          for (auto &op : inst->getOperands()) {
            if (!values.count(op)) {
              continue;
            }
            std::queue<BasicBlock *> Q;
            if (inst->isPHI()) {
              int cnt = 0;
              Value *pre_op = nullptr;
              for (auto &op_phi : inst->getOperands()) {
                if (pre_op == op) {
                  assert(dynamic_cast<BasicBlock *>(op_phi));
                  auto x = static_cast<BasicBlock *>(op_phi);
                  Q.push(x);
                  live_out[x].insert(op);
                }
                pre_op = op_phi;
              }
            } else {
              bool flag = false;
              for (auto inst_prev : bb->getInstructions()) {
                if (inst_prev == inst) {
                  break;
                }
                if (inst_prev == op) {
                  flag = true;
                  break;
                }
              }
              if (flag) {
                continue;
              }
              for (auto &prev_bb : bb->getPreBasicBlocks()) {
                Q.push(prev_bb);
                live_out[prev_bb].insert(op);
              }
            }
            while (!Q.empty()) {
              auto x = Q.front();
              Q.pop();
              bool flag = false;
              for (auto &inst_prev : x->getInstructions()) {
                if (inst_prev == op) {
                  flag = true;
                  break;
                }
              }
              if (flag) {
                continue;
              }
              for (auto &prev_bb : x->getPreBasicBlocks()) {
                if (!live_out[prev_bb].count(op)) {
                  Q.push(prev_bb);
                  live_out[prev_bb].insert(op);
                }
              }
            }
          }
        }
      }
    }
    // debug
    if (this->debug && false) { // disabled
      std::cerr << "LIVE IN:" << std::endl;
      for (auto &p : live_in) {
        std::cerr << "  BB:" << p.first->getName() << "  ";
        for (auto &v : p.second) {
          std::cerr << "%" << v->getName() << " ";
        }
        std::cerr << std::endl;
      }
      std::cerr << "LIVE OUT:" << std::endl;
      for (auto &p : live_out) {
        std::cerr << "  BB:" << p.first->getName() << "  ";
        for (auto &v : p.second) {
          std::cerr << "%" << v->getName() << " ";
        }
        std::cerr << std::endl;
      }
    }
    // create IG
    {
      for (auto &i : func->getArgs()) {
        for (auto &j : func->getArgs()) {
          if (i != j) {
            IG[i].insert(j);
          }
        }
      }
      for (auto &bb : func->getBasicBlocks()) {
        std::set<Value *> living = live_in[bb];
        std::map<Value *, int> remain;
        // get initial remain
        for (auto &v : live_in[bb]) {
          bool flag = true;
          for (auto &inst : bb->getInstructions()) {
            for (auto &op : inst->getOperands()) {
              if (op == v && !inst->isPHI()) {
                remain[v]++;
              }
            }
            if (inst == v) {
              flag = false;
              break;
            }
          }
          if (flag) {
            if (live_out[bb].count(v)) {
              remain[v]++;
            }
          }
        }
        // connect all phis
        {
          std::set<Value *> phis;
          for (auto &inst : bb->getInstructions()) {
            if (inst->isPHI()) {
              phis.insert(inst);
            }
          }
          for (auto &i : phis) {
            for (auto &j : phis) {
              if (i != j) {
                IG[i].insert(j);
              }
            }
          }
        }
        // process all insts
        for (auto &inst : bb->getInstructions()) {
          if (!inst->isPHI()) {
            for (auto &op : inst->getOperands()) {
              if (remain[op]) {
                remain[op]--;
              }
            }
          }
          if (values.count(inst)) {
            // get context active vars
            if (inst->isCall()) {
              this->context_active_vars[inst].insert(nullptr);
              for (auto &i : living) {
                if (remain[i]) {
                  this->context_active_vars[inst].insert(i);
                }
              }
            }
            // add edges
            for (auto &i : living) {
              if (remain[i]) {
                assert(inst != i);
                IG[inst].insert(i);
                IG[i].insert(inst);
              }
            }
            // process living and remain
            living.insert(inst);
            auto insts = bb->getInstructions();
            for (auto it = insts.rbegin(); it != insts.rend(); it++) {
              auto inst_next = *it;
              if (inst_next->isPHI()) {
                continue;
              }
              if (inst_next == inst) {
                break;
              }
              for (auto &op : inst_next->getOperands()) {
                if (op == inst) {
                  remain[inst]++;
                }
              }
            }
            if (live_out[bb].count(inst)) {
              remain[inst]++;
            }
          }
        }
      }
    }
    // phi bonus and zext bonus and abi bonus and spill cost
    {
      for (auto &bb : func->getBasicBlocks()) {
        int nested_level = 0;
        for (auto bs = lf.getBaseLoop(bb); bs != nullptr;
             bs = lf.getParentLoop(bs)) {
          nested_level++;
        }
        // std::cerr << nested_level << std::endl;
        double scale_factor = pow(loop_scale, nested_level);
        for (auto &inst : bb->getInstructions()) {
          const auto &ops = inst->getOperands();
          // phi bonus
          if (inst->getInstrType() == Instruction::PHI) {
            for (auto &op : ops) {
              if (values.count(op)) {
                phi_bonus[inst][op] += mov_cost * scale_factor;
                phi_bonus[op][inst] += mov_cost * scale_factor;
                if (values.count(inst)) {
                  weight[inst] += mov_cost * scale_factor;
                }
                if (values.count(op)) {
                  weight[op] += mov_cost * scale_factor;
                }
              }
            }
          }
          // zext bonus
          if (inst->getInstrType() == Instruction::ZExt) {
            auto op = ops.at(0);
            if (values.count(op)) {
              phi_bonus[inst][op] += mov_cost * scale_factor;
              phi_bonus[op][inst] += mov_cost * scale_factor;
              weight[inst] += mov_cost * scale_factor;
              weight[op] += mov_cost * scale_factor;
            }
          }
          // abi bonus
          if (inst->getInstrType() == Instruction::Call) {
            int cnt = 0;
            for (auto &op : ops) {
              if (op == ops.front()) {
                continue;
              }
              if (cnt < 4) {
                abi_bonus[op][cnt] += mov_cost * scale_factor;
                weight[op] += mov_cost * scale_factor;
              }
              cnt++;
            }
          }
          // spill cost
          for (auto &op : inst->getOperands()) {
            if (values.count(op)) {
              if (inst->isAlloca()) {
                spill_cost[op] += alloca_cost * scale_factor;
                weight[op] += alloca_cost * scale_factor;
              } else {
                spill_cost[op] += load_cost * scale_factor;
                weight[op] += load_cost * scale_factor;
              }
            }
          }
          if (values.count(inst) && !inst->isAlloca()) {
            spill_cost[inst] += store_cost * scale_factor;
            weight[inst] += store_cost * scale_factor;
          }
        }
      }
      // abi coloring
      {
        int cnt = 0;
        for (auto &arg : func->getArgs()) {
          if (cnt < 4) {
            abi_bonus[arg][cnt] += mov_cost;
            weight[arg] += mov_cost;
          }
          spill_cost[arg] += store_cost;
          weight[arg] += store_cost;
          cnt++;
        }
      }
    }
    // function insts and args coloring
    {
      std::vector<std::pair<double, Value *>> inst_list;
      for (auto &inst : values) {
        inst_list.push_back({weight[inst], inst});
      }
      std::sort(inst_list.begin(), inst_list.end());
      std::reverse(inst_list.begin(), inst_list.end());
      for (auto &p : inst_list) {
        auto inst = p.second;
        if (!mapping.count(inst)) {
          auto allowed_regs = allocate_regs;
          // erase thread id reg in allowed list
          if (mt_inside) {
            allowed_regs.erase(InstGen::Reg(thread_id_reg));
          }
          for (auto &adj : IG[inst]) {
            if (mapping.count(adj)) {
              allowed_regs.erase(InstGen::Reg(mapping.at(adj)));
            }
          }
          if (!allowed_regs.empty()) {
            double bonus = -1;
            int id = allowed_regs.begin()->getID();
            for (auto &i : allowed_regs) {
              int new_id = i.getID();
              double new_bonus = 0;
              for (auto &v : phi_bonus[inst]) {
                if (mapping.count(v.first) && mapping.at(v.first) == new_id) {
                  new_bonus += v.second;
                }
              }
              new_bonus += abi_bonus[inst][new_id];
              if (new_bonus > bonus) {
                bonus = new_bonus;
                id = new_id;
              }
            }
            this->color_bonus_total += bonus;
            mapping[inst] = id;
          } else {
            this->spill_cost_total += spill_cost[inst];
          }
        }
      }
    }
  }
  return mapping;
}

void CodeGen::printIR() {
  LoopFind lf(this->module.get());
  lf.run();
  for (auto &func : this->module->getFunctions()) {
    if (func->getBasicBlocks().empty()) {
      continue;
    }
    std::cerr << "define " << func->getFunctionType()->CommentPrint() << "@"
              << func->getName() << "(";
    for (auto &arg : func->getArgs()) {
      std::cerr << "["
                << (this->register_mapping.count(arg)
                        ? std::string("REG ") +
                              std::to_string(this->register_mapping.at(arg))
                        : "STACK")
                << "]" << arg->getType()->CommentPrint() << " "
                << "%" << arg->getName()
                << (arg == func->getArgs().back() ? "" : ",  ");
    }
    std::cerr << ")" << std::endl;
    for (auto &bb : func->getBasicBlocks()) {
      int nested_level = 0;
      for (auto bs = lf.getBaseLoop(bb); bs != nullptr;
           bs = lf.getParentLoop(bs)) {
        nested_level++;
      }
      std::cerr << "<label>" << bb->getName() << ":   "
                << "level=" << nested_level << "   "
                << "preds=";
      for (auto &pred_bb : bb->getPreBasicBlocks()) {
        if (pred_bb == bb->getPreBasicBlocks().front()) {
          std::cerr << "(";
        }
        std::cerr << "%" << pred_bb->getName();
        if (pred_bb == bb->getPreBasicBlocks().back()) {
          std::cerr << ")";
        } else {
          std::cerr << " ";
        }
      }
      std::cerr << "   ";
      std::cerr << "succs=";
      for (auto &succ_bb : bb->getSuccBasicBlocks()) {
        if (succ_bb == bb->getSuccBasicBlocks().front()) {
          std::cerr << "(";
        }
        std::cerr << "%" << succ_bb->getName();
        if (succ_bb == bb->getSuccBasicBlocks().back()) {
          std::cerr << ")";
        } else {
          std::cerr << " ";
        }
      }
      std::cerr << std::endl;
      for (auto &inst : bb->getInstructions()) {
        std::cerr << (this->in_mt_env[inst] != nullptr ? " MT " : "    ")
                  << (this->register_mapping.count(inst)
                          ? std::string("REG ") +
                                std::to_string(this->register_mapping.at(inst))
                          : (inst->getType()->getSize() > 0
                                 ? std::string("STACK")
                                 : std::string("     ")))
                  << "    " << inst->CommentPrint() << std::endl;
      }
    }
  }
}

void CodeGen::printStat() {
  int max_reg_used = -1;
  int spill_count = 0;
  for (auto &func : this->module->getFunctions()) {
    std::set<int> used_regs;
    if (func->getBasicBlocks().empty()) {
      continue;
    }
    for (auto &arg : func->getArgs()) {
      if (!this->register_mapping.count(arg)) {
        spill_count++;
      } else {
        used_regs.insert(this->register_mapping.at(arg));
      }
    }
    for (auto &bb : func->getBasicBlocks()) {
      for (auto &inst : bb->getInstructions()) {
        if (inst->getType()->getSize() > 0) {
          if (!this->register_mapping.count(inst)) {
            spill_count++;
          } else {
            used_regs.insert(this->register_mapping.at(inst));
          }
        }
      }
    }
    max_reg_used = std::max(max_reg_used, (int)used_regs.size());
  }
  std::cerr << "Number of Used Regs: " << max_reg_used << std::endl;
  std::cerr << "Number of Spills: " << spill_count << std::endl;
  std::cerr << "Total Spill Costs: " << this->spill_cost_total << std::endl;
  std::cerr << "Total Color Bonus: " << this->color_bonus_total << std::endl;
}