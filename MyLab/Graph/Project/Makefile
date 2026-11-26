# MyLab / Graph / Project —— 终端迷宫寻路
#
#   make / mingw32-make    编译 build/graph_project（Windows 下为 build/graph_project.exe）
#   make clean             删除 build/
#
# 跨平台约定（与 MyLab 其他 Project 一致）：
#   - 目标名带 $(EXEEXT)，Windows 下 g++ 生成 .exe，Makefile 用同一个名字记账；
#   - 每条 recipe 只用一个命令，不写 shell 变量 / 管道 / `&&`，cmd.exe 和 sh 都能跑；
#   - Windows 下加 -static 静态链接 C++ 运行时，避免 PATH 上另一套 MinGW 的
#     不兼容 DLL 导致启动时访问冲突。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
LDFLAGS  ?=

ifeq ($(OS),Windows_NT)
EXEEXT  := .exe
MKDIR   := mkdir
RM_TREE := rmdir /s /q
LDFLAGS += -static
else
EXEEXT  :=
MKDIR   := mkdir -p
RM_TREE := rm -rf
endif

BASELINE := ../Baseline

BUILD := build
EXE   := $(BUILD)/graph_project$(EXEEXT)

SRCS := project.cpp \
        $(BASELINE)/adjacency_list/graph.cpp \
        $(BASELINE)/adjacency_matrix/graph.cpp \
        $(BASELINE)/unweighted_shortest_path/shortest_path.cpp \
        $(BASELINE)/dijkstra/dijkstra.cpp \
        $(BASELINE)/mst_prim/mst_prim.cpp \
        $(BASELINE)/mst_kruskal/mst_kruskal.cpp

.PHONY: all clean

all: $(EXE)

$(BUILD):
	@-$(MKDIR) $(BUILD)

# 目录用 order-only 依赖：目录时间戳变化不会触发重编。
$(EXE): $(SRCS) | $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(LDFLAGS)

clean:
	-$(RM_TREE) $(BUILD)