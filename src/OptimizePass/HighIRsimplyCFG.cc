#include "HighIRsimplyCFG.h"

void HighIRsimplyCFG::run() {
  for (auto fun : m_->getFunctions()) {
    func_ = fun;
    MergeSinglePredecessorBB();
  }
}

void HighIRsimplyCFG::MergeSinglePredecessorBB() {
  for (auto basebb = func_->getBaseBlocks().begin();
       basebb != func_->getBaseBlocks().end();) {
    if ((*basebb)->isBasicBlock()) {
      basebb++;
      auto succ_bb = basebb;
      basebb--;
      if (succ_bb != func_->getBaseBlocks().end() &&
          MergeBasicBlock(dynamic_cast<BasicBlock *>(*basebb), *succ_bb)) {
        func_->removeBaseBlock(*succ_bb);
      } else {
        basebb++;
      }

    } else if ((*basebb)->isIfBlock()) {
      MergeIfBB(dynamic_cast<IfBlock *>((*basebb)));
      basebb++;
    } else if ((*basebb)->isWhileBlock()) {
      MergeWhileBB(dynamic_cast<WhileBlock *>((*basebb)));
      basebb++;
    }
  }
  // removeAllBB();
  // for( auto basebb : wait_delete_ )
  // {
  //     func_->removeBaseBlock(basebb);
  // }
  // wait_delete_.clear();
}

void HighIRsimplyCFG::MergeWhileBB(WhileBlock *whilebb) {
  auto &list1 = whilebb->getCondBBs();
  for (auto basebb = list1.begin(); basebb != list1.end();) {
    if ((*basebb)->isBasicBlock()) {
      basebb++;
      auto succ_bb = basebb;
      basebb--;
      if (succ_bb != list1.end() &&
          MergeBasicBlock(dynamic_cast<BasicBlock *>(*basebb), *succ_bb)) {
        whilebb->removeCondBB(*succ_bb);
      } else {
        basebb++;
      }
    } else if ((*basebb)->isIfBlock()) {
      MergeIfBB(dynamic_cast<IfBlock *>((*basebb)));
      basebb++;
    } else if ((*basebb)->isWhileBlock()) {
      MergeWhileBB(dynamic_cast<WhileBlock *>((*basebb)));
      basebb++;
    }
  }

  // for( auto basebb : wait_delete_ )
  // {
  //     whilebb->removeCondBB(basebb);
  // }
  // wait_delete_.clear();

  auto &list2 = whilebb->getBodyBBs();
  for (auto basebb = list2.begin(); basebb != list2.end();) {
    if ((*basebb)->isBasicBlock()) {
      basebb++;
      auto succ_bb = basebb;
      basebb--;
      if (succ_bb != list2.end() &&
          MergeBasicBlock(dynamic_cast<BasicBlock *>(*basebb), *succ_bb)) {
        whilebb->removeWhileBodyBB(*succ_bb);
      } else {
        basebb++;
      }
    } else if ((*basebb)->isIfBlock()) {
      MergeIfBB(dynamic_cast<IfBlock *>((*basebb)));
      basebb++;
    } else if ((*basebb)->isWhileBlock()) {
      MergeWhileBB(dynamic_cast<WhileBlock *>((*basebb)));
      basebb++;
    }
  }

  // for( auto basebb : wait_delete_ )
  // {
  //     whilebb->removeWhileBodyBB(basebb);
  // }
  // wait_delete_.clear();
}

void HighIRsimplyCFG::MergeIfBB(IfBlock *ifbb) {
  auto &list1 = ifbb->getCondBBs();
  for (auto basebb = list1.begin(); basebb != list1.end();) {
    if ((*basebb)->isBasicBlock()) {
      basebb++;
      auto succ_bb = basebb;
      basebb--;
      if (succ_bb != list1.end() &&
          MergeBasicBlock(dynamic_cast<BasicBlock *>(*basebb), *succ_bb)) {
        ifbb->removeCondBB(*succ_bb);
      } else {
        basebb++;
      }
    } else if ((*basebb)->isIfBlock()) {
      MergeIfBB(dynamic_cast<IfBlock *>((*basebb)));
      basebb++;
    } else if ((*basebb)->isWhileBlock()) {
      MergeWhileBB(dynamic_cast<WhileBlock *>((*basebb)));
      basebb++;
    }
  }

  auto &list2 = ifbb->getIfBodyBBs();
  for (auto basebb = list2.begin(); basebb != list2.end();) {
    if ((*basebb)->isBasicBlock()) {
      basebb++;
      auto succ_bb = basebb;
      basebb--;
      if (succ_bb != list2.end() &&
          MergeBasicBlock(dynamic_cast<BasicBlock *>(*basebb), *succ_bb)) {
        ifbb->removeIfBodyBB(*succ_bb);
      } else {
        basebb++;
      }
    } else if ((*basebb)->isIfBlock()) {
      MergeIfBB(dynamic_cast<IfBlock *>((*basebb)));
      basebb++;
    } else if ((*basebb)->isWhileBlock()) {
      MergeWhileBB(dynamic_cast<WhileBlock *>((*basebb)));
      basebb++;
    }
  }

  auto &list3 = ifbb->getElseBodyBBs();
  for (auto basebb = list3.begin(); basebb != list3.end();) {
    if ((*basebb)->isBasicBlock()) {
      basebb++;
      auto succ_bb = basebb;
      basebb--;
      if (succ_bb != list3.end() &&
          MergeBasicBlock(dynamic_cast<BasicBlock *>(*basebb), *succ_bb)) {
        ifbb->removeElseBodyBB(*succ_bb);
      } else {
        basebb++;
      }
    } else if ((*basebb)->isIfBlock()) {
      MergeIfBB(dynamic_cast<IfBlock *>((*basebb)));
      basebb++;
    } else if ((*basebb)->isWhileBlock()) {
      MergeWhileBB(dynamic_cast<WhileBlock *>((*basebb)));
      basebb++;
    }
  }
}

bool HighIRsimplyCFG::MergeBasicBlock(BasicBlock *bb, BaseBlock *succbb) {
  if (succbb == nullptr) {
    return false;
  } else if (succbb->isWhileBlock()) {
    return false;
  } else if (succbb->isIfBlock()) {
    return false;
  } else if (succbb->isBasicBlock()) {

    for (auto instr : dynamic_cast<BasicBlock *>(succbb)->getInstructions()) {
      bb->addInstruction(instr);
    }
    // wait_delete_.push_back(succbb);
    return true;
  } else {
    return false;
  }

  return false;
}

// void HighIRsimplyCFG::removeAllBB()
// {
//     for( auto basebb : wait_delete_ )
//     {
//         auto father = basebb->getBaseFather();
//         if( father)
//         {
//             if( father->isIfBlock() )
//             {
//                 dynamic_cast<IfBlock *>(father)->removeBB(basebb);
//             }
//             else if ( father->isWhileBlock() )
//             {
//                 dynamic_cast<WhileBlock *>(father)->removeBB(basebb);
//             }
//             else
//             {
//                 std::cerr<<"error in HighIRsimplyCFG removeBB"<<std::endl;
//                 abort();
//             }
//         }
//         else
//         {
//             func_->removeBaseBlock(basebb);
//         }
//     }
//     wait_delete_.clear();
// }