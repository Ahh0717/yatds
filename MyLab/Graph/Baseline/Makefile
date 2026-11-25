# Graph Baseline —— 六个独立模块：邻接表 / 邻接矩阵 / 无权最短路 / Dijkstra / Prim / Kruskal
#
# 产物统一放在顶层 build/ 目录，可执行文件名：
#   adj_list  adj_matrix  shortest_path  dijkstra  prim  kruskal （均带 $(EXEEXT)）
#
# 一键命令：
#   make                 编译六个驱动到 build/
#   make test            依次跑六个 test-xxx，任一失败立即停止
#   make test-adj-list   只编译邻接表驱动并与 test.out 比对（其余模块同理）
#   make clean           删除整个 build/ 目录

ifeq ($(OS),Windows_NT)
EXEEXT  := .exe
LDFLAGS += -static
PYTHON  := python
else
EXEEXT  :=
PYTHON  := python3
endif

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g

BUILD_DIR := build

.PHONY: all test test-adj-list test-adj-matrix test-shortest-path test-dijkstra test-prim test-kruskal clean

# 默认目标：编译六个驱动，方便单独调试。
all: \
	$(BUILD_DIR)/adj_list$(EXEEXT) \
	$(BUILD_DIR)/adj_matrix$(EXEEXT) \
	$(BUILD_DIR)/shortest_path$(EXEEXT) \
	$(BUILD_DIR)/dijkstra$(EXEEXT) \
	$(BUILD_DIR)/prim$(EXEEXT) \
	$(BUILD_DIR)/kruskal$(EXEEXT)

# 用 Python 建目录（不用 mkdir -p，纯 cmd 也能跑）；已存在时无副作用。
$(BUILD_DIR):
	$(PYTHON) -c "import os; os.makedirs('$(BUILD_DIR)', exist_ok=True)"

$(BUILD_DIR)/adj_list$(EXEEXT): adjacency_list/graph.cpp adjacency_list/driver.cpp adjacency_list/graph.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ adjacency_list/graph.cpp adjacency_list/driver.cpp

$(BUILD_DIR)/adj_matrix$(EXEEXT): adjacency_matrix/graph.cpp adjacency_matrix/driver.cpp adjacency_matrix/graph.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ adjacency_matrix/graph.cpp adjacency_matrix/driver.cpp

test-adj-list: $(BUILD_DIR)/adj_list$(EXEEXT)
	$(PYTHON) check.py $(BUILD_DIR)/adj_list$(EXEEXT) adjacency_list/test.in adjacency_list/test.out

test-adj-matrix: $(BUILD_DIR)/adj_matrix$(EXEEXT)
	$(PYTHON) check.py $(BUILD_DIR)/adj_matrix$(EXEEXT) adjacency_matrix/test.in adjacency_matrix/test.out

$(BUILD_DIR)/shortest_path$(EXEEXT): unweighted_shortest_path/shortest_path.cpp unweighted_shortest_path/driver.cpp unweighted_shortest_path/shortest_path.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ unweighted_shortest_path/shortest_path.cpp unweighted_shortest_path/driver.cpp

test-shortest-path: $(BUILD_DIR)/shortest_path$(EXEEXT)
	$(PYTHON) check.py $(BUILD_DIR)/shortest_path$(EXEEXT) unweighted_shortest_path/test.in unweighted_shortest_path/test.out

$(BUILD_DIR)/dijkstra$(EXEEXT): dijkstra/dijkstra.cpp dijkstra/driver.cpp dijkstra/dijkstra.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ dijkstra/dijkstra.cpp dijkstra/driver.cpp

test-dijkstra: $(BUILD_DIR)/dijkstra$(EXEEXT)
	$(PYTHON) check.py $(BUILD_DIR)/dijkstra$(EXEEXT) dijkstra/test.in dijkstra/test.out

$(BUILD_DIR)/prim$(EXEEXT): mst_prim/mst_prim.cpp mst_prim/driver.cpp mst_prim/mst_prim.hpp edge.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ mst_prim/mst_prim.cpp mst_prim/driver.cpp

test-prim: $(BUILD_DIR)/prim$(EXEEXT)
	$(PYTHON) check.py $(BUILD_DIR)/prim$(EXEEXT) mst_prim/test.in mst_prim/test.out

$(BUILD_DIR)/kruskal$(EXEEXT): mst_kruskal/mst_kruskal.cpp mst_kruskal/driver.cpp mst_kruskal/mst_kruskal.hpp edge.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ mst_kruskal/mst_kruskal.cpp mst_kruskal/driver.cpp

test-kruskal: $(BUILD_DIR)/kruskal$(EXEEXT)
	$(PYTHON) check.py $(BUILD_DIR)/kruskal$(EXEEXT) mst_kruskal/test.in mst_kruskal/test.out

# 依次执行六个子 make，每条 recipe 是独立命令：串行、任一失败即中止后续。
test:
	$(MAKE) test-adj-list
	$(MAKE) test-adj-matrix
	$(MAKE) test-shortest-path
	$(MAKE) test-dijkstra
	$(MAKE) test-prim
	$(MAKE) test-kruskal

clean:
	$(PYTHON) -c "import shutil, os; shutil.rmtree('$(BUILD_DIR)') if os.path.isdir('$(BUILD_DIR)') else None"