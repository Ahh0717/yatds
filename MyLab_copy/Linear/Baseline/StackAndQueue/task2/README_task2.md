# 任务 2：循环队列 / 平移式队列 / 链式队列

对应题目：`src/Linear/Baseline/StackAndQueue/README.md` 的 **Baseline 任务 2**。

题目重点要求设计**至少两种方法**解决数组实现中 `front`/`rear` 走到缓冲区
末尾的问题。本任务用三种队列覆盖：前两种是两种数组解法，第三种是链表解法。

## 文件结构

```text
Makefile                  # 只编译任务 2
test                      # 测试程序：读取 inputs/，与 outputs/ 逐行比对
README_task2.md           # 本文件
src/queue.hpp/.cpp        # dsaac::CircularQueue<T>：方法一，下标回绕
src/shift_queue.h/.cpp    # dsaac::ShiftQueue<T>：方法二，出队后整体左移
src/linked_queue.h/.cpp   # dsaac::LinkedQueue<T>：自建结点，尾插头删
src/main_task2.cpp        # 读 stdin 的驱动程序 + benchmark()
inputs/  outputs/         # 两种数组队列共用（容量语义相同）
capacity_inputs/  capacity_outputs/   # 容量 0/1/刚好装满等边界
linked_inputs/  linked_outputs/       # 链式队列专属（忽略 capacity）
```

头文件名 `queue.hpp` 与类名 `dsaac::CircularQueue<T>` 沿用题目给定的骨架。

## 编译与运行

```bash
make                      # 编译 build/task2
make test                 # circular + shift + capacity 边界 + linked 四组
./build/task2 < inputs/case01.txt          # 循环队列（默认）
./build/task2 --shift < inputs/case01.txt  # 平移式队列
./build/task2 --linked < linked_inputs/case01.txt
```

## 两种解决 front/rear 越界的方法

### 方法一：循环缓冲区（`CircularQueue`）

把 `capacity + 1` 个槽看成首尾相接的环。`front_` 指向队首元素，`back_`
指向下一个入队位置，两者只增不减：

```text
next(i) = (i + 1 == buffer_.size()) ? 0 : i + 1
push: buffer_[back_] = x; back_ = next(back_);
pop : front_ = next(front_);          // 一个元素都不搬
```

**用多出来的一个空槽区分空和满**，不额外维护 `count_`：

| 状态 | 判据 |
| --- | --- |
| 空 | `front_ == back_` |
| 满 | `next(back_) == front_` |

- `push` / `pop` / `front` 全部 **O(1)**，出队不搬任何元素。
- 代价：内部多一个 `T` 的空间（`capacity = N` 时缓冲区是 `N + 1` 个槽）。
- 对外语义：构造参数 `capacity` 就是**能放的元素个数**，所以
  `capacity = 6` 可以连续 push 6 个元素，第 7 个才抛 `overflow_error`。
  空槽只存在于内部，不占用对外可见的容量。

### 方法二：出队后整体左移（`ShiftQueue`）

不维护队首下标：`front` 恒为下标 0，元素永远紧凑地排在数组前部。

```text
push: buffer_[size_] = x; ++size_;
pop : move(begin()+1, begin()+size_, begin()); --size_;
```

- 不存在“队首/队尾下标越界”——位置每次都从 0 重新算起。
- `push` 是 **O(1)**，但 **`pop` 是 O(n)**：每次出队都要搬 `n - 1` 个元素。
- `front` 永远读下标 0，对缓存更友好；代价是出队成本随队列长度线性增长。
- 适合“出队很少、队列很短”的场景，是最省逻辑的实现。

### 方法三：链式队列（`LinkedQueue`）

链表没有下标，`head_` / `tail_` 是两个指针：出队只是
`head_ = head_->next`，不存在需要向前推进的位置变量，因此天然没有越界问题。

- `push` / `pop` / `front` 全部 **O(1)**，无容量上限。
- 代价：每元素多一个 `next` 指针，每次入队/出队一次 `new`/`delete`。

### 复杂度对比

| 操作 | CircularQueue | ShiftQueue | LinkedQueue |
| --- | --- | --- | --- |
| `push` | O(1) | O(1) | O(1) + 分配 |
| `pop` | **O(1)** | **O(n)** | O(1) + 释放 |
| `front` / `back` | O(1) | O(1) | O(1) |
| `size` / `empty` / `full` | O(1) | O(1) | O(1)（无 `full`） |
| 空间 | O(capacity)，一次分配 | O(capacity)，一次分配 | O(n)，逐结点分配 |

## 接口

| 接口 | Circular | Shift | Linked | 说明 |
| --- | --- | --- | --- | --- |
| `CircularQueue(capacity)` / `ShiftQueue(capacity)` | ✓ | ✓ | — | 构造即固定容量 |
| `empty()` / `full()` | ✓ | ✓ | ✓ / 无 | 链式队列没有“满” |
| `push(x)` | ✓ | ✓ | ✓ | 数组实现在满时抛 `std::overflow_error` |
| `pop()` / `front()` | ✓ | ✓ | ✓ | 空队列抛 `std::out_of_range` |
| `size()` | ✓ | ✓ | ✓ | |
| `printFrontToBack(os)` | ✓ | ✓ | ✓ | 队首到队尾、空格分隔，空队列 `EMPTY` |
| `back()` | ✓ | ✓ | ✓ | `std::queue::back` 的对应物 |
| `clear()` | ✓ | ✓ | ✓ | 容量不变 |
| `swap(other)` | ✓ | ✓ | ✓ | |
| `toVector()` | ✓ | ✓ | ✓ | 队首 → 队尾 |
| `operator== / !=` | ✓ | ✓ | ✓ | 逐元素比较 |
| `emplace(args...)` | ✓ | ✓ | ✓ | 就地构造后入队 |
| 拷贝/移动 | 默认即可 | 默认即可 | 手写深拷贝 + 移动 | 链式实现持有裸指针 |

扩展功能 9 个，含要求的 `size()` 与 `printFrontToBack()`。

## 实现要点

- **`CircularQueue` 的 `size` 是 O(1)**：额外维护 `count_`。判空判满虽然
  用空槽即可完成，但 `size()` 若靠遍历环去数就是 O(n)，与
  `std::queue` 的语义不符。`count_` 与两个下标在 push/pop/clear/swap
  里同步更新。
- **`ShiftQueue::pop` 用 `std::move` 搬移**，避免不必要的拷贝；搬完后
  被移走的尾部元素保持在“有效但未指定”的状态，不参与任何读取。
- **`LinkedQueue::pop` 删空时必须把 `tail_` 一起置空**，否则下次 `push`
  会写到已释放的结点。这是这类实现最常见的悬垂指针来源，已用不变量
  检查专门覆盖。
- **读写分离**：`pop()` 不返回值（与 `std::queue` 一致），驱动先 `front()`
  再 `pop()` 才能输出被删元素；空队列时两步都在 `try` 里，输出 `ERROR`。

## 性能对比（`benchmark()`）

功能输出结束后，用**完全相同**的初始数据与操作序列分别测三种队列：

```text
PERF circular_queue n=100000 ops=200000 time_ms=...
PERF linked_queue   n=100000 ops=200000 time_ms=...
PERF std_queue      n=100000 ops=200000 time_ms=...
```

序列固定种子生成：50% `PUSH`、30% `POP`、20% `FRONT`，保证不对空队列
操作。计时覆盖“压入初始元素 + 执行全部操作”。

> `ShiftQueue` 没有进入 PERF：它的 `pop` 是 O(n)，在 `n = 100000` 的规模上
> 要搬约 `200000 × 50000` 个元素，单这一项就要几分钟，和另外三者不在
> 一个量级。它作为“方法二”的实现与正确性验证留在样例与不变量检查里，
> 复杂度差异已经写在代码注释和上面的表里。

## 测试样例

| 样例 | 覆盖 |
| --- | --- |
| `inputs/case01.txt` | 题目原样例 |
| `inputs/case02.txt` | 空队列、连续 PUSH 到满、满队列 PUSH、弹空后再用 |
| `inputs/case03.txt` | 反复出队 + 入队，让队首/队尾下标越过缓冲区末尾 |
| `inputs/case04.txt` | `std::string` 元素、满队列边界 |
| `inputs/case05.txt` | 负数 / `INT_MAX`、扩展操作 `BACK` 与 `CLEAR` |
| `inputs/case06.txt` | 容量 2 反复装满再弹空，队首/队尾下标反复绕过缓冲区末尾；两种数组队列共用 |
| `capacity_inputs/case01-03` | 容量 1、容量 0、刚好装满 |
| `linked_inputs/case01-02` | 初始/后续元素超过 capacity，链式队列持续增长 |

期望输出全部由 `../tools/oracle_task2.py`（独立的 Python 参考实现，
用 `collections.deque`）生成。

## 其他验证

```bash
cd .. && make test        # task1 + task2 全部样例 + 不变量检查
make fuzz                 # 两个任务各 200 轮随机差分
make sanitize             # ASan/UBSan + 泄漏检查下重跑全部验证
```

随机测试轮流跑三种实现，并刻意使用 0–6 的小容量、大量 PUSH/POP 交替，
尽量让 `front_`/`back_` 跨过缓冲区末尾。

## 已知取舍

- `capacity = 0` 的数组队列视为“已满”，任何 PUSH 都会被丢弃。
- 数组实现在满队列 PUSH 时静默丢弃（题目约定 PUSH 不输出，且未规定
  满队列行为）；链式实现没有这个分支。
- `capacity_inputs/` 只给两种数组实现跑：它的期望输出依赖“满队列丢弃”
  这一数组语义，链式队列不会满，结果必然不同。