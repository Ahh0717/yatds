# tools/ —— 跨任务工具

本目录存放**不属于单个任务**、但需要跨任务复用的工具。任务自己的
测试程序在 `taskN/test`，不放在这里。本目录全部是 Python 脚本，
**不需要编译**（顶层 `make` 会如实报告"暂无可编译源码"）。

| 位置 | 作用 | 被谁调用 |
| --- | --- | --- |
| `oracle_task3.py` | 任务 3 的独立参考实现（暴力匹配 + 暴力计数），用于产出 `outputs/` 与差分 | `fuzz_task3.py`、手工 |
| `fuzz_task3.py` | 任务 3 随机差分（7 类生成器，覆盖重叠 / 同 L / 极小 N / 含 `*`） | `cd task3 && make fuzz` |
| `check_query_task3.py` | 任务 3 **全区间穷举**：枚举 1 ≤ l ≤ r ≤ N 的每个区间，前缀和答案 vs 暴力计数 | `cd task3 && make check` |
| `gen_large_task3.py` | 任务 3 大样例生成器（n ≈ 1e6、q = 2e5，定点植入已知匹配） | `bench_large_task3.py` |
| `bench_large_task3.py` | 任务 3 大样例：生成 → 逐字节比对 → 分档计时 → N 加倍扫描 | `cd task3 && make large` |

约定：

- oracle 与 fuzz 只用标准库，不依赖被测 C++ 代码，保证“独立”。
- oracle 写 `outputs/` 时**不写** `PERF ` 行（性能数据不可复现，
  且 `test` 比对时本就忽略）。
- 大样例不写进 `task3/inputs/`：默认落在 `task3/build/`，避免 `make test`
  每次多跑一份 3.5 MB 输入。`make large` 会现场生成并校验。
- 所有脚本都接受 `--exe`，相对路径按 `task3/` 解析，可在任意目录调用。
- task2 的 bench 是 C++ 实现（`task2/src/bench.cpp`），因为它要测的是
  C++ 算法本身；它属于该任务，所以不放在这里，入口是 `cd task2 && make bench`
  或顶层 `make bench`。