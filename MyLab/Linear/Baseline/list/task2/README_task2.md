# 任务 2：可变数组（DynamicArray）

对应题目：`src/Linear/Baseline/List/README.md` 的 **Baseline 任务 2**。
任务 1 的说明见 `../task1/README_task1.md`，本文件只讲任务 2 及其性能对比。

## 文件

本目录是任务 2 的独立提交物，只包含任务 2 的源文件：

```text
Makefile               # 只编译任务 2
test                   # 测试程序：读取 inputs/，与 outputs/ 逐行比对
README_task2.md        # 本文件
src/dynamic_array.h    # template<typename T> class DynamicArray
src/dynamic_array.cpp  # int / std::string 显式实例化（编译期自检）
src/main_task2.cpp     # 读 stdin 的驱动程序
inputs/caseMM.txt      # 测试输入
outputs/caseMM.txt     # 对应的期望输出
```

性能对比 `bench.cpp` 需要同时用到任务 1 与任务 2 的类，因此放在顶层
`../tools/bench.cpp`，不属于任何单个任务；独立参考实现
`../tools/oracle_task2.py` 与随机差分测试 `../tools/fuzz_task2.py` 同理。

## 编译与运行

```bash
make                    # 编译 build/task2
make test               # 运行 ./test：自动读取 inputs/ 与 outputs/ 比对
make clean              # 删除 build/
./build/task2 < inputs/case01.txt
```

性能对比在顶层目录：

```bash
cd .. && make bench     # 默认 n=100000, ops=200000
./build/bench --n=50000 --ops=100000
```

## 数据结构与容量策略

成员只有三个：

```cpp
T *data_;            // 由 ::operator new 申请的一段未初始化存储
std::size_t size_;
std::size_t cap_;
```

- 元素一律用 placement new 构造、显式调用析构函数销毁；缓冲区本身只走
  `::operator new` / `::operator delete`。两条路径严格分开，不会把析构当成
  `operator delete` 的替代品。
- 扩容时申请新块 → 逐个搬迁（`std::move_if_noexcept`）并析构旧元素 → 释放旧块，
  与 `std::vector` 的做法一致。
- 容量按 2 倍增长：`ensureCapacity` 从当前容量翻倍，直到装得下。
- **最小容量 `kMinCapacity = 4`**：默认构造的数组也会立刻分配 4 个位置，
  因此 `cap_` 在对象生命周期内恒不为 0，不会出现 `need / cap_` 之类的除零。

## CLEAR 的语义（题目要求写清的选择）

**`clear()` 只析构元素并把 `size_` 置 0，不释放 `data_`、也不改变 `cap_`。**

理由：清空后通常马上要重新填充，保留容量可以省掉一次重新分配；`std::vector` 的
`clear()` 同样不保证缩容（标准里只规定不减少 `size()`），此处的差别只是本类
*保证*容量一个字节都不动。确实需要把内存还回去时，显式调用 `shrinkToFit()`。

这条选择对样例的影响是可见的：`inputs/case02.txt` 中数组扩容到 8 之后
`CLEAR`，再读 `CAPACITY` 仍然是 8；`case03.txt` 里字符串数组也有同样的检查。

## 接口

题目要求：

| 接口 | 说明 |
| --- | --- |
| `pushBack(x)` | 尾部追加，必要时扩容 |
| `bool insertAt(i, x)` | 下标 `i` 处插入，`i > size` 返回 `false` |
| `bool eraseAt(i)` | 删除下标 `i`，`i >= size` 返回 `false` |
| `bool get(i, out)` | 读取下标 `i` 处元素 |
| `bool set(i, x)` | 修改下标 `i` 处元素 |
| `size()` / `capacity()` / `empty()` | 元素个数 / 容量 / 是否为空 |
| `clear()` | 清空元素，不缩容 |
| `print(os)` | 空格分隔，空数组输出 `EMPTY` |

额外实现（vector 式，共 9 个）：

| 接口 | 说明 |
| --- | --- |
| `reserve(n)` | 预留容量，只增不减，返回最终容量 |
| `resize(n)` | 调整元素个数（变大补默认值，变小截断），不缩容 |
| `at(i)` | 带边界检查，越界抛 `std::out_of_range` |
| `operator[](i)` | 不检查边界 |
| `front()` / `back()` | 首 / 尾元素引用 |
| `popBack(out)` | 弹出尾元素 |
| `emplaceBack(args...)` | 尾部就地构造 |
| `shrinkToFit()` | 显式缩容到 `max(size, kMinCapacity)` |

拷贝构造与拷贝赋值做深拷贝（先拷贝再交换，自赋值安全），并支持移动构造 / 移动赋值。

`pushBack` 与 `insertAt` 会先把传入值拷到局部变量再扩容，因此
`a.pushBack(a[0])`、`a.insertAt(0, a.at(3))` 这类自引用不会因为扩容而悬垂
（`test_link.cpp` 里有对应用例）。`emplaceBack` 就地构造，无法察觉这种自引用，
注释里已写明请改用前两个接口。

## 性能对比

`../tools/bench.cpp` 用同一份随机操作序列（种子固定）分别驱动三种结构。操作分布：
一半是 `PUSH_BACK`，四分之一是 `INSERT`，八分之一 `ERASE`，其余 `GET` / `SET`；
插入与删除的下标均匀分布，因此约一半的修改落在数组前半段（搬移成本最高）。

实测（本机，`-O2`，取稳态的一次运行，单位 ms）：

```text
PERF vector_like n=100000 ops=200000 time_ms=1559.37
PERF std_vector  n=100000 ops=200000 time_ms=1561.58
PERF linked_list n=100000 ops=200000 time_ms=28550.52
```

规模减半，耗时近似按比例下降（`linked_list` 因为每步都是 $O(n)$ 指针追逐，
常数更大）：

```text
PERF vector_like n=50000 ops=100000 time_ms=388.37
PERF std_vector  n=50000 ops=100000 time_ms=388.47
PERF linked_list n=50000 ops=100000 time_ms=5448.20
```

读法：

- **手写数组 ≈ `std::vector`**：差距在 1% 以内，说明扩容、搬移、就地构造这套
  实现没有明显低效之处，剩下的主要是 `insertAt` 的 `memmove` 与随机访存。
- **链表慢约 130 倍**：`INSERT` / `ERASE` 虽然不用搬移元素，但要先 $O(n)$ 遍历到
  下标；`GET` / `SET` 也是 $O(n)$。本序列里下标访问占多数，链表的缓存局部性
  又远差于数组，所以差距被进一步放大。
- 这个结果对应题目“分析同样可动态扩容的链表与可变数组在不同场景下的性能差异”：
  如果操作几乎都是尾插（或带尾指针的头插），链表的 $O(1)$ 优势才可能体现；
  一旦以下标随机访问或中间修改为主，数组的连续内存明显更优。

计时只覆盖“执行固定序列”这一段，数据生成与输出不计入；每个对象先预热一次再正式
计时。由于耗时每次不同，PERF 行不放进 `outputs/`。

## 测试样例

`outputs/case*.txt` 下的期望输出由 `../tools/oracle_task2.py` 生成，它独立实现了
“翻倍扩容 + CLEAR 不缩容”的容量模型，因此 `CAPACITY` 的期望值不是从被测程序
抄来的。

| 样例 | 覆盖 |
| --- | --- |
| `case01` | 题目给定样例（4 个元素装满 4 容量，`PUSH_BACK` 触发扩容到 8） |
| `case02` | 空数组：容量 0 起步、越界 GET/ERASE/SET、扩容、`CLEAR` 后容量不变 |
| `case03` | `std::string` 实例、头部插入、连续扩容到 8、清空后再次清空 |
| `case04` | 声明容量大于 0 但初始为空、负数、`INSERT` 恰好接到尾部 |

`../tools/fuzz_task2.py` 另做 300+ 轮随机差分测试，int 与 string 交替，覆盖越界、
空数组操作与容量变化。ASan/UBSan 下无报错、无泄漏。