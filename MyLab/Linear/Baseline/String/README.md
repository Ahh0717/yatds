# MyLab / Linear / Baseline / String —— 字符串实验

对应题目：`src/Linear/Baseline/String/README.md`（原始题面，含统一输入输出格式与样例）。

本目录是字符串实验的独立提交物，按三个任务拆成三个互不依赖的子目录。
三个任务各自可单独构建测试，也可以从顶层一键跑完。

## 目录结构

```text
MyLab/Linear/Baseline/String/
├── Makefile              # 一键入口：make / make test / make bench / make clean
├── README.md             # 本文件：结构说明 + 复杂度小结 + 跨平台运行说明
├── ai_record.md          # 各阶段的 Prompt 要点与完成工作记录
├── include/
│   └── input_util.h      # 三个任务共用的输入解析（token 化后按序读取）
├── tools/                # 跨任务 Python 工具（oracle / fuzz / 全区间穷举 / 大样例）
│   ├── oracle_task3.py       # 任务 3 独立参考实现
│   ├── fuzz_task3.py         # 任务 3 随机差分
│   ├── check_query_task3.py  # 任务 3 全区间穷举
│   ├── gen_large_task3.py    # 任务 3 大样例生成器
│   └── bench_large_task3.py  # 任务 3 大样例计时
├── task1/                # 顺序字符串 MyString
│   ├── Makefile          # 只编译本任务，make && make test 可单独通过
│   ├── test              # Python 脚本：比对 inputs/ 与 outputs/，忽略 PERF 行
│   ├── README_task1.md   # 分析报告（功能实现 + 扩容策略性能分析）
│   ├── inputs/  outputs/ # caseMM.txt 同名对应
│   └── src/              # mystring.hpp/.cpp, main_task1.cpp
├── task2/                # 模式匹配：朴素 / KMP / nextval
│   ├── Makefile          # 另有 bench 目标（最坏情况性能取证）
│   ├── test
│   ├── README_task2.md   # 分析报告（最坏情况耗时 + 复杂度分析）
│   ├── inputs/  outputs/
│   └── src/              # match.hpp/.cpp, main_task2.cpp, bench.cpp
└── task3/                # 双链 DNA 目标蛋白编码片段检索
    ├── Makefile          # 另有 fuzz / check / large 目标
    ├── test
    ├── README_task3.md   # 分析报告（算法设计 + 坐标映射 + 查询优化）
    ├── inputs/  outputs/
    └── src/              # dna.hpp/.cpp, main_task3.cpp
```

## 一键构建与测试

```bash
make            # 构建 tools/ 下的跨任务可编译工具（当前无 C++ 源码，空转）
make test       # 依次进入 task1 / task2 / task3 跑各自的 make test
make bench      # 编译并运行 task2 的 bench（朴素 vs KMP vs nextval）
make clean      # 清理三个任务与本目录的构建产物、Python 缓存
```

也可以单独跑某个任务：

```bash
cd task1 && make && make test
cd task2 && make && make test
cd task3 && make && make test
```

任务 3 还有几个补充目标（见 `task3/README_task3.md`）：

```bash
cd task3 && make fuzz     # 随机差分：C++ vs tools/oracle_task3.py
cd task3 && make check    # 全区间穷举：前缀和 vs 暴力
cd task3 && make large    # n=1e6、q=2e5 大样例：逐字节比对 + 计时
```

## 跨平台运行说明

| 平台 | 构建命令 | 说明 |
| --- | --- | --- |
| Linux / macOS | `make`、`make test` | GNU Make + g++（C++17）+ Python 3 |
| Windows (MinGW) | `mingw32-make`、`mingw32-make test` | 同上，把 `make` 换成 `mingw32-make` |

实现上做的跨平台处理：

- **可执行文件后缀**：Makefile 里统一用 `$(EXEEXT)`，Windows 下为 `.exe`，
  其他平台为空。因此 `build/task1` 与 `build/task1.exe` 都是目标名的一部分，
  增量构建能正确记账，连跑两次不会重复编译。
- **静态链接**：Windows 分支加 `LDFLAGS += -static`。MinGW 动态链接时，
  conda 等环境里旧版 `libstdc++-6.dll` 会被优先加载，导致程序以
  `0xC0000005` 退出；静态链接标准库可绕开。
- **Python 解释器名**：Windows 用 `python`，其他平台用 `python3`，
  都通过 `PYTHON ?=` 定义，可在命令行覆盖。
- **recipe 写法**：每条 recipe 尽量只用一条命令，不写 shell 循环、变量或
  `&&`，`cmd.exe` 与 POSIX `sh` 都能执行。子 make 一律用 `$(MAKE)`。
- **test 脚本**：`taskN/test` 会自动补 `.exe` 后缀并转绝对路径，文件读写
  显式指定 `encoding="utf-8"`，输出重配置为 UTF-8，避免 Windows 控制台乱码。

## 通用约定

- 每个任务一个 `main_*.cpp`：从 stdin 读、往 stdout 写。
- 只有“产生输出的操作”才输出；其余操作静默执行。
- 性能统计以 `PERF ` 开头，追加在功能输出之后；`time_ms` 每次不同，
  `test` 逐行比对时忽略以 `PERF ` 开头的行。任务 3 不输出 PERF，
  其 `test` 因此不做任何行过滤——多一行少一行都算失败。
- 输入没有“空行”语义：所有字段都是空白分隔的非空 token，
  第 1 行的初始串用 `EMPTY` 表示空串。
- 共享解析统一走 `include/input_util.h`，任务之间不互相 include。
- 编译选项统一为 `-std=c++17 -Wall -Wextra -Wpedantic -O2 -g`，零警告。

## 三个任务的算法与复杂度小结

### 任务 1：顺序字符串 `MyString`

动态字符数组 `char* data_` + 长度 + 容量，满足三条不变量
（非空指针、`cap_ >= len_+1` 且 `data_[len_] == 0`、`new[]`/`delete[]` 配对）。

| 操作 | 复杂度 | 说明 |
| --- | --- | --- |
| `length` / `empty` | O(1) | 直接读 `len_` |
| `append` | 均摊 O(m) | 容量翻倍；总搬移量 < 2N |
| `substr(pos, k)` | O(k) | 越界抛 `out_of_range`，超尾截断 |
| `find` / `replace` / `compare` | O(nm) / O(n+m) / O(min(n,m)) | `find` 刻意用朴素匹配，与任务 2 对照 |

扩容策略对比：固定增量总搬移 ≈ N²/(2k)，翻倍总搬移 < 2N——这才是均摊 O(1) 的来源。
`reserve` 可消除 O(log N) 次重新分配。

### 任务 2：朴素 / KMP / nextval 模式匹配

全部 0-based；`next[i] = pi(pattern[0..i])`，`next[0] = 0`。

| 算法 | 时间 | 空间 | 最坏用例实测（n≈2e5, m≈2e4） |
| --- | --- | --- | --- |
| 朴素 | O(nm) | O(1) | 3 600 200 001 次比较，1478–1487 ms |
| KMP | O(n+m) | O(m) | 380 001 次比较，0.551–0.564 ms |
| nextval | O(n+m) | O(m) | 380 001 次比较，0.543–0.550 ms |

`nextval` 只改变失配后的跳转（跳过必然失配的相同前缀），不改变复杂度量级；
在 `(A×20000+C)×20` 这类用例上把比较次数从 800 020 降到 400 060。
题面公式的 `nextval` 表与可直接驱动匹配的跳转表语义不同，代码里分成两张表：
`getNextVal` 严格按题面公式（仅用于 `NEXTVAL` 输出），`getNextValTable`
按 0-based 下标语义修正（用于驱动 `kmpMatchNextVal`）。

### 任务 3：双链 DNA 目标蛋白编码片段检索

正向链 3 个阅读框 + 反向互补链 3 个阅读框，共 6 条蛋白序列；
KMP 找全 P 的所有（可重叠）出现，映射回原 DNA 坐标后按
`(L 升序, '+' 在前, frame 升序)` 输出，再答 `q` 个区间包含计数。

| 阶段 | 时间 | 空间 |
| --- | --- | --- |
| 反向互补 + 6 帧翻译 | O(N) | O(N) |
| `pi` 数组 | O(m) | O(m) |
| 6 条蛋白序列上跑 KMP | O(N + m) | O(m) |
| 坐标映射 + 稳定计数排序 | O(N + K) | O(N + K) |
| 前缀和预处理 | O(N + K) | O(N) |
| 每个查询 | O(1) | — |
| **合计** | **O(N + m + q)** | **O(N + m)** |

关键点：所有匹配长度恒为 `3m`，所以"被 `[l,r]` 完整包含"等价于
`l <= L <= r-3m+1`，在 L 坐标轴上做前缀和即可 O(1) 查询；L 值域为 1..N，
排序用稳定计数排序而不是比较排序，去掉 log 因子；K ≤ 2N，所以 K 项被 N 吸收。
实测 N=1e6、q=2e5 约 71 ms，峰值内存 29.3 MB。

> 各任务的实测性能数据与更细的分析见对应的 `README_taskN.md`。