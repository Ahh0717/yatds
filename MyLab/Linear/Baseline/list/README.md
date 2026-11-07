# MyLab / Linear / list —— 线性表基础实验（任务 1–3）

对应题目：`src/Linear/Baseline/List/README.md` 的 Baseline 任务 1、2、3。

三个任务是**彼此独立**的提交物，各自在自己的子目录里：

| 目录 | 内容 | 单独构建 |
| --- | --- | --- |
| `task1/` | 单链表模板类 | `cd task1 && make && make test` |
| `task2/` | 动态数组 | `cd task2 && make && make test` |
| `task3/` | 三个经典链表算法 | `cd task3 && make && make test` |

每个子目录都有 `src/`、`inputs/`、`outputs/`、`Makefile`、`README_taskN.md`
和名为 `test` 的测试程序；它的 `Makefile` 只编译自己那个任务，不依赖其他任务。
详细设计说明见各目录下的 README。

## 公共头文件与跨任务工具

- `include/input_util.h` —— 三个驱动程序共用的输入解析，各任务 Makefile 用
  `-I../include` 引用。
- `tools/` —— **不属于任何单个任务**的跨任务工具：`bench.cpp`（性能对比，
  需要同时用到任务 1 和任务 2 的类）、`invariants.cpp`（算法级不变量检查）、
  `run_tests.py`（供 `make sanitize` 复用的比对脚本）、`oracle_task*.py`
  （独立参考实现，用于生成期望输出）、`fuzz_task*.py`（随机差分测试）。

顶层 Makefile 只构建这些跨任务工具：

```bash
make          # 编译 build/bench 与 build/invariants
make test     # 依次在 task1/2/3 里跑 make test，再跑不变量检查
make bench    # 性能对比，输出 PERF 行
make fuzz      # 随机差分测试（需要 Python 3）
make sanitize  # ASan/UBSan + 泄漏检查
make clean     # 清理全部构建产物
```

## 目录结构

```text
MyLab/Linear/list/
├── Makefile                 # 只构建跨任务工具
├── README.md                # 本文件：结构、构建入口与复杂度小结
├── bench_results/           # 实测的 PERF 输出
├── include/
│   └── input_util.h         # 三个任务共用的输入解析
├── tools/                   # 跨任务工具，不属于任何单个任务
│   ├── bench.cpp            # 性能对比：DynamicArray / std::vector / LinkedList
│   ├── invariants.cpp       # 算法级不变量检查
│   ├── run_tests.py         # 样例比对的通用实现（--exe / --inputs / --outputs）
│   ├── run_invariants.py    # 不变量检查的汇总包装
│   ├── oracle_task1/2/3.py  # 独立参考实现，用于生成期望输出
│   └── fuzz_task1/2/3.py    # 随机差分测试
├── task1/
│   ├── Makefile             # 只编译任务 1
│   ├── test                 # 读取 inputs/ 与 outputs/ 比对
│   ├── README_task1.md
│   ├── inputs/  outputs/    # caseMM.txt
│   └── src/                 # linked_list.h/.cpp, main_task1.cpp
├── task2/
│   └── ...                  # dynamic_array.h/.cpp, main_task2.cpp
└── task3/
    └── ...                  # list_advanced.h/.cpp, main_task3.cpp
```

## 样例约定

`taskN/inputs/caseMM.txt` ↔ `taskN/outputs/caseMM.txt`。新增样例只要按这个
格式放进对应任务的两个目录，`make test` 会自动纳入。比对时忽略行尾空白与
文件末尾空行，以及以 `PERF ` 开头的行（性能统计，耗时每次不同）。

## 设计思路

### 需求拆分

题目按「链表基础 → 可变数组 → 链表进阶算法」三个递进任务展开：

1. **任务 1 单链表**：手写模板类，要求元素存储禁用 STL 容器、内存全部手动管理，覆盖增删改查与越界、空表、重复值等边界。
2. **任务 2 动态数组**：自己实现扩容 / 搬移 / 释放，并分别与 `std::vector`、链表做同规模性能对比。
3. **任务 3 链表算法**：判环、求交、公共段反转，约束 O(n) 时间、O(1) 额外空间，且相交必须按「节点地址相等」判定。

### 结构选择

- **链表**：单链表维护 `head_` / `tail_` / `size_`，使 `pushBack` 与 `size` 都为 O(1)；头尾插入删除走 O(1) 分支，中间才 O(n) 遍历。
- **动态数组**：连续内存 + 容量翻倍扩容，随机访问 O(1)；默认初始容量 `kMinCapacity = 4`，避免容量 0 与除零。
- **链表算法**：判环用快慢指针 + 环入口数学推导（O(1) 空间），求交用长度差对齐后同步前进，公共段只就地反转一次；共享节点统一由 `NodePool` 登记释放。

### 接口与取舍

- 越界操作返回 `false`（或输出 `ERROR`）且不改结构，不抛异常，与题目样例口径一致。
- `clear()` 只清元素不缩容（题目明确要求）；需要归还内存时显式调用 `shrinkToFit()`。
- 相交按**指针相等**判定而非值相等，这是任务 3 的关键语义，测试数据里公共尾只 `new` 一次。
- 元素存储全部手动 `new` / `delete`（或 `::operator new` / `::operator delete` + placement new），不借助任何 STL 容器。

## 复杂度小结

### 任务 1：单链表（`n` 为元素个数）

| 操作 | 时间 | 说明 |
| --- | --- | --- |
| `pushFront` | O(1) | 改 `head_` |
| `pushBack` | O(1) | 维护 `tail_`，无需遍历 |
| `insertAt(i, x)` | O(n) | 需走到下标 `i-1`；头部插入可走 O(1) 分支 |
| `eraseAt(i)` | O(n) | 同上；头尾都能 O(1) 处理 |
| `access(i)` / `find(x)` / `contains(x)` | O(n) | 只能顺序遍历 |
| `size` / `empty` | O(1) | 维护 `size_` |
| `clear` / 析构 | O(n) | 逐节点释放 |
| 扩展：`removeAll`、`unique`、`reverse`、`append` | O(n)～O(n²) | `unique` 不借助哈希表，为 O(n²) |
| 空间 | O(n) | 每节点一个 `next` 指针的额外开销 |

### 任务 2：动态数组（`n` 为元素个数，`C` 为容量）

| 操作 | 时间 | 说明 |
| --- | --- | --- |
| `pushBack` | 均摊 O(1)，单次最坏 O(n) | 容量翻倍，扩容时搬迁全部元素 |
| `insertAt(i, x)` / `eraseAt(i)` | O(n) | 需要搬移 `i` 之后的元素 |
| `get` / `set` / `at` / `operator[]` / `front` / `back` | O(1) | 连续内存随机访问 |
| `size` / `capacity` / `empty` | O(1) | |
| `reserve` | O(n) | 只在需要扩容时发生 |
| `resize` | O(Δn) | 补默认值或析构多余元素 |
| `clear` | O(n) 析构，**不缩容** | 见 README_task2.md |
| `shrinkToFit` | O(n) | 显式缩容 |
| 空间 | O(C) | 扩容时峰值会短暂达到 O(C + n) |

实测（`n = 100000`，`ops = 200000`，详见 README_task2.md）：

```text
PERF vector_like n=100000 ops=200000 time_ms=1559.37
PERF std_vector  n=100000 ops=200000 time_ms=1561.58
PERF linked_list n=100000 ops=200000 time_ms=28550.52
```

以下标随机访问/中间修改为主时，数组比链表快两个数量级；只有在几乎全是
尾部操作的场景，链表的 O(1) 才可能追上。

### 任务 3：进阶链表算法（`n`、`m` 为两条链表的长度）

| 算法 | 时间 | 额外空间 | 关键点 |
| --- | --- | --- | --- |
| `detectAndRemoveCycle` | O(n) | O(1) | 快慢指针 + 环入口数学推导，再沿环找入口前驱断开 |
| `getIntersectionNode` | O(n + m) | O(1) | 长度差对齐后同步前进 |
| `reverseSharedPath` | O(n + m) | O(1) | 公共段只就地反转一次，两段前缀接到同一新头 |

三者都不使用哈希表、数组或标记位，也不修改节点值。内存由 `NodePool`
统一释放，避免共享节点被重复 `delete`、以及带环链表无法遍历到头的问题。

## 其他目标

```bash
make bench     # 性能对比，输出 PERF 行
make fuzz      # 随机差分测试（需要 Python 3）
make sanitize  # ASan/UBSan + 泄漏检查跑样例与模糊测试
```