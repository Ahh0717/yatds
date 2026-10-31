# 任务 1：数组栈 + 链式栈

对应题目：`src/Linear/Baseline/StackAndQueue/README.md` 的 **Baseline 任务 1**。

## 文件结构

```text
Makefile                # 只编译任务 1
test                    # 测试程序：读取 inputs/，与 outputs/ 逐行比对
README_task1.md         # 本文件
src/stackar.hpp/.cpp    # dsaac::ArrayStack<T>：固定容量、std::vector 存储
src/stackli.hpp/.cpp    # dsaac::LinkedStack<T>：std::forward_list 存储，栈顶在头
src/main_task1.cpp      # 读 stdin 的驱动程序 + benchmark()
inputs/  outputs/       # 数组栈样例（capacity 是硬约束）
linked_inputs/          # 链式栈专属样例（忽略 capacity）
linked_outputs/
```

共用的输入解析头文件在 `../include/input_util.h`；独立参考实现
`../tools/oracle_task1.py` 与随机差分测试 `../tools/fuzz_task1.py` 属于跨任务工具。

头文件名沿用题目模板给定的 `stackar.hpp` / `stackli.hpp`，类名与命名空间
`dsaac::ArrayStack<T>` / `dsaac::LinkedStack<T>` 也保持不变。

## 编译与运行

```bash
make                  # 编译 build/task1
make test             # 数组栈样例 + 链式栈样例
./build/task1 < inputs/case01.txt              # 默认数组栈
./build/task1 --linked < linked_inputs/case01.txt
```

`test` 支持 `--arg=--linked`，用同一套比对逻辑跑链式栈样例：

```bash
./test --inputs linked_inputs --outputs linked_outputs --arg=--linked
```

## 接口

| 接口 | 数组栈 | 链式栈 | 说明 |
| --- | --- | --- | --- |
| `ArrayStack(capacity)` | ✓ | — | 构造即固定容量 |
| `empty()` / `full()` | ✓ | ✓ / 无 | 链式栈没有“满”，不提供 `full()` |
| `push(x)` | ✓ | ✓ | 数组栈满时抛 `std::overflow_error` |
| `pop()` / `top()` | ✓ | ✓ | 空栈抛 `std::out_of_range` |
| `size()` | ✓ | ✓ | 链式栈单独维护 `size_`，恒 O(1) |
| `printTopToBottom(os)` | ✓ | ✓ | 栈顶到栈底、空格分隔，空栈 `EMPTY` |
| `clear()` | ✓ | ✓ | 数组栈只清元素、不改变容量 |
| `setTop(x)` | ✓ | ✓ | 显式改栈顶（不能靠满栈 push 更新） |
| `swap(other)` | ✓ | ✓ | 数组栈连容量一起交换 |
| `toVector()` | ✓ | ✓ | 按栈底 → 栈顶导出 |
| `assign(other)` | ✓ | ✓ | 按另一座栈的形状覆盖自己 |
| `operator== / !=` | ✓ | ✓ | 只看元素，不看容量 |
| `emplace(args...)` | ✓ | ✓ | 就地构造后走 `push` 的容量检查 |

扩展功能共 9 个，满足“至少 5 个、必须含 `size` 与 `printTopToBottom`”的要求。

## 实现要点

**数组栈**：题目允许内部用 `std::vector<T>`，构造时 `reserve(capacity)`
一次拿到全部存储，此后 `push` 只是尾插、`pop` 只是尾删，不会触发扩容，
所以 `capacity_` 一经设定就是固定上限。`full()` 用
`values_.size() >= capacity_` 判断。

**链式栈**：`std::forward_list` 的 `begin()` 即栈顶，`push` →
`push_front`、`pop` → `pop_front`、`top` → `front`，全部 O(1)。
`forward_list::size()` 不保证 O(1)，因此单独维护 `size_`，在
`push`/`pop`/`clear`/`swap` 上同步更新。`toVector()` 需要逆序，代价 O(n)。

**异常语义**：`pop()` 与 `top()` 在空栈时都抛 `std::out_of_range`；
满栈 `push` 抛 `std::overflow_error`。驱动捕获后按题目约定输出
`ERROR`（POP/TOP）或静默忽略（满栈 PUSH）。

**并行维护的两种栈**：功能逻辑在驱动里只写一遍（`processOps<T, Stack>`），
两种栈共用同一套输入输出处理，避免两边语义漂移。

## 性能对比（`benchmark()`）

功能输出结束后，用**完全相同**的初始数据与操作序列分别测三种栈，
各打印一行：

```text
PERF name=array_stack n=100000 ops=200000 time_ms=...
PERF name=linked_stack n=100000 ops=200000 time_ms=...
PERF name=std_stack    n=100000 ops=200000 time_ms=...
```

操作序列固定种子生成：50% `PUSH`、30% `POP`、20% `TOP`，且保证不对空栈
`POP`/`TOP`。计时覆盖“压入初始元素 + 执行全部操作”，不含对象构造与
数据生成。`PERF` 行不进 `outputs/`，`test` 比对时忽略以 `PERF ` 开头的行。

数组栈的容量按 `n + ops` 预留，保证 benchmark 中不会触发满栈。

## 测试样例

| 样例 | 覆盖 |
| --- | --- |
| `inputs/case01.txt` | 题目原样例 |
| `inputs/case02.txt` | 容量 1：push 即满、满栈 push 静默丢弃、弹空后 ERROR |
| `inputs/case03.txt` | 初始为空（n = 0 空行）、空栈 SIZE/EMPTY/PRINT/POP/TOP |
| `inputs/case04.txt` | 连续 push 到满、满栈 push、弹到空栈后 TOP |
| `inputs/case05.txt` | `std::string` 元素 |
| `inputs/case06.txt` | 负数与 `INT_MAX` |
| `inputs/case07.txt` | 扩展操作 `SETTOP` / `CLEAR`，CLEAR 后复用 |
| `inputs/case08.txt` | 满栈边界、部分弹出后的 PRINT |
| `inputs/case09.txt` | 容量 0：任何 PUSH 都被丢弃，SIZE/TOP/POP/PRINT 全部是空栈行为 |
| `linked_inputs/case01-03` | 初始元素超过 capacity、链式栈持续增长、string + CLEAR |

期望输出全部由 `../tools/oracle_task1.py`（独立的 Python 参考实现）生成，
数组栈样例用 `array` 语义、链式栈样例用 `linked` 语义分别重算。

## 其他验证

```bash
cd .. && make test        # 两套样例 + 不变量检查
make fuzz                 # 200 轮随机差分（int/string × array/linked）
make sanitize             # ASan/UBSan + 泄漏检查下重跑样例、不变量、模糊测试
```

- `../tools/invariants.cpp` 直接构造对象断言异常类型、容量 0/1、满栈、
  clear 不缩容、swap 交换容量、assign 失败不破坏原栈等样例覆盖不到的语义。
- 随机测试里链式栈那几轮会继续在“容量”之外 push，验证它不会满。

## 已知取舍

- 链式栈不提供 `full()`：链式结构没有容量上限，强行返回常量 `false`
  容易误导调用方。
- `ArrayStack` 需要显式容量，没有默认构造；驱动必须读到 `capacity`。
- 数组栈样例要求 `n <= capacity`，否则驱动向 stderr 报错并以非零码退出
  （无法装入就不能构造出合法初始状态）。