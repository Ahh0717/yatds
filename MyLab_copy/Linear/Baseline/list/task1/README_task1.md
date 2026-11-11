# 任务 1：单链表模板类

对应题目：`src/Linear/Baseline/List/README.md` 的 **Baseline 任务 1**。
任务 2（可变数组与性能对比）见 `../task2/README_task2.md`。

## 文件结构

本目录是任务 1 的独立提交物，只包含任务 1 的源文件：

```text
Makefile               # 只编译任务 1
test                   # 测试程序：读取 inputs/，与 outputs/ 逐行比对
README_task1.md        # 本文件
src/linked_list.h      # template<typename T> class LinkedList
src/linked_list.cpp    # 显式实例化 int / std::string（编译期自检）
src/main_task1.cpp     # 读 stdin 的驱动程序
inputs/case01.txt      # 题目样例
inputs/case02.txt      # 边界（空表、越界、CLEAR 后操作、重复值）
inputs/case03.txt      # std::string
inputs/case04.txt      # 负数与混合操作
outputs/caseMM.txt     # 与输入同名的标准输出
```

任务外的东西不在这里：共用的输入解析在 `../include/input_util.h`；
独立参考实现 `../tools/oracle_task1.py` 与随机差分测试
`../tools/fuzz_task1.py` 属于跨任务工具，不属于本任务的提交物。

## 编译与运行

```bash
make            # 编译 build/task1
make test       # 运行 ./test：自动读取 inputs/ 与 outputs/ 比对
make clean      # 删除 build/
./build/task1 < inputs/case01.txt
```

也可以不用 Makefile：

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -Isrc -I../include \
    -o build/task1 src/main_task1.cpp src/linked_list.cpp
```

## 实现要点

- 节点为 `Node { T val; Node *next; }`，全部用 `new` / `delete` 手动管理；类内维护
  `head_`、`tail_`、`size_`，析构函数逐个释放节点。
- 只维护单个指针遍历，不借助任何 STL 容器存元素（驱动程序里的 `std::vector`
  仅用于缓存输入 token，与链表存储无关）。
- `tail_` 在头插、尾删、整表清空等路径上都会同步维护，保证删空后再次
  `pushBack` 不会写到已释放的内存。
- 下标语义与题意一致：`INSERT i x` 插入后 `x` 的下标就是 `i`；
  `i` 越界返回 `false` 且不改链表；`FIND` 返回第一个匹配下标，找不到返回 `-1`。
- 拷贝构造与拷贝赋值做深拷贝（先拷贝再交换），因此不会出现两条链表共享节点、
  析构时重复释放的问题。

## 接口

题目要求的接口：

| 接口 | 说明 |
| --- | --- |
| `pushFront(x)` / `pushBack(x)` | 头插 / 尾插 |
| `bool insertAt(i, x)` | 下标 `i` 处插入，`i > size` 返回 `false` |
| `bool eraseAt(i)` | 删除下标 `i`，`i >= size` 返回 `false` |
| `bool access(i, out)` | 读取下标 `i` 处元素 |
| `long long find(x)` | 返回首个下标，不存在返回 `-1` |
| `contains(x)` | 是否存在 |
| `size()` / `empty()` | 元素个数 / 是否为空 |
| `clear()` | 清空并释放全部节点 |
| `print(os)` | 空格分隔；空链表输出 `EMPTY` |

额外实现的功能：

| 接口 | 说明 |
| --- | --- |
| `set(i, x)` | 修改下标 `i` 处元素 |
| `popFront(out)` / `popBack(out)` | 弹出首 / 尾元素 |
| `removeAll(x)` | 删除全部值为 `x` 的节点，返回删除个数 |
| `unique()` | 去重（保留首次出现），返回删除个数 |
| `append(other)` | 在尾部追加另一条链表 |
| `reverse()` | 原地反转，$O(n)$ 时间、$O(1)$ 额外空间 |
| `hasCycle()` / `cycleEntry()` | 快慢指针判环 / 求环入口 |

## 测试样例

`outputs/case*.txt` 下的期望输出由 `../tools/oracle_task1.py`（一份独立于 C++ 实现的 Python 参考实现）
生成，避免"用被测程序证明自己正确"。C++ 驱动的输出与它逐字节比对。

此外 `../tools/fuzz_task1.py` 生成 300 轮随机操作序列（int 与 string 交替），同时与
参考实现比对，并在 ASan/UBSan 下运行，覆盖越界、非法下标、空链表操作等路径。

## 已知取舍

- 驱动程序默认根据**初始元素**判断实例类型（全是整数用 `int`，否则用 `std::string`）。
  初始为空但要插入字符串时，需要显式加 `--string`。
- 输入格式非法（如 `INSERT 0 abc` 用在 `int` 实例上、参数缺失）会向 stderr
  报错并以非零码退出；标准输出在此之前已经写出的内容保持有效。
- `unique()` 为 $O(n^2)$ 实现。题目约束不允许借助哈希表或额外容器，这一复杂度
  在链表上属于常规取舍。