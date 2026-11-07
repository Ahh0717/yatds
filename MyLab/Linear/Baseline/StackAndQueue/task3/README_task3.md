# 任务 3：栈与队列的相互实现和转换

对应题目：`src/Linear/Baseline/StackAndQueue/README.md` 的 **Baseline 任务 3**。

任务要求用队列实现栈、用栈实现队列，并在一对 `std::vector` 实现的
栈/队列之间做相互转换。

## 文件结构

```text
Makefile                  # 只编译任务 3
test                      # 测试程序：读取 inputs/，与 outputs/ 逐行比对
README_task3.md           # 本文件
src/stack_by_queue.h/.cpp   # dsaac::StackByQueue<T>：单个队列 + 轮转
src/queue_by_stack.h/.cpp   # dsaac::QueueByStack<T>：两个栈 in_/out_
src/vector_stack.h/.cpp     # dsaac::VectorStack<T>：std::vector 栈 + toQueue()
src/vector_queue.h/.cpp     # dsaac::VectorQueue<T>：std::vector 队列 + toStack()
src/main_task3.cpp          # 读 stdin 的驱动程序 + benchmark()
inputs/  outputs/           # int 样例（含题目原样例）
string_inputs/  string_outputs/   # std::string 样例
```

## 编译与运行

```bash
make                      # 编译 build/task3
make test                 # int 6 个 + string 2 个样例
./build/task3 < inputs/case01.txt          # 自动判断元素类型
./build/task3 --string < string_inputs/case01.txt
```

## 输入输出协议

```text
第 1 行   T                          场景个数
之后每个场景以类型行开头：
  STACK_BY_QUEUE  n / 栈底→栈顶 n 个元素 / m / m 行栈操作
  QUEUE_BY_STACK  n / 队首→队尾 n 个元素 / m / m 行队列操作
  CONVERT         n / n 个元素 / STACK_TO_QUEUE 或 QUEUE_TO_STACK
```

栈操作 `PUSH x / POP / TOP / SIZE / EMPTY / PRINT`；队列操作把 `TOP` 换成
`FRONT`。输出每个场景先打印 `CASE t`；空结构访问输出 `ERROR`，空结构
`PRINT` 输出 `EMPTY`；`PUSH` 不输出。

## 四个类的设计

### `StackByQueue<T>`：一个队列，轮转出栈顶

内部只有一个 `std::queue`，约定 **队首是栈底、队尾是栈顶**。

```text
push(x):  data_.push(x);                       // 直接入队，O(1)

pop():    前 n-1 个元素依次 “出队再入队”  → 原队尾（栈顶）来到队首
          队首出队丢掉。

top():    同样转 n-1 次取到栈顶值，再额外转 1 次把顺序转回去。
```

`top()` 的第二次轮转很关键：转了 `n-1` 次再转 `1` 次，总共 `n` 次轮转，
恰好是队列的恒等变换，所以 `TOP` 不会改变栈的内容。第一版实现转了两遍
`n-1` 次，结果把元素顺序打乱了（`2 4 1` 而不是 `3 2 1`），被样例抓出来。

**复杂度**：`push` O(1)；`pop`/`top` **单次 O(n)**，连续 k 次操作是
k·O(n)。这里没有把代价摊到 push 上的技巧（那是“两个队列、push 时搬家”
的做法），所以“均摊”只是题目的叫法，实际每次都要真转一圈。

`top()` 按值返回而不是返回引用：轮转过程中队首元素会被弹出，
`std::queue`（默认 `std::deque`）的 `pop_front` 会让指向它的引用失效。

### `QueueByStack<T>`：两个栈，倒一次用很久

`in_` 负责入队，`out_` 负责出队。`front()`/`pop()` 时如果 `out_` 为空，
就把 `in_` 全部弹出并压进 `out_`——两次逆序抵消，`out_` 的栈顶正好是
最早入队的元素。

```text
push(x):        in_.push(x);                    // O(1)
front()/pop():  if (out_.empty()) pour();       // 只在 out_ 空时倒
                取/弹 out_.top();
```

**复杂度**：每个元素一生只经历“压入 in_ → 倒入 out_ 时弹出+压入 →
从 out_ 弹出”共 O(1) 次栈操作，因此 m 次混合操作总代价 O(m)，
**单次 `front`/`pop` 均摊 O(1)**，最坏情况（恰好触发一次倒入）是 O(n)。
空间 O(n)，元素只存在两个栈中的一个里。

### `VectorStack<T>` / `VectorQueue<T>`：vector 实现 + 相互转换

`VectorStack` 栈顶在 vector 尾部，`push/pop/top` 都 O(1)。
`VectorQueue` 用队首下标 `front_` 避免出队搬移：`pop` 只前移下标（O(1)），
有效元素不到一半时整体压缩回下标 0，摊还到 `push` 上是 O(1)。

转换语义（**两个方向的结果都是把输入序列反转**）：

| 方向 | 输入理解 | 转换动作 | 输出 | 结果 |
| --- | --- | --- | --- | --- |
| `QUEUE_TO_STACK` | 队首→队尾 | `toStack()` 按队首→队尾压栈 | 栈顶→栈底 | 输入逆序 |
| `STACK_TO_QUEUE` | 栈底→栈顶 | `toQueue()` 按栈顶→栈底入队 | 队首→队尾 | 输入逆序 |

`toStack()` 把队列原样搬进栈（栈底=队首），打印方向自然反转；
`toQueue()` 必须从栈顶（vector 尾部）往回遍历入队，队列本身也变成逆序。
这是 `VectorStack` 的实现在 `vector_queue.cpp` 里而不是头文件里的原因：
`toQueue()` 的返回类型 `VectorQueue<T>` 在定义处必须是完整类型。
`VectorStack` 的显式实例化也放在 `vector_stack.cpp`（同一个特化只能定义一次）。

## 接口

| 接口 | StackByQueue | QueueByStack | VectorStack | VectorQueue |
| --- | --- | --- | --- | --- |
| `empty()` / `size()` | ✓ | ✓ | ✓ | ✓ |
| `push(x)` | ✓ | ✓ | ✓ | ✓ |
| `pop()` | ✓ | ✓ | ✓ | ✓ |
| `top()` / `front()` | ✓ | ✓ | ✓ | ✓ |
| `printTopToBottom(os)` / `printFrontToBack(os)` | ✓ | ✓ | ✓ | ✓ |
| `clear()` | ✓ | ✓ | ✓ | ✓ |
| `swap(other)` | ✓ | ✓ | ✓ | ✓ |
| `toVector()` | ✓ | ✓ | ✓ | ✓ |
| `operator== / !=` | ✓ | ✓ | ✓ | ✓ |
| `emplace(args...)` | ✓ | ✓ | ✓ | ✓ |
| `toQueue()` / `toStack()` | — | — | ✓ | ✓ |
| `back()` | — | — | — | ✓ |

每个类至少 5 个扩展功能，均含题目要求的 `size()` 与打印函数。
空结构上的 `pop`/`top`/`front` 一律抛 `std::out_of_range`。

## 性能对比（`benchmark()`）

四种实现跑**完全相同**的初始数据与操作序列（50% PUSH / 30% POP / 20% TOP，
队列一侧把 TOP 换成 FRONT）：

```text
PERF stack_by_queue n=2000 ops=2000 time_ms=...
PERF queue_by_stack n=2000 ops=2000 time_ms=...
PERF vector_based   n=2000 ops=2000 time_ms=...   (VectorStack)
PERF vector_queue   n=2000 ops=2000 time_ms=...   (VectorQueue)
```

本机实测（多次运行取中位数）：

```text
PERF stack_by_queue n=2000 ops=2000 time_ms≈13–16   (8.1–26.8)
PERF queue_by_stack n=2000 ops=2000 time_ms≈0.03
PERF vector_based   n=2000 ops=2000 time_ms≈0.03
PERF vector_queue   n=2000 ops=2000 time_ms≈0.03
```

差距约有 500 倍，来源是渐进复杂度而不是常数因子：

- `stack_by_queue` 每次 `pop`/`top` 都要轮转整个队列，n=2000 时一次操作
  2000 次出队+入队，2000 次操作约 10^7 次队列操作，加上 deque 的
  指针间接寻址，落在十几毫秒。
- `queue_by_stack` 每个元素最多被倒一次，均摊 O(1)，全程约 4000 次栈操作。
- `vector_based` / `vector_queue` 是 vector 尾部增删 + 队首下标前移，
  最直接，与 `queue_by_stack` 同量级。

题目要求分别对比“栈实现队列 vs `<vector>` 实现队列”和“队列实现栈 vs
`<vector>` 实现栈”，所以队列一侧也单独补了 `vector_queue` 一行：
`queue_by_stack` 与 `vector_queue` 几乎相同，说明两栈倒换的均摊 O(1)
在常量上也没有明显劣势；而 `stack_by_queue` 因为单次操作 O(n)，
比两者慢了三个数量级。

> **规模为什么从 100000 降到 2000**：`StackByQueue` 的 `pop`/`top` 是 O(n)，
> 按题目的 `n=100000 / ops=200000` 就是约 10^10 次队列操作，跑不完。
> 这里让三者跑同一序列、同一规模，只比较相对代价；PERF 行的 `n`/`ops`
> 如实打印当前实际规模（2000/2000）。

## 测试样例

| 样例 | 覆盖 |
| --- | --- |
| `inputs/case01.txt` | 题目原样例（三种场景各一个） |
| `inputs/case02.txt` | 三个场景全部初始为空：空 TOP/POP/PRINT、空 FRONT/POP |
| `inputs/case03.txt` | 栈反复 PUSH/POP 后 PRINT，验证 `top()` 不破坏顺序；负数 |
| `inputs/case04.txt` | 队列在 out_ 非空时继续 push、倒空后再 push 的交错路径 |
| `inputs/case05.txt` | `INT_MAX` / 0 / 负数、队列弹空后的打印 |
| `inputs/case06.txt` | 单元素边界、连续 PUSH/POP 交替（`in_`/`out_` 来回倒）、单元素 CONVERT 两个方向 |
| `string_inputs/case01-02` | `std::string` 元素、空 CONVERT |

期望输出全部由 `../tools/oracle_task3.py`（独立的 Python 参考实现，
用 list/deque 模拟）生成。

## 其他验证

```bash
cd .. && make test        # task1/2/3 全部样例 + 不变量检查
make fuzz                 # 三个任务各 200 轮随机差分
make sanitize             # ASan/UBSan + 泄漏检查下重跑全部验证
```

不变量检查（`tools/invariants.cpp`）覆盖：空结构异常、`top()` 重复调用
不改变内容、`StackByQueue`/`QueueByStack` 与 `std::stack`/`std::queue`
的随机差分、两种转换方向的反转语义、两次转换的净效果是一次反转、
`VectorQueue` 大量 `pop` 后的压缩路径。