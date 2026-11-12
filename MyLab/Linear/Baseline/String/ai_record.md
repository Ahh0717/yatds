# AI 协作记录 —— MyLab / Linear / Baseline / String（字符串实验）

记录每一阶段的 prompt 要点与对应完成的工作。
最近更新：2026-11-11

## 0. 硬约束

- 所有产出只在 `MyLab/Linear/Baseline/String/` 下，不动仓库根目录与 `src/`。
- 「你只需按照每阶段任务执行，不要自行做修改」；有疑问先问，不擅自扩展范围。
- C++17，`-Wall -Wextra -Wpedantic -O2 -g`，零 warning，统一 `namespace dsaac`。
- 三个任务各自独立提交：各自的 `Makefile`、`test`、`README_taskN.md`、
  `inputs/`、`outputs/`、`src/`；任务之间不互相 include。
- 跨平台：目标名带 `$(EXEEXT)`；test 是 Python 脚本、用 `$(PYTHON)` 运行；
  Windows 分支 `LDFLAGS += -static`（绕开 conda 旧版 libstdc++ 的 0xC0000005）。
- 输入解析统一走 `include/input_util.h`（token 化后按序读取，无空行语义）。
- 性能统计以 `PERF ` 开头追加在功能输出之后；`test` 逐行比对时忽略。
  任务 3 不输出 PERF，`test` 也不做任何行过滤。

## 1. 阶段 1：交付结构与共享解析

**Prompt 要点**
- 先理解本实验的最终交付结构，后续每个任务都按它落位，不要返工；
  本阶段只搭结构，不实现算法。
- 三个任务拆成三个互不依赖的子目录，各自 `make && make test` 可单独通过。
- 共享输入解析放在 `include/`，任务之间不互相依赖。
- 跨平台要求：`EXEEXT`、`python` / `python3`、Windows 静态链接。
- 输出一份结构说明，写清每个任务的落位与复杂度小结。

**完成的工作**
- 建出 `Makefile`、`README.md`、`include/input_util.h`、`tools/`、
  `task1/`、`task2/`、`task3/` 骨架。
- `input_util.h`：`readAllTokens` / `Cursor` / `looksLikeSize` / `reportError` /
  `readString` / `readSize` / `readOp` / `decodeEmptyToken`。
  `readSize` 只接受纯数字，拒绝 `-1`、`1x`、空串这类静默截断。
- 顶层 `Makefile` 只构建 `tools/` 下的跨任务 C++ 工具，不代跑任务测试；
  `clean` 递归进三个任务目录。
- `README.md` 写清目录树、通用约定（`EMPTY` 哨兵、PERF 行）、三个任务复杂度表。

**踩过的坑**
- 用户写的是 `MyLab/Linear/String/`，实际仓库路径是
  `MyLab/Linear/Baseline/String/`。先确认再动手，避免在错误位置建目录。

## 2. 阶段 2：任务 1 的 MyString 类本体

**Prompt 要点**
- 完善 `dsaac::MyString` 类本体，本轮只做类实现 + 自检，不写最终驱动。
- 扩容规则：容量翻倍；`append` 均摊 O(1)。
- 至少 5 个方法；`substr` 边界要明确（越界抛异常、超尾截断）。
- 内存安全；自检要覆盖到边界情况。

**完成的工作**
- `task1/src/mystring.hpp/.cpp`：`char* data_` + `len_` + `cap_`，
  三条不变量（`data_ != nullptr`、`cap_ >= len_+1` 且 `data_[len_] == 0`、
  `new[]`/`delete[]` 一一对应）。
- 方法：`length` / `empty` / `c_str` / `substr` / `append` / `operator+=` /
  `find` / `replace` / `compare` / `clear` / `reserve`。
- `ensureCapacity(need)` 反复翻倍直到够；`find` 用朴素逐位比较（刻意与任务 2 的
  KMP 形成对照）；`replace` 用 `memmove` 搬尾（源目标可能重叠）。
- 拷贝构造 / 拷贝赋值全部深拷贝；`append(const MyString&)` 与
  `replace(old, new)` 对「参数就是本对象」做了先深拷贝的保护。
- 临时自检程序覆盖 pos=0/中间/末尾、`clear` 后复用、自拼接、自替换，
  普通构建与 ASan/UBSan 均通过；验证后删除，未留在 `src/`。

**踩过的坑**
- 自拼接时扩容会释放 `other` 指向的旧块，若不先深拷贝就是
  `heap-use-after-free`。把保护去掉跑 ASan 确实报错，说明不是过度防御。
- `reserve(npos)` 会因 `n + 1` 回绕成 0 退化为空操作，改为显式抛
  `std::length_error`。

## 3. 阶段 3：任务 1 最终驱动 + 样例 + 构建 + 分析

**Prompt 要点**
- 写 `task1/src/main_task1.cpp`，补齐样例、`Makefile`、`test`、
  `README_task1.md` 分析。
- 输入输出约定；PERF 行的格式；至少 6 个样例；包含题面样例。
- 跨平台 Makefile/test；README 要有扩容策略的性能分析。

**完成的工作**
- `main_task1.cpp`：操作分发（`PRINT` / `LENGTH` / `APPEND` / `SUBSTR` /
  `FIND` / `REPLACE` / `COMPARE` / `CLEAR`），只有产生输出的操作才打印；
  末尾追加 PERF 行。
- 7 个样例 `case01..07`：空串初始、`SUBSTR` 截断与 `len=0`、`FIND` 命中/未命中、
  `REPLACE` 首次替换、`COMPARE` 三种结果、`CLEAR` 后复用、多次 `APPEND` 触发扩容。
- `README_task1.md` 里把「翻倍 vs 每次加固定量」的总搬移量算清楚：
  固定增量总搬移 ≈ N²/(2k)，翻倍总搬移 < 2N，这才是均摊 O(1) 的来源。
- bench 对比 `my_string` / `std_string` / `my_string_reserve`：
  1.15 ms / 1.24 ms / 0.70 ms（320 万字符、20 万次 append）。

**踩过的坑**
- 首次触达几 MB 新内存要付缺页中断（预热轮 1700+ 次 minor fault，
  预热后只剩 65 次）。固定「先测 my_string」的顺序会让它独吞这笔成本，
  看起来比 `std::string` 慢 2 倍**——那测的是缺页，不是扩容策略**。
  改成三类对象轮转 + 第 1 轮热身 + 其后取最小值。
- 结论要落在常数项：`MyString` 与 `std::string` 同策略、同量级；
  真正能提速的是 `reserve` 消除 O(log N) 次重新分配。

## 4. 阶段 4：任务 2 匹配实现 + 极端用例性能对比

**Prompt 要点**
- 补全 `dsaac` 命名空间下两个函数（`getNextVal`、`kmpMatch`），
  骨架在 `task2/src/`；本轮只做匹配实现 + bench，不写最终驱动。
- `getNextVal` 按题面公式；自检要求 `AAAAB` 得到 `0 0 0 0 0`。
- 做极端用例性能对比（朴素 vs KMP vs nextval），报告分析。

**完成的工作**
- `task2/src/match.hpp/.cpp`：`naiveMatch`、`getNext`、`kmpMatch`、
  `getNextVal`、`getNextValTable`、`kmpMatchNextVal`。
- 全 0-based；`next[i] = pi(pattern[0..i])`，`next[0] = 0`（与骨架一致，骨架未改）。
- `task2/src/bench.cpp` 两组极端用例，统计耗时**与精确比较次数**：
  - case A（`A×200000+B` / `A×20000+B`）：朴素 3 600 200 001 次、
    1478–1487 ms；KMP 380 001 次、0.551–0.564 ms；nextval 同 380 001 次、
    0.543–0.550 ms。
  - case B（`(A×20000+C)×20` / `A×20000+B`）：朴素 3 800 590 020 次、
    1486–1514 ms；KMP 800 020 次、1.313–1.344 ms；nextval 400 060 次、
    0.466–0.467 ms。

**踩过的坑**
- 一开始在报告里写「g++ 会向量化朴素内层循环」。用
  `-fopt-info-vec-all` 一查是 `unsupported control flow in loop`
  （循环出口依赖内层比较结果），**根本没有向量化**。
  改成「标量循环的吞吐极限、约 0.41 ns/字符」。
- KMP 每次比较 1.45 ns vs 朴素 0.41 ns，为确认差距来源做了对照：
  在 `next[j-1] == j-1` 恒成立的 case A 上写 `j = j - 1` 变体
  （比较次数完全相同、不查表），轮转 + 热身 + 取最小值后稳定在
  2.26–2.30 倍。说明差距来自 `next[]` 的**串行 load-use 依赖链**，
  不是分支预测。
- 同一条探针不轮转、不预热时曾跑出 8.20 倍异常值——又是首次触页成本，
  与任务 1 是同一个坑。
- 朴素比较次数与手算逐位对账：case A 为 `20 000 + 360 000 + 1 = 380 001`；
  case B 为 `19 × 200 030 001 + 20 001 = 3 800 590 020`。

## 5. 阶段 5：任务 2 最终驱动 + 样例 + README

**Prompt 要点**
- 写 `task2/src/main_task2.cpp`，补齐样例、`Makefile`、`test`、
  `README_task2.md`。
- 输入格式、输出约定、PERF 三行；题面样例；≥6 个样例覆盖。
- 说明 `nextval` 的两张表怎么处理。

**完成的工作**
- `main_task2.cpp`：`NEXT` / `NEXTVAL` / `NAIVE` / `KMP` / `KMP_NEXTVAL` /
  `ALL` 六种操作；每种算法只跑一次并输出
  `PERF <name> n=… m=… time_ms=…` 三行。
- 11 个样例 `case01..11`：题面自检 `AAAAAAAAAAB`/`AAAAB`、单字符、
  完全不匹配、多次出现返回首次下标、`next` 与 `nextval` 中段即不同、
  首个字符即匹配、操作顺序打乱、模式串比主串长、匹配落在末尾、
  31 字符长模式串 + 近似匹配、操作次数 0。
- 用户提问后选择「显示用公式，匹配用标准表」：
  `getNextVal` 严格按题面公式，仅用于 `NEXTVAL` 输出；
  `getNextValTable` 按 0-based 下标语义修正（`k = next[j-1]`），
  驱动 `kmpMatchNextVal` 时 `j = nv[j]`。
- 穷举验证：`{A,B}`、主串 ≤ 8、模式串 ≤ 5 共 32 193 例，
  `kmpMatch` 与 `kmpMatchNextVal` 均与朴素一致；扩大到
  主串 ≤ 8/模式 ≤ 8（261 121 例）与主串 ≤ 10/模式 ≤ 5（128 961 例），结论不变。

**踩过的坑**
- 题面公式的 `nextval` **不能直接当跳转表**：它取 `k = next[i]` 而非
  `next[j-1]`，且把连续相同字符压到 0。拿它跑自检样例会返回 −1（正确是 6）。
  反过来，强行用展示表驱动 KMP（`j = nv[j-1]` / `j = nv[j]`）分别在
  610 / 1 854 例上算错，证明拆表不是多余的。
- case10 第一版模式串 `ABABABABACABABABABAC` 的 `next == nextval`，
  演示不了差别。换成 `(AABAAB)×5 + C`，并配一段前 24 字符近似、第 25 位
  是 `X` 的主串，匹配真正落在下标 30。
- 之前报告里写的穷举总数 396 880 是错的，重写 `/tmp/exhaust.cpp`
  实测为 32 193，已更正。

## 6. 阶段 6：任务 3 正确性（翻译 / 匹配 / 坐标映射）

**Prompt 要点**
- 在 `task3/` 从零实现双链 DNA 目标蛋白编码片段检索，本任务无骨架。
- 本轮先做正确性：翻译、匹配、坐标映射、输出格式；
  用小样例与参考实现差分验证。
- 必须写 `tools/oracle_task3.py` 与 `tools/fuzz_task3.py`（独立实现）。
- 必须包含题面自检样例；ASan/UBSan 跑样例。
- 报告：坐标映射公式如何推导、自测样例是否逐行一致、随机差分多少组一致。

**完成的工作**
- `task3/src/dna.hpp/.cpp`：`translateCodon`、`reverseComplement`、
  `translateSixFrames`、`computePi`、`findAllOccurrences`、`collectMatches`、
  `MatchCounter`。
- 64 项静态密码子表，下标 `A:0 C:1 G:2 T:3` → `b1*16 + b2*4 + b3`；
  `'*'` 当普通字符参与匹配。
- `readFrame(seq, f)` 从 0-based `f-1` 起每 3 个一读，`i + 2 < n` 截断尾部残基。
- 坐标映射（1-based 闭区间）：
  正链 `L = (f-1) + 3j + 1`、`R = (f-1) + 3j + 3m`；
  反链 `L = N - (f-1) - 3j - 3m + 1`、`R = N - (f-1) - 3j`。
- `tools/oracle_task3.py`：手抄 64 条密码子字典、暴力滑窗匹配、
  线性扫描区间计数、按公式映射坐标，与 C++ 零共享逻辑。
- `tools/fuzz_task3.py`：7 类生成器轮转（翻译结果截 P、纯随机 P、
  密码子重复造重叠、全同碱基造高密度命中、近似回文造双链命中、N=1..6 极小、
  P 含 `*`），逐字段比较 pi / K / 匹配表 / 答案。
- 9 个样例 `case01..09`；题面样例输出逐行一致。
- 差分规模：默认 300 + 6 个种子各 400 + 700 + 4 个种子各 350
  + 400（Sanitizer 版），合计 5 000+ 组全部一致。

**踩过的坑**
- `translateSixFrames` 调用了尚未定义的 `readFrame`，编译失败。
  把 `readFrame` 移到匿名命名空间、放到使用点之前。
- 坐标验证脚本里把 `frame-1` 写成了 `frame`，出现假 AssertionError；
  修正后正反两个方向对全部 (j, m) 都通过。
- README 4.2 第一版验算例子写乱了、自相矛盾。换成
  `S = AATGGCTTTAAGCCATCATGGCC`（N=23），RC 第 2 帧 `AMMA*SH`、
  P=`MA` 命中 j=2，公式给出 `11 16 - 2`，与样例输出对上。
- README 4.4 一开始写「同 L 必然区间不同」，**是错的**：所有匹配长度都是
  3m，同 L 必然同 R，即完全相同的区间，只能靠 `(strand, frame)` 区分。
  改成正确结论并引用 case03（`CACGTG`/`HV` 是回文，两链同在 `[1,6]`）。
- 随机生成几乎撞不到重叠与同 L（600 组里只有 1 例和 3 例，同 L 同链为 0）。
  补 5 类定向生成器后，2100 组里有 75 例重叠、27 例同 L（全部跨链）。
  并证明「同 L 同链」结构上不可能：`f₁ - f₂ = 3(j₂ - j₁)`，左边 ≤ 2，
  右边是 3 的倍数，只能同为 0。
- 同上：`build_same_position_case` 原注释写成「造同链多帧共享 L」，按上述
  证明这是**不可能的**，改成「高密度重叠」；真正造同 L 的是回文生成器。

## 7. 阶段 7：任务 3 性能优化与交付

**Prompt 要点**
- 前缀和查询：q 最多 2e5，每次 O(1)。每条匹配长度恒为 3m，
  故 `[L,R]` 被 `[l,r]` 完整包含 ⟺ `l <= L` 且 `R <= r`
  ⟺ `L ∈ [l, r-3m+1]`。用 `cnt[x]` 记左端点 x 的匹配数、
  `pref[x] = sum cnt[1..x]`，则 `answer = pref[r-3m+1] - pref[l-1]`；
  `r-3m+1 < l` 时为 0；下标夹到 [1,N]。总复杂度 O(n+m+q)。
- 鲁棒性（评分点，必须写进样例）：重叠匹配、`r-l+1 < 3m` → 0、
  翻译里的 `*` 不影响匹配且不会被 P 匹配、`l=1`/`r=N`/`l=r`、
  正反链同时命中、同 L 不同链/帧的排序。
- 大样例：n ≈ 1e6、q = 2e5，用 `tools/` 脚本生成；验证运行时间在
  O(n+m+q) 预期内（秒级）；确认前缀和答案与暴力在小规模上一致。
- `Makefile` / `test` 与之前一致（`EXEEXT`、`resolve_exe`、`-static`）；
  本任务没有 PERF 行，`test` 不需要特殊忽略。
- `README_task3.md` 要含：设计思路（pi 数组、反向互补坐标映射、前缀和）、
  核心函数代码原理、运行说明与 O(n+m+q) 推导。
- 汇报：大样例实测耗时、前缀和与暴力差分一致性、
  `make && make test` 全 PASS、README 覆盖了哪些要点。

**完成的工作**
- 确认 `MatchCounter` 已实现题面语义：`span_ = 3m-1`、`hi = r - span_`、
  `lo = l-1`、夹到 [0,N]、`hi < lo` 返回 0、P 为空时 `span_ = -1`。
  把 `r-3m+1 < l` 这条**显式**写成 `if (right - span_ < left) return 0;`。
- **排序从 `std::sort`（O(K log K)）改成稳定计数排序（O(N+K)）**：
  生成顺序改为 `+1,+2,+3,-1,-2,-3`，恰好就是「L 相同：`+` 在前、
  frame 升序」，稳定排序后不需要比较器；删除无用的 `matchLess`
  与 `<algorithm>`。总复杂度因此严格为 O(N+m+q)。
- 新增 `tools/gen_large_task3.py`：N=1e6 随机 DNA + 定点植入已知匹配
  （正链槽写 P 的密码子编码、反链槽写其反向互补，`frame = s mod 3 + 1`），
  q=2e5 四类查询（围绕植入匹配、短于 3m、全随机、边界）。
  期望输出用 `str.find` + `bisect` 独立算，另抽 300 条查询用线性扫描复核，
  并断言每个植入点都出现在期望匹配表里。
  默认生成 24 999 个植入匹配、K = 24 999。
- 新增 `tools/check_query_task3.py`：对每个样例枚举**全部**
  `1 <= l <= r <= N` 的区间做穷举（不是抽样）。
- 新增 `tools/bench_large_task3.py` + `make large`：现场生成 → 跑程序 →
  与独立期望逐字节比对 → 分档计时（取多次最小值）；`--scaling` 做 N 加倍扫描。
- `Makefile` 增加 `check`、`large` 两个目标；`test` 保持原样。
- 新增鲁棒性样例 `case10..13`：
  - `case10` `AAAAAAAAA`/`KK`：同帧重叠（L=1,2,3,4）、`l=r`、短区间、`l=1`/`r=N`；
  - `case11` `AAAATTT`/`KF`：**同 L=2 同时命中 `+2` 与 `-1`**，压满三级排序键；
  - `case12` `ATGTAAGGGTAAATG`/`M`：翻译结果含 `*`（`M*G*M`）且仍能匹配 `M`；
  - `case13` `ATGAAAATGAAA`/`MK`：区间刚够 3m，端点平移一格即归零。

**实测数据**（Intel Xeon Platinum 8358P，`g++ 15.2.0 -O2`，
共享云主机 `cpu MHz` 在 0.9–2.6 GHz 漂移，多次取最小值）：

```text
大样例 N=1e6, K=24999, q=2e5    73 ms 量级（多次 71–83 ms）
                 q=0            27 ms
                 换 P 使 K=0    20–25 ms
单条查询边际成本                ≈ 210–240 ns
K 扫描（S=A×1e6, P=K/KK/KKK）   K≈1e6 时 177–184 ms
N 加倍扫描 q=0                  4.7 → 6.9 → 14.2 → 26.8 ms（N 每档 ×2）
查询数扫描                      q=0/5e4/1e5/2e5/4e5 → 36/44/57/78/124 ms
```

**验证结果**
- `make && make test`：13/13 PASS。
- `make check`：13 组、1 504 个区间全部一致；再加 400 组随机小用例
  共 413 组、28 205 个区间全部一致。
- `make fuzz`：300/300、另跑 500/500 全部一致。
- `make large`：22.5 万行输出与独立期望**逐字节一致**。
- ASan+UBSan：13 个样例输出与期望逐字节一致、无报错；随机差分 150/150；
  K=199 998 的高命中输入也干净退出。
- 畸形输入（非 ACGT、`r > N`、`l > r`）退出码 1 且无半截输出。

**踩过的坑**
- 本机没有 `/usr/bin/time`，最初计时命令直接报错。改用
  `date +%s%N` 前后取差，并用 `taskset -c 2` 固定核、多轮取最小值。
- 用户给的 `answer = pref[r-3m+1] - pref[l-1]` 与实现里的
  `hi = right - (3m-1)` 是同一个数，只是为了让代码和规范字面对上，
  把 `r-3m+1 < l` 单独提成早返回；行为不变。
- 大样例不能放进 `inputs/`：3.5 MB 输入 + 22 万行输出会让
  `make test` 每次多跑一份。改为写到 `task3/build/`，由 `make large` 触发。


## 8. 阶段 8：整目录收尾（一键构建 / 一键测试 / 干净提交）

**Prompt 要点**
- 对整个 `MyLab/Linear/String/`（实际路径 `MyLab/Linear/Baseline/String/`）
  做收尾：确保一键编译、一键测试、干净提交。
- 顶层 `Makefile`：`make` 编译跨任务工具、`make test` 依次进入三个任务、
  `make clean` 清理。
- 顶层 `README.md`：结构说明 + 三个任务复杂度/算法小结 + 跨平台运行说明。
- 检查清单：① 三个任务各自 `make && make test` 全 PASS；② 顶层 `make test`
  依次跑通；③ 连跑两次第二次不再编译（`EXEEXT` 记账正确）；④ 全仓零警告；
  ⑤ 无临时自检文件残留，每个 `taskN/src` 只留本任务源码。

**完成的工作**
- 顶层 `Makefile` 重写：
  - 新增 `make test`，三条独立 recipe 依次 `$(MAKE) -C taskN test`，
    任一段失败 make 立即停下（实测把 `PYTHON` 指向不存在的解释器，
    task1 报 Error 后 task2/3 未执行，退出码 2）；
  - 新增 `make bench`（转发到 task2）；
  - **去掉 `MAKEFLAGS += --no-print-directory`**，让顶层测试输出能看出
    当前在哪个任务里；
  - `make` / `make tools` 在 `tools/` 无 C++ 源码时如实报告"暂无可编译源码"。
- 顶层 `README.md` 重写：目录树、一键构建测试、跨平台表
  （`mingw32-make`、`EXEEXT`、`-static`、`PYTHON`、recipe 写法、test 的
  `.exe` 与 UTF-8 处理）、通用约定，以及三个任务各自的算法与复杂度小结。
- 删除阶段 1 遗留的空壳目录 `tools/bench/`（只有 `.gitkeep`）：bench 的
  真实实现在 `task2/src/bench.cpp`，两者语义冲突，`tools/README.md`
  也随之改写为"tools/ 全部是 Python 脚本、无需编译"。

**验证结果**
- 三个任务独立：task1 7/7、task2 11/11、task3 13/13，全 PASS。
- 顶层 `make test`：依次 7/7、11/11、13/13，最后打印"三个任务全部通过"。
- 增量记账：`make clean` 后第一次跑出 3 条 `g++`，第二次 0 条。
- 零警告：全部 7 个 C++ 源（含 `bench.cpp`，`make test` 不编译它）
  用 `-Wall -Wextra -Wpedantic` 严格 `-fsyntax-only` 检查均无输出；
  两次 `make test` 的日志 grep `warning` 均为 0。
- 残留检查：无 `.o` / `*.tmp` / `a.out` / `*.exe` / `*.orig` / `*~`；
  `task1/src` 只有 `mystring.hpp/.cpp + main_task1.cpp`，`task2/src` 只有
  `match.hpp/.cpp + main_task2.cpp + bench.cpp`，`task3/src` 只有
  `dna.hpp/.cpp + main_task3.cpp`。
- 复跑工具：`make fuzz` 100/100、`make check` 13 组 1504 区间全一致、
  `oracle_task3.py` 输出正常；`make bench` 可编译运行。

**踩过的坑**
- 顶层 `test` 第一版写了两份（POSIX 用 `foreach`、Windows 用专用分支），
  make 报 `overriding recipe` 警告。改成单一定义、多条独立 recipe 后，
  两个平台都能执行，且不再需要平台分支。
- 本机没有 `mingw32-make`，Windows 分支只能做静态审查（`EXEEXT`、
  `-static`、recipe 单命令、`$(MAKE)` 传递、test 的 `.exe` 与 UTF-8），
  未能实机验证；已在 README 的跨平台表里写明对应关系。
- `make clean` 的输出看起来像"没有删除 build 目录"，实际是
  `rm -rf build` 在子 shell 里先执行、随后 `find` 才跑；重新在
  clean 之后单独 `find` 确认三个 `build/` 与 `__pycache__` 都已清空。

## 9. 当前状态

- 任务 1：完成，7 个样例 PASS。
- 任务 2：完成，11 个样例 PASS，bench 可复现。
- 任务 3：完成，13 个样例 PASS，`check` / `fuzz` / `large` 全部通过。
- 顶层 `make test` 可一键依次跑通三个任务；`make clean` 清理干净；
  连跑两次第二次不重复编译；全仓零警告。
- 目录树已清理：删除阶段 1 遗留的空壳 `tools/bench/`；每个 `taskN/src`
  只保留本任务源码；无临时自检文件残留。
- 本目录在工作区中尚未纳入 git 跟踪（`git status` 显示为 `??`）。
  按用户要求**不执行 git add/commit**，只交付工作区。
