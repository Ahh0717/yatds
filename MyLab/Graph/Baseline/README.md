# 图 Baseline 实验

本目录是「图」实验的六个独立 Baseline 模块：邻接表、邻接矩阵、无权最短路、Dijkstra、
Prim、Kruskal。每个模块独立源码、独立驱动、独立测试，互不依赖（仅共享顶层 `edge.hpp`）。

## 目录树

```text
Baseline/
├── Makefile                 顶层：all / test / test-xxx / clean
├── check.py                 通用测试脚本：跑驱动并与 test.out 逐行比对
├── edge.hpp                 共享边结构体（mst_prim、mst_kruskal 使用）
├── README.md                本文件
├── .gitignore               忽略 build/、__pycache__/
├── adjacency_list/
│   ├── graph.hpp  graph.cpp  driver.cpp  test.in  test.out
├── adjacency_matrix/
│   ├── graph.hpp  graph.cpp  driver.cpp  test.in  test.out
├── unweighted_shortest_path/
│   ├── shortest_path.hpp  shortest_path.cpp  driver.cpp  test.in  test.out
├── dijkstra/
│   ├── dijkstra.hpp  dijkstra.cpp  driver.cpp  test.in  test.out
├── mst_prim/
│   ├── mst_prim.hpp  mst_prim.cpp  driver.cpp  test.in  test.out
└── mst_kruskal/
    ├── mst_kruskal.hpp  mst_kruskal.cpp  driver.cpp  test.in  test.out
```

`.hpp` 接口、建图/加边/打印等已实现部分原样保留；补全的是各模块中标注 `TODO(student)`
的算法函数。`driver.cpp` 自己写 `main`，从 `argv[1]` 读入 `test.in`，调用算法模块并按
`test.out` 的格式输出到 stdout。

## 一键命令

```sh
make                       # 编译六个驱动到 build/（方便单独调试）
make test                  # 依次跑六个 test-xxx，任一失败立即停止
make test-adj-list         # 邻接表：BFS / DFS / 拓扑排序
make test-adj-matrix       # 邻接矩阵：BFS / DFS / 拓扑排序
make test-shortest-path    # 无权图 BFS 单源最短路
make test-dijkstra         # 非负权图 Dijkstra 单源最短路
make test-prim             # Prim 最小生成树
make test-kruskal          # Kruskal 最小生成树
make clean                 # 删除 build/
```

每个 `test-xxx` 目标先编译对应驱动到 `build/`，再用
`python3 check.py build/<name> <module>/test.in <module>/test.out` 逐行比对；
全部一致打印 `PASS` 并返回 0，否则打印首个不同行并返回非零。

可执行文件统一放在顶层 `build/`：

| 目标 | 可执行文件 |
| --- | --- |
| test-adj-list | `build/adj_list$(EXEEXT)` |
| test-adj-matrix | `build/graph_matrix$(EXEEXT)` |
| test-shortest-path | `build/shortest_path$(EXEEXT)` |
| test-dijkstra | `build/dijkstra$(EXEEXT)` |
| test-prim | `build/prim$(EXEEXT)` |
| test-kruskal | `build/kruskal$(EXEEXT)` |

## 六个模块的算法与复杂度

记 V 为顶点数、E 为边数。

| 模块 | 算法 | 关键数据结构 | 复杂度 |
| --- | --- | --- | --- |
| 邻接表 | BFS / DFS / Kahn 拓扑排序 | 邻接链表 + 队列 / 递归栈 / 入度数组 | 遍历 **O(V + E)** |
| 邻接矩阵 | BFS / DFS / Kahn 拓扑排序 | 二维矩阵 + 队列 / 递归栈 / 入度数组 | 遍历 **O(V²)** |
| 无权最短路 | BFS 单源最短路 | 邻接表 + 队列 | **O(V + E)**，首访即最短边数 |
| Dijkstra | 朴素选最小 + 松弛 | 邻接表 + `best[]` / `visited[]` | **O(V² + E)**（线性选点；堆优化为 O((V+E)logV)） |
| Prim | 扩展生成树 | `best_weight[]` / `parent[]` / `selected[]` | **O(V² + E)**（线性选点） |
| Kruskal | 排序边 + 并查集 | 排序 + 路径压缩 / 按秩合并 | **O(E log E)**（排序主导，并查集近 O(1)） |

要点：

- **邻接表按加边顺序**枚举邻接点；**邻接矩阵按顶点编号升序**扫描。因此同一张图在两种
  表示下，BFS/DFS/拓扑的输出顺序可能不同（当加边顺序与编号顺序不一致时）。
- 无权最短路与 Dijkstra 不可达点 `dist`、`prev` 均保持 `-1`。
- Prim / Kruskal 选满 V−1 条边即成功；不连通返回失败（`TOTAL 0`、`EDGE_COUNT 0`）；
  单顶点图成功，总权重 0、边数 0。

## 跨平台说明

顶层 `Makefile` 同时支持类 Unix 与 Windows（`cmd.exe`），约定如下：

- **`$(EXEEXT)`**：`ifeq ($(OS),Windows_NT)` 分支下为 `.exe`，其余为空。可执行文件名统一
  追加该变量，两种平台都能正确引用产物。
- **`-static`**：Windows 分支 `LDFLAGS += -static -s`，把运行库静态链接进 exe 并 strip 符号，避免目标机缺少
  MinGW/MSYS 运行库导致无法启动。
- **`PYTHON`**：Windows 下常用 `python`，其他平台用 `python3`，在分支里各取其一。
- **每条 recipe 只写一条命令**：不用 shell 循环、`&&` 或内联变量，纯 `cmd.exe` 也能执行；
  需要多步时拆成多条独立 recipe（如 `test` 里逐条 `$(MAKE) test-xxx`）。
- **建目录 / 删目录走 Python**：不用 `mkdir -p`（`cmd.exe` 不支持），改用
  `os.makedirs(..., exist_ok=True)`；`make clean` 用 `shutil.rmtree` 整目录删除，不依赖
  `rm`/`rmdir`，Windows 下同样可用。

## 测试数据

各模块 `test.in` / `test.out` 为公开样例，测试即「跑驱动 → 输出与 `test.out` 逐行比对」。
`check.py` 忽略行尾空白与文件末尾空行，避免不同平台换行/尾随空格造成误判。
覆盖场景包括：有向无环图、有向环、无向图调拓扑、不连通图、单顶点图、不可达点、等权图、
非法顶点等。