//
// Created by cqy on 2020/7/8.
//
#include "mem2reg.h"
#include "IRBuilder.h"
#include "ReturnVal.h"

std::map<Value *, std::vector<Value *>>
    newest_live_var; //全局变量初值提前存入栈中

void Mem2RegPass::run() {
  // for (auto global_val : m_->getGlobalVariables() )
  // {
  //     if ( global_val->getInit() )
  //     {
  //         newest_live_var.insert({global_val, {global_val->getInit()}});
  //     }
  // }
  for (auto f : m_->getFunctions()) {
    func_ = f;
    if (func_->getBasicBlocks().size() >= 1) {
      GenPhi();
      ReName(func_->getEntryBlock());
    }

    for (auto bb : func_->getBasicBlocks()) {
      std::vector<Instruction *> wait_delete;
      builder = new IRBuilder(bb);
      for (auto instr : bb->getInstructions()) {
        auto alloca = dynamic_cast<AllocaInst *>(instr);
        if (alloca) {
          if (alloca->getType()->getPointerElementType()->isIntegerTy()) {
            wait_delete.push_back(instr);
          }
        }
      }
      for (auto ins : wait_delete) {
        builder->deleteInstr(ins);
      }
    }
  }
  m_->setIRLevel(Module::MIR_SSA);
}

void Mem2RegPass::GenPhi() {
  std::set<Value *> globals;
  std::map<Value *, std::set<BasicBlock *>> var_blocks;
  for (auto bb : func_->getBasicBlocks()) {
    std::set<Value *> var_kill;
    for (auto instr : bb->getInstrList()) {
      if (instr->getInstrType() == Instruction::Store) {
        auto r_val = static_cast<StoreInst *>(instr)->getRVal();
        auto l_val = static_cast<StoreInst *>(instr)->getLVal();
        auto glob = dynamic_cast<GlobalVariable *>(l_val);
        auto ins = dynamic_cast<GetElementPtrInst *>(l_val);
        if (!glob && !ins) {
          if (var_kill.find(r_val) == var_kill.end()) {
            globals.insert(l_val);
          }
          var_kill.insert(l_val);

          if (var_blocks.find(l_val) == var_blocks.end()) {
            var_blocks.insert({l_val, {bb}});
          } else {
            var_blocks.find(l_val)->second.insert(bb);
          }
        }
      }
    }
  }
  // //debug
  // for ( auto val : globals)
  // {

  //     std::cout<<"globals: "<<val->getName()<<std::endl;
  // }

  // for ( auto pair : var_blocks)
  // {
  //     std::cout<<pair.first->getName()<<" def:"<<std::endl;
  //     for ( auto def : pair.second )
  //     {
  //         std::cout<<"    "<<def->getName()<<std::endl;
  //     }
  // }
  // //debug
  std::map<BasicBlock *, std::set<Value *>> bb_phis;
  builder = new IRBuilder(nullptr);

  for (auto var : globals) {
    std::vector<BasicBlock *> work_list;
    work_list.assign(var_blocks.find(var)->second.begin(),
                     var_blocks.find(var)->second.end());
    for (int i = 0; i < work_list.size(); i++) {
      auto bb = work_list[i];
      // std::cout<<"varname: "<<var->getName()<<"work_list: "<<std::endl;
      // for ( auto pri_bb : work_list)
      // {
      //     std::cout<<pri_bb->getName()<<std::endl;
      // }
      for (auto bb_df : bb->getDominanceFrontier()) {
        if (bb_phis.find(bb_df) != bb_phis.end()) {
          builder->SetInsertPoint(bb_df);
          if (bb_phis.find(bb_df)->second.find(var) !=
              bb_phis.find(bb_df)
                  ->second.end()) { // have phi for bb_df and var
                                    // std::cout<<"have"<<std::endl;
          } else {                  // not have
            auto phi =
                builder->CreatePhi(var->getType()->getPointerElementType());
            phi->setLVal(var);
            bb_df->insertInstr(*bb_df->getInstructions().begin(), phi);
            // std::cout<<"insert";
            work_list.push_back(bb_df);
            bb_phis.find(bb_df)->second.insert(var);
          }
        } else { // not have
          auto phi =
              builder->CreatePhi(var->getType()->getPointerElementType());
          phi->setLVal(var);
          bb_df->insertInstr(*bb_df->getInstructions().begin(), phi);
          // std::cout<<"insert";
          work_list.push_back(bb_df);
          bb_phis.insert({bb_df, {var}});
        }
      }
    }
  }
  // func_->print();
}

void Mem2RegPass::ReName(BasicBlock *bb) {
  // std::cout<<"BB "<<bb->getName()<<std::endl;
  std::vector<Instruction *> wait_delete;
  for (auto instr : bb->getInstructions()) {
    if (instr->isPHI()) {
      auto l_val = static_cast<PhiInst *>(instr)->getLVal();
      if (newest_live_var.find(l_val) != newest_live_var.end()) {
        newest_live_var.find(l_val)->second.push_back(instr);
      } else {
        newest_live_var.insert({l_val, {instr}});
      }
    }
  }
  // //debug
  // for ( auto pair : newest_live_var)
  // {
  //     std::cout<<pair.first->getName()<<" def:"<<std::endl;
  //     for ( auto def : pair.second )
  //     {
  //         std::cout<<"    "<<def->getName()<<std::endl;
  //     }
  // }
  // //debug

  for (auto instr : bb->getInstructions()) {
    if (instr->getInstrType() == Instruction::Load) {
      auto l_val = static_cast<LoadInst *>(instr)->getLVal();
      auto glob = dynamic_cast<GlobalVariable *>(l_val);
      auto ins = dynamic_cast<GetElementPtrInst *>(l_val);
      if (!glob && !ins) {
        if (newest_live_var.find(l_val) != newest_live_var.end()) {
          auto iter = newest_live_var.find(l_val);
          auto newest_val = iter->second[iter->second.size() - 1];
          for (auto use : instr->getUseList()) {
            // std::cout<<use.arg_no_<<" "<<use.val_->getName()<<std::endl;
            // if(use.val_->getName() == "135")
            // {
            //     for ( auto pair : newest_live_var)
            //     {
            //         std::cout<<pair.first->getName()<<" def:"<<std::endl;
            //         for ( auto def : pair.second )
            //         {
            //             std::cout<<"    "<<def->getName()<<std::endl;
            //         }
            //     }
            // }
            static_cast<User *>(use.val_)->setOperand(use.arg_no_, newest_val);
          }
          wait_delete.push_back(instr);
        } else {
          ; //数组索引，
        }
      }
    }
    if (instr->isStore()) {
      auto l_val = static_cast<StoreInst *>(instr)->getLVal();
      auto r_val = static_cast<StoreInst *>(instr)->getRVal();
      auto glob = dynamic_cast<GlobalVariable *>(l_val);
      auto ins = dynamic_cast<GetElementPtrInst *>(l_val);
      if (!glob && !ins) {
        if (newest_live_var.find(l_val) != newest_live_var.end()) {
          newest_live_var.find(l_val)->second.push_back(r_val);
        } else {
          newest_live_var.insert({l_val, {r_val}});
        }
        // delete alloca store
        wait_delete.push_back(instr);
      }
    }
  }

  // debug

  // debug
  for (auto succ_bb : bb->getSuccBasicBlocks()) {
    // std::cout<<succ_bb->getName()<<std::endl;
    for (auto instr : succ_bb->getInstructions()) {
      if (instr->isPHI()) {
        auto l_val = static_cast<PhiInst *>(instr)->getLVal();
        // std::cout<<l_val->getName();
        if (newest_live_var.find(l_val) != newest_live_var.end()) {
          // m_->print();
          // std::cout<<"
          // "<<newest_live_var.find(l_val)->second.size()<<std::endl;
          if (newest_live_var.find(l_val)->second.size() != 0) {
            auto newest_val =
                newest_live_var.find(l_val)
                    ->second[newest_live_var.find(l_val)->second.size() - 1];
            static_cast<PhiInst *>(instr)->setParams(newest_val, bb);
          }
        }
      }
    }
  }

  // //debug
  // for ( auto pair : newest_live_var)
  // {
  //     std::cout<<pair.first->getName()<<" def:"<<std::endl;
  //     for ( auto def : pair.second )
  //     {
  //         std::cout<<"    "<<def->getName()<<std::endl;
  //     }
  // }
  // //debug

  for (auto dom_succ_bb : bb->getDomTreeSuccBlocks()) {
    // std::cout<<"bb "<<dom_succ_bb->getName()<<std::endl;
    ReName(dom_succ_bb);
  }

  for (auto instr : bb->getInstructions()) {
    if (instr->isStore() || instr->isPHI()) {
      if (instr->isStore()) {
        auto l_val = static_cast<StoreInst *>(instr)->getLVal();
        auto glob = dynamic_cast<GlobalVariable *>(l_val);
        auto ins = dynamic_cast<GetElementPtrInst *>(l_val);
        if (!glob && !ins) {
          newest_live_var.find(l_val)->second.pop_back();
          for (auto pair : newest_live_var) {
            // std::cout<<pair.first->getName()<<" def:"<<std::endl;
            // for ( auto def : pair.second )
            // {
            //     std::cout<<"    "<<def->getName()<<std::endl;
            // }
          }
        }
      } else {
        auto l_val = static_cast<PhiInst *>(instr)->getLVal();
        if (newest_live_var.find(l_val) != newest_live_var.end()) {
          newest_live_var.find(l_val)->second.pop_back();
        } else {
          exit_ifnot(_GenPhi_Mem2RegPass, false);
        }
      }
    }
  }
  builder->SetInsertPoint(bb);
  for (auto ins : wait_delete) {
    builder->deleteInstr(ins);
  }
  // std::cout<<"exit BB "<<bb->getName()<<std::endl;
}
