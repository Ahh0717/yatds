# AI 协作记录 —— MyLab / Linear / list

记录每一轮的需求要点与对应完成的工作，便于下次接着做。
最近更新：2026-10-30

## 0. 项目定位与硬约束

- 课程仓库：`https://gitlab.arcsysu.cn/data-stru-26spring/yatds`
- 当前实验目录：`MyLab/Linear/list/`（线性表，Baseline 任务 1–3）
- **所有新增文件与改动只能落在 `MyLab/` 下，不能动仓库根目录。**
- 后续还会有 `MyLab/Linear/Stack/`，所以本目录用 lowercase `list` 与之并列。
- 元素存储不许用 STL 容器；链表/数组内存全部自己 `new`/`delete` 或
  `::operator new` / `::operator delete` + placement new。

## 1. 当前状态（一键复核）

```bash
cd MyLab/Linear/list

# 三个任务各自独立
cd task1 && make && make test && cd ..     # 4/4 PASS
cd task2 && make && make test && cd ..     # 4/4 PASS
cd task3 && make && make test && cd ..     # 3/3 PASS

# 顶层：跨任务工具 + 汇总测试
make && make test                          # 11/11 PASS，不变量 115 项全过
make bench                                 # PERF 三行
make clean
```

编译零 warning（`-Wall -Wextra -Wpedantic`），ASan/UBSan 下无泄漏、无越界。

## 2. 逐轮记录

### 第 1 轮：单链表模板类（任务 1）

**Prompt 要点**
- C++17 手写 `template<typename T> class LinkedList`，`Node{T val; Node* next;}`，
  类内维护 `head`/`tail`/`size`，手动 `new`/`delete`，析构逐个释放。
- 接口：`pushFront`、`pushBack`、`insertAt(i,x)`、`eraseAt(i)`、`access(i)`、
  `find(x)`（无则 -1）、`contains(x)`、`size`、`clear`、`print`。
- 自己查下标越界；越界返回失败且不改链表。
- stdin 驱动：`n` / 初始元素 / `m` / m 行操作；
  操作码 `PUSH_FRONT/PUSH_BACK/INSERT/ERASE/ACCESS/FIND/CONTAINS/SIZE/PRINT/CLEAR`。
- 只有 `ACCESS/FIND/CONTAINS/SIZE/PRINT` 输出；越界输出 `ERROR`；
  `PRINT` 空格分隔、空表输出 `EMPTY`；`CONTAINS` 输出 `true/false`。
- 模板必须能用 `int` 和 `std::string` 各测一遍。
- 边界用例必须覆盖：空表打印、空表 ACCESS、越界 INSERT/ERASE、
  CLEAR 后再操作、重复值 FIND 取第一个。
- 给定自测样例（5 个元素 / 10 个操作）需逐行一致。

**完成的工作**
- `task1/src/linked_list.h` / `.cpp` / `main_task1.cpp`。
- 实现 `tail_` 的精细维护：头删/尾删/`clear` 后 `tail_` 都正确，清空后链表可复用。
- 额外功能：`get`、`set`、`popFront`、`popBack`、`removeAll`、`unique`、
  `append`、`reverse`、`hasCycle`、`cycleEntry`、`head()`；
  外加深拷贝构造、拷贝赋值（copy-and-swap）、移动构造/赋值，完整符合五法则。
- `.cpp` 里做 `template class LinkedList<int>; template class LinkedList<std::string>;`
  显式实例化，当编译期自检。
- 样例：`case01` 题目样例、`case02` 边界、`case03` string、`case04` 负数/混合。

**踩过的坑**
- 初始用重载函数做 token→值转换，重载解析选错 → 改成
  `template<typename T> struct TokenValue` 特化。
- `std::stoi` 遇到格式错误会抛 `std::invalid_argument` 直接 core dump →
  引入 `Cursor` + `readIndex`/`readValue` 返回 bool 并写 stderr 诊断。
- string 样例在 int 模式下失败 → 加 `--int` / `--string` 开关，并从首行元素自动判断类型。

### 第 2 轮：动态数组 + 性能对比（任务 2）

**Prompt 要点**
- 手写类 `std::vector` 的动态数组，成员恰好 `T* data_`、`size_t size_`、`size_t cap_`，
  自己申请/拷贝/释放，禁止内部包 `std::vector`。
- 接口：`pushBack`、`insertAt`、`eraseAt`、`get`、`set`、`size`、`capacity`、
  `empty`、`clear`、`print`，**再加至少 5 个 vector 风格功能**。
- 扩容策略**翻倍**；新建空数组要给最小容量，避免容量 0。
- 驱动输入：`capacity n` / n 个元素 / `m` / m 行操作；
  操作码 `PUSH_BACK/INSERT/ERASE/GET/SET/SIZE/CAPACITY/EMPTY/PRINT/CLEAR`。
- 越界输出 `ERROR` 且不改数组；`PRINT` 空数组输出 `EMPTY`。
- **`CLEAR` 只清元素不缩容，且必须在注释里写清这个选择。**
- `bench.cpp`：同规模同操作数跑自己的数组、`std::vector`、任务 1 的链表，
  各输出一行 `PERF name=... n=... ops=... time_ms=...`；PERF 行不进 `outputs/`。
- 给定自测样例期望 `CAPACITY` 为 8（翻倍结果）。

**完成的工作**
- `task2/src/dynamic_array.h` / `.cpp` / `main_task2.cpp`。
- 内存用 `::operator new` 取原始存储 + placement new 构造，析构后 `::operator delete`；
  迁移时用 `std::move_if_noexcept` 保证强异常安全。
- 默认构造即分配 `kMinCapacity = 4`，满足"最小容量"要求。
- 额外功能（超过 5 个）：`reserve`、`resize`、`at`（抛 `std::out_of_range`）、
  `operator[]`、`front`、`back`、`popBack`、`shrinkToFit`、`emplaceBack`。
- 头部注释明确写出 `CLEAR` 不缩容的决策，以及"要归还内存请显式 `shrinkToFit()`"。

**踩过的坑**
- `std::align_val_t(alignof(T) * 0 + alignof(T))` 是垃圾写法，且 `new[]`/`delete` 不匹配
  → 统一到 `::operator new` / `::operator delete`。
- `emplaceBack` 在 `cap_ == 0` 时算出 `0 * 2` → 改为 `cap_ == 0 ? kMinCapacity : cap_ * 2`。
- 默认构造容量为 0，违反需求 → 自己的不变量测试抓到，默认构造里 `reserve(kMinCapacity)`。
- **自引用别名**：`a.pushBack(a[0])` 在扩容后会悬垂 →
  `pushBack`/`insertAt` 先把参数拷贝到局部 `T copy(x)` 再做可能的重分配，并加回归用例。
- 样例里的操作行数写错（26 vs 27、22 vs 25），按实际内容修正表头。

### 第 3 轮：三个经典链表算法（任务 3）

**Prompt 要点**
- 三个函数，**O(n) 时间 / O(1) 额外空间**，禁止哈希表、数组、标记位：
  - `detectAndRemoveCycle(head)`：快慢指针判环，数学推导找入口，断掉入口前驱，返回入口或 `nullptr`。
  - `getIntersectionNode(a,b)`：先求长度，长链先走差值，同步前进，返回首个相同节点或 `nullptr`。
  - `reverseSharedPath(a,b,intersection)`：只反转交汇点之后的公共段，各自前缀保持原序。
- **相交 = 共享同一批节点对象（同一地址），不是值相等**；测试数据公共尾只 `new` 一次。
- 输入格式：`T`；每个场景 `CYCLE n entry` + 值 / `NO_INTERSECTION nA 值 nB 值` /
  `INTERSECTION nA 前缀 nB 前缀 k 公共尾`。
- 输出：`CASE t`，然后 `CYCLE_ENTRY <值>|NONE` + `LIST ...`；
  或 `INTERSECTION NONE`；或 `INTERSECTION <值>` + `LIST_A` / `LIST_B`。
- 所有构造的节点最后必须释放；每个算法注释里写一句复杂度。

**完成的工作**
- `task3/src/list_advanced.h` / `.cpp` / `main_task3.cpp`。
- 解环入口定位后，沿环走一圈找 `tail->next == entry` 再断开，
  自环、整链成环、普通环用同一套逻辑。
- `reverseSharedPath` 记录两条链各自交汇点的前驱，公共段只就地反转一次，
  再分别把两段前缀接到新的公共头。
- 内存用 `NodePool` 登记每一次 `new`，析构时统一释放——
  共享节点不能沿 `next` 走（会二次释放），有环链表也走不到头。

**踩过的坑**
- `main_task3.cpp` 第一版写坏（残留未用的 `wantedCycle`、`int main(){return 0;}`）→ 整文件重写。
- 未使用变量 `aPrefixCount` / `bPrefixCount` → 删除（标准是零 warning）。
- `runSampleDir(task3, "task3", -1)` 会取 `tokens[-1]` → 改语义为
  `headerTokens == 0` 表示"从不使用 `--string`"。

### 第 4 轮：一键编译 / 一键测试（整合）

**Prompt 要点**
- `make` 编译全部；`make test` 遍历 `inputs/`，按文件名或前缀分派到对应任务程序，
  与 `outputs/` 同名文件逐行比对，**忽略 PERF 行**，最后打印 PASS/FAIL 汇总；`make clean`。
- 测试程序必须叫 `test`（脚本或 C++ 均可），由 `make test` 调用。
- 验收：`make && make test` 全 PASS；`make clean` 之后重跑仍 PASS。
- 汇报结果与每个任务的复杂度小结。

**完成的工作**
- 统一 Makefile：任务程序、`bench`、`invariants`、`test` 入口。
- `test` 由 Makefile 生成一个 shell 包装，真实逻辑在 `tools/run_tests.py`：
  按 `taskN_` 前缀分派，`normalize()` 去行尾空白/末尾空行/PERF 行，
  `first_difference()` 报第几行不同，退出码 0/1。
- 额外加了算法级不变量检查（`tools/invariants.cpp`，115 项），覆盖样例抓不到的
  下标语义、容量策略、解环、节点共享、深拷贝等。
- 验证过测试器本身有效：改一个期望值 → `FAIL ... 第 3 行不同`，退出码 1；
  往期望文件追加 PERF 行 → 仍然 PASS。

**踩过的坑**
- 目标名 `test` 和生成文件 `test` 同名 → `overriding recipe` 警告、
  `Circular test <- all dependency dropped`、`./test: No such file or directory`
  → 用 `$(BUILD)/.test-script` 作 stamp 目标，由它生成 `./test`。
- 生成的包装里 `set -e` 会在不变量汇总前退出 → 去掉 `set -e`，改用 `|| status=1`。

### 第 5 轮：目录位置调整

**Prompt 要点**
- 所有新文件夹和改动都必须在 `MyLab/` 下，不能动根目录。
- 本任务应放在 `MyLab/Linear/list`，因为后续还有 `MyLab/Linear/Stack`。
- insert 的语义确认：`INSERT i x` 就是"在下标 i 处插入"。

**完成的工作**
- 清理了误建在 `~/inputs`、`~/outputs` 的空目录。
- 目录从 `MyLab/List` → `MyLab/Linear/list`，同步更新 Makefile 头注释与 README 中的路径。
- 重跑 `make clean && make && make test`：11/11 PASS、不变量 115 项全过、rc=0，
  并确认 `git status` 里根目录只有 `?? MyLab/`。

**踩过的坑**
- 一次复合命令里 `cd` 失败后，后续相对路径写到了错误位置，产生了根目录污染。
  教训：**一律用绝对路径**，写完用 `pwd` / `git status` 复核。

### 第 6 轮：三个任务提交物彼此独立（当前结构）

**Prompt 要点**
- 不再共用一个 `src/`，每个任务一个子目录：
  `taskN/{src, inputs, outputs, Makefile, README_taskN.md}`。
- 每个子目录的 Makefile 只编译自己那个任务，`make && make test` 能单独通过。
- 共享的 `input_util.h` 放到公共 include 目录，用 `-I` 引用。
- 跨任务的 `bench`/`invariants` 留在顶层 `tools/`，并说明它不属于任一任务。
- **每个任务都要提交名为 `test` 的测试程序**，自动读取本地 `inputs/` 并与 `outputs/` 比对。
- 要求：主要是整理结构，简单完成，不做多余工作。

**完成的工作**
- 目录重排：

  ```text
  MyLab/Linear/list/
  ├── Makefile              # 只构建跨任务工具
  ├── README.md             # 结构 + 复杂度小结
  ├── ai_record.md          # 本文件
  ├── include/input_util.h  # 三个任务共用
  ├── tools/                # 不属于任何单个任务
  │   ├── bench.cpp         # 需任务 1 + 任务 2 的类
  │   ├── invariants.cpp    # 三个任务的算法级检查
  │   ├── run_tests.py      # 通用比对器：--exe / --inputs / --outputs
  │   ├── run_invariants.py
  │   ├── oracle_task1/2/3.py
  │   └── fuzz_task1/2/3.py
  ├── task1/{Makefile, test, README_task1.md, src/, inputs/, outputs/}
  ├── task2/{...}
  └── task3/{...}
  ```

- 样例去掉了冗余前缀：`inputs/task1_case01.txt` → `task1/inputs/case01.txt`。
- 每个 `taskN/test`：Python 脚本，`chmod +x`，可单独执行，自动 `chdir` 到自身目录，
  默认跑 `build/taskN`，支持 `--exe/--inputs/--outputs` 覆盖。
- 顶层 `make test` 变成"依次进入三个任务目录跑各自的 `make test`，再跑跨任务不变量检查"。
- `tools/run_tests.py` 改成通用工具，供 `make sanitize` 在顶层复用。
- 三个 README 与顶层 README 的路径、构建命令全部同步更新。

**验证**
- `task1` 4/4 PASS、`task2` 4/4 PASS、`task3` 3/3 PASS（各自 `make && make test`）。
- 顶层 `make test`：11/11 PASS + 不变量 115 项全过，rc=0。
- `make clean` 之后逐目录重建仍全部 PASS；`make bench`、`make fuzz` 正常。

## 3. 关键设计决策（以后别推翻）

| 决策 | 位置 | 原因 |
| --- | --- | --- |
| `clear()` 只清元素不缩容 | 任务 2 | 题目明确要求，且已在注释写明；缩容走 `shrinkToFit()` |
| 空数组初始容量 `kMinCapacity = 4` | 任务 2 | 避免容量 0 与除零 |
| 相交按**指针相等**判定 | 任务 3 | 题目关键语义，值相等不算相交 |
| 节点统一走 `NodePool` 释放 | 任务 3 | 共享节点会二次释放、有环链表走不到头 |
| 驱动自动判断 `int`/`string` | 任务 1、2 | 样例混合类型；同时保留 `--int`/`--string` 显式开关 |
| 期望输出全部由独立 Python oracle 生成 | `tools/oracle_task*.py` | 避免用被测程序自证 |
| 比较时忽略 PERF 行 | `test` / `run_tests.py` | `time_ms` 每次不同 |

## 4. 验证手段

1. **独立 oracle**：`tools/oracle_task1/2/3.py` 是 Python 参考实现，
   与 C++ 实现无关。任务 2 的 oracle 独立模拟翻倍扩容与"CLEAR 不缩容"；
   任务 3 的 oracle 用"节点下标 + 后继下标"模拟指针，相交时真的共享同一批下标。
2. **随机差分测试**：`tools/fuzz_task1/2/3.py`，int/string 交替，覆盖越界、
   重复值、CLEAR 后再操作等。
3. **不变量检查**：`tools/invariants.cpp`，直接构造对象做断言（115 项），
   补样例覆盖不到的部分。
4. **ASan/UBSan**：`make sanitize`，无泄漏、无越界、无二次释放。

## 5. 复杂度小结

### 任务 1：单链表（n 为元素个数）

| 操作 | 时间 |
| --- | --- |
| `pushFront` / `pushBack` | O(1)（维护 `tail_`） |
| `insertAt` / `eraseAt` | O(n)（头尾走 O(1) 分支） |
| `access` / `find` / `contains` | O(n) |
| `size` / `empty` | O(1) |
| `clear` / 析构 | O(n) |
| 空间 | O(n) |

### 任务 2：动态数组（n 元素，C 容量）

| 操作 | 时间 |
| --- | --- |
| `pushBack` | 均摊 O(1)，单次最坏 O(n) |
| `insertAt` / `eraseAt` | O(n) |
| `get`/`set`/`at`/`operator[]`/`front`/`back` | O(1) |
| `size`/`capacity`/`empty` | O(1) |
| `clear` | O(n) 析构，不缩容 |
| `shrinkToFit` | O(n) |
| 空间 | O(C) |

实测（`n = 100000`，`ops = 200000`）：

```text
PERF name=vector_like n=100000 ops=200000 time_ms≈230
PERF name=std_vector  n=100000 ops=200000 time_ms≈230
PERF name=linked_list n=100000 ops=200000 time_ms≈30700
```

以下标随机访问/中间修改为主时数组比链表快约两个数量级。

### 任务 3：链表算法（n、m 为两链长度）

| 算法 | 时间 | 额外空间 |
| --- | --- | --- |
| `detectAndRemoveCycle` | O(n) | O(1) |
| `getIntersectionNode` | O(n + m) | O(1) |
| `reverseSharedPath` | O(n + m) | O(1) |

## 6. 下次继续的入口

- 本目录（`MyLab/Linear/list/`）任务 1–3 已完成且全部通过，一般不需要再动。
- 下一阶段预计是 `MyLab/Linear/Stack/`，建议沿用本次的结构：
  公共 `include/`、跨任务 `tools/`、每个任务一个自包含子目录带自己的 `test`。
- 若要新增样例：放进对应 `taskN/inputs/`，用对应 `tools/oracle_taskN.py` 生成同名
  `taskN/outputs/` 文件，然后 `make test` 会自动纳入。
- 提交前建议顺手跑一遍：`make test && cd ..`（顶层），以及 `make sanitize`。