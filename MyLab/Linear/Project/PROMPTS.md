# 祖玛游戏（Project1）任务 Prompt

一段一个阶段，粘贴到聊天框，做完再给下一段。下面的要求是**我给你的计划**，按它执行；`src/Linear/Project/README.md` 只当背景和样例参考，以本文为准。有疑问先问我，别猜。

> 使用说明（给用户看，不用粘给智能体）：每个 ` ``` ` 代码块是一个阶段，逐段粘给智能体，等它做完并汇报后再粘下一段。

---

## 阶段 0 · 总纲（先读，搭建结构 + 通用约定）

```
先理解本实验的最终交付结构，后续每阶段都按它落位，不要返工。

【背景】你在实现数据结构课程「栈、队列和链表的综合应用」实验——祖玛游戏。
  项目要求原文在 src/Linear/Project/README.md，可读作背景；实现细节以本计划为准。
  所有产出统一放在 MyLab/Linear/Project/ 下，**不要改动其他任何目录**。

【输出根目录】MyLab/Linear/Project/，最终结构：

  Project/
  ├── Makefile              # 一键编译 + 一键测试
  ├── README.md             # 报告：设计思路、核心函数原理解释、运行说明
  ├── src/
  │   ├── track.h / track.cpp          # 双向链表轨道（核心数据结构）
  │   ├── ball_queue.h / ball_queue.cpp # 循环队列（待发射队列）
  │   ├── undo_stack.h / undo_stack.cpp # 栈（撤销记录）
  │   ├── game.h / game.cpp            # 游戏逻辑（初始化/发射/插入/消除/连锁/胜负/道具）
  │   └── main.cpp                     # 驱动：交互模式 + --script 批处理模式
  ├── inputs/   case01.txt …           # 输入样例
  ├── outputs/  case01.txt …           # 同名标准输出
  └── test                             # 测试程序（Python 脚本，静态文件）

【数据结构选型（报告里要写清原因和优点，这占 30% 分）】
  - 轨道 = 手写**双向链表**：插入/删除/查找连续同色段都是 O(1)（有节点指针时），
    且消除后左右两段重新连接、撤销恢复，双向指针都最自然。禁用 std::list。
  - 待发射队列 = 手写**循环队列**（数组环形缓冲区，front/back 回绕）：队首出队
    O(1)。禁用 std::queue 存发射球。
  - 撤销 = 手写**栈**（底层可用 std::vector，但栈的 push/pop 接口自己封装）：
    每次状态改变前压入一份状态快照，UNDO 弹出恢复。禁用 std::stack 直接当撤销栈用。
  - 游戏状态 = 一个聚合结构 GameState { track, queue, … }，作为快照和撤销单元。

【通用约定（贯穿全项目）】
  - 彩球颜色用单个大写字母 A–Z 表示（char）。样例里用 R/G/B/Y。
  - 所有类放进 namespace zuma。
  - C++17，g++ 编译，-std=c++17 -Wall -Wextra -Wpedantic -O2。
  - main.cpp 支持两种模式（用命令行参数区分）：
      · 无参数            → 交互模式（真人玩，带提示和菜单）
      · --script（或 -s）→ 批处理模式：从 stdin 读一段脚本，输出确定结果，
                            供 test 程序自动比对。此模式不得输出任何提示语/菜单。
  - 本项目没有 PERF 性能行，test 做「逐行精确比对」。

【批处理模式 I/O 格式（test 就靠这个，务必严格）】
  输入：
    第1行 n        ：初始轨道球数
    第2行          ：n 个颜色，空格分隔，从轨道头到轨道尾；n=0 时是空行
    第3行 q        ：待发射队列球数
    第4行          ：q 个颜色，空格分隔，队首在前；q=0 时是空行
    第5行 m        ：命令条数
    之后 m 行命令，每行一条
  命令与输出（每条命令的输出恰好是下列之一，每行一条）：
    FIRE pos   ：从队列队首取一球，插入轨道 pos 位置（0 ≤ pos ≤ 当前长度，
                 插入后新球位于下标 pos），随后执行自动消除与连锁消除。
                 成功→输出一行「轨道行」；若轨道清空→紧接着再输出一行 WIN；
                 若队列已空且轨道非空→紧接着再输出一行 LOSE。
    BOMB pos   ：小道具。删除轨道 pos 位置（0 ≤ pos < 长度）的一颗球，随后消除
                 与连锁消除。成功→输出「轨道行」；轨道清空→再输出 WIN。
    UNDO       ：撤销上一步状态改变操作。成功→输出恢复后的「轨道行」。
    PAINT c    ：小道具。把队列队首球颜色改成 c。成功→输出「队列行」。
    SHOW       ：输出两行：第一行「轨道行」，第二行「队列行」。
  格式定义：
    「轨道行」= 头→尾颜色空格分隔，空轨道输出 EMPTY。
    「队列行」= 队首→队尾颜色空格分隔，空队列输出 EMPTY。
  非法操作输出一行 ERROR 且不改变任何状态。非法情形：
    FIRE 时队列空、或 pos > 当前长度；BOMB 时轨道空、或 pos 越界；
    UNDO 时撤销栈空；PAINT 时队列空、或 c 不是 A–Z。

【自动消除与连锁消除规则（核心算法）】
  - 一次插入/删除后，只有受影响位置附近可能新产生「连续 ≥3 同色」段
    （轨道其它部分原本已稳定，不会凭空冒出）。
  - 从受影响节点出发找「极大同色段」（向左右扩展直到颜色不同）：
      · 若长度 ≥3 → 删除整段；删除后其左右相邻节点现在相邻，若同色则把两段合并
        再判断，长度 ≥3 则继续删除（这就是连锁消除），如此反复直到不再有可消除段。
      · 若长度 <3 → 本轮不消除，直接结束。
  - 例：轨道 R R B B R R，FIRE 4 插入 B → R R B B B R R，删 B B B → R R R R，
    再删 R R R R → 空 → WIN。

【跨平台要求（务必遵守，否则 Windows 上 make test 会挂）】
  - Makefile 用 EXEEXT 区分 .exe（见阶段 5 模板），目标名写成 build/zuma$(EXEEXT)，
    避免 MinGW 每次全量重编。
  - test 是静态 Python 脚本，内部必须：
      1) 定义 resolve_exe(path)：Windows 下补 .exe 后缀并 os.path.abspath()，
         因为 MinGW 的 g++ 把 -o build/zuma 编译成 build/zuma.exe，且 subprocess
         传裸相对路径在 Windows 会 WinError 2。
      2) 调用时用 subprocess.run([exe, '--script'], stdin=..., ...)，
         加 encoding='utf-8'。
      3) 顶部 sys.stdout.reconfigure(encoding='utf-8') 避免中文乱码。
  - Makefile 里用 PYTHON ?= python，test 目标写成 @$(PYTHON) test。

理解后回复一句「结构已明确，I/O 约定已掌握」，先不写代码，等我确认再开始阶段 1。
```

---

## 阶段 1 · 三个核心数据结构

```
在 MyLab/Linear/Project/src/ 完成三个手写数据结构，各自能独立编译。先不写游戏逻辑。

【文件】track.h/.cpp、ball_queue.h/.cpp、undo_stack.h/.cpp。

【1. Track —— 双向链表（轨道）】
  namespace zuma { class Track { ... }; }
  私有：struct Node { char color; Node* prev; Node* next; }; 头尾哨兵节点 head_/tail_、
        size_t size_。
  手动 new/delete，析构逐个释放，禁止 std::list。
  接口：
    Node* insertAt(size_t pos, char color);  // 0 ≤ pos ≤ size，返回新节点
    void  erase(Node* p);                    // 从链表摘下并 delete
    Node* nodeAt(size_t pos);                // 0 ≤ pos < size，返回节点（供消除逻辑用）
    size_t size() const;  bool empty() const;
    std::vector<char> toVector() const;      // 头→尾，供打印/快照/比对
    void  clear();
  Node 的 prev/next 和 color 允许 Game 层读取（消除逻辑要用），可把 Node 设为
  public 嵌套结构或提供 getPrev/getNext/colorOf 访问器，二选一，别把哨兵暴露出去。

【2. BallQueue —— 循环队列（待发射队列）】
  namespace zuma { class BallQueue { ... }; }
  私有：char* buf_（或 std::vector<char> 作底层缓冲）、front_、back_、size_、cap_。
  环形缓冲，下标到末尾回绕到 0；用「额外一个空槽」区分空/满（cap 表示可存个数，
  底层分配 cap+1 槽）。禁止 std::queue。
  接口：
    bool push(char c);          // 满返回 false（游戏里不会满，但接口要有）
    char front() const;         // 空时抛 std::out_of_range
    char pop();                 // 出队并返回；空时抛 std::out_of_range
    void setFront(char c);      // 改队首颜色（给 PAINT 用）；空时抛 std::out_of_range
    size_t size() const;  bool empty() const;
    std::vector<char> toVector() const;  // 队首→队尾
    void  clear();

【3. UndoStack —— 栈（撤销）】
  namespace zuma { class UndoStack { ... }; }
  底层用 std::vector<GameState>，但接口自己封装成 push/pop/top/empty/size/clear，
  并在注释里写清「这里用栈管理撤销历史」。
  这里 GameState 先定义一个最小占位 struct（阶段 2 会补全）：
    struct GameState {
      std::vector<char> track;   // 轨道头→尾
      std::vector<char> queue;   // 队列队首→队尾
    };
  接口：
    void push(const GameState& s);  bool pop(GameState& out);  bool empty() const;
    size_t size() const;  void clear();

【自检】写一个临时 main 或直接加一段测试代码，验证：
  - Track 在 pos=0/中间/末尾插入、删除、toVector 顺序正确、clear 后复用；
  - BallQueue 把 front/back 推到缓冲区末尾触发回绕、空队 front/pop 抛异常；
  - UndoStack 的 push/pop 后进先出顺序。
  验证完把临时测试删掉或挪到注释里，不要留在 src/ 里影响最终构建。

做完汇报：三个类的接口清单、自检结果（回绕、越界异常是否通过）。有疑问先问。
```

---

## 阶段 2 · 游戏核心逻辑 + 批处理驱动

```
在 MyLab/Linear/Project/src/ 完成 Game 核心玩法，并让 main.cpp 的 --script 模式
能跑通基本样例。

【文件】game.h/.cpp、main.cpp（先只做 --script 批处理，交互模式阶段 4 再做）。

【Game 类（namespace zuma）】
  持有：Track track_; BallQueue queue_; UndoStack undo_; 以及当前胜负状态。
  关键方法：
    void  init(const std::vector<char>& track, const std::vector<char>& queue);
    bool  fire(size_t pos, std::string* out_track, bool* won, bool* lost);
    bool  undo(std::string* out_track);           // 恢复后轨道
    bool  bomb(size_t pos, std::string* out_track, bool* won);
    bool  paint(char c, std::string* out_queue);
    std::string trackStr() const;  std::string queueStr() const;  // 供 SHOW/打印
  规则（与阶段 0 的 I/O 约定严格一致）：
    - fire：队首出队 → track_.insertAt(pos, color) → eliminate(newNode) → 判胜负。
    - bomb：erase(nodeAt(pos)) → 若删除导致左右相邻同色，做一轮合并消除（连锁）。
    - undo：undo_.pop 快照，恢复 track_ 和 queue_。
    - 每次 fire/bomb/paint 成功前，先把当前 GameState 压入 undo_（paint 也要压，
      这样 undo 也能撤销 paint）。

【消除函数（写清楚，这是核心）】
  // 从节点 p 出发：找包含 p 的极大同色段；长度≥3 则删除并连锁，返回是否发生了消除
  bool eliminateFrom(Node* p);
  实现思路：
    1) 记 color = p->color；left = p，right = p；分别向左右扩展 while 相邻同色。
    2) 若该段长度 < 3，返回 false。
    3) 删除整段（逐个 erase）。令 L = 段首前一节点、R = 段尾后一节点（哨兵不算）。
    4) 若 L、R 均存在且同色：把 L、R 当作新受影响节点继续（它们现在相邻，可能
       合并成 ≥3），递归/循环回到第 1 步判断 L（或 R）所在段；否则结束，返回 true。
  注意：删除整段后，若 L、R 同色但合并后 <3，则本轮结束（不会再长出别的段）。

【main.cpp（--script 批处理模式）】
  - argv 含 --script（或 -s）→ 批处理模式：按阶段 0 的输入格式读 stdin，逐条命令
    执行并严格按输出约定打印。不得输出提示语。
  - 用 std::cin 读即可（n、q、m 和命令；颜色之间空格分隔，读成 char）。
  - 输出约定再核对一遍：
      FIRE/BOMB/UNDO 成功 → 打印轨道行（空=EMPTY）；FIRE/BOMB 后轨道清空 → 再打 WIN；
      FIRE 后队列空且轨道非空 → 再打 LOSE。
      PAINT 成功 → 打印队列行（空=EMPTY）。
      SHOW → 轨道行 + 队列行两行。
      非法 → 一行 ERROR，状态不变。

【自测样例（放到 inputs/case01.txt 与 case02.txt，outputs/ 放期望，先手写确认）】
  case01（插入+自动消除+连锁消除+WIN）：
    输入：
      6
      R R B B R R
      1
      B
      1
      FIRE 4
    期望输出：
      EMPTY
      WIN
  case02（无消除 + UNDO + SHOW）：
    输入：
      5
      R R G G R
      2
      B B
      4
      FIRE 2
      UNDO
      FIRE 2
      SHOW
    期望输出：
      R R B G G R
      R R G G R
      R R B G G R
      R R B G G R
      B
  （这里 FIRE 2 插入 B 后是 R R B G G R，没有连续 3 个同色，所以不消除。）

【验收】编译后 `./zuma --script < inputs/case01.txt` 和 case02 都要逐行一致。
做完汇报：编译命令、case01/02 比对结果、eliminateFrom 的连锁处理是否正确。有疑问先问。
```

---

## 阶段 3 · 撤销 + 胜负 + 小道具

```
在 MyLab/Linear/Project/src/ 补全撤销、胜负判定和小道具，并补样例。

【撤销】阶段 2 已实现快照栈，本阶段重点核验：
  - UNDO 能回退 fire/bomb/paint 三种操作，恢复轨道与队列。
  - UNDO 栈空时输出 ERROR 且不改状态。
  - 撤销后继续操作应正常（比如 undo 后再次 fire 用的是恢复后的队列）。

【胜负判定】统一在每次 fire/bomb 后判定：
  - 轨道清空 → WIN（游戏结束）。
  - FIRE 后队列空且轨道非空 → LOSE（没球可打了）。
  输出顺序：先打印轨道行，再打印 WIN 或 LOSE（若触发）。

【小道具（至少实现这两个，可再加）】
  - BOMB pos：删除轨道 pos 位置一颗球，再触发消除与连锁消除。
    （用法/触发方式在交互模式里说明；批处理模式直接用 BOMB pos 命令。）
  - PAINT c  ：把待发射队列队首球颜色改成 c（不消耗球、不改轨道）。
  这两个道具都要能被 undo 撤销。

【补样例（inputs/outputs 同名）】
  case03（BOMB 合并触发消除 + WIN，无需队列）：
    输入：
      5
      R R B R R
      0

      1
      BOMB 2
    期望输出：
      EMPTY
      WIN
  case04（PAINT + LOSE）：
    输入：
      3
      R G R
      1
      B
      2
      PAINT G
      FIRE 1
    期望输出：
      G
      R G G R
      LOSE
  case05（边界 ERROR：空栈 UNDO、空队列 FIRE、LOSE 后继续）：
    输入：
      2
      R G
      1
      B
      4
      FIRE 2
      FIRE 0
      UNDO
      UNDO
    期望输出：
      R G B
      LOSE
      ERROR
      R G
      ERROR
  （FIRE 2 后队列空→LOSE；FIRE 0 队列空→ERROR；UNDO 恢复 R G；再 UNDO 栈空→ERROR。）

【验收】5 个样例全部逐行一致。
做完汇报：5 个样例比对结果、胜负判定触发点、两个道具的实现方式。有疑问先问。
```

---

## 阶段 4 · 用户交互（交互模式）

```
在 MyLab/Linear/Project/src/main.cpp 补上交互模式（无参数运行进入）。

【要求】
  - 无参数运行 → 交互模式；--script → 批处理模式（已实现，别破坏）。
  - 开局：提示输入初始轨道（一行颜色）与待发射队列（一行颜色），或用几个预设关卡
    让玩家选（二选一，简单即可）。
  - 每轮显示：当前轨道（带位置编号，如 0:R 1:R 2:B …）、队首下一球颜色、
    队列剩余球数、已发射数。
  - 命令菜单：fire <pos> / bomb <pos> / paint <c> / undo / show / help / quit。
    非法输入给友好提示，不崩溃。
  - 每次 fire/bomb 后，若发生消除，打印一句「消除了 N 个 X 球」（N≥3），
    让连锁消除肉眼可见；这是交互模式额外输出，批处理模式不要有。
  - 判定 WIN / LOSE 后打印结果，并询问是否再来一局（重开清空 track/queue/undo）。
  - 全部用标准输入输出，中文提示即可（代码文件存 UTF-8）。

【验收】交互模式能正常玩通一局：发球→消除→连锁→WIN/LOSE→重开；undo/bomb/paint
  都能用且被 undo 正确回退。把一局的关键截图/文字过程简要汇报。
做完汇报。有疑问先问。
```

---

## 阶段 5 · 收尾（Makefile + test + README 报告 + 验收）

```
把 MyLab/Linear/Project/ 收成可一键编译、一键测试、可交付的状态。

【Makefile】
    CXX      ?= g++
    CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
    PYTHON   ?= python
    ifeq ($(OS),Windows_NT)
    EXEEXT := .exe
    else
    EXEEXT :=
    endif
    BUILD := build
    EXE   := $(BUILD)/zuma$(EXEEXT)
    SRCS  := src/main.cpp src/game.cpp src/track.cpp src/ball_queue.cpp src/undo_stack.cpp
    DEPS  := $(wildcard src/*.h)
    .PHONY: all test clean
    all: $(EXE)
    $(BUILD):
    	@mkdir -p $(BUILD)
    $(EXE): $(SRCS) $(DEPS) | $(BUILD)
    	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)
    test: $(EXE)
    	@$(PYTHON) test
    clean:
    	rm -rf $(BUILD)
  （SRCS 里的 .cpp 名以实际为准；注意用制表符缩进。）

【test 测试程序（静态 Python 脚本，文件名就是 test，无扩展名）】
  逻辑：遍历 inputs/case*.txt，逐个用 build/zuma --script 运行（stdin 重定向到样例），
  与 outputs/ 同名文件逐行精确比对，最后打印 PASS/FAIL 汇总，全过则退出码 0。
  必须满足阶段 0 的跨平台要求：
    import os, subprocess, sys
    sys.stdout.reconfigure(encoding='utf-8')
    def resolve_exe(path):
        if os.name == 'nt' and not path.lower().endswith('.exe'):
            path += '.exe'
        return os.path.abspath(path)
    EXE = resolve_exe(os.path.join('build', 'zuma'))
  运行用 subprocess.run([EXE, '--script'], stdin=..., capture_output=True,
  encoding='utf-8')。逐行 strip 后比对，忽略末尾空白差异。

【README.md（报告，评分基于它 + 代码，务必写足）】
  至少包含：
    1) 设计思路（可直接放你的 prompt）。
    2) 各核心功能函数的代码原理解释——重点解释：
       双向链表轨道为什么这么选、insertAt/erase 怎么做；
       eliminateFrom 自动消除与连锁消除的算法步骤与复杂度；
       循环队列的环形缓冲与空/满判定；
       撤销栈（快照）的工作原理与取舍；
       小道具的设计。
    3) 数据结构选型原因和优点（对应 30% 数据结构设计分）。
    4) 运行说明：如何编译（make / mingw32-make）、如何跑测试（make test）、
       如何玩（./zuma 交互；./zuma --script 批处理）。
    5) 每个数据结构/关键操作一句复杂度小结。

【样例完整性】确认 inputs/outputs 覆盖：插入无消除、自动消除、连锁消除、WIN、
  LOSE、BOMB、PAINT、UNDO、空结构访问/越界的 ERROR，缺的补齐。

【验收（务必在 Windows 上实测，本机用 mingw32-make）】
  1. mingw32-make && mingw32-make test 全 PASS。
  2. mingw32-make 连跑两次，第二次不出现编译命令（验证 EXEEXT 不重编）。
  3. ./zuma 交互模式能玩通一局。
  4. 干净 git status 下，MyLab/Linear/Project/ 之外无改动。
把验收结果逐条汇报给我。
```
