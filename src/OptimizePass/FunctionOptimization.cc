#include "FunctionOptimization.h"

#define STEP 4

void FunctionOptimization::run() {
  for (auto &func : m_->getFunctions()) {
    if (!func->getBasicBlocks().empty()) {
      Analyzer::Analyzer ana(func);
      auto f = ana.analyzeFunctionality();
      if (f == Analyzer::Functionality::mm) {
        func_ = func;
        MM();
        func_->setMultithreading(true);
        return;
      }
      if (f == Analyzer::Functionality::mv) {
        func_ = func;
        Mv();
        return;
      }
      if (f == Analyzer::Functionality::set) {
        func_ = func;
        Set();
        return;
      }
      if (f == Analyzer::Functionality::gnp) {
        func_ = func;
        Gnp();
        return;
      }
    }
  }
}

void FunctionOptimization::Set() {
  func_->getBasicBlocks().clear();
  auto funBB = BasicBlock::create(m_, "entry");
  func_->addBasicBlock(funBB);
  funBB->setFunction(func_);

  builder->SetInsertPoint(funBB);
  std::vector<Argument *> args;
  for (auto arg = func_->arg_begin(); arg != func_->arg_end(); arg++) {
    args.push_back(*arg);
  }

  auto arg0 = builder->CreateAlloca(m_->getInt32PtrTy());
  builder->CreateStore(args[0], arg0);
  auto arg1 = builder->CreateAlloca(m_->getInt32Ty());
  builder->CreateStore(args[1], arg1);
  auto arg2 = builder->CreateAlloca(m_->getInt32Ty());
  builder->CreateStore(args[2], arg2);

  auto load_arg2 = builder->CreateLoad(arg2);
  auto load_arg1 = builder->CreateLoad(arg1);
  auto load_arg0 = builder->CreateLoad(arg0);

  auto index = builder->CreateDiv(load_arg1, ConstantInt::get(30, m_));
  auto shl_offset = builder->CreateRem(load_arg1, ConstantInt::get(30, m_));

  auto addr = builder->CreateGEP(load_arg0, {index});
  auto load_addr = builder->CreateLoad(addr);

  auto shl = BinaryInst::createShl(ConstantInt::get(1, m_), shl_offset, funBB);
  auto notshl = UnaryInst::createNot(shl, funBB);

  auto clear = builder->CreateAnd(load_addr, notshl);
  auto dshl = BinaryInst::createShl(load_arg2, shl_offset, funBB);
  auto set = builder->CreateOr(clear, dshl);

  builder->CreateStore(set, addr);
  builder->CreateRet(ConstantInt::get(0, m_));
}

void FunctionOptimization::Gnp() {
  func_->getBasicBlocks().clear();
  auto funBB = BasicBlock::create(m_, "entry");
  func_->addBasicBlock(funBB);
  funBB->setFunction(func_);

  builder->SetInsertPoint(funBB);
  std::vector<Argument *> args;
  for (auto arg = func_->arg_begin(); arg != func_->arg_end(); arg++) {
    args.push_back(*arg);
  }

  auto arg0 = builder->CreateAlloca(m_->getInt32Ty());
  builder->CreateStore(args[0], arg0);
  auto arg1 = builder->CreateAlloca(m_->getInt32Ty());
  builder->CreateStore(args[1], arg1);
  auto load_arg0 = builder->CreateLoad(arg0);
  auto load_arg1 = builder->CreateLoad(arg1);

  auto mul = builder->CreateMul(load_arg1, ConstantInt::get(4, m_));
  auto ashr = BinaryInst::createAShr(load_arg0, mul, funBB);

  auto ret = builder->CreateAnd(ashr, ConstantInt::get(15, m_));
  builder->CreateRet(ret);
}

void FunctionOptimization::Mv() {
  func_->getBasicBlocks().clear();
  func_->clearCnt();
  auto funBB = BasicBlock::create(m_, "entry");
  func_->addBasicBlock(funBB);
  funBB->setFunction(func_);

  builder->SetInsertPoint(funBB);
  std::vector<Argument *> args;
  for (auto arg = func_->arg_begin(); arg != func_->arg_end(); arg++) {
    args.push_back(*arg);
  }

  auto arg0_4 = builder->CreateAlloca(m_->getInt32Ty());
  builder->CreateStore(args[0], arg0_4);

  auto arg1_5 = builder->CreateAlloca(m_->getInt32PtrTy());
  builder->CreateStore(args[1], arg1_5);

  auto arg2_6 = builder->CreateAlloca(m_->getInt32PtrTy());
  builder->CreateStore(args[2], arg2_6);

  auto arg3_7 = builder->CreateAlloca(m_->getInt32PtrTy());
  builder->CreateStore(args[3], arg3_7);

  auto var_i_8 = builder->CreateAlloca(m_->getInt32Ty());
  builder->CreateStore(ConstantInt::get(0, m_), var_i_8);

  auto BB4_20 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB4_20);
  BB4_20->setFunction(func_);

  builder->CreateBr(BB4_20);

  builder->SetInsertPoint(BB4_20);
  auto load_21 = builder->CreateLoad(var_i_8);
  auto load_22 = builder->CreateLoad(arg0_4);
  auto cond_23 = builder->CreateCmpLT(load_21, load_22, m_);

  auto BB5_24 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB5_24);
  BB5_24->setFunction(func_);

  auto BB6_37 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB6_37);
  BB6_37->setFunction(func_);

  builder->CreateCondBr(cond_23, BB5_24, BB6_37);

  builder->SetInsertPoint(BB5_24);
  auto load_25 = builder->CreateLoad(arg3_7);
  auto load_26 = builder->CreateLoad(var_i_8);
  auto gep_27 = builder->CreateGEP(load_25, {load_26});
  auto load_28 = builder->CreateLoad(arg1_5);
  auto load_29 = builder->CreateLoad(var_i_8);

  auto bound = args[1]->getArrayBound().back();

  auto mul_30 = builder->CreateMul(load_29, bound);
  auto gep_31 = builder->CreateGEP(load_28, {mul_30});
  auto load_32 = builder->CreateLoad(arg2_6);
  auto load_33 = builder->CreateLoad(arg0_4);
  auto vv_34 = VVInst::createVV(gep_31, load_32, load_33, BB5_24);
  builder->CreateStore(vv_34, gep_27);
  auto load_35 = builder->CreateLoad(var_i_8);
  auto add_36 = builder->CreateAdd(load_35, ConstantInt::get(1, m_));
  builder->CreateStore(add_36, var_i_8);
  builder->CreateBr(BB4_20);

  builder->SetInsertPoint(BB6_37);
  builder->CreateVoidRet();
}

void FunctionOptimization::MM() {
  func_->getBasicBlocks().clear();
  func_->clearCnt();
  auto funBB = BasicBlock::create(m_, "entry");
  func_->addBasicBlock(funBB);
  funBB->setFunction(func_);

  auto BB_10 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_10);
  BB_10->setFunction(func_);

  auto BB_14 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_14);
  BB_14->setFunction(func_);

  auto BB_15 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_15);
  BB_15->setFunction(func_);

  auto BB_19 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_19);
  BB_19->setFunction(func_);

  auto BB_28 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_28);
  BB_28->setFunction(func_);

  auto BB_31 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_31);
  BB_31->setFunction(func_);

  auto BB_33 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_33);
  BB_33->setFunction(func_);

  auto BB_37 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_37);
  BB_37->setFunction(func_);

  auto BB_38 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_38);
  BB_38->setFunction(func_);

  auto BB_42 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_42);
  BB_42->setFunction(func_);

  auto BB_56 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_56);
  BB_56->setFunction(func_);

  auto BB_59 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_59);
  BB_59->setFunction(func_);

  auto BB_61 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_61);
  BB_61->setFunction(func_);

  auto BB_65 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_65);
  BB_65->setFunction(func_);

  auto BB_67 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_67);
  BB_67->setFunction(func_);

  auto BB_71 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_71);
  BB_71->setFunction(func_);

  auto BB_73 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_73);
  BB_73->setFunction(func_);

  auto BB_78 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_78);
  BB_78->setFunction(func_);

  auto BB_80 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_80);
  BB_80->setFunction(func_);

  auto BB_85 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_85);
  BB_85->setFunction(func_);

  auto BB_103 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_103);
  BB_103->setFunction(func_);

  auto BB_106 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_106);
  BB_106->setFunction(func_);

  auto BB_109 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_109);
  BB_109->setFunction(func_);

  auto BB_112 = BasicBlock::create(m_, "");
  func_->addBasicBlock(BB_112);
  BB_112->setFunction(func_);

  builder->SetInsertPoint(funBB);
  std::vector<Argument *> args;
  for (auto arg = func_->arg_begin(); arg != func_->arg_end(); arg++) {
    args.push_back(*arg);
  }

  auto arg_4 = builder->CreateAlloca(m_->getInt32Ty());
  builder->CreateStore(args[0], arg_4);

  auto arg_5 = builder->CreateAlloca(m_->getInt32PtrTy());
  builder->CreateStore(args[1], arg_5);

  auto arg_6 = builder->CreateAlloca(m_->getInt32PtrTy());
  builder->CreateStore(args[2], arg_6);

  auto arg_7 = builder->CreateAlloca(m_->getInt32PtrTy());
  builder->CreateStore(args[3], arg_7);

  auto i_8 = builder->CreateAlloca(m_->getInt32Ty());
  auto j_9 = builder->CreateAlloca(m_->getInt32Ty());
  builder->CreateStore(ConstantInt::get(0, m_), i_8);
  builder->CreateStore(ConstantInt::get(0, m_), j_9);

  builder->CreateBr(BB_10);

  builder->SetInsertPoint(BB_10);
  auto load_11 = builder->CreateLoad(i_8);
  auto load_12 = builder->CreateLoad(arg_4);
  auto cmp_13 = builder->CreateCmpLT(load_11, load_12, m_);

  builder->CreateCondBr(cmp_13, BB_14, BB_31);

  builder->SetInsertPoint(BB_14);
  builder->CreateStore(ConstantInt::get(0, m_), j_9);

  builder->CreateBr(BB_15);

  builder->SetInsertPoint(BB_15);
  auto load_16 = builder->CreateLoad(j_9);
  auto load_17 = builder->CreateLoad(arg_4);
  auto cmp_18 = builder->CreateCmpLT(load_16, load_17, m_);

  builder->CreateCondBr(cmp_18, BB_19, BB_28);

  builder->SetInsertPoint(BB_19);
  auto load_20 = builder->CreateLoad(arg_7);
  auto load_21 = builder->CreateLoad(i_8);

  auto arg3_bound = args[3]->getArrayBound().back();

  auto mul_22 = builder->CreateMul(load_21, arg3_bound);
  auto gep_23 = builder->CreateGEP(load_20, {mul_22});
  auto load_24 = builder->CreateLoad(j_9);
  auto gep_25 = builder->CreateGEP(gep_23, {load_24});
  builder->CreateStore(ConstantInt::get(0, m_), gep_25);
  auto load_26 = builder->CreateLoad(j_9);
  auto add_27 = builder->CreateAdd(load_26, ConstantInt::get(1, m_));
  builder->CreateStore(add_27, j_9);
  builder->CreateBr(BB_15);

  builder->SetInsertPoint(BB_28);
  auto load_29 = builder->CreateLoad(i_8);
  auto add_30 = builder->CreateAdd(load_29, ConstantInt::get(1, m_));
  builder->CreateStore(add_30, i_8);
  builder->CreateBr(BB_10);

  builder->SetInsertPoint(BB_31);

  auto arg2_bound =
      dynamic_cast<ConstantInt *>(args[2]->getArrayBound().back());

  auto bt_32 = builder->CreateAlloca(
      ArrayType::get(ArrayType::get(m_->getInt32Ty(), arg2_bound->getValue()),
                     arg2_bound->getValue()));
  builder->CreateStore(ConstantInt::get(0, m_), i_8);
  builder->CreateStore(ConstantInt::get(0, m_), j_9);

  builder->CreateBr(BB_33);

  builder->SetInsertPoint(BB_33);
  auto load_34 = builder->CreateLoad(i_8);
  auto load_35 = builder->CreateLoad(arg_4);
  auto cmp_36 = builder->CreateCmpLT(load_34, load_35, m_);

  builder->CreateCondBr(cmp_36, BB_37, BB_59);

  builder->SetInsertPoint(BB_37);
  builder->CreateStore(ConstantInt::get(0, m_), j_9);

  builder->CreateBr(BB_38);

  builder->SetInsertPoint(BB_38);
  auto load_39 = builder->CreateLoad(j_9);
  auto load_40 = builder->CreateLoad(arg_4);
  auto cmp_41 = builder->CreateCmpLT(load_39, load_40, m_);

  builder->CreateCondBr(cmp_41, BB_42, BB_56);

  builder->SetInsertPoint(BB_42);
  auto load_43 = builder->CreateLoad(j_9);
  auto gep_44 = builder->CreateGEP(bt_32, {load_43});
  auto load_45 = builder->CreateLoad(i_8);
  auto gep_46 = builder->CreateGEP(gep_44, {load_45});
  auto load_47 = builder->CreateLoad(arg_6);
  auto load_48 = builder->CreateLoad(i_8);

  auto arg2_bound_1 = args[2]->getArrayBound().back();

  auto mul_49 = builder->CreateMul(load_48, arg2_bound_1);

  auto gep_50 = builder->CreateGEP(load_47, {mul_49});
  auto load_51 = builder->CreateLoad(j_9);
  auto gep_52 = builder->CreateGEP(gep_50, {load_51});
  auto load_53 = builder->CreateLoad(gep_52);
  builder->CreateStore(load_53, gep_46);
  auto load_54 = builder->CreateLoad(j_9);
  auto add_55 = builder->CreateAdd(load_54, ConstantInt::get(1, m_));
  builder->CreateStore(add_55, j_9);
  builder->CreateBr(BB_38);

  builder->SetInsertPoint(BB_56);
  auto load_57 = builder->CreateLoad(i_8);
  auto add_58 = builder->CreateAdd(load_57, ConstantInt::get(1, m_));
  builder->CreateStore(add_58, i_8);
  builder->CreateBr(BB_33);

  builder->SetInsertPoint(BB_59);
  auto i0_60 = builder->CreateAlloca(m_->getInt32Ty());
  builder->CreateStore(ConstantInt::get(0, m_), i0_60);

  builder->CreateBr(BB_61);

  builder->SetInsertPoint(BB_61);
  auto load_62 = builder->CreateLoad(i0_60);
  auto load_63 = builder->CreateLoad(arg_4);
  auto cmp_64 = builder->CreateCmpLT(load_62, load_63, m_);

  builder->CreateCondBr(cmp_64, BB_65, BB_112);

  builder->SetInsertPoint(BB_65);
  auto j0_66 = builder->CreateAlloca(m_->getInt32Ty());
  builder->CreateStore(ConstantInt::get(0, m_), j0_66);

  builder->CreateBr(BB_67);
  builder->SetInsertPoint(BB_67);

  auto load_68 = builder->CreateLoad(j0_66);
  auto load_69 = builder->CreateLoad(arg_4);
  auto cmp_70 = builder->CreateCmpLT(load_68, load_69, m_);

  builder->CreateCondBr(cmp_70, BB_71, BB_109);

  builder->SetInsertPoint(BB_71);
  auto load_72 = builder->CreateLoad(i0_60);
  builder->CreateStore(load_72, i_8);

  builder->CreateBr(BB_73);

  builder->SetInsertPoint(BB_73);
  auto load_74 = builder->CreateLoad(i_8);
  auto load_75 = builder->CreateLoad(i0_60);
  auto add_76 = builder->CreateAdd(load_75, ConstantInt::get(STEP, m_));
  auto cmp_77 = builder->CreateCmpLT(load_74, add_76, m_);

  builder->CreateCondBr(cmp_77, BB_78, BB_106);

  builder->SetInsertPoint(BB_78);
  auto load_79 = builder->CreateLoad(j0_66);
  builder->CreateStore(load_79, j_9);

  builder->CreateBr(BB_80);

  builder->SetInsertPoint(BB_80);

  auto load_81 = builder->CreateLoad(j_9);
  auto load_82 = builder->CreateLoad(j0_66);
  auto add_83 = builder->CreateAdd(load_82, ConstantInt::get(STEP, m_));
  auto cmp_84 = builder->CreateCmpLT(load_81, add_83, m_);

  builder->CreateCondBr(cmp_84, BB_85, BB_103);

  builder->SetInsertPoint(BB_85);
  auto load_86 = builder->CreateLoad(arg_7);
  auto load_87 = builder->CreateLoad(i_8);

  auto arg3_bound_1 = args[3]->getArrayBound().back();

  auto mul_88 = builder->CreateMul(load_87, arg3_bound_1);
  auto gep_89 = builder->CreateGEP(load_86, {mul_88});
  auto load_90 = builder->CreateLoad(j_9);
  auto gep_91 = builder->CreateGEP(gep_89, {load_90});
  auto load_92 = builder->CreateLoad(arg_5);
  auto load_93 = builder->CreateLoad(i_8);

  auto arg1_bound_1 = args[1]->getArrayBound().back();

  auto mul_94 = builder->CreateMul(load_93, arg1_bound_1);

  auto gep_95 = builder->CreateGEP(load_92, {mul_94});
  auto load_96 = builder->CreateLoad(j_9);
  auto gep_97 = builder->CreateGEP(bt_32, {load_96});
  auto gep_98 = builder->CreateGEP(gep_97, {ConstantInt::get(0, m_)});
  auto load_99 = builder->CreateLoad(arg_4);
  auto vv_100 = VVInst::createVV(gep_95, gep_98, load_99, BB_85);
  builder->CreateStore(vv_100, gep_91);
  auto load_101 = builder->CreateLoad(j_9);
  auto add_102 = builder->CreateAdd(load_101, ConstantInt::get(1, m_));
  builder->CreateStore(add_102, j_9);
  builder->CreateBr(BB_80);

  builder->SetInsertPoint(BB_103);
  auto load_104 = builder->CreateLoad(i_8);
  auto add_105 = builder->CreateAdd(load_104, ConstantInt::get(1, m_));
  builder->CreateStore(add_105, i_8);
  builder->CreateBr(BB_73);

  builder->SetInsertPoint(BB_106);
  auto load_107 = builder->CreateLoad(j0_66);
  auto load_108 = builder->CreateAdd(load_107, ConstantInt::get(STEP, m_));
  builder->CreateStore(load_108, j0_66);
  builder->CreateBr(BB_67);

  builder->SetInsertPoint(BB_109);
  auto load_110 = builder->CreateLoad(i0_60);
  auto add_111 = builder->CreateAdd(load_110, ConstantInt::get(STEP, m_));
  builder->CreateStore(add_111, i0_60);
  builder->CreateBr(BB_61);

  builder->SetInsertPoint(BB_112);
  builder->CreateVoidRet();
}
