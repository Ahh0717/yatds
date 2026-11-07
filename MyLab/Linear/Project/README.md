# 祖玛（Zuma）—— 双向链表 · 循环队列 · 撤销栈

> 大一小学期「线性表综合应用」项目。在简化规则下实现祖玛的核心玩法：
> 从待发射队列取球插入轨道，连续 3 个及以上同色球自动消除，消除后
> 左右合拢可能继续**连锁消除**；同时提供炸弹、改色两个小道具与完整撤销。

---

## 1. 设计思路

整体思路是「把游戏状态拆成三个独立的数据结构，再用一个 Game 层组合起来」：

| 层 | 组件 | 职责 |
| --- | --- | --- |
| 数据结构层 | `Track` | 双向链表 + 首尾哨兵，表示轨道 |
| | `BallQueue` | 环形缓冲区（循环队列），表示待发射球 |
| | `UndoStack` | 栈，保存历史快照，支持撤销 |
| 组合层 | `Game` | 持有三者，实现 fire / bomb / paint / undo 与胜负判定 |
| 输入层 | `main.cpp` | `--script` 批处理模式；无参数进入交互模式 |
| | `interactive.cpp` | 命令行界面：预设关卡、局面显示、提示与播报 |

设计时给自己定的几条硬约束：

1. **不用 `std::list` / `std::queue` / `std::stack`**，三个核心结构全部手写，
   节点与缓冲区手动 `new/delete`，以体现对底层实现的掌握。
2. **调度与显示分离**：`Game` 只产生确定性的结果字符串，交互模式的所有
   提示语都放在 `interactive.cpp`。这样 `--script` 的输出可以被测试脚本
   逐行精确比对，不会因为界面文案变化而破坏样例。
3. **状态改变统一先快照**：`FIRE / BOMB / PAINT` 在真正修改前先把整份状态
   压入撤销栈；非法操作在校验阶段直接返回 `false`，既不压栈也不改状态。
4. **输入按行解析**：一条命令格式错误只影响它自己，不会连带吞掉下一行，
   避免 `std::cin >>` 失败后 `ignore` 到行尾造成后续输出整体错位。

游戏规则（本项目的简化版）：
- 轨道上的球静止不动；玩家把队首球插入轨道任意下标 `pos`（`0 ≤ pos ≤ 长度`）。
- 插入/炸掉一颗球后，若出现长度 ≥ 3 的同色连续段，整段消除；消除后左右
  两段合拢，若再次形成 ≥ 3 同色段则继续消除（连锁），直到稳定。
- 轨道被清空 → **WIN**；`FIRE` 后队列已空且轨道非空 → **LOSE**。
- 小道具：`BOMB pos` 炸掉一颗球；`PAINT c` 把队首球改成颜色 `c`（不消耗球）。
- 任何状态改变都可以 `UNDO` 撤销。

---

## 2. 核心功能原理

### 2.1 轨道 `Track`：双向链表 + 首尾哨兵

**为什么选双向链表。**

消除算法需要以某颗球为起点，向**左**和向**右**两个方向同时扫描同色段；
删掉一段之后又要把它左右两边的球重新接上。单链表达不到「向左走」的要求
（除非再维护前驱，那本质就是双向链表）；而数组在中间删除需要整体搬移，
一次消除的代价是 `O(n)`，连锁消除还会反复发生。双向链表在**已经持有节点
指针**时，删除本身是 `O(1)`，左右两侧都能直接访问，天然契合这个场景。

**哨兵的作用。** 头尾各放一个颜色为 `'\0'` 的哨兵节点，链表永远非空。
这样 `insertAt` 在 `pos == 0`、`pos == size()` 时不必特判头尾；
`erase` 也只需改 `prev->next` / `next->prev` 两个指针。
哨兵不是合法彩球色，`Game` 层用 `p->color != '\0'` 就能判断「左右是否
还有球」，避免把哨兵当成彩球参与比较。

**`insertAt(pos, color)` 的步骤。**

```text
1. 校验 0 <= pos <= size，否则抛 out_of_range（链表不变）。
2. 找到「插入位置之后」的节点 next：
     pos == size 时直接取尾哨兵（追加，无需遍历）；
     否则 next = nodeAt(pos)。
3. prev = next->prev。
4. new 一个新节点 node{color, prev, next}，接进 prev 与 next 之间，size++。
```

定位是 `O(min(pos, n-pos))`（`nodeAt` 从较近的一端走），**链接本身是 `O(1)`**；
追加到尾部是纯 `O(1)`。

**`erase(p)` 的步骤。** 校验 `p` 非空且不是哨兵后，只做两件事：

```text
p->prev->next = p->next;
p->next->prev = p->prev;
delete p; --size;
```

给定指针是严格 `O(1)`。注意调用方若之后还要访问 `p` 的邻居，必须在
`erase` 之前先把它们存下来——`erase` 之后 `p` 已失效。消除算法与 `bomb`
都遵守了这一点。

**拷贝语义。** 快照、赋值都走深拷贝：逐个节点复制（`Track(const Track&)`
委托构造出空链表后按序追加），赋值用 copy-and-swap，异常安全。

### 2.2 自动消除与连锁消除 `Game::eliminateFrom(Node* p)`

这是整个项目的核心算法。入口是插入/删除后「最可能受影响」的节点 `p`，
循环执行三步：

```text
while (p 是真实彩球) {
  1) 以 p 为中心，向左扩张到 first、向右扩张到 last，
     得到包含 p 的极大同色段 [first, last]，长度为 length；
  2) 若 length < 3 —— 不会消除，直接结束；
  3) 否则：
     a. 先记下删除后将成为左右邻居的 left_ball / right_ball
        （哨兵记为 nullptr）；
     b. 从 first 删到 last（逐节点 erase）；
     c. 若 left_ball 与 right_ball 都存在且同色，
        令 p = left_ball 继续下一轮（连锁）；否则结束。
}
```

**为什么第 3c 步只判断左右两个邻居。** 归纳来看，每次操作前轨道都是
「稳定」的（不存在 ≥ 3 的同色段）。删掉一段后，唯一可能新形成的同色段
必然跨越被删区间的两侧，也就一定由 `left_ball` 与 `right_ball` 构成；
它们不同色时，不可能在别处凭空冒出新的三连。`bomb` 也用同一条性质：
删除一颗球只在它的左右邻居同色时才可能产生新段。

**复杂度。** 设轨道长度为 `n`，一次 `eliminateFrom` 最多把每颗球删除一次，
找段与删除都是线性的，因此**单次 FIRE/BOMB 触发的全部连锁消除总计 `O(n)`**；
额外空间 `O(1)`（只用若干指针）。连锁的轮数不超过被删球数，已包含在上述
界内。

**一个容易写错的点。** 删除循环里不能「先 `erase(q)` 再比较 `q == last`」，
因为 `q` 已经失效；必须先保存 `next`、先判断是否到达段尾，再删除。

### 2.3 待发射队列 `BallQueue`：环形缓冲区

**为什么用循环队列。** 游戏只用到两种操作：队首出一颗、队尾进一颗。
普通数组队列出队后需要整体前移（`O(n)`）；循环队列让 `front/pop/push`
都是 `O(1)`，并且**复用同一块缓冲区**，反复发射不会产生新的分配。

**环形缓冲与空/满判定。** 底层分配 `cap_ + 1` 个槽，多留一格专门用来
区分空与满，不额外占一个标记位：

```text
front_ == back_                  → 空
nextIndex(back_) == front_        → 满
nextIndex(i) = (i + 1 == cap_ + 1) ? 0 : i + 1
```

因此实际最多存 `cap_` 个元素，多出的那一格永远空着。

- `push`：先算 `next = nextIndex(back_)`，若 `next == front_` 返回 `false`
  （队列不变），否则写入 `buf_[back_]`、`back_ = next`、`size_++`。
- `pop`：空队抛 `out_of_range`，否则取 `buf_[front_]` 并让 `front_` 回绕。
- `setFront`：`PAINT` 用，改队首颜色但不移动指针。
- `clear`：只把下标与计数归零，保留缓冲区与容量，随后可继续使用。

所有操作都是 `O(1)`；`toVector()` 按队首→队尾绕一圈，是 `O(队列长度)`。

### 2.4 撤销栈 `UndoStack`（快照法）

**做法。** 每次 `FIRE / BOMB / PAINT` 真正改状态之前，把这一刻的完整状态
打成一份 `GameState` 压栈：

```cpp
struct GameState {
    std::vector<char> track;   // 轨道头 → 尾
    std::vector<char> queue;   // 队列队首 → 队尾
    bool won, lost;            // 该时刻的胜负标志
    std::size_t shots;         // 该时刻的累计发射数
};
```

`UNDO` 时弹出栈顶、用它覆盖当前状态。栈的 LIFO 语义天然对应「撤销上一步」，
连续 `UNDO` 就能一路回退到开局。

**为什么把胜负标志也放进快照。** 如果只恢复轨道和队列，「FIRE 判负后
UNDO」会留下残留的 `lost_` 标志，游戏会被错误地判定为已结束。把 `won/lost`
与 `shots` 一起快照，撤销后的显示与计数才完全一致。

**取舍。** 快照法的优点是**实现简单、正确性显然**——不依赖每种操作自己写
逆操作，也就不会漏掉某个字段；缺点是每步要复制整份状态（时间与空间都是
`O(n + q)`），并且历史全部保留，总内存为 `O(步数 × 状态大小)`。对本项目
（轨道规模小、步数有限）完全可接受；若轨道很大，可换成「操作日志 + 逆操作」
或「持久化数据结构」来降低开销，代价是每种操作都要单独实现并验证逆操作。

**实现细节。** `UndoStack` 底层是 `std::vector<GameState>`，但对外只暴露
栈接口（`push / pop / top / empty / size / clear`），不允许随机访问，从接口上
保证「只能操作栈顶」；不使用 `std::stack`。

### 2.5 小道具

- **`BOMB pos`（炸弹）**：删除轨道上指定下标的一颗球，然后处理由此产生的
  消除与连锁。因为炸弹**不消耗待发射队列**，它不会被用来判定失败；它只在
  真正清空轨道时宣告胜利，并清除先前残留的失败标志。炸弹同样会压栈，
  可以撤销。
- **`PAINT c`（改色）**：把队首球改成 `c`（`A–Z`），**不消耗球**，可以
  反复调整首发颜色以凑出三连。空队列或非法颜色返回失败，不改状态。
  改色同样可撤销。

这两个道具的设计不只是「加功能」，也各有针对性：`BOMB` 用于打破僵局
（例如挖掉中间一颗让左右同色合拢），`PAINT` 用于在没有合适首发球时
自救，都直接复用了已有的链表与队列接口。

### 2.6 胜负判定

- `FIRE` 成功后：轨道为空 → `won`；否则若队列为空 → `lost`。
- `BOMB` 成功后：轨道为空 → `won`（并清除 `lost`）。
- `UNDO`：恢复快照中的 `won/lost/shots`。
- `PAINT` 不影响胜负（只是改颜色）。

---

## 3. 数据结构选型与优点

| 结构 | 选型 | 优点 | 不用什么 |
| --- | --- | --- | --- |
| 轨道 | 手写**双向链表 + 首尾哨兵** | 左右双向扫描同色段；持有指针时删除 `O(1)`；中间插入不搬移；哨兵消除头尾特判 | 不用 `std::list`，也不把轨道存成数组 |
| 待发射球 | 手写**循环队列（环形缓冲区，`cap+1` 槽）** | `push/pop` 均摊 `O(1)`，缓冲区复用；「多留一格」免标记位 | 不用 `std::queue`；不用会整体前移的普通数组队列 |
| 撤销 | **栈 + 整份快照** | LIFO 天然对应「撤销上一步」；正确性显然，不会漏字段 | 不用 `std::stack`；不用操作日志+逆操作 |
| 快照内容 | `std::vector<char>` + 标志位 | 紧凑、易比较、易打印；与 `toVector()` 直接对接 | — |

三者分工明确、互不耦合：`Track` 只管链表，`BallQueue` 只管队列，`UndoStack`
只管历史；`Game` 只负责编排与胜负。任何一层都可以单独编译、单独测试。

---

## 4. 运行说明

### 4.1 编译

在 `MyLab/Linear/Project/` 目录下：

```bash
make                 # Linux / macOS
mingw32-make         # Windows（MinGW）
```

产物为 `build/zuma`（Windows 下自动变成 `build/zuma.exe`）。编译参数为
`-std=c++17 -Wall -Wextra -Wpedantic -O2 -g`，零警告。

**Windows 下额外加 `-static`（由 Makefile 自动处理）。** MinGW 默认动态链接
`libstdc++-6.dll` / `libgcc_s_seh-1.dll` / `libwinpthread-1.dll`，这三个 DLL 不会
出现在 `build/` 里，Windows 会按「exe 目录 → 系统目录 → PATH」的顺序查找。只要
PATH 上有另一套 MinGW（最典型的是 conda 自带的 `Library\mingw-w64\bin`，装了
R、`m2w64-gcc-libs` 等包就会有），就会加载到 ABI 不兼容的同名 DLL：程序能启动，
但在第一次 `std::cin` / `std::string` 操作时访问冲突退出，退出码 `3221225477`
（`0xC0000005`），表现为 `test` 里**所有 case 一起 FAIL**。静态链接后 exe 只依赖
系统自带的 `KERNEL32.dll` 与 `msvcrt.dll`，与 PATH 上的 DLL 无关，代价是体积从
约 1.2 MB 增至约 3.5 MB。

若在别处编译、想自行确认依赖是否正确：

```powershell
# 查看 exe 的 DLL 依赖；-static 后应只剩 KERNEL32.dll / msvcrt.dll
objdump -p build\zuma.exe | findstr "DLL Name"
```

### 4.2 测试

```bash
make test            # 或 mingw32-make test
```

`test` 会自动遍历 `inputs/case*.txt`，用 `build/zuma --script` 逐个运行，
与 `outputs/` 中的同名文件逐行比对，最后打印 `PASS/FAIL` 汇总；
全部通过时退出码为 0。也可以直接 `python test`。

### 4.3 玩游戏（交互模式）

```bash
./build/zuma         # Windows: build\zuma.exe
```

开局可选择预设关卡或自定义轨道/队列。每轮会显示带下标编号的轨道、队首、
队列剩余、已发射数与可撤销步数。可用命令：

| 命令 | 说明 |
| --- | --- |
| `fire <pos>` | 从队首取一球插入到下标 `pos` |
| `bomb <pos>` | 炸掉下标 `pos` 处的一颗球 |
| `paint <c>` | 把队首球改成颜色 `c`（不消耗球） |
| `undo` | 撤销上一步 `fire / bomb / paint` |
| `show` | 重新显示局面 |
| `help` | 显示帮助 |
| `quit` | 退出 |

发生连锁消除时会逐条播报「消除了 N 个 X 球」；`WIN / LOSE` 后询问是否
再来一局。

### 4.4 批处理模式（测试用）

```bash
./build/zuma --script < inputs/case01.txt
```

输入格式（按行组织）：

```text
n            初始轨道球数
颜色×n       轨道颜色（头 → 尾）；n = 0 时为空行
q            待发射队列球数
颜色×q       队列颜色（队首 → 队尾）；q = 0 时为空行
m            命令条数
命令×m       每行一条：FIRE pos / BOMB pos / UNDO / PAINT c / SHOW
```

输出约定：`FIRE` 成功打印轨道行，轨道清空追加 `WIN`，队列空且轨道非空
追加 `LOSE`；`BOMB` 成功打印轨道行，清空追加 `WIN`；`UNDO` 打印恢复后的
轨道行；`PAINT` 打印队列行；`SHOW` 打印轨道行与队列行；空轨道/空队列
打印 `EMPTY`；非法操作打印 `ERROR` 且不改变任何状态。

### 4.5 样例覆盖

| 样例 | 覆盖内容 |
| --- | --- |
| case01 | 插入 → 自动消除 → 连锁消除 → WIN |
| case02 | 插入无消除、UNDO、SHOW |
| case03 | BOMB 让左右合拢消除 → WIN |
| case04 | PAINT 改色 → FIRE → LOSE |
| case05 | 越界 FIRE 的 ERROR、合法 FIRE/LOSE、连续 UNDO 直到空栈 ERROR |
| case06 | 插入形成三连 → 单轮消除（无连锁） |
| case07 | 多种非法参数（小写颜色、越界下标、空栈 UNDO）、越界 BOMB/FIRE |
| case08 | 空轨道 + 空队列：空结构访问全部 ERROR |
| case09 | 道具组合、PAINT 后 UNDO、LOSE |
| case10 | 最小局：一发 FIRE 清空轨道 → WIN |

---

## 5. 复杂度小结

| 操作 | 时间 | 空间 |
| --- | --- | --- |
| `Track::insertAt` 定位下标 | `O(min(pos, n-pos))` | `O(1)` |
| `Track::insertAt` 链接（含尾部追加） | `O(1)` | `O(1)` |
| `Track::erase(节点指针)` | `O(1)` | `O(1)` |
| `Track::nodeAt` | `O(min(pos, n-pos))` | `O(1)` |
| `Track::toVector` | `O(n)` | `O(n)` |
| `BallQueue::push / pop / front / setFront` | `O(1)` | `O(1)` |
| `BallQueue::toVector` | `O(q)` | `O(q)` |
| 一次 FIRE / BOMB（含全部连锁消除） | `O(n)` | `O(1)` 额外 |
| 一次 PAINT | `O(1)` | `O(1)` |
| 压入快照 | `O(n + q)` | `O(n + q)` |
| 一次 UNDO | `O(n + q)` | 释放一份快照 |
| 撤销全部历史 | — | `O(步数 × (n + q))` |

其中 `n` 为轨道球数，`q` 为待发射球数。

---

## 6. 文件结构

```text
Project/
├── Makefile          一键编译 / 测试
├── test              样例自动比对脚本（Python，无扩展名）
├── README.md         本报告
├── ai_record.md      分阶段开发与排错记录
├── src/
│   ├── track.h/.cpp         双向链表轨道
│   ├── ball_queue.h/.cpp    循环队列
│   ├── undo_stack.h/.cpp    撤销栈
│   ├── game.h/.cpp          游戏逻辑
│   ├── interactive.h/.cpp   交互界面
│   └── main.cpp             入口 + 批处理解析
├── inputs/           测试输入 case01–case10
└── outputs/          对应标准输出
```