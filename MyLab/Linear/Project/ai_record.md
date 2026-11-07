# AI 协作记录 —— MyLab / Linear / Project（祖玛）

记录每一阶段的 prompt 要点与对应完成的工作。
最近更新：2026-11-07

## 0. 硬约束

- 所有产出只在 `MyLab/Linear/Project/` 下，不动仓库根目录与 `src/`。
- C++17，`-Wall -Wextra -Wpedantic -O2`，零 warning，全部 `namespace zuma`。
- 禁用 `std::list`、`std::queue`（存发射球）、`std::stack`（当撤销栈）。
- 交付结构：`Makefile`、`README.md`、`src/`、`inputs/`、`outputs/`、`test`。
- `--script` / `-s` 为纯批处理模式，不得输出任何提示语。

## 1. 阶段 1：三个手写数据结构

**Prompt 要点**
- `Track`：双向链表 + 首尾哨兵，`insertAt / erase / nodeAt / size / empty /
  toVector / clear`，手动 `new/delete`；`Node` 的结构公开供 Game 层读取，
  哨兵不暴露。
- `BallQueue`：环形缓冲区，`cap_+1` 槽用「额外一个空槽」区分空满；
  `push / front / pop / setFront / size / empty / toVector / clear`，
  空队 `front/pop/setFront` 抛 `std::out_of_range`。
- `UndoStack`：底层 `std::vector<GameState>`，只暴露栈接口
  （`push / pop / empty / size / clear`，另按接口清单补了 `top`）。
- 临时自检不得留在 `src/`。

**完成的工作**
- `src/track.h/.cpp`、`src/ball_queue.h/.cpp`、`src/undo_stack.h/.cpp`。
- 三个类各自可独立 `-c` 编译。临时自检程序（2174 项断言）覆盖
  pos=0/中间/末尾插入删除、clear 后复用、回绕、空队异常、LIFO 顺序，
  普通构建与 ASan/UBSan 均通过；验证后删除。
- 哨兵 `color` 固定为 `'\0'`，作为 Track 与 Game 之间的约定写入注释。

**踩过的坑**
- Track 构造时第二个哨兵分配失败会泄漏第一个 → 构造体内 try/catch 兜底。
- 自检里先把「第 99 个节点的后继是哨兵」写成了 98，属断言笔误，非实现问题。

## 2. 阶段 2：Game 核心玩法 + --script

**Prompt 要点**
- `Game` 持有 `Track` / `BallQueue` / `UndoStack` 与胜负状态；
  提供 `init / fire / undo / bomb / paint / trackStr / queueStr`。
- `eliminateFrom(Node*)`：找极大同色段，≥3 删除，删除后左右同色则继续连锁。
- 批处理 I/O 按阶段 0 约定；`FIRE/BOMB/UNDO` 成功打印轨道行，
  轨道空追加 `WIN`，FIRE 后队列空且轨道非空追加 `LOSE`。
- 给定 case01（连锁消除 + WIN）与 case02（无消除 + UNDO + SHOW）。

**完成的工作**
- `src/game.h/.cpp`、`src/main.cpp`，`inputs/case01-02.txt` 与期望输出。
- `eliminateFrom` 用「找段 → 删段 → 换探针」循环：删段前记下
  `left_ball` / `right_ball`，只有两者都存在且同色才把探针移过去。
- `GameState` 增加 `won` / `lost`：否则「FIRE 判负后 UNDO」会残留胜负标志。
- 独立 Python 参考实现做 3000 组随机差分，0 处不一致；ASan/UBSan 下复跑同样通过。

**踩过的坑**
- 参考实现最初用 `list.insert(-1, x)` 当合法操作，导致误报；
  负数下标应判 `ERROR`。
- 删除循环里先 `erase(q)` 再比较 `q == last` 是 UB → 改成先判 `at_last`。

## 3. 阶段 3：撤销核验 / 胜负 / 道具 + 样例

**Prompt 要点**
- 核验 UNDO 能回退 `fire` / `bomb` / `paint`，空栈 `UNDO` → `ERROR`，
  撤销后继续操作正常。
- 胜负统一在 `fire` / `bomb` 后判定，先轨道行再 `WIN` / `LOSE`。
- 道具 `BOMB pos` 与 `PAINT c` 均可被 undo。
- 新增 case03（BOMB 合并消除 + WIN）、case04（PAINT + LOSE）、
  case05（空栈 UNDO / 空队列 FIRE / LOSE 后继续）。

**完成的工作**
- 补 `inputs/outputs/case03-05.txt`，5 个样例全部逐行一致。
- `bomb` 原来无条件 `lost_ = false`；改为只在清空轨道（`won_`）时清除失败标志。
- `main.cpp` 输入解析改为**整份输入按行切分、每条命令独立解析**：
  `parseCount` 拒绝 `-1` / `1x` / 空串，命令 token 数量也校验；
  `n=0` / `q=0` 的空行仍读掉，避免后续 `q` / `m` 错位；兼容 CRLF。
- 4000 组随机差分（含非法参数、连续 UNDO）0 处不一致。

**踩过的坑**
- 原实现用 `std::cin >> int` 失败后 `ignore` 到行尾，会把**下一行命令**
  一起吞掉，导致后续输出整体错位。改为按行解析后解决。

## 4. 阶段 4：交互模式

**Prompt 要点**
- 无参数 → 交互模式；`--script` 不变。
- 开局可选预设关卡或自定义；每轮显示带编号的轨道、队首、队列剩余、已发射数。
- 命令 `fire / bomb / paint / undo / show / help / quit`，非法输入友好提示。
- 每次消除打印 `消除了 N 个 X 球`（仅交互模式）。
- WIN / LOSE 后询问是否再来一局，重开清空全部状态。

**完成的工作**
- 新增 `src/interactive.h/.cpp`，`main.cpp` 只负责分发。
- `Game` 增加 `Elimination` / `lastEliminations()` / `shots()` /
  `undoSize()` / `colorAt()` / `frontColor()`；`GameState` 带上 `shots`。
- `Track` 增加 const 版 `nodeAt()`，可写版复用它。
- 3 个预设关卡；队列为空时提示「只能用 bomb 清球，或用 quit 退出」。
- 实测一局：`fire 4` → 消除 3 个 B + 4 个 R → WIN → 重开复位；
  自定义局验证 `bomb` 合并消除 → `undo` 完全恢复 → `paint` → `fire` → LOSE。
- 10 类非法输入均给出针对性提示，无崩溃；交互会话在 ASan/UBSan 下干净退出。
- 批处理 5 个样例在普通与 sanitizer 构建下全部保持 PASS。

**踩过的坑**
- `colorAt` 最初漏了 `const`，在 `printTrack(const Game&)` 里编译失败 →
  补 const 重载并让可写版 `const_cast` 复用。
- 写 `printTrack` 时留了一段无用的空 vector 占位，已删。

## 5. 阶段 5：构建 / 测试 / 交付

**Prompt 要点**
- `Makefile`：`EXEEXT`、`build/zuma$(EXEEXT)`、`PYTHON ?= python`，
  制表符缩进。
- `test`：无扩展名的 Python 脚本，`resolve_exe` 补 `.exe`、stdout 设
  utf-8、`subprocess.run([EXE, '--script'], stdin=...)`；逐行比对，
  全 PASS 退出码 0。
- 补齐样例：ERROR / 空结构 / PAINT / 道具组合等。
- README 写足报告（设计思路、核心算法、选型、运行说明、复杂度）。

**完成的工作**
- 新增 `Makefile`、`test`、`.gitignore`；重写 `README.md` 为完整报告。
- 新增 case06–case10：单轮消除、非法参数与越界、空轨道空队列、
  道具组合 + PAINT 撤销、最小 FIRE→WIN。
- 10 个样例在普通构建与 ASan/UBSan 下全部 PASS，零警告。

**踩过的坑**
- 只用 `-o build/zuma`（不带 `$(EXEEXT)`）时 Windows 下目标名与实际产物
  不一致，每次 make 都会重编；目标名必须带 `$(EXEEXT)`。
- `test` 原先 `open()` 后没关文件句柄，改成 `with open(...)`。
- 本机没有 Windows 工具链，用 `make OS=Windows_NT` 模拟验证了 `.exe`
  记账路径（详见验收记录）；真实 Windows 上仍需复跑一次。