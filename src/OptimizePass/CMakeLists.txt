add_library(
        Pass STATIC
        mem2reg.cc
        Dominators.cc
        FunctionInline.cc
        SimplifyCFG.cc
#        ControlTree.cc
        LoopFind.cc
        IRCheck.cc
        BBConstPropagation.cc
        ConstFlod.cc
        DeadCodeEliminate.cc
        GlobalVariableLocal.cc
        ConstLoopExpansion.cc
        RefactorPartIns.cc
        LowerIR.cc
        LoopInvariant.cc
        InstructionSchedule.cc
        BBCommonSubExper.cc
        PowerArray.cc
        MergeCond.cc
        Multithreading.cc
        Vectorization.cc
        HIRToMIR.cc
        HighIRsimplyCFG.cc
        ../HIR/BBExps.cc
        ../HIR/LoopMerge.cc
        ../HIR/AccumulatePattern.cc
        CondSimplify.cc
        FunctionOptimization.cc
)

add_library(
        ActiveVars STATIC
        ActiveVars.cc
)

add_library(
        RegisterAllocation STATIC
        RegisterAllocation.cc
)

#add_library(
#        ControlTree STATIC
#        ControlTree.cc
#)
