# Tree Baseline —— 森林与二叉树转换 / 哈夫曼树

> 大一小学期「树结构」基础实验。两个相互独立的 Baseline 任务，合起来是本目录的
> 全部交付内容（`甘锦珊_24325083_Tree`）。

## 任务一览

| 任务 | 目录 | 题面要求的文件 | 待实现函数 |
| --- | --- | --- | --- |
| 二叉树与森林的转换 | `forest/` | `forest.h` / `forest-impl.hpp` / `forest-demo.cpp` | `forest_to_binary_tree`、`binary_tree_to_forest`、`forest_preorder`、`forest_postorder` |
| 哈夫曼树 | `huffman/` | `huffman-tree.h` / `huffman-tree-impl.hpp` / `huffman-coding.cpp` | `construct_huffman_tree`、`build_code_table`、`huffman_wpl` |

两个任务互不 include，各自 `make && make test` 都能单独通过。

## 一键命令

在 `Baseline/` 目录下：

```bash
make          # 编译两个任务（forest → huffman）
make test     # 依次跑两个任务的样例比对，任何一步失败立刻停下
make clean    # 递归清理两个任务的 build/
```

顶层 `make test` 拆成两个独立目标、按顺序执行；`forest` 的测试一旦失败，`make`
立即退出，不会再去跑 `huffman`，也不会打印「全部通过」。单独调试某一个任务时，
直接进它的目录：

```bash
cd forest  && make && make test
cd huffman && make && make test
```

## 目录结构

```text
Baseline/
├── Makefile              一键入口：make / make test / make clean
├── README.md             本文件：结构、命令、算法小结、跨平台说明
├── .gitignore            忽略 build/ 与 __pycache__/
├── forest/
│   ├── README_forest.md  左孩子右兄弟原理、转换正确性证明、测试设计
│   ├── Makefile          编译到 build/forest-demo$(EXEEXT)
│   ├── test              Python 比对脚本（严格逐行）
│   ├── inputs/           测试输入 case01–case06
│   ├── outputs/          对应期望输出（实跑生成并人工核对）
│   └── code/
│       ├── forest.h              仅数据结构定义 + 四个必做函数声明
│       ├── forest-impl.hpp       四个函数的实现 + 演示用二叉树遍历
│       ├── forest-demo.cpp       演示与测试驱动
│       └── tree_io.h             输入解析与输出排版（格式契约）
└── huffman/
    ├── README_huffman.md 构造原理、复杂度推导、前缀码与最优性证明
    ├── Makefile          编译到 build/huffman$(EXEEXT)
    ├── test              Python 比对脚本（严格逐行）
    ├── inputs/           测试输入 case01–case08（整份文件即 stdin）
    ├── outputs/          对应期望输出（实跑生成，WPL 先手算核对）
    └── code/
        ├── huffman-tree.h          节点与载荷定义 + 三个函数声明
        ├── huffman-tree-impl.hpp   最小堆贪心构造、DFS 编码表、WPL 计算
        └── huffman-coding.cpp      读第一行、频次统计、三段输出
```

`build/` 是编译产物目录，不提交；`make clean` 后目录树里不会残留它。

## 算法与复杂度小结

### forest：森林 ↔ 二叉树（左孩子右兄弟）

**表示法。** 每个节点的第一个孩子成为二叉树中的左孩子，其余孩子依次挂成
前一个兄弟的右孩子；森林中第二棵及以后的树，根节点依次成为前一棵树根的右孩子。
于是「任意多叉、任意多棵树」被压成固定两个指针，且该映射是双射（可逆）。

| 函数 | 做法 | 时间 | 空间 |
| --- | --- | --- | --- |
| `forest_to_binary_tree` | 递归：首个孩子接 left，其余用 `tail` 指针串成 right 链 | O(n) | O(n) + O(h) 栈 |
| `binary_tree_to_forest` | 递归只消费 left，right 链由外层循环接管 | O(n) | O(n) + O(h) 栈 |
| `forest_preorder` | 按前序递归访问每棵树 | O(n) | O(n) + O(h) 栈 |
| `forest_postorder` | 先递归孩子再访问根，逐棵拼接 | O(n) | O(n) + O(h) 栈 |

**两条不变式**（README_forest.md 中给出按节点归纳的证明）：

- 森林前序 = 对应二叉树前序；
- 森林后序 = 对应二叉树中序（注意是**中序**，不是后序）。

**复杂度要点。** 每个节点在转换中恰好创建一次、在遍历中恰好访问一次；右兄弟链用
`tail` 指针 O(1) 追加，而不是每次重新找尾，因此总时间是 O(n)，不会退化成 O(n²)。
递归栈深度取树高 `h`，退化链上 `h = n`。

### huffman：最小堆贪心构造与编码

**构造。** 每个字符一个叶子入最小堆；反复弹出权值最小的两棵子树，合并成新内部节点
（权值为两者之和）再压回，直到只剩一棵。每个节点带 `id`，堆按 `(weight, id)` 排序，
先弹出的作左孩子、后弹出的作右孩子，因此结果确定、可逐行复现。

**核心恒等式。** WPL = 所有内部节点权值之和。因此「最小化 WPL」等价于「最小化每次
合并的代价之和」，这正是贪心每步取最小的动机；严格证明用交换论证（最优树中权值最小
的两个叶子必可取为最深的兄弟对，合并它们不增大 WPL）。

| 函数 | 做法 | 时间 |
| --- | --- | --- |
| `construct_huffman_tree` | 建堆 + n−1 次合并 | O(n log n) |
| `build_code_table` | 自根 DFS，左 0 右 1，叶子处记录路径 | O(n log n) |
| `huffman_wpl` | Σ(频次 × 码长) | O(n log n) |

**O(n log n) 推导。** 建堆 O(n)；每次合并 2 次 `pop_heap` + 1 次 `push_heap`，
各 O(log n)；共 n−1 次合并，故 O(n) + (n−1)·O(log n) = O(n log n)。编码阶段每个
节点访问一次、每个叶子插入一次 `map`（O(log n)），同为 O(n log n)。

**前缀码。** 只有叶子携带字符，叶子之间不存在祖先关系，而编码就是根到叶的路径，
所以任意两条编码互不为前缀，解码无歧义。**单字符边界**：只有一个字符时根即叶子，
约定编码为 `0`（长度 1），WPL = 该字符频次，而不是 0。

## 进度与结果

| 任务 | 状态 | 样例 |
| --- | --- | --- |
| `forest/` | 四个函数实现完毕，三条不变式人工核对无误 | 6/6 PASS |
| `huffman/` | 最小堆贪心 + 确定性平局规则，ASan/UBSan 与 3000 组 fuzz 通过 | 8/8 PASS |

`make` 零警告，`make test` 两个任务共 14 个样例全部通过。

### 确定性

- `forest`：转换与遍历都是纯确定过程，无随机性。
- `huffman`：节点按 `(weight, id)` 弹出，`id` 由叶子输入顺序与内部节点递增计数器
  决定，同一份输入永远得到同一棵树、同一张编码表，因此样例可以严格逐行比对。

## 输入 / 输出格式速查

| 任务 | 输入 | 输出 |
| --- | --- | --- |
| `forest` | 首行棵数 k，随后 k 行括号表示法，如 `A(B,C,D)` | 三节六行：原森林前/后序、二叉树前/中序、还原森林前/后序 |
| `huffman` | stdin **第一行**整行文本，可含空格，不做 trim；空行/空文件视为空串 | 频次段 + 编码段 + 末行 `WPL <n>`；空输入输出一行 `EMPTY` 加一行 `WPL 0` |

细节见各自的 `README_forest.md`、`README_huffman.md`。

## 跨平台约定

- C++17，编译参数 `-std=c++17 -Wall -Wextra -Wpedantic -O2 -g`，要求零警告。
- 目标名统一带 `$(EXEEXT)`，Windows 下自动变成 `.exe`；`make test` 传的是带
  `$(EXEEXT)` 的路径，测试脚本在 Windows 上还会再补一次 `.exe` 并转绝对路径。
- Windows 分支 `LDFLAGS += -static`：MinGW 默认动态链接 `libstdc++-6.dll`，而
  conda 等环境里可能存在 ABI 不兼容的同名 DLL，会以 `0xC0000005`
  （退出码 `3221225477`）崩溃。静态链接后只依赖系统 DLL。
- 每条 recipe 只写一条命令，不用 shell 循环、变量或 `&&`，`cmd.exe` 也能执行。
- `mkdir` 不带 `-p`（`cmd.exe` 会把 `-p` 当成目录名）；目录已存在时报错由 `-` 忽略。
- 删目录按平台取命令：Windows 用 `rmdir /s /q`，其他平台用 `rm -rf`。
- 子 make 一律用 `$(MAKE)`，保证 `-n` / `-j` / 变量覆盖向下传递；顶层加
  `.NOTPARALLEL:`，让 `make test` 的 forest → huffman 次序在 `-j` 下也成立。
- `test` 是 Python 脚本，统一用 `$(PYTHON)` 运行（Windows 上没有执行位，
  所以不写成 `./test`）。

### 测试脚本

两个 `test` 都是 Python 脚本，行为一致：

- 开头把 stdout 重配置为 utf-8，所有文件读写显式 `encoding="utf-8"`；
- 把 `inputs/` 下每个文件的内容整体喂给被测程序的 stdin；
- 逐行比对 `outputs/` 下的同名文件，忽略行尾空白与文件末尾空行；
- 程序退出码非 0 直接判 FAIL 并打印 stderr 摘要；
- 失败时打印第一个不同行的行号与两边的值；
- 全部通过返回 0，否则返回 1。

`huffman` 的编码由确定性平局规则唯一确定，因此和 `forest` 一样直接逐行精确比对，
不设宽松模式，也不需要另写 oracle 判定编码形状。

## 相关文档

- 两个任务的详细说明见 `forest/README_forest.md`、`huffman/README_huffman.md`。
- 对应题面：仓库中 `src/Tree/Baseline/forest/README.md` 与
  `src/Tree/Baseline/huffman/README.md`（相对仓库根目录）。