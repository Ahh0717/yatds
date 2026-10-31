# MyLab / Linear / StackAndQueue —— 栈与队列实验

对应题目：`src/Linear/Baseline/StackAndQueue/README.md`。

## 当前进度

| 阶段 | 目录 | 内容 | 状态 |
| --- | --- | --- | --- |
| 阶段 1 | `task1/` | 数组栈 `ArrayStack<T>` + 链式栈 `LinkedStack<T>` | 已完成 |
| 阶段 2 | `task2/` | 循环队列 + 平移式队列 + 链式队列 | 已完成 |
| 阶段 3 | `task3/` | 队列实现栈 / 栈实现队列 + `<vector>` 版相互转换 | 已完成 |

每个任务是独立提交物，有自己的 Makefile、`test`、`README_taskN.md`：

```bash
cd task1 && make && make test
cd task2 && make && make test
cd task3 && make && make test
```

## 目录结构

```text
MyLab/Linear/StackAndQueue/
├── Makefile              # 只构建跨任务工具（不变量检查），并汇总测试入口
├── README.md             # 本文件
├── ai_record.md          # 逐轮协作记录
├── include/
│   └── input_util.h      # 与 list 实验一致的输入解析
├── tools/                # 不属于单个任务
│   ├── oracle_task1/2/3.py   # 独立参考实现，生成期望输出
│   ├── fuzz_task1/2/3.py     # 随机差分测试
│   └── invariants.cpp        # 算法级不变量检查
├── task1/
│   ├── Makefile          # 只编译任务 1
│   ├── test              # 读取 inputs/ 与 outputs/ 比对，忽略 PERF 行
│   ├── README_task1.md
│   ├── src/              # stackar.hpp/.cpp, stackli.hpp/.cpp, main_task1.cpp
│   ├── inputs/  outputs/ # 数组栈样例
│   └── linked_inputs/  linked_outputs/   # 链式栈专属样例
├── task2/
│   ├── Makefile
│   ├── test
│   ├── README_task2.md
│   ├── src/              # queue.hpp/.cpp, shift_queue, linked_queue, main_task2.cpp
│   ├── inputs/  outputs/ # 两种数组队列共用
│   ├── capacity_inputs/  capacity_outputs/   # 容量 0/1 等边界
│   └── linked_inputs/  linked_outputs/       # 链式队列专属样例
└── task3/
    ├── Makefile
    ├── test
    ├── README_task3.md
    ├── src/              # stack_by_queue, queue_by_stack,
    │                     # vector_stack, vector_queue, main_task3.cpp
    ├── inputs/  outputs/ # int 样例
    └── string_inputs/  string_outputs/   # std::string 样例
```

## 构建与测试

一键编译 / 一键测试，Linux、macOS 与 Windows（MinGW）通用：

```bash
make            # 编译 build/invariants
make test       # task1 + task2 + task3 全部样例 + 不变量检查
make fuzz       # 三个任务各 200 轮随机差分（需要 Python 3）
make sanitize   # ASan/UBSan + 泄漏检查下重跑全部验证（仅 Linux/macOS）
make clean      # 清理全部构建产物
```

Windows 上把 `make` 换成 `mingw32-make`：

```bat
mingw32-make
mingw32-make test
mingw32-make clean
```

可单独构建、测试某个任务：

```bash
cd task1 && make && make test      # 数组栈 + 链式栈
cd task2 && make && make test      # 循环队列 / 平移队列 / 链式队列
cd task3 && make && make test      # int + string 两套样例
```

跨平台约定（四个 Makefile 一致）：

- 目标名都带 `$(EXEEXT)`：Windows 下是 `.exe`，其他平台为空；
- 每条 recipe 只写一条命令，不用 shell 循环和 `&&`，`cmd.exe` 与 `sh`
  都能执行；`test` 是静态 Python 脚本，统一用 `$(PYTHON)` 调用
  （Windows 是 `python`，其他平台是 `python3`），不依赖可执行位；
- `test` 脚本在 Windows 上自动补 `.exe` 后缀、用绝对路径调起被测程序，
  所有文件按 UTF-8 读写，输出重配置为 UTF-8；比对时忽略 `PERF ` 行
  （耗时每次不同）。

## 复杂度小结

`n` 为元素个数，`C` 为数组栈容量。

| 操作 | ArrayStack | LinkedStack |
| --- | --- | --- |
| `push` | O(1)，满时抛 `overflow_error` | O(1) |
| `pop` / `top` | O(1)，空时抛 `out_of_range` | O(1) |
| `size` / `empty` / `full` | O(1) | O(1)（无 `full`） |
| `clear` | O(n) 析构，不缩容 | O(n) 释放结点 |
| `printTopToBottom` | O(n) | O(n) |
| `toVector` | O(n) | O(n)（需逆序） |
| `swap` | O(1) | O(1) |
| `operator==` | O(n) | O(n) |
| 空间 | O(C)，构造时一次分配 | O(n)，每结点一个 `next` 指针 |

实测（`n = 100000`，`ops = 200000`）：

```text
PERF name=array_stack n=100000 ops=200000 time_ms≈2.1
PERF name=linked_stack n=100000 ops=200000 time_ms≈5.6
PERF name=std_stack    n=100000 ops=200000 time_ms≈1.3
```

数组栈是连续内存上的尾插尾删，缓存局部性好、每次操作只有一次自增；
链式栈每次 `push` 都要单独 `new` 一个结点（`pop` 再 `delete`），额外付出
分配器开销与指针跳转，因此慢 2～3 倍。`std::stack` 默认基于 `std::deque`，
分块连续存储、无需逐次分配，所以最快。三者复杂度都是 O(1)，差距来自
常数因子与内存布局，而不是渐进复杂度。

### 任务 2：队列（`n` 为元素个数，`C` 为容量）

| 操作 | CircularQueue | ShiftQueue | LinkedQueue |
| --- | --- | --- | --- |
| `push` | O(1) | O(1) | O(1) + 分配 |
| `pop` | **O(1)** | **O(n)**（整体左移） | O(1) + 释放 |
| `front` / `back` | O(1) | O(1) | O(1) |
| `size` / `empty` / `full` | O(1) | O(1) | O(1)（无 `full`） |
| 空间 | O(C)，一次分配 | O(C)，一次分配 | O(n)，逐结点分配 |

**解决 front/rear 走到缓冲区末尾的两种数组方案**：

1. **循环缓冲区**：`front_`/`back_` 取模回绕，出队只挪下标，O(1)；
   用多出的一个空槽区分空满。
2. **出队后整体左移**：`front` 恒为 0，出队时把剩余元素前移，位置永远
   从 0 重新算起；逻辑最简单，但出队退化成 O(n)。

链式队列没有下标，头删只改指针，天然不存在越界问题。

实测（`n = 100000`，`ops = 200000`）：

```text
PERF name=circular_queue n=100000 ops=200000 time_ms≈2.0-3.3
PERF name=linked_queue   n=100000 ops=200000 time_ms≈5.8-7.1
PERF name=std_queue      n=100000 ops=200000 time_ms≈1.3
```

`ShiftQueue` 未参加 PERF：O(n) 的出队在这个规模上要搬约 10^10 个元素，
与另外三者不在一个量级（见 `task2/README_task2.md`）。

### 任务 3：栈/队列互转（`n` 为元素个数）

| 操作 | StackByQueue | QueueByStack | VectorStack | VectorQueue |
| --- | --- | --- | --- | --- |
| `push` | O(1)（入队） | O(1)（压 `in_`） | O(1) 均摊 | O(1) 均摊（压缩） |
| `pop` | **O(n)**（轮转 n-1 个） | **均摊 O(1)**，最坏 O(n) | O(1) | O(1) |
| `top` / `front` | **O(n)** | 均摊 O(1)，最坏 O(n) | O(1) | O(1) |
| `toStack` / `toQueue` | — | — | O(n) | O(n) |
| 空间 | O(n)（一个队列） | O(n)（两个栈） | O(n) | O(n) |

`StackByQueue` 用队首当栈底、队尾当栈顶：`pop`/`top` 把前 n-1 个元素
“出队再入队”，把栈顶转到队首。`top` 取完值后必须再转 1 次（总共 n 次
轮转 = 恒等）才能恢复顺序。`QueueByStack` 用两个栈：`push` 进 `in_`，
`out_` 空时把 `in_` 整体倒过去，每个元素最多倒一次，所以均摊 O(1)。

两个 `<vector>` 实现之间的转换**两个方向都是把输入序列反转**：
`toStack()` 按队首→队尾压栈（打印栈顶→栈底即逆序），`toQueue()` 按
栈顶→栈底入队（队首→队尾即逆序）。

实测（`n = 2000`，`ops = 2000`；规模缩小的原因见 `task3/README_task3.md`）：

```text
PERF name=stack_by_queue n=2000 ops=2000 time_ms≈13–16
PERF name=queue_by_stack n=2000 ops=2000 time_ms≈0.03
PERF name=vector_based   n=2000 ops=2000 time_ms≈0.03
PERF name=vector_queue   n=2000 ops=2000 time_ms≈0.03
```