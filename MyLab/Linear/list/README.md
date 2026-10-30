# MyLab / Linear / list —— 线性表基础实验（任务 1–3）

对应题目：`src/Linear/Baseline/List/README.md` 的 Baseline 任务 1、2、3。
各任务的详细说明分别见 [README_task1.md](README_task1.md)、
[README_task2.md](README_task2.md)、[README_task3.md](README_task3.md)。

## 一键构建与测试

```bash
make          # 编译全部程序
make test     # 遍历 inputs/ 逐行比对 outputs/，再跑算法级不变量检查
make clean    # 清理全部构建产物
```

`make` 产出：

| 产物 | 内容 |
| --- | --- |
| `build/task1` | 单链表模板类的驱动程序 |
| `build/task2` | 动态数组的驱动程序 |
| `build/task3` | 进阶链表算法的场景驱动程序 |
| `build/bench` | 性能对比（DynamicArray / std::vector / LinkedList），`make bench` |
| `build/invariants` | 算法级不变量检查（样例覆盖不到的部分） |
| `test` | 测试入口脚本，由 `make test` 调用 |

`make test` 的行为：

1. 遍历 `inputs/`，按**文件名前缀**分派：`task1_*.txt` → `build/task1`，
   `task2_*.txt` → `build/task2`，`task3_*.txt` → `build/task3`；
2. 与 `outputs/` 中的同名文件逐行比对。行尾空白与文件末尾空行不计；
   以 `PERF ` 开头的行（性能统计，耗时每次不同）比对时**忽略**；
3. 打印每个样例的 PASS/FAIL 与最后汇总，失败时退出码为 1；
4. 再运行 `build/invariants` 做算法级检查（下标语义、容量策略、解环、
   节点共享、深拷贝等）。

样例文件命名约定：`inputs/taskN_caseMM.txt` ↔ `outputs/taskN_caseMM.txt`。
新增样例只要按这个格式放进两个目录，`make test` 会自动纳入。

## 目录结构

```text
MyLab/Linear/list/
├── Makefile
├── README.md            # 本文件：构建入口与复杂度小结
├── README_task1/2/3.md  # 各任务的设计说明
├── bench_results/       # 实测的 PERF 输出
├── inputs/              # taskN_caseMM.txt
├── outputs/             # 与 inputs 同名
├── src/
│   ├── linked_list.h/.cpp      # 任务 1：template<typename T> class LinkedList
│   ├── dynamic_array.h/.cpp    # 任务 2：template<typename T> class DynamicArray
│   ├── list_advanced.h/.cpp    # 任务 3：三个经典链表算法
│   ├── input_util.h            # 驱动共用的输入解析
│   ├── main_task1/2/3.cpp      # 三个驱动程序
│   ├── bench.cpp               # 性能对比
│   └── invariants.cpp          # 算法级不变量检查
└── tools/
    ├── run_tests.py            # make test 的分派与比对逻辑
    ├── run_invariants.py       # 不变量检查的汇总包装
    ├── oracle_task1/2/3.py     # 独立参考实现，用于生成期望输出
    └── fuzz_task1/2/3.py       # 随机差分测试（make fuzz）
```

## 复杂度小结

### 任务 1：单链表（`n` 为元素个数）

| 操作 | 时间 | 说明 |
| --- | --- | --- |
| `pushFront` | O(1) | 改 `head_` |
| `pushBack` | O(1) | 维护 `tail_`，无需遍历 |
| `insertAt(i, x)` | O(n) | 需走到下标 `i-1`；头部插入可走 O(1) 分支 |
| `eraseAt(i)` | O(n) | 同上；头尾都能 O(1) 处理 |
| `access(i)` / `find(x)` / `contains(x)` | O(n) | 只能顺序遍历 |
| `size` / `empty` | O(1) | 维护 `size_` |
| `clear` / 析构 | O(n) | 逐节点释放 |
| 扩展：`removeAll`、`unique`、`reverse`、`append` | O(n)～O(n²) | `unique` 不借助哈希表，为 O(n²) |
| 空间 | O(n) | 每节点一个 `next` 指针的额外开销 |

### 任务 2：动态数组（`n` 为元素个数，`C` 为容量）

| 操作 | 时间 | 说明 |
| --- | --- | --- |
| `pushBack` | 均摊 O(1)，单次最坏 O(n) | 容量翻倍，扩容时搬迁全部元素 |
| `insertAt(i, x)` / `eraseAt(i)` | O(n) | 需要搬移 `i` 之后的元素 |
| `get` / `set` / `at` / `operator[]` / `front` / `back` | O(1) | 连续内存随机访问 |
| `size` / `capacity` / `empty` | O(1) | |
| `reserve` | O(n) | 只在需要扩容时发生 |
| `resize` | O(Δn) | 补默认值或析构多余元素 |
| `clear` | O(n) 析构，**不缩容** | 见 README_task2.md |
| `shrinkToFit` | O(n) | 显式缩容 |
| 空间 | O(C) | 扩容时峰值会短暂达到 O(C + n) |

实测（`n = 100000`，`ops = 200000`，详见 README_task2.md）：

```text
PERF name=vector_like n=100000 ops=200000 time_ms=230.12
PERF name=std_vector  n=100000 ops=200000 time_ms=229.41
PERF name=linked_list n=100000 ops=200000 time_ms=30679.01
```

以下标随机访问/中间修改为主时，数组比链表快两个数量级；只有在几乎全是
尾部操作的场景，链表的 O(1) 才可能追上。

### 任务 3：进阶链表算法（`n`、`m` 为两条链表的长度）

| 算法 | 时间 | 额外空间 | 关键点 |
| --- | --- | --- | --- |
| `detectAndRemoveCycle` | O(n) | O(1) | 快慢指针 + 环入口数学推导，再沿环找入口前驱断开 |
| `getIntersectionNode` | O(n + m) | O(1) | 长度差对齐后同步前进 |
| `reverseSharedPath` | O(n + m) | O(1) | 公共段只就地反转一次，两段前缀接到同一新头 |

三者都不使用哈希表、数组或标记位，也不修改节点值。内存由 `NodePool`
统一释放，避免共享节点被重复 `delete`、以及带环链表无法遍历到头的问题。

## 其他目标

```bash
make bench     # 性能对比，输出 PERF 行
make fuzz      # 随机差分测试（需要 Python 3）
make sanitize  # ASan/UBSan + 泄漏检查跑样例与模糊测试
```