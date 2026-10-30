# 任务 3：进阶链表算法（环 / 相交 / 局部反转）

对应题目：`src/Linear/Baseline/List/README.md` 的 **Baseline 任务 3：幽灵交叉路口**。
任务 1 见 [README_task1.md](README_task1.md)，任务 2 见 [README_task2.md](README_task2.md)。

## 文件

```text
src/list_advanced.h    # Node 与三个算法的声明（含复杂度注释）
src/list_advanced.cpp  # 三个算法的实现（每个函数上方注释给出思路与复杂度）
src/main_task3.cpp     # 场景驱动：读 T 个场景，按类型构造链表并输出
inputs/task3_*.txt      # 3 组样例
outputs/task3_*.txt     # 对应期望输出
tools/oracle_task3.py  # 独立参考实现
tools/fuzz_task3.py    # 随机场景差分测试
```

## 三个算法

| 函数 | 复杂度 | 做法 |
| --- | --- | --- |
| `detectAndRemoveCycle(head)` | O(n) 时间 / O(1) 空间 | 快慢指针相遇后，一个指针回到头部、另一个从相遇点出发，同速前进，相遇处即环入口；再沿环走一圈找到入口的前驱并断开 |
| `getIntersectionNode(a, b)` | O(n + m) 时间 / O(1) 空间 | 先各求长度，长链先走长度差，再同步前进，首次指针相等即交汇点 |
| `reverseSharedPath(a, b, x)` | O(n + m) 时间 / O(1) 空间 | 记录各自交汇点的前驱，就地反转公共段，再把两段前缀接到新公共头 |

三者都只移动指针，没有哈希表、数组或标记位，也不修改节点的值（值可能重复，
用值做标记不可靠）。

### 环入口的数学推导

设头到入口距离为 $a$，入口到相遇点距离为 $b$，环长为 $L$。相遇时慢指针走了
$a+b$，快指针走了 $a+b+kL$，而快指针速度是慢指针的两倍，所以
$a+b = kL$，即 $a = kL - b$。因此从相遇点继续向前走 $a$ 步、与从头节点走
$a$ 步，会同时落在入口节点上——这就是第二阶段同步前进的依据。

### 两个容易写错的点

1. **整链成环**：尾节点指回 `head` 时，入口就是头节点，“入口的前驱”并不是
   头节点之前的某个节点，而是环内的尾节点。实现里统一从入口沿环走一圈、
   找到 `next` 指回入口的那个节点，因此自环、整链成环、普通环都是同一条路径。
   如果直接写“从头走到入口前一个”，这种情况会漏判并导致死循环。
2. **公共段只反转一次**：两条链表共享公共节点，如果按“先反转 A 的公共段、
   再反转 B 的公共段”分别做，会把公共段转回去。实现里只在交汇点上反转一次，
   然后让两段前缀分别接上同一个新头。

## 输入输出

```text
T
CYCLE                n entry / 下一行 n 个值   （entry = -1 表示无环）
NO_INTERSECTION      nA + nA 个值；nB + nB 个值
INTERSECTION         nA + A 前缀；nB + B 前缀；k + 公共尾部
```

输出：

```text
CASE t
CYCLE:            CYCLE_ENTRY <值> 或 CYCLE_ENTRY NONE
                  LIST <解环后的链表>
NO_INTERSECTION:  INTERSECTION NONE
INTERSECTION:     INTERSECTION <值>
                  LIST_A <公共段反转后的 A>
                  LIST_B <公共段反转后的 B>
```

链表元素空格分隔，空链表输出 `EMPTY`。

## 内存管理

每个场景构造的所有节点都登记在 `NodePool` 里，场景处理结束后统一析构。
之所以不“沿 `next` 遍历再释放”，是因为：

- 相交场景里公共段被两条链表**共享**，沿任意一条链遍历都会重复访问同一批节点，
  逐个 `delete` 会二次释放；
- 解环之前链表本身可能带环，遍历根本走不到头。

只要“每个 `new` 出来的节点恰好登记一次”，由登记表负责释放，就既能覆盖全部
节点、又不会重复释放。ASan/UBSan 跑全部样例与 100 轮随机场景，确认无泄漏、
无 double free。

## 测试

三个任务已整合为统一的构建入口：

```bash
make          # 编译全部程序（build/task1|task2|task3|bench|invariants 与 ./test）
make test     # 遍历 inputs/ 按前缀分派，逐行比对 outputs/（忽略 PERF 行），
              # 再跑算法级不变量检查，最后打印 PASS/FAIL 汇总
make clean    # 清理全部构建产物
```

`outputs/task3_*.txt` 由 `tools/oracle_task3.py` 生成。该 oracle 用“节点下标 + 后继
下标”模拟指针，相交时两条链表引用**同一批下标**，因此“共享节点对象”在参考实现
里也是真实共享，而不是靠值相等蒙混。

| 样例 | 覆盖 |
| --- | --- |
| `case01` | 题目给定样例 |
| `case02` | 自环、整链成环、尾节点自指、无环单点、空链表；空↔空、全共享、值重复但不相交 |
| `case03` | 9 个随机场景，含零前缀相交、公共段长度 1、大量重复值 |

`src/invariants.cpp` 里另外直接构造链表，用**指针相等**断言算法返回的是同一个节点对象，
并验证“值相同但节点不同必须判为不相交”这一关键语义。