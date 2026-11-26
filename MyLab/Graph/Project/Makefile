# Graph Project —— 终端迷宫寻路（自包含）
#
# 代码统一放在 code/ 下：project.cpp 直接调用 code/baseline/ 下的图算法源码，
# 不依赖本目录以外的任何文件。
#
#   make / mingw32-make     编译 build/graph_project$(EXEEXT)
#   make clean              删除 build/
#   运行：./build/graph_project            交互菜单
#         ./build/graph_project --dump     非交互打印四个任务结果

ifeq ($(OS),Windows_NT)
EXEEXT  := .exe
LDFLAGS += -static -s
PYTHON  := python
else
EXEEXT  :=
PYTHON  := python3
endif

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g

BUILD_DIR := build
CODE_DIR  := code
TARGET    := $(BUILD_DIR)/graph_project$(EXEEXT)

BASELINE_SRCS := \
	$(CODE_DIR)/baseline/adjacency_list/graph.cpp \
	$(CODE_DIR)/baseline/adjacency_matrix/graph.cpp \
	$(CODE_DIR)/baseline/unweighted_shortest_path/shortest_path.cpp \
	$(CODE_DIR)/baseline/dijkstra/dijkstra.cpp \
	$(CODE_DIR)/baseline/mst_prim/mst_prim.cpp \
	$(CODE_DIR)/baseline/mst_kruskal/mst_kruskal.cpp

SRCS := $(CODE_DIR)/project.cpp $(BASELINE_SRCS)

.PHONY: all clean

all: $(TARGET)

# 用 Python 建目录，不用 mkdir -p（cmd.exe 会把 -p 当成目录名）。
$(BUILD_DIR):
	$(PYTHON) -c "import os; os.makedirs('$(BUILD_DIR)', exist_ok=True)"

$(TARGET): $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SRCS)

clean:
	$(PYTHON) -c "import shutil, os; shutil.rmtree('$(BUILD_DIR)') if os.path.isdir('$(BUILD_DIR)') else None"
