# CSC2020-USTC-FlammingMyCompiler

2020 年全国大学生计算机系统能力大赛编译系统设计赛项目

队伍学校：中国科学技术大学

队伍名称：燃烧我的编译器

队伍成员：陈清源、黄奕桐、曾明亮、章耀辉

## 项目简介

这是一个 SysY 语言（简化的 C 语言）的编译器，目标平台是树莓派（ARMv8）（32bit）。

这个编译器有很强的优化能力，在比赛提供的大多数性能测试用例上超过了 GCC -O3 的优化效果。

这个项目在决赛取得了最好的成绩。[链接](https://compiler.educg.net/)

## 如何构建与运行本项目

### 构建

进入项目根目录然后执行：

```bash
mkdir build && cd build && cmake .. && make -j 4
```

### 运行

使用 build 目录下的可执行文件 compiler 即可将源代码文件（test.sy）编译得到汇编文件（test.s）：

```bash
echo "int main() { int a = getint(); putint(a * a); }" > test.sy
```
```bash
./compiler test.sy
```
```bash
cat test.s
```

### 测试

用 GCC 将汇编文件（test.s）汇编并与 SysY 运行时库（libsysy.a）链接得到可执行文件然后运行即可：

#### 在树莓派上测试

```bash
gcc test.s libsysy.a -o test
./test
```

输入 111 即可看到输出 12321。

#### 在其它平台上交叉编译测试

```bash
arm-linux-gnueabihf-gcc test.s libsysy.a -o test
qemu-arm -L /usr/arm-linux-gnueabihf/ ./test
```

输入 111 即可看到输出 12321。

### SysY 语言定义与运行时库

* [SysY 语言定义](SysY_def.pdf)
* [SysY 运行时库定义](SysY_lib.pdf)
* [SysY 运行时库](libsysy.a?raw=true)

### SysY 语言测试用例

可以到大赛官方网站上下载：[SysY 语言测试用例](https://gitlab.eduxiji.net/windcome/sysyruntimelibrary)

## 编译器的中间代码优化部分的设计

中间代码部分包含三个层次的 IR（每个层次都列出了相关的 Pass）

### 高层 IR

设计上保留了源代码 if while 等结构的信息，方便结构级变换。

* AccumulatePattern：累加变量外提
* BBExps: 分析得到表达式结构
* HighIRsimplyCFG: 控制流图化简
* LoopMerge：while 循环合并
* MergeCond：嵌套 if 条件块合并
  
### 中层 IR

设计上接近 LLVM IR，适合于各类通用优化。

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

### 低层 IR

设计上贴近硬件架构，与后端相配合共同完成指令融合、调度和选择等优化。

* InstructionSchedule: 指令调度软流水
* LowerIR：将中层 IR 翻译到低层 IR 并在上面做一系列相关优化
  * SplitGEP：将 GEP 指令拆分为子指令（Mul + Add）
  * SplitRem：将 Rem 指令拆分成子指令（Div + Mul + Sub）
  * FuseCmpBr：将比较指令和分支指令融合（这样比较结果就不占用寄存器）
  * FuseMulAdd：将乘法和加法融合为乘加指令
  * ConvertMulToShift：将部分常数乘法替换成左移
  * ConvertRemToAnd：将部分常数取模替换成逻辑与
  * RemoveUnusedOp：消除不被用到的操作数
  * ...... （还有很多）
* RegisterAllocation: 初版寄存器分配

## 编译器的后端部分的设计

后端与中间代码优化的低层 IR Pass 相配合共同完成指令融合、调度和选择等优化。后端部分包含三个层次的虚拟指令，上层指令会选择最小代价智能翻译成一系列下层指令（不一定要是相邻层）（在多线程区域会有不同的翻译行为）。智能翻译的过程会充分利用 ARM 指令的特性，例如 flexible operand 之类的（需要优化过的低层 IR 来提供某些机会）。

### 指令层次与相关约束

* 高层虚拟指令：不可使用临时寄存器。
* 中层虚拟指令：可用低频临时寄存器 r12 r14。
* 低层虚拟指令：可用高频临时寄存器 r11。
* 物理指令：直接对应到 ARM 指令

### 寄存器分配

* 后端尽量节省了临时寄存器的使用（例如没有使用帧指针 fp），所以留出了 11 个通用寄存器（r0-r10），有效减少了寄存器分配时的溢出。
* 新版寄存器分配器直接集成在后端（初版为 IR Pass），与创新多线程框架有一些配合（创新多线程框架共享部分栈空间所以在并行区域被赋值的虚拟变量分配溢出或者调用函数时需要特殊处理）。
* 分配器使用图着色算法，会衡量各种方面的奖励与代价来优化分配。
  * 各种环境下溢出的代价（写入？读取？栈上数组指针？）
  * Phi 节点同色时的奖励
  * 染色与 ABI 规定一致时的奖励
  * ZExt 之类的指令同色时的奖励
  * 按照循环嵌套深度来放大奖励与代价

### 创新多线程框架

与经典框架 OpenMP 对比：

* **共享栈空间**：不需要将被并行化的区域拆分出来变成函数
* **框架更加易于实现**：不需要保存上下文和维护各种信息
* **更便于代码变换**：前后就地插个 `tid=__mtstart()` 和 `__mtend(tid)` 就行了

```C
for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
            A[i][j] = A[i][j] + B[i][k] * C[k][j];
        }
    }
}
```

变换为：

```C
int tid = __mtstart(); 
for (int i = starti(tid) ; i < endi(tid) ; i++) {
    for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
            A[i][j] = A[i][j] + B[i][k] * C[k][j];
        }
    }
}
__mtend(t);
```

（注：为了方便调试和测试，编译器支持在 SysY 语言中直接调用 `__mtstart` 和 `__mtend` 函数。）

（注：这里使用源代码来表述是为了便于理解，实际上的自动多线程化的变换是在中间代码层次上由 Multithreading Pass 完成的。）

* **更高的运行性能**：栈上的资源仍然可以直接通过栈指针加偏移访问

函数调用相关的开销倒不是关键，提升性能的主要因素是这样在某些场景下可以减少寄存器的使用，避免寄存器分配溢出或者减轻其影响。以矩阵乘法为例，其包含三个数组 `a` `b` `c`，被函数内联与全局数组局部化优化之后就在栈上了，`a` `b` `c` 的地址都可以直接通过栈指针加上一个编译期确定的常量得到。一般的多线程框架例如 OpenMP 栈空间不共享，每个线程的栈指针是不同的，那么就至少需要用一个新寄存器保存一下以前的栈指针才能访问 `a` `b` `c` 里面的东西了（如果保存每个数组的地址代价会更大）；而创新多线程框架在启动多线程后栈指针不会变化，所有线程栈指针是一样的，就不需要新的寄存器了。

![创新多线程框架](mt.png?raw=true)
