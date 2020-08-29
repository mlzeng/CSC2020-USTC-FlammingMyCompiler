# CSC2020-USTC-FlammingMyCompiler

全国大学生计算机系统能力大赛编译系统设计赛项目

## 编译器的中间代码优化部分的设计

中间代码部分包含三个层次的 IR（每个层次都列出了相关的 Pass）：

* 高层 IR：设计上保留了源代码 if while 等结构的信息，方便结构级变换。
  * AccumulatePattern：累加变量外提
  * BBExps: 分析得到表达式结构
  * HighIRsimplyCFG: 控制流图化简
  * LoopMerge：while 循环合并
  * MergeCond：嵌套 if 条件块合并
  
* 中层 IR：设计上接近 LLVM IR，适合于各类通用优化。
  * ActiveVars: 活跃变量分析
  * BBCommonSubExper: 块内公共子表达式消除
  * BBConstPropagation: 块内常量传播
  * BranchMerge：分支合并
  * CondSimplify：条件块简化
  * ConstFlod：常量折叠
  * ConstLoopExpansion：循环展开
  * DeadCodeEliminate：死代码删除
  * Dominators：支配树分析
  * FunctionInline：函数智能内联
  * GlobalVariableLocal：全局变量局部化
  * HIRToMIR：将高层 IR 翻译到中层 IR
  * IRCheck：检测 IR 的数据结构是否有不一致
  * LoopFind：循环查找
  * LoopInvariant：循环不变量外提
  * Multithreading: 循环多线程化
  * PowerArray：将特殊形式数组的访问替换为计算
  * ReachDefinition：到达定值分析
  * RefactorParlins：交换操作数位置
  * SimplifyCFG：基本块合并与删除
  * SparseCondConstPropagation：稀疏有条件常量传播
  * Mem2Reg：半剪枝算法构造 SSA 形式 IR
  * Vectorizaiton：循环向量化

* 低层 IR：设计上贴近硬件架构，与后端相配合共同完成指令融合、调度和选择等优化。
  * InstructionSchedule: 指令调度软流水
  * LowerIR：将中层 IR 翻译到低层 IR 并在上面做一系列相关优化
    * SplitGEP：将 GEP 指令拆分为子指令
    * FuseToMLA：将乘法和加法融合到乘加指令
    * MulToShift：将乘法替换成左移
    * ... （还有很多很多）
  * RegisterAllocation: 初版寄存器分配

## 编译器的后端部分的设计

后端与中间代码优化的低层 IR Pass 相配合共同完成指令融合、调度和选择等优化。后端部分包含三个层次的虚拟指令，上层指令会选择最小代价智能翻译成一系列下层指令（不一定要是相邻层）（在多线程区域会有不同的翻译行为）。

指令层次与相关约束：

* 高层虚拟指令：不可使用临时寄存器。
* 中层虚拟指令：可用低频临时寄存器 r12 r14。
* 低层虚拟指令：可用高频临时寄存器 r11。
* 物理指令：直接对应到 ARM 指令

寄存器分配：

* 后端尽量节省了临时寄存器的使用（例如没有使用帧指针 fp），所以留出了 11 个通用寄存器（r0-r10），有效减少了寄存器分配时的溢出。
* 新版寄存器分配器直接集成在后端（初版为 IR Pass），与多线程框架有一些配合（多线程框架共享栈空间所以在并行区域被赋值的虚拟变量分配溢出时需要特殊处理）。
* 分配器使用图着色算法，会衡量各种方面的代价来优化分配。

多线程框架（与经典框架 openmp 对比）：

* 共享栈空间（不需要将被并行化的区域拆分出来变成函数）
* 框架更加易于实现（不需要保存上下文和维护各种信息）
* 更便于代码变换（前后就地插个 `tid=mtstart()` 和 `mtend(tid)` 就行了）
* 更高的运行性能（栈上的资源可以直接访问而不需要通过函数参数传递）
