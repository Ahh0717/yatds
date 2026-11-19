# 任务 1：顺序存储的字符串 `MyString`

## 1. 数据成员与扩容策略

```cpp
class MyString {
    char* data_;        // 以 '\0' 结尾的字符缓冲区（owns it）
    std::size_t len_;   // 有效字符数，不含结尾 '\0'
    std::size_t cap_;   // 缓冲区字节数，总是 >= len_ + 1
};
```

三个不变量贯穿全文：

1. `data_ != nullptr` 恒成立（默认构造也分配 1 字节），所以 `c_str()` 永远安全；
2. `cap_ >= 1` 且 `cap_ >= len_ + 1`，`data_[len_] == '\0'`；
3. 一块 `char[]` 从 `new[]` 到 `delete[]` 只有一次归属，拷贝一律深拷贝。

### 扩容：容量翻倍

`ensureCapacity(need)` 保证 `cap_ >= need`（need 含结尾 `'\0'`）：

```text
if (need <= cap_) return;           // 快路径：绝大多数 append 走这里
newCap = max(cap_, 1);
while (newCap < need) newCap *= 2;  // 反复翻倍直到够（溢出时退化为 need）
block = new char[newCap];
memcpy(block, data_, len_); block[len_] = 0;
delete[] data_; data_ = block; cap_ = newCap;
```

**为什么翻倍而不是“每次加固定量”**：把 N 次单字符 append 的总成本记下来看。

- 每次只加 k：容量依次为 k, 2k, 3k, …，第 i 次扩容搬移 `i*k` 字节，
  总计 Σ i·k ≈ N²/(2k)。N = 3.2×10⁶ 时是 10¹¹ 量级字节，根本跑不完。
- 翻倍：容量依次为 1, 2, 4, …, 2^t ≥ N。第 i 次扩容搬移 2^(i-1)，
  总计 < 2^t ≤ 2N。**总搬移量与 N 成正比**，摊到每次 append 是 O(1)。

也就是说翻倍策略下“某一次 append 可能很贵（要搬整个缓冲区），但平均下来每一次都很便宜”，
这就是**均摊 O(1)** 的由来；而每次加固定量的策略是均摊 O(N/k)，退化成 O(N) 每次。

**实测的扩容次数**：本任务基准里每次 append 16 字节、共 200000 次（总 3.2 MB），
倍增策略的容量序列为 32, 64, 128, …, 4194304，共 **18 次重新分配**，
累计搬移 4,194,000 字节，约等于最终长度的 **1.31 倍**——与上面“< 2N”的推导一致。

> 一个容易忽略的细节：翻倍意味着峰值内存最多约为最终长度的 2 倍，因为扩容瞬间
> 新旧两块同时存在。这是拿空间换时间的典型取舍。

### 拷贝语义

- 拷贝构造 / 拷贝赋值都按源对象的 `cap_` 分配新块并逐字节复制，**不共享缓冲区**；
- 赋值重载先申请新块、再释放旧块，且用 `if (this != &other)` 挡掉自赋值；
- `append(const MyString&)` 里检测 `this == &other`（自拼接）：扩容会释放
  `other` 指向的旧块，必须先深拷贝一份再拼；
- `replace(old, new)` 里 `new` 可能直接是本对象，同样要先固定其内容再扩容。
  （这两处我特意把保护去掉跑过 ASan，确实报 `heap-use-after-free`，说明不是过度防御。）

## 2. 各方法的语义与复杂度

| 方法 | 语义 | 复杂度 |
| --- | --- | --- |
| `length()` | 有效字符数 | O(1) |
| `empty()` | 是否为空串 | O(1) |
| `c_str()` | 内部缓冲区的只读视图，恒以 `'\0'` 结尾 | O(1) |
| `substr(pos, n)` | `pos >= len_` 抛 `out_of_range`；`pos+n` 超尾截到末尾 | O(k)，k 为结果长度 |
| `append(other)` | 末尾拼接，容量不足时翻倍扩容 | 均摊 O(`other.len_`)，最坏 O(n+m) |
| `operator+=(other)` | 等价于 `append`，返回 `*this` | 同上 |
| `find(sub)` | 首次出现下标，未找到返回 `npos`，空模式返回 0 | 最坏 O((n−m+1)·m) |
| `replace(old, new)` | 替换**首次**出现的 `old`；`old` 为空或不存在时原样返回 | O(n+m) |
| `compare(other)` | 字典序：小于 −1、相等 0、大于 1 | O(min(n,m)) |
| `clear()` | 重置为 `""`，**保留容量**不缩容 | O(1) |
| `reserve(n)` | 预分配至少 n+1 字节，已有容量更大时不缩容 | O(n) |

几处实现上的说明：

- `substr` 的越界判定在**访问内存之前**完成，因此空串上调用 `substr(0,0)` 也按
  约定抛异常，不会读到 `data_[0]` 之外的东西。
- `find` 是朴素逐位比较，最坏 O(nm)，与任务 2 的 KMP 形成对照；本任务的数据规模小，
  保持实现简单。`npos` 取 `static_cast<std::size_t>(-1)`，与 `std::string` 同款约定。
- `replace` 先 `find` 定位，再用 `memmove` 把尾部整体搬移（`memmove` 而非 `memcpy`，
  因为源和目标区间可能重叠），最后写入新内容。变长替换会走向 `ensureCapacity`。
- `clear()` 只改 `len_` 并把 `data_[0]` 置零，**不释放**缓冲区，所以“清空再重新拼”
  不会重新付出扩容代价——样例 `case06` / `case07` 覆盖了这条路径。
- `reserve(npos)` 会被 `n + 1` 回绕成 0 从而退化成空操作，所以显式抛 `std::length_error`。

## 3. 性能对比：MyString vs std::string vs reserve

### 3.1 基准设计

三种对象做**完全相同**的工作：从空串开始，连续 `append` 同一个 16 字符片段
200000 次，累计 3 200 000 个字符。

| 对象 | 是否有扩容 | 说明 |
| --- | --- | --- |
| `my_string` | 有 | `MyString` 靠翻倍扩容 |
| `std_string` | 有 | `std::string` 靠标准库自身策略（本机 libstdc++ 为 ×2） |
| `my_string_reserve` | 无 | 先 `reserve(3 200 000)`，再执行同样的 append 序列 |

**计时方法**：三类对象轮转着测量，第 1 轮只热身不计时，其后 6 轮各取最小值。
这一点很关键：首次触达几 MB 的新内存要付出缺页中断（page fault）的代价，
实测预热轮约有 1700+ 次 minor fault，而预热后只剩 65 次。
如果固定“先测 my_string”的顺序，my_string 就会把这份一次性成本全部吃下，
看起来比 `std::string` 慢 2 倍——**那测的是缺页，不是扩容策略**。

### 3.2 实测结果

```text
PERF my_string n=3200000 ops=200000 time_ms=1.15
PERF std_string n=3200000 ops=200000 time_ms=1.24
PERF my_string_reserve n=3200000 ops=200000 time_ms=0.70
```

（Intel Xeon 8358P，`-O2`，单核绑定；连续多次运行波动在 ±0.05 ms 内。）

### 3.3 差异从哪来

**（1）reserve 组快——因为它一次扩容都没有。**
扩容的代价不只是 `memcpy`。每次重新分配实际包含三笔开销：

1. 分配器要找到一块更大的空闲块（大块可能走 `mmap`，是系统调用）；
2. 旧内容整块搬移，搬移量随容量增长；
3. 新块第一次被写入时触发缺页中断，内核要把物理页映射进来。

倍增策略把前两笔压缩成“总共搬 1.31 倍最终长度”，但**无法消除第三笔**：
无论如何分块，累计分配的总空间仍是 O(N) 个新页。
`reserve(n)` 把这 18 次重新分配变成 1 次，更关键的是它让缓冲区**一次成型**，
后续 append 只往已映射的页里写——这正是 `my_string` 与 `my_string_reserve`
约 1.6 倍差距的主要来源（1.15 ms vs 0.70 ms）。

**（2）my_string 与 std_string 基本持平（1.15 vs 1.24），符合预期。**
两者都用倍增策略：本机 `std::string` 的容量序列是 30, 60, 120, …, 3932160，
同样是 18 次重新分配、搬移 3 932 145 字节，与我们的 4 194 000 字节在同一量级。
既然算法一致，耗时自然一致；剩下的差别是常数项：`std::string` 多了一层
`size()`/`capacity()` 维护、部分实现对短字符串走 SSO 分支，而我们的 `append`
是“判断容量 → memcpy → 补 '\0'”三步，指令更少。这也解释了为什么我们**略快**。

这说明：**在这类顺序拼接场景下，自定义实现的性能上限就是标准库，
真正的优化空间在“避免扩容”，而不是“把扩容写得更快”。**

**（3）为什么 `find`/`replace` 的复杂度不体现在这里。**
基准只测 append，因为任务要求比较的是“频繁拼接”+“扩容策略”。
`replace` 的 O(n+m) 里 m 是搬移量，扩容策略对它影响同样存在（变长替换可能触发扩容），
但它的主成本是 `memmove`，与这里讨论的分配次数无关。

### 3.4 结论

1. 翻倍扩容保证 N 次 append 的总搬移量为 O(N)，均摊每次 O(1)；
   代价是峰值内存可达 2 倍、且整个过程仍有 O(log N) 次重新分配。
2. 已知最终规模时，`reserve` 把 O(log N) 次分配降为 1 次，实测约提速 40%；
   代价是要么预知规模，要么接受可能的内存浪费。
3. `MyString` 与 `std::string` 走同一策略，性能同一量级；差异在常数项而非复杂度。

## 4. 构建与测试

```bash
cd task1
make            # 编译 build/task1
make test       # 跑 inputs/ 下全部样例，与 outputs/ 逐行比对（忽略 PERF 行）
make clean
```

7 个样例覆盖：空串初始（`EMPTY`）的打印/长度/查找/越界 `SUBSTR`、
`SUBSTR` 截断与 `len=0`、`FIND` 命中与未命中、`REPLACE` 首次替换与不存在、
`COMPARE` 三种结果、`CLEAR` 后再操作、多次 `APPEND` 触发扩容。

内存安全另用 `-fsanitize=address,undefined`（`detect_leaks=1`）把 7 个样例
和“连续 append 一百万次 + 变长 replace”压力测试各跑一遍，无报错、无泄漏。
注：泄漏检测只覆盖 C++ 堆，`std::string` 由标准库自行管理。