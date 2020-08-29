//
// Created by cqy on 2020/7/5.
//
#include "ActiveVars.h"
#include "BBConstPropagation.h"
#include "CodeGen.hh"
#include "DeadCodeEliminate.h"
#include "Dominators.h"
#include "FunctionInline.h"
#include "GlobalVariableLocal.h"
#include "LoopFind.h"
#include "PassManager.h"
#include "RegisterAllocation.h"
#include "SYSYCBuilder.h"
#include "SimplifyCFG.h"
#include "mem2reg.h"
#include "syntax_tree.hh"
#include "sysy_driver.hh"

#include "BBCommonSubExper.h"
#include "ConstLoopExpansion.h"
#include "RefactorPartIns.h"

#include "InstructionSchedule.h"
#include "LoopInvariant.h"
#include "LowerIR.h"
#include "PowerArray.h"

#include "AccumulatePattern.h"
#include "Analyzer.hh"
#include "BBExps.h"
#include "CondSimplify.h"
#include "FunctionOptimization.h"
#include "HIRToMIR.h"
#include "HighIRsimplyCFG.h"
#include "LoopMerge.h"
#include "MergeCond.h"
#include "Multithreading.h"
#include "Vectorization.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_set>

using namespace std::literals::string_literals;

extern SyntaxCompUnit *MonkeyType(const std::string filename);

void print_help(std::string exe_name) {
  std::cout << "Usage: " << exe_name
            << " [ -h | --help ] [ -o <target-file> ] [ -emit ] [ -test ] "
               "<input-file>"
            << std::endl;
}

bool disable_div_optimization = false;

std::string input_path;

int main(int argc, char **argv) {
  std::string target_path;
  bool emit = false;
  bool test = false;
  bool opti = false;
  bool debug = true;

  for (int i = 1; i < argc; ++i) {
    if (argv[i] == "-h"s || argv[i] == "--help"s) {
      print_help(argv[0]);
      return 0;
    } else if (argv[i] == "-o"s) {
      if (target_path.empty() && i + 1 < argc) {
        target_path = argv[i + 1];
        i += 1;
      } else {
        print_help(argv[0]);
        return 0;
      }
    } else if (argv[i] == "-emit"s) {
      emit = true;
    } else if (argv[i] == "-test"s) {
      test = true;
    } else if (argv[i] == "-debug"s) {
      debug = true;
    } else if (argv[i] == "-O2"s || argv[i] == "-O1"s) {
      opti = true;
    } else if (argv[i] == "-S"s) {
    } else {
      if (input_path.empty()) {
        input_path = argv[i];
      } else {
        print_help(argv[0]);
        return 0;
      }
    }
  }
  if (input_path.empty()) {
    print_help(argv[0]);
    return 0;
  }

  if (target_path.empty()) {
    auto pos = input_path.rfind('.');
    if (pos == std::string::npos) {
      std::cerr << argv[0] << ": input file " << input_path
                << " has unknown filetype!" << std::endl;
      return -1;
    } else {
      if (input_path.substr(pos) != ".sy") {
        std::cerr << argv[0] << ": input file " << input_path
                  << " has unknown filetype!" << std::endl;
        return -1;
      }
      if (emit) {
        target_path = input_path.substr(0, pos) + ".ll";
      } else {
        target_path = input_path.substr(0, pos) + ".s";
      }
    }
  }

  SYSYCBuilder builder;
  SyntaxCompUnit *root;
  root = MonkeyType(input_path);
  auto tree = syntax_tree(root);
  tree.run_visitor(builder);

  if (true) {
    PassManager PM(builder.getModule().get());
    // HIR optimization
    PM.addPass<HighIRsimplyCFG>();
    PM.addPass<LoopMerge>();
    PM.addPass<AccumulatePattern>();
    PM.addPass<HighIRsimplyCFG>();
    // HIR to MIR
    PM.addPass<HIRToMIR>();
    PM.addPass<SimplifyCFG>();
    // FunctionOptimization
    PM.addPass<FunctionOptimization>();
    PM.addPass<DeadCodeEliminate>();
    // Inline
    PM.addPass<FunctionInline>();
    PM.addPass<GlobalVariableLocal>();
    PM.addPass<DeadCodeEliminate>();
    PM.addPass<SimplifyCFG>();
    // SSA
    PM.addPass<Dominators>();
    PM.addPass<Mem2RegPass>();
    PM.addPass<DeadCodeEliminate>();
    // Const Propagation
    PM.addPass<BBConstPropagation>();
    PM.addPass<DeadCodeEliminate>();
    PM.addPass<SimplifyCFG>();
    // Common Sub Exper
    PM.addPass<BBCommonSubExper>();
    // SSA Inline
    PM.addPass<FunctionInline>();
    PM.addPass<DeadCodeEliminate>();
    PM.addPass<SimplifyCFG>();
    // Const Loop Expansion
    PM.addPass<RefactorPartIns>();
    // PM.addPass<ConstLoopExpansion>();
    PM.addPass<SimplifyCFG>();
    PM.addPass<BBConstPropagation>();
    PM.addPass<DeadCodeEliminate>();
    PM.addPass<SimplifyCFG>();
    // PowerArray: remove bitset.sy x array
    // PM.addPass<PowerArray>();
    PM.addPass<DeadCodeEliminate>();
    PM.addPass<SimplifyCFG>();
    // LoopInvariant
    PM.addPass<LoopInvariant>();
    PM.addPass<SimplifyCFG>();
    // CommonSubExper
    PM.addPass<BBCommonSubExper>();
    PM.addPass<DeadCodeEliminate>();
    PM.addPass<SimplifyCFG>();
    // Multithreading
    PM.addPass<RefactorPartIns>();
    PM.addPass<Multithreading>();
    PM.addPass<DeadCodeEliminate>();
    PM.addPass<SimplifyCFG>();
    // SIMD
    // PM.addPass<Vectorization>();
    PM.addPass<DeadCodeEliminate>();
    // MIR to LIR and LIR optimization
    PM.addPass<LowerIR>();
    PM.addPass<DeadCodeEliminate>();
    PM.addPass<BBCommonSubExper>();
    PM.addPass<DeadCodeEliminate>();
    PM.run();
    // CodeGen
    CodeGen code_generator(builder.getModule(), debug);
    std::cerr << std::endl
              << "### IR (without RegAlloc) ###" << std::endl
              << std::endl;
    builder.getModule().get()->print();
    std::string asm_code = code_generator.generateModuleCode(true);
    std::cerr << std::endl
              << "### IR (with RegAlloc) ###" << std::endl
              << std::endl;
    code_generator.printIR();
    code_generator.printStat();
    std::ofstream out(target_path);
    out << asm_code;
    out.close();
    return 0;
  }
}