# AI 协作记录 —— MyLab / Linear / StackAndQueue

最近更新：2026-10-31

## 0. 约束

- 课程仓库：`https://gitlab.arcsysu.cn/data-stru-26spring/yatds`
- **所有改动只在 `MyLab/` 下，不动仓库根目录。**
- 结构沿用 `MyLab/Linear/list/` 的做法：公共 `include/`、跨任务 `tools/`、
  每个任务一个自包含子目录，自带 `Makefile` / `test` / `README_taskN.md`。
- 头文件名与类名沿用题目模板：`stackar.hpp`（`dsaac::ArrayStack<T>`）、
  `stackli.hpp`（`dsaac::LinkedStack<T>`）。模板允许内部用 `std::vector` /
  `std::forward_list`（与 list 实验“禁用 STL 容器”不同，这里是题目明确允许）。

## 1. 一键复核

```bash
cd MyLab/Linear/StackAndQueue
make test        # task1 栈 9/9 + 3/3；task2 队列 6/6 + 6/6 + 3/3 + 3/3 + 2/2；
                 # task3 int 6/6 + string 2/2；最后不变量检查 1154 项
make fuzz        # 三个任务各 200 轮随机差分
make sanitize    # ASan/UBSan + 泄漏检查
make clean
```

Windows 上把 `make` 换成 `mingw32-make`，已实测（见第 8 节）。
编译零 warning（`-Wall -Wextra -Wpedantic`）。

## 2. 第 7 轮：阶段 1，数组栈 + 链式栈

**Prompt 要点**
- `ArrayStack<T>`：固定容量、私有 `std::vector<T> values_`、维护 size/capacity；
  实现构造(capacity)/empty/full/push/pop/top。
- `LinkedStack<T>`：`std::forward_list` 或自建结点，栈顶在头；无 `full`。
- 满栈 push 抛 `std::overflow_error`；空栈 pop/top 抛 `std::out_of_range`。
- 每类至少 5 个扩展功能，必须含 `size()` 与 `printTopToBottom()`；
  其余自选（clear/swap/toVector/operator== 等），参考 `std::stack`。
- 输入 `capacity n` / 初始元素（栈底到栈顶）/ `m` / m 行操作；
  操作 `PUSH/POP/TOP/SIZE/EMPTY/PRINT`；空栈 POP/TOP 输出 `ERROR`，
  EMPTY 输出 true/false，PRINT 栈顶到栈底、空栈 `EMPTY`，PUSH 不输出。
- `main_task1.cpp` 里写 `benchmark()`：功能输出后追加三行 PERF
  （array_stack / linked_stack / std_stack，n=100000 ops=200000）。
- 自测样例放 `inputs/case01.txt` + `outputs/case01.txt`；比对忽略 PERF 行。

**完成的工作**
- `task1/src/stackar.hpp/.cpp`：`std::vector` + 构造时 `reserve`，容量固定；
  扩展 `size/capacity/clear/setTop/swap/toVector/assign/operator==/emplace`
  （9 个）。
- `task1/src/stackli.hpp/.cpp`：`forward_list` 头即栈顶；单独维护 `size_`
  保证 `size()` 是 O(1)；同一组扩展功能。
- `task1/src/main_task1.cpp`：一份 `processOps<T, Stack>` 同时服务两种栈，
  用 `--array`（默认）/ `--linked` 选择；`benchmark()` 用固定种子生成
  “50% PUSH / 30% POP / 20% TOP”且不对空栈操作的序列。
- 样例：`inputs/` 8 个（题目样例、容量 1、空栈、压满再弹空、string、
  负数/INT_MAX、SETTOP+CLEAR、满栈边界）；`linked_inputs/` 3 个
  （初始元素超过 capacity、持续增长、string + CLEAR）。
- `tools/oracle_task1.py` 生成全部期望输出；`tools/fuzz_task1.py` 随机差分；
  `tools/invariants.cpp` 断言异常类型与容量边界。
- `test` 在 list 版基础上加了 `--arg` 透传，用 `--arg=--linked` 跑链式栈样例。

**踩过的坑**
- `emplace` 若是成员模板，在 `.cpp` 里无法显式实例化 → 定义在头文件内。
- 一开始让两套样例共用 `inputs/` 并要求 array/linked 结果一致，但
  “初始元素超过 capacity”本身就是链式栈专属用例 → 拆成
  `inputs/`（数组栈）与 `linked_inputs/`（链式栈）两套目录。
- `ArrayStack` 的 `swap` 若拒绝不同容量，swap 就失去通用性 →
  改为容量随元素一起交换。
- `test` 会 `chdir` 到自身目录，顶层 `make sanitize` 里 `--exe`/`--inputs`
  传相对路径会找不到 → 用 `$(abspath ...)`。
- 样例文件里 `m` 与实际行数写错两次（多写/少写 1 行），oracle 的
  “多余 token”检查直接报错，改回实际条数后正常。
- 随机测试里 `n = rng.choice([..., capacity])` 在 capacity 较小时
  仍可能超过容量 → 对数组栈改用 `min(..., capacity)`。

## 3. 关键设计决策

| 决策 | 原因 |
| --- | --- |
| 链式栈不提供 `full()` | 链式结构没有容量上限，返回常量 `false` 会误导 |
| 数组栈 `clear()` 不缩容 | 容量是构造时定死的固定上限 |
| 数组栈样例要求 `n <= capacity` | 装不下就无法构造合法初始状态，驱动报错退出 |
| 满栈 PUSH 静默忽略 | 题目约定 PUSH 不输出，且未规定满栈行为 |
| `SETTOP` 作为扩展操作 | 满栈时无法用 push 更新栈顶，需要一个显式接口 |
| `test` 加 `--arg` | 一套比对逻辑同时服务两种栈，避免复制脚本 |

## 4. 实测 PERF

```text
PERF name=array_stack n=100000 ops=200000 time_ms=2.65
PERF name=linked_stack n=100000 ops=200000 time_ms=6.37
PERF name=std_stack    n=100000 ops=200000 time_ms=1.34
```

三次运行波动范围：array 2.1–4.2 ms，linked 5.4–6.4 ms，std 1.3–1.4 ms。

## 5. 第 8 轮：阶段 2，队列

**Prompt 要点**
- 重点是解决数组队列 front/rear 走到缓冲区末尾的问题。
- `CircularQueue<T>`（骨架 `queue.hpp`）：循环缓冲区，私有
  `std::vector<T> buffer_` + `front_`/`back_`，下标回绕；额外一个空槽
  区分空满；满 push 抛 `overflow_error`，空 pop/front 抛 `out_of_range`。
- `ShiftQueue<T>`：出队后剩余元素整体左移，front 恒为 0；注释写清与
  循环队列的复杂度差异（出队 O(n) vs O(1)）。
- `LinkedQueue<T>`：`Node* head_/tail_/size_`，尾插头删，无容量限制。
- 每类至少 5 个功能，必须含 `size()` 与 `printFrontToBack()`。
- 输入 `capacity n` / 元素（队首→队尾）/ `m` / 操作；
  操作 `PUSH/POP/FRONT/SIZE/EMPTY/PRINT`；空队列 POP/FRONT 输出 ERROR。
- `benchmark()` 功能输出后追加 circular_queue / linked_queue / std_queue
  三行 PERF。

**一处需要确认的语义**
- 题目消息写“容量 N 实际存 N-1 个元素”，骨架 README 写“创建可容纳指定
  数量元素的队列”。**已问过用户，选定：capacity = 可存元素个数**
  （内部缓冲区开 capacity+1 个槽），与骨架 README 一致。
  这样 `capacity 6` 的样例可以连续 push 6 个元素。

**完成的工作**
- `task2/src/queue.hpp/.cpp`：环上 `next()` 回绕；`buffer_(capacity+1)`；
  额外维护 `count_` 让 `size()` 是 O(1)（空满仍用空槽判断，count 只服务 size）。
  扩展：`size/capacity/back/clear/swap/toVector/operator==/emplace`。
- `task2/src/shift_queue.h/.cpp`：`pop` 用 `std::move` 整体左移；
  文件头注释写明与循环队列的复杂度对比。
- `task2/src/linked_queue.h/.cpp`：自建 `Node`，五法则齐全（深拷贝 +
  移动）；`pop` 删空时同步清 `tail_`。
- `task2/src/main_task2.cpp`：一份 `processOps<T, Queue>` 服务三种队列，
  `--circular`（默认）/ `--shift` / `--linked` 选择；`benchmark()` 同 task1
  的模式（50% PUSH / 30% POP / 20% FRONT）。
- 样例：`inputs/` 5 个（含题目样例）、`capacity_inputs/` 3 个（容量 0/1/装满）、
  `linked_inputs/` 2 个（超过 capacity）。
- `tools/oracle_task2.py` / `tools/fuzz_task2.py` 新增；
  `tools/invariants.cpp` 增加队列部分（3 种队列的通用语义 + 各自专项）。

**踩过的坑**
- 顶层 `main()` 里按 impl 分支直接 `return run(...)`，导致 `benchmark()`
  变成未使用函数（`-Wunused-function`）→ 改成先存 `rc` 再统一调用。
- `capacity_inputs/` 用链式队列跑会失败：样例依赖“满队列丢弃”的数组
  语义 → 容量样例只给两种数组实现跑。
- invariants 里回绕/连续出入队的循环忘记判满，直接把队列压爆
  （`terminate: overflow_error`）→ 改成“满了先出队”。
- 样例 `m` 与实际行数又写错两次（19 vs 20、11 vs 12）→ 用脚本统一核对。
- 顶层 `make test` 原来只跑 task1 → 改成遍历 task1/task2 并汇总退出码。

## 6. 第 9 轮：阶段 3，栈与队列互转

**Prompt 要点**
- `StackByQueue<T>`：单个 `std::queue`；push 入队，pop/top 轮转前 n-1 个
  元素让栈顶来到队首；均摊 O(n)。
- `QueueByStack<T>`：两个 `std::stack`（`in_`/`out_`）；`out_` 空时把
  `in_` 倒过去；均摊 O(1)。
- `VectorStack<T>` / `VectorQueue<T>`：`std::vector` 存储，提供
  `toQueue()` / `toStack()` 相互转换。
- 每类至少 `size`/`empty`/`print`，空结构操作抛 `out_of_range`。
- 输入含三种场景（STACK_BY_QUEUE / QUEUE_BY_STACK / CONVERT），
  每个场景先输出 `CASE t`。
- 每个类注释写一句关键操作复杂度，尤其是 StackByQueue 的均摊分析。
- 汇报编译命令、比对结果、PERF 数据。

**完成的工作**
- `task3/src/stack_by_queue.h/.cpp`：单个 `std::queue`，`front`=栈底、
  `back`=栈顶；`rotate()` 转 n-1 次、`rotateOnce()` 转 1 次，`top()` 用
  `rotate + rotateOnce`（合计 n 次 = 恒等）；`top()` 按值返回，避免
  `pop_front` 后引用悬垂。扩展：`clear/swap/toVector/operator==/
  printTopToBottom/emplace`。
- `task3/src/queue_by_stack.h/.cpp`：`mutable std::stack<T> in_, out_`，
  `pour()` 声明为 const，`pop()/front()` 在 `out_` 空时倒入。扩展同上，
  另加 `toVector/printFrontToBack`。
- `task3/src/vector_stack.h/.cpp`、`vector_queue.h/.cpp`：栈顶在 vector
  尾部；队列用 `front_` 下标 + `compactIfNeeded()` 压缩。
  `VectorStack::toQueue()` 定义在 `vector_stack.cpp`（那里
  `VectorQueue` 才是完整类型），`VectorQueue::toStack()` 在
  `vector_queue.cpp`；`VectorStack` 的显式实例化只放 `vector_stack.cpp`，
  避免重复特化。
- `task3/src/main_task3.cpp`：`runStackOps`/`runQueueOps` 模板复用；
  `runConvert` 统一处理两个方向；`benchmark()` 输出四行 PERF
  （stack_by_queue / queue_by_stack / vector_based / vector_queue），
  规模 n=2000 ops=2000 并如实打印。
- 样例：`inputs/case01-05`（含题目原样例、三个空场景、`top()` 不破坏顺序、
  `out_` 非空时继续 push、INT_MAX）、`string_inputs/case01-02`。
- `tools/oracle_task3.py`、`tools/fuzz_task3.py` 新增；
  `tools/invariants.cpp` 增加 `checkStackByQueue` / `checkQueueByStack` /
  `checkVectorConversions`（含与 `std::stack`/`std::queue` 的随机差分）。
- 顶层 `Makefile`：加 `TASK3_SRCS`、`-Itask3/src`、test/fuzz/sanitize/clean
  都接入 task3（sanitize 加 `task3_san` 与 60 轮 fuzz）。

**踩过的坑**
- **`top()` 轮转次数错误**：第一版转两次 `n-1`，n 为奇数时不是恒等变换，
  `PRINT` 输出 `3 / 4 / 2 4 1` 而不是 `3 2 1` → 改成 `rotate()`（n-1）
  + `rotateOnce()`（1），合计 n 次。
- **`STACK_TO_QUEUE` 方向反了**：`toQueue()` 原先按栈底→栈顶入队，
  得到自然序；题目要求两个方向都输出逆序 → 改成从 `rbegin()` 往
  `rend()` 入队。
- **头文件循环 / 不完整类型**：`toQueue()` 返回 `VectorQueue<T>`，
  不能写在 `vector_stack.h` 里；先尝试在 `vector_queue.h` 末尾定义，
  又因显式实例化看不到函数体而链接失败 → 最终把定义搬进
  `vector_stack.cpp`（先 include `vector_queue.h`），并删掉
  `vector_queue.cpp` 里重复的 `VectorStack` 实例化。
- **不变量检查的往返期望写错**：以为“两次转换回到原序列”，实际两个
  方向各自都是反转，往返净效果是一次反转 → 按实际语义改断言并写清理由。
- **PERF 规模跑不完**：`StackByQueue` 的 O(n) pop 在 n=100000 时要
  10^10 次队列操作 → 降到 2000/2000，并在代码注释、README 里说明原因。
- **样例 `m` 行数又对不上**（case02 写了 `SIZE_IGNORED` 占位）→ 删除该行、
  m 从 16 改成 15。

**实测（`-O2`，7 次运行取中位数）**
```text
PERF name=stack_by_queue n=2000 ops=2000 time_ms≈14.1 (8.1–26.8)
PERF name=queue_by_stack n=2000 ops=2000 time_ms≈0.03
PERF name=vector_based   n=2000 ops=2000 time_ms≈0.03
PERF name=vector_queue   n=2000 ops=2000 time_ms≈0.03
```

## 7. 第 10 轮：收口为可交付状态（一键编译 / 一键测试 / Windows 实测）

**Prompt 要点**
- 顶层 `Makefile` 只构建跨任务工具：`make` → `build/invariants`，
  `make test` → 依次 `make -C taskN test` 再跑不变量，`make clean` 清空
  本级与三个任务的 `build`；加 `EXEEXT`，`INVAR` 目标名带 `$(EXEEXT)`。
- 三个 `taskN/test` 收口为静态 Python 脚本，满足总纲跨平台要求
  （`resolve_exe` + `encoding="utf-8"` + `reconfigure` + 忽略 `PERF ` 行）。
- 不变量检查输出「通过 N 项，失败 0 项」。
- 样例补齐边界（空结构、满结构、回绕、容量边界、连续操作）。
- README.md 写清目录结构、每个任务的复杂度小结、构建命令。

**做了什么**
- 四个 Makefile 统一跨平台前缀：`EXEEXT` / `MKDIR` / `RM_TREE` /
  `ECHO_BLANK` / `PYTHON`。每条 recipe 只写一条命令，`cmd.exe` 与 `sh`
  都能执行；`test` 一律 `$(PYTHON) test`，不依赖可执行位。
- 不变量检查改成 pass/fail 计数，覆盖 push→`top` 等于刚入栈值、
  pop 后 size 减一、满队列 push 抛异常、两种转换方向都反转等，
  实测 1154 项全过。
- 新增边界样例：`task1/inputs/case09`（容量 0）、
  `task2/inputs/case06`（容量 2 反复装满弹空的回绕）、
  `task3/inputs/case06`（单元素、in_/out_ 来回倒、单元素双向 CONVERT）。
- `README.md` 重写「构建与测试」一节，补 Windows 命令与跨平台约定；
  三个 `README_taskN.md` 的样例表、`.gitignore`（补全三个 build 与 core dump）
  同步更新。
- 顶层 `sanitize` 里的 `taskN/test` 也改成 `$(PYTHON) ...`，消除对可执行位的依赖。

**踩过的坑**
- **Windows 的 `echo` 会打印 `Echo is OFF`**：recipe 里的空行分隔
  `@echo` 在 cmd.exe 下变成 `Echo is OFF` → 改成 `ECHO_BLANK`
  （Windows 用 `echo.`，POSIX 用 `echo`）。
- **顶层 `clean` 原先用 `rm -rf`**，Windows 下没有 → 换成 `$(RM_TREE)`
  并在前面加 `-` 忽略“目录不存在”的返回码。
- **`./$(INVAR)` 在 Windows 下找不到裸相对路径** → 改成 `$(abspath $(INVAR))`。
- **MinGW 工具链缺少运行时 DLL**：`libwinpthread-1.dll`、`libintl-8.dll`
  缺失时 g++ 静默失败（只打印一行命令、Error 1，不打印原因）→
  从 msys2 包装齐 `libwinpthread` / `gettext` / `libiconv` 后正常。

**Windows 实测环境**
- Wine 11.0（WineHQ 包，系统自带的 Wine 10 在 glibc 2.43 上崩溃）
  + Windows 版 Python 3.12.7（embeddable）
  + 用 MinGW 交叉编译的 GNU Make 4.4.1 并改名 `mingw32-make.exe`
  + msys2 的 `mingw-w64-x86_64-gcc 16.2.0`（真实 Windows PE 编译器和
  `cc1plus.exe`）。Windows Python 报告 `os.name='nt'`，走的是真实的
  `.exe` 补全 / 绝对路径 / UTF-8 重配置分支。

**验收结果（逐条）**
1. `cd task1/2/3 && mingw32-make && mingw32-make test`
   —— task1 9/9 + 3/3、task2 6/6 + 6/6 + 3/3 + 3/3 + 2/2、
   task3 int 6/6 + string 2/2，全部 PASS，退出码 0。
2. 顶层 `mingw32-make test`：三个任务全 PASS，不变量检查
   `通过 1154 项，失败 0 项`，`EXITCODE=0`。
3. 顶层 `mingw32-make` 连跑两次：第一次编译 `build/invariants.exe`
   （因为已 `clean`），第二次输出 `Nothing to be done for 'all'.`，
   不再出现编译命令，`RC1=RC2=0`。
4. `git status`：`StackAndQueue/` 下只有源码、样例、文档与 `Makefile`，
   构建产物与 `core.*` 已被 `.gitignore` 挡住，无冗余旧文件。

## 8. 下次继续的入口

- `tools/oracle_task*.py` 已经稳定，新任务照抄 `--write` 模式。
- 若样例依赖某种实现的容量语义，继续按 `inputs/` + `xxx_inputs/` 拆目录。
- 三个阶段的样例、oracle、fuzz、不变量、sanitize 已全部接入顶层 Makefile；
  后续任务只要新增 `TASKn_SRCS` 与对应目录即可。
- 新增任务时记得同步四处：顶层 `Makefile` 的 `TASKn_SRCS` / test，
  `.gitignore` 的 `taskN/build/`，`README.md` 的目录树与复杂度表，
  以及 `taskN/test` 里的默认 `--exe build/taskN` 路径。