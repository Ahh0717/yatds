# Prompt 截图与过程证据

---

### 一、人机协作方式

本实验采用「分阶段 prompt 驱动」的协作模式，两个模块（`list`、`StackAndQueue`）各经历若干轮：

1. **需求拆分**：先对课程 README 的每个 Baseline 任务做需求拆分（数据结构选型、接口、
   边界、性能要求），据此规划出分阶段的 prompt。
2. **发出 prompt**：把每阶段的 prompt 粘贴进 AI 对话，AI 依 prompt 产出 `.cpp` / `.h`、
   `Makefile`、`test` 与样例。
3. **验证迭代**：对产出做编译、运行、`make test`、不变量检查、随机差分、ASan/UBSan 复核，
   发现问题就补一条修正 prompt 再迭代，直到全绿。

---

### 二、list 模块：线性表实验（Baseline 任务 1–3）

#### 2.1 任务 1 —— 单链表模板类

第一阶段：手写 `template<typename T> class LinkedList`，禁用 STL 容器存元素，
`Node{T val; Node* next;}`，类内维护 `head_`/`tail_`/`size_`，手动 `new`/`delete`；
实现 `pushFront`/`pushBack`/`insertAt`/`eraseAt`/`access`/`find`/`contains`/`size`/
`clear`/`print`，越界返回失败且不改链表，模板要能用 `int` 和 `std::string` 各测一遍，
边界覆盖空表、越界、`CLEAR` 后再操作、重复值。

![任务 1 的 prompt 与输出](../AI_record/list/0_prompt_output.png)

**AI 产出**：`task1/src/linked_list.h/.cpp` + `main_task1.cpp`。维护 `tail_` 保证
`pushBack`、`size` 为 O(1)；额外实现 `get`/`set`/`popFront`/`popBack`/`removeAll`/
`unique`/`append`/`reverse`/`hasCycle`/`cycleEntry` 及完整五法则（深拷贝 + 移动）。

**验证**：`make test` 4/4 PASS；样例覆盖题目样例、边界、string、负数混合四类。

#### 2.2 任务 2 —— 动态数组（实现）

第二阶段：手写 `std::vector` 风格的动态数组，成员仅 `T* data_` / `size_t size_` /
`size_t cap_`，自己申请 / 拷贝 / 释放；接口含 `pushBack`/`insertAt`/`eraseAt`/`get`/`set`/
`size`/`capacity`/`empty`/`clear`/`print` 再加至少 5 个 vector 式功能；扩容策略翻倍，
`CLEAR` 只清元素不缩容。

![任务 2 的 prompt](../AI_record/list/1_prompt.png)

**AI 产出**：`task2/src/dynamic_array.h/.cpp` + `main_task2.cpp`。内存用 `::operator new` +
placement new 管理，迁移用 `std::move_if_noexcept` 保证强异常安全；默认容量
`kMinCapacity = 4`；额外实现 `reserve`/`resize`/`at`/`operator[]`/`front`/`back`/
`popBack`/`shrinkToFit`/`emplaceBack`（共 9 个）。

![动态数组实现代码](../AI_record/list/1_output.png)

**验证**：`make test` 4/4 PASS；`CAPACITY` 期望值验证了翻倍扩容策略。

#### 2.3 任务 2 —— 性能对比（链表 vs 数组 vs `std::vector`）

**阶段任务**：写 `bench.cpp`，用同一规模、同一固定操作序列分别驱动手写数组、
`std::vector`、任务 1 的链表，各输出一行 `PERF <名字> n=... ops=... time_ms=...`。

![性能对比的 prompt](../AI_record/list/2_prompt.png)

**AI 产出**：`tools/bench.cpp`，操作序列 5/9 尾插、其余均匀分散在下标修改上，刻意让
插入 / 删除靠前以暴露数组搬移成本；`volatile` 汇总量防优化。实测数据见
`bench_results/perf_n100000_ops200000.txt`（数组 ≈ `std::vector` ≈ 1.5s，链表约 28.5s）。

**验证**：`make bench` 输出三行 PERF；后续把 `benchmark()` 内联进 `main_task2.cpp`，
让 `make test` 也能顺带产出性能数据。

#### 2.4 任务 3 —— 链表进阶算法（判环 / 求交 / 公共段反转）

**阶段任务**：三个函数均 O(n) 时间 / O(1) 空间，禁止哈希表 / 数组 / 标记位：
`detectAndRemoveCycle`（快慢指针 + 环入口推导 + 断开）、`getIntersectionNode`（长度差
对齐）、`reverseSharedPath`（只反转交汇后的公共段）；**相交 = 共享同一地址，不是值相等**。

![任务 3 的 prompt（题目）](../AI_record/list/3_prompt.png)

![任务 3 的 prompt（实现）](../AI_record/list/4_prompt.png)

**AI 产出**：`task3/src/list_advanced.h/.cpp` + `main_task3.cpp`。解环沿环走一圈找
`tail->next == entry` 断开；公共段只就地反转一次、再分别接两段前缀；内存用 `NodePool`
登记每一次 `new` 统一释放（避免共享节点二次释放、带环链表遍历不到头）。

![任务 3 的输出](../AI_record/list/3_output.png)

**验证**：`make test` 3/3 PASS；不变量检查覆盖自环、整链成环、普通环、节点共享等。

#### 2.5 收口 —— 一键编译 / 一键测试 / 独立交付

**阶段任务**：`make` 编译全部、`make test` 遍历 `inputs/` 与 `outputs/` 逐行比对
（忽略 PERF 行）并打印汇总、`make clean`；测试程序必须名为 `test`；每个任务独立子目录、
彼此不依赖。

![收口整理的 prompt](../AI_record/list/5_prompt.png){width=100%}**AI 产出**：顶层 `Makefile` 只构建跨任务工具；`tools/run_tests.py` 作通用比对器；

`tools/invariants.cpp`（115 项）与 `oracle_task*.py` / `fuzz_task*.py` 补齐样例之外的验证。

**验证**：顶层 `make test` 11/11 PASS + 不变量 115 项全过；`make clean` 后重建仍全绿。

---

### 三、StackAndQueue 模块：栈与队列实验（Baseline 任务 1–3）

#### 3.1 任务 1 —— 数组栈 + 链式栈

**阶段任务**：`ArrayStack<T>`（固定容量、私有 `std::vector`、满栈 `push` 抛
`overflow_error`）+ `LinkedStack<T>`（栈顶在头、无 `full`），空栈 `pop`/`top` 抛
`out_of_range`；每类至少 5 个扩展功能（含 `size` 与 `printTopToBottom`）；`main_task1.cpp`
功能输出后追加三行 PERF。

![阶段 1 的 prompt](../AI_record/StackAndQueue/0_prompt.png)

**AI 产出**：`task1/src/stackar.hpp/.cpp` + `stackli.hpp/.cpp` + `main_task1.cpp`。
一份 `processOps<T, Stack>` 同时服务两种栈，`--array` / `--linked` 选择；`benchmark()`
用固定种子生成 50% PUSH / 30% POP / 20% TOP 的序列。

![阶段 1 的输出](../AI_record/StackAndQueue/1_output.png)

**验证**：`make test` 数组栈 9/9 + 链式栈 3/3 PASS。

#### 3.2 任务 2 —— 循环队列 / 平移队列 / 链式队列

**阶段任务**：重点解决数组队列 front/rear 走到缓冲区末尾的问题：
`CircularQueue`（下标回绕、空槽区分空满）、`ShiftQueue`（出队整体左移、front 恒 0）、
`LinkedQueue`（尾插头删、无容量）；每类至少 5 个功能；`benchmark()` 追加三行 PERF。

![阶段 2 的 prompt](../AI_record/StackAndQueue/2_prompt.png)

**AI 产出**：`task2/src/queue.hpp/.cpp` + `shift_queue` + `linked_queue` + `main_task2.cpp`。
循环队列额外维护 `count_` 使 `size()` O(1)；`ShiftQueue::pop` 用 `std::move` 整体左移。

![阶段 2 的输出](../AI_record/StackAndQueue/2_output.png)

**验证**：`make test` 6/6 + 6/6 + 3/3 + 3/3 + 2/2 PASS（含容量 0/1、回绕、链式超容量）。

#### 3.3 任务 3 —— 栈与队列互转

**阶段任务**：`StackByQueue`（单队列轮转，pop/top O(n)）、`QueueByStack`（双栈，
均摊 O(1)）、`VectorStack`/`VectorQueue`（`<vector>` 存储 + `toQueue()`/`toStack()` 互转）；
三种场景（STACK_BY_QUEUE / QUEUE_BY_STACK / CONVERT），每个场景先输出 `CASE t`。

![阶段 3 的 prompt](../AI_record/StackAndQueue/3_prompt.png)

**AI 产出**：`task3/src/stack_by_queue` + `queue_by_stack` + `vector_stack/queue` +
`main_task3.cpp`。`StackByQueue::top` 用「rotate(n-1) + rotateOnce(1) = 恒等」恢复顺序；
`QueueByStack` 的 `out_` 空时把 `in_` 整体倒入。

![阶段 3 的输出](../AI_record/StackAndQueue/3_output.png)**验证**：`make test` int 6/6 + string 2/2 PASS；转换两个方向都正确输出反转序列。

#### 3.4 收口 —— 跨平台 / 一键测试 / Windows 实测

**阶段任务**：顶层 `Makefile` 只构建跨任务工具并汇总测试入口；四个 Makefile
统一跨平台前缀（`EXEEXT` / `PYTHON` / `MKDIR` / `RM_TREE`）；`test` 收口为静态 Python
脚本，Windows 上自动补 `.exe`、绝对路径、UTF-8 重配置；不变量检查输出「通过 N 项」。

![收口的 prompt](../AI_record/StackAndQueue/4_prompt.png)

![收口的输出（Windows 实测）](../AI_record/StackAndQueue/4_output1.png)

**验证**：Linux / macOS / Windows（MinGW）三平台 `mingw32-make test` 全 PASS，
不变量检查通过；`git status` 确认构建产物与 `core.*` 已被 `.gitignore` 挡住。
