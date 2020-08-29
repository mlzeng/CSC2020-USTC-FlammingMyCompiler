//
// Created by cqy on 2020/7/8.
//
#include "ReachDefinitions.h"

bool out_change_flag = true;
std::map<Value *, std::set<Value *>> GlobalDefinitions;

ReachDefinitionsPass::ReachDefinitionsPass(Module *m) : module_(m) {
  runModule();
  // GenPhiDeleteLS();
}

ReachDefinitionsPass::~ReachDefinitionsPass() {}

void ReachDefinitionsPass::runModule() {
  for (auto global_val : module_->getGlobalVariables()) {
    if (global_val->getInit() != nullptr) {
      GlobalDefinitions.insert({global_val, {global_val->getInit()}});
    }
  }

  for (auto func : module_->getFunctions()) {
    // std::cout<<func->getName()<<std::endl;
    runFunction(func);
  }
}

void ReachDefinitionsPass::runFunction(Function *f) {
  while (out_change_flag) {
    out_change_flag = false;
    for (auto bb : f->getBasicBlocks()) {
      auto &in = bb->getBBAssignIn();
      in.clear();
      for (auto pre_bb : bb->getPreBasicBlocks()) {
        auto &pre_out = pre_bb->getBBAssignOut();
        // in.insert(pre_out.begin(),pre_out.end());
        for (auto out_iter : pre_out) {
          auto in_iter = in.find(out_iter.first);
          if (in_iter != in.end()) { // union opreation
            for (auto str_instr : out_iter.second) {
              in_iter->second.insert(str_instr);
            }
          } else {
            in.insert(out_iter);
          }
        }
        in.insert(GlobalDefinitions.begin(), GlobalDefinitions.end());
      }

      runBasicBlock(bb);
    }
  }
  out_change_flag = true;
}

void ReachDefinitionsPass::runBasicBlock(BasicBlock *bb) {
  auto &bb_in_map = bb->getBBAssignIn();
  auto &bb_out_map = bb->getBBAssignOut();
  std::map<Value *, std::set<Value *>> cur_map;
  cur_map.insert(bb_in_map.begin(), bb_in_map.end());

  for (auto instr : bb->getInstrList()) {
    if (instr->getInstrType() == Instruction::Store ||
        instr->getInstrType() == Instruction::PHI) {
      runStorePhiInst(instr, cur_map);
    }
  }
  // 更新 out_change_flag
  if (cur_map.size() != bb_out_map.size()) {
    out_change_flag = true;
    bb_out_map.clear();
    bb_out_map.insert(cur_map.begin(), cur_map.end());
    return;
  } else {
    for (auto cur_map_iter : cur_map) {
      auto out_map_iter = bb_out_map.find(cur_map_iter.first);
      if (out_map_iter != bb_out_map.end()) {
        if (out_map_iter->second.size() != cur_map_iter.second.size()) {
          out_change_flag = true;
          bb_out_map.clear();
          bb_out_map.insert(cur_map.begin(), cur_map.end());
          return;
        } else {
          for (auto cur_map_instr : cur_map_iter.second) {
            auto out_map_instr = out_map_iter->second.find(cur_map_instr);
            if (out_map_instr == out_map_iter->second.end()) {
              out_change_flag = true;
              bb_out_map.clear();
              bb_out_map.insert(cur_map.begin(), cur_map.end());
              return;
            }
          }
        }
      } else {
        out_change_flag = true;
        bb_out_map.clear();
        bb_out_map.insert(cur_map.begin(), cur_map.end());
        return;
      }
    }
  }
  // get out according to in
}

void ReachDefinitionsPass::runStorePhiInst(
    Instruction *instr, std::map<Value *, std::set<Value *>> &out) {
  if (instr->getInstrType() == Instruction::Store) {
    auto l_val = static_cast<StoreInst *>(instr)->getLVal();
    auto map_iter = out.find(l_val);
    if (map_iter != out.end()) {
      auto set_iter = map_iter->second.find(instr);
      if (set_iter != map_iter->second.end() &&
          map_iter->second.size() == 1) { // set中只有这一个元素，则不更新out
        ;
      } else {
        map_iter->second.clear();
        map_iter->second.insert(instr);
      }
    } else {
      std::set<Value *> instr_assign_set;
      instr_assign_set.insert(instr);
      out.insert({l_val, instr_assign_set});
    }
    return;
  } else if (instr->getInstrType() == Instruction::PHI) {
    auto l_val = static_cast<PhiInst *>(instr)->getLVal();
    auto map_iter = out.find(l_val);
    if (map_iter != out.end()) {
      auto set_iter = map_iter->second.find(instr);
      if (set_iter != map_iter->second.end() &&
          map_iter->second.size() == 1) { // set中只有这一个元素，则不更新out
        ;
      } else {
        map_iter->second.clear();
        map_iter->second.insert(instr);
      }
    } else {
      std::set<Value *> instr_assign_set;
      instr_assign_set.insert(instr);
      out.insert({l_val, instr_assign_set});
    }
    return;
  }
}

// void ReachDefinitionsPass::GenPhiDeleteLS()
// {
//     auto builder = new IRBuilder(nullptr);
//     for ( auto func : module_->getFunctions())
//     {
//         for ( auto bb : func->getBasicBlocks())
//         {
//             std::vector<Instruction *> wait_delete;
//             builder->SetInsertPoint(bb);

//             auto bb_in_map = bb->getBBAssignIn();
//             std::map<Value *, std::set<Instruction *>> cur_map;
//             cur_map.insert(bb_in_map.begin(), bb_in_map.end());

//             for (auto instr : bb->getInstrList())
//             {
//                 if ( instr->getInstrType()==Instruction::Load )
//                 {
//                     auto l_val = static_cast<LoadInst *>(instr)->getLVal();
//                     auto map_iter = cur_map.find(l_val);
//                     if ( map_iter != cur_map.end() )
//                     {
//                         if ( map_iter->second.size()>1)
//                         {//gen phi
//                             std::vector<Value *> vals;
//                             std::vector<BasicBlock *> pre_bbs;

//                             for ( auto assign_instr : map_iter->second)
//                             {
//                                 if (
//                                 assign_instr->getInstrType()==Instruction::Store
//                                 )
//                                 {
//                                     vals.push_back(static_cast<StoreInst
//                                     *>(assign_instr)->getRVal());
//                                 }
//                                 else if
//                                 (assign_instr->getInstrType()==Instruction::PHI)
//                                 {
//                                     vals.push_back(assign_instr);
//                                 }
//                                 pre_bbs.push_back(assign_instr->getParent());
//                             }

//                             auto phi = builder->CreatePhi(vals, pre_bbs,
//                             instr->getType()); phi->setLVal(l_val);
//                             builder->insertInstr( instr, phi );
//                             runFunction(func);
//                             // builder->deleteInstr(instr);
//                             for ( auto use : instr->getUseList())
//                             {
//                                 auto user = use.val_;
//                                 static_cast<User *>(user)->setOperand(
//                                 use.arg_no_, phi);
//                             }
//                             //add phi to instr set
//                         }
//                         else
//                         {
//                             for ( auto use : instr->getUseList())
//                             {
//                                 auto user = use.val_;
//                                 auto new_assign = map_iter->second.begin();
//                                 if (
//                                 (*new_assign)->getInstrType()==Instruction::Store
//                                 )
//                                 {
//                                     static_cast<User *>(user)->setOperand(
//                                     use.arg_no_, static_cast<StoreInst
//                                     *>(*new_assign)->getRVal());
//                                 }
//                                 else if
//                                 ((*new_assign)->getInstrType()==Instruction::PHI)
//                                 {
//                                     static_cast<User *>(user)->setOperand(
//                                     use.arg_no_, (*new_assign));
//                                 }
//                             }
//                         }
//                     }
//                     wait_delete.push_back(instr);
//                 }
//                 else if (
//                 instr->getInstrType()==Instruction::Store||instr->getInstrType()==Instruction::PHI
//                 )
//                 {//维护cur_map
//                     runStorePhiInst( instr, cur_map);
//                 }
//             }
//             for ( auto map_iter : bb->getBBAssignOut() )
//             {
//                 //遍历cur map来生成phi
//                 if( map_iter.second.size()>1 )
//                 {
//                     std::vector<Value *> vals;
//                     std::vector<BasicBlock *> pre_bbs;

//                     for ( auto assign_instr : map_iter.second)
//                     {
//                         if ( assign_instr->getInstrType()==Instruction::Store
//                         )
//                         {
//                             vals.push_back(static_cast<StoreInst
//                             *>(assign_instr)->getRVal());
//                         }
//                         else if
//                         (assign_instr->getInstrType()==Instruction::PHI)
//                         {
//                             vals.push_back(assign_instr);
//                         }
//                         pre_bbs.push_back(assign_instr->getParent());
//                     }

//                     auto phi = builder->CreatePhi(vals, pre_bbs,
//                     vals[0]->getType()); phi->setLVal(map_iter.first );
//                     builder->insertInstr(
//                     builder->GetInsertBlock()->getTerminator(), phi );
//                     runFunction(func);
//                 }
//             }
//             // std::cout<<"main"<<std::endl;
//             for ( auto instr : wait_delete )
//             {
//                 builder->deleteInstr(instr);
//             }
//         }
//         for ( auto bb : func->getBasicBlocks())
//         {
//             std::vector<Instruction *> wait_delete;
//             builder->SetInsertPoint(bb);
//             for (auto instr : bb->getInstrList())
//             {
//                 if (
//                 instr->getInstrType()==Instruction::Store||instr->getInstrType()==Instruction::Alloca
//                 )
//                 {
//                     wait_delete.push_back(instr);
//                 }
//             }
//             for ( auto instr : wait_delete )
//             {
//                 builder->deleteInstr(instr);
//             }
//         }
//     }
// }