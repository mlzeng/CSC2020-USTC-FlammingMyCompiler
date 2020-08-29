#include "SimplifyCFG.h"
#include "ReturnVal.h"

// #define DEBUG

void SimplifyCFG::run() {
  for (auto fun : m_->getFunctions()) {
    func_ = fun;
    // std::cout<< fun->getName() <<std::endl;
    RemoveNoPredecessorBB();
    // std::cout<< fun->getName() <<std::endl;
    EliminateSinglePredecessorPhi();
    MergeSinglePredecessorBB();
    // std::cout<< fun->getName() <<std::endl;
    // std::cout<< fun->getName() <<std::endl;
    EliminateSingleUnCondBrBB();

    RemoveSelfLoopBB();
    RemoveNoPredecessorBB();

    EliminateSinglePredecessorPhi();
  }
}

void SimplifyCFG::RemoveSelfLoopBB() {
  std::vector<BasicBlock *> wait_delete;
  for (auto bb : func_->getBasicBlocks()) {
    if (bb->getPreBasicBlocks().size() == 1) {
      if (*(bb->getPreBasicBlocks().begin()) == bb) {
        wait_delete.push_back(bb);
      }
    }
  }
  for (auto bb : wait_delete) {
    func_->removeBasicBlock(bb);
  }
}

void SimplifyCFG::RemoveNoPredecessorBB() {
  std::vector<BasicBlock *> wait_delete;
  for (auto bb : func_->getBasicBlocks()) {
    if (bb->getPreBasicBlocks().empty() && bb != func_->getEntryBlock()) {
      wait_delete.push_back(bb);
      for (auto succ_bb : bb->getSuccBasicBlocks()) {
        succ_bb->removePreBasicBlock(bb);
        for (auto instr : succ_bb->getInstructions()) {
          if (instr->isPHI()) {
            for (int i = 0; i < instr->getNumOperand(); i++) {
              if (i % 2 == 1) {
                if (instr->getOperand(i) == bb) {
                  instr->removeOperand(i - 1, i);
                }
              }
            }
          }
        }
      }
    }
  }

  for (auto bb : wait_delete) {
    func_->removeBasicBlock(bb);
  }
#ifdef DEBUG
  std::cerr << "RemoveNoPredecessorBB: " << std::endl;
  m_->print();
#endif
}

void SimplifyCFG::MergeSinglePredecessorBB() {
  std::vector<BasicBlock *> wait_delete;
  for (auto bb : func_->getBasicBlocks()) {
    if (bb->getPreBasicBlocks().size() == 1) {
      auto pre_bb = bb->getPreBasicBlocks().begin();
      auto br = (*pre_bb)->getTerminator();
      exit_ifnot(_MergeSinglePredecessorBB_SimplifyCFG, br->isBr());
      if (br->getNumOperand() == 1) {
        (*pre_bb)->deleteInstr(br);
        // merge
        for (auto instr : bb->getInstructions()) {
          instr->setParent(*pre_bb);
          (*pre_bb)->addInstruction(instr);
        }

        (*pre_bb)->clearSuccBasicBlock();
        for (auto succ_bb : bb->getSuccBasicBlocks()) {
          (*pre_bb)->addSuccBasicBlock(succ_bb);
          succ_bb->getPreBasicBlocks().remove(bb);
          succ_bb->addPreBasicBlock(*pre_bb);
        }
        bb->replaceAllUseWith(*pre_bb);
        wait_delete.push_back(bb);
        // bb->replaceAllUseWith(*pre_bb);
      }
    }
  }
  for (auto bb : wait_delete) {
    func_->removeBasicBlock(bb);
  }
}

void SimplifyCFG::EliminateSinglePredecessorPhi() {
#ifdef DEBUG
  std::cerr << "before EliminateSinglePredecessorPhi: " << std::endl;
  m_->print();
#endif
  for (auto bb : func_->getBasicBlocks()) {
    if (bb->getPreBasicBlocks().size() == 1 || bb == func_->getEntryBlock()) {
      std::vector<Instruction *> wait_delete;
      for (auto instr : bb->getInstructions()) {
        if (instr->isPHI()) {
          if (instr->getOperand(1) != *(bb->getPreBasicBlocks().begin()) &&
              bb != func_->getEntryBlock()) {
            std::cerr << "error in EliminateSinglePredecessorPhi" << std::endl;
            std::cerr << instr->getOperand(1)->getName() << "  #"
                      << (*(bb->getPreBasicBlocks().begin()))->getName()
                      << std::endl;
            exit(_EliminateSinglePredecessorPhi_SimplifyCFG);
          }
#ifdef DEBUG
          std::cout << "phi replace: " << instr->getOperand(0)->getName()
                    << std::endl;
          for (auto use : instr->getUseList()) {
            std::cout << "usename: " << (use.val_)->getName() << std::endl;
          }
#endif
          // instr->replaceAllUseWith(instr->getOperand(0));
          for (auto use : instr->getUseList()) {
            static_cast<User *>(use.val_)->setOperand(use.arg_no_,
                                                      instr->getOperand(0));
            // bb->removeUse(use.val_);
          }

          wait_delete.push_back(instr);
        }
      }
      for (auto instr : wait_delete) {
        bb->deleteInstr(instr);
      }
    }
  }
}

void SimplifyCFG::EliminateSingleUnCondBrBB() {
  std::vector<BasicBlock *> wait_delete;
  for (auto bb : func_->getBasicBlocks()) {
    if (bb->getNumOfInstr() == 1 && bb != func_->getEntryBlock()) {
      auto terminator = bb->getTerminator();
      auto succ_bb = bb->getSuccBasicBlocks().begin();
      exit_ifnot(_EliminateSingleUnCondBrBB_SimplifyCFG, terminator);
      if (terminator->isBr() && terminator->getNumOperand() == 1) {
#ifdef DEBUG
        std::cout << "succ: " << (*succ_bb)->getName() << std::endl;
        std::cout << "bb: " << bb->getName() << std::endl;
#endif
        bool bb_delete = true;
        for (auto pre_bb : bb->getPreBasicBlocks()) {
          auto br = static_cast<BranchInst *>(pre_bb->getTerminator());
          if (br->isCondBr()) {
            auto target1 = br->getOperand(1);
            auto target2 = br->getOperand(2);
            auto target3 = static_cast<BranchInst *>(terminator)->getOperand(0);
            if ((target1 == bb && target2 == target3) ||
                (target2 == bb && target1 == target3)) {
              bb_delete = false;
              break;
            }
          } else if (br->isCmpBr()) {
            auto target1 = br->getOperand(2);
            auto target2 = br->getOperand(3);
            auto target3 = static_cast<BranchInst *>(terminator)->getOperand(0);
            if ((target1 == bb && target2 == target3) ||
                (target2 == bb && target1 == target3)) {
              bb_delete = false;
              break;
            }
          }
        }

        if (bb_delete) {
          // ssa格式
          for (auto instr : (*succ_bb)->getInstructions()) {
            if (instr->isPHI()) {
              for (int i = 0; i < instr->getNumOperand(); i++) {
                if (i % 2 == 1) {
                  if (instr->getOperand(i) == bb) {
#ifdef DEBUG
                    std::cout << "label bb: " << bb->getName() << std::endl;
#endif
                    auto idx = 0;
                    auto val = instr->getOperand(i - 1);
                    for (auto pre_bb : bb->getPreBasicBlocks()) {
                      if (idx == 0) {
                        instr->setOperand(i, pre_bb);
                        bb->removeUse(instr, i);
                      } else {
                        instr->addOperand(val);
                        instr->addOperand(pre_bb);
                      }
                      idx++;
                    }
                  }
                }
              }
            }
          }
          // ssa格式

          for (auto use : bb->getUseList()) {
            auto instr = dynamic_cast<Instruction *>(use.val_);
            if (instr) {
              if (instr->isBr()) {
                static_cast<User *>(use.val_)->setOperand(use.arg_no_,
                                                          *succ_bb);
              }
            }
          }

          (*succ_bb)->removePreBasicBlock(bb);
          for (auto pre_bb : bb->getPreBasicBlocks()) {
            pre_bb->removeSuccBasicBlock(bb);
            pre_bb->addSuccBasicBlock(*succ_bb);
            (*succ_bb)->addPreBasicBlock(pre_bb);
          }
          wait_delete.push_back(bb);
        }
      }
    }
  }
  for (auto bb : wait_delete) {
    func_->removeBasicBlock(bb);
  }
}