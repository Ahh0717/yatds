# MyLab / Linear / Baseline / String / task2 —— 朴素 / KMP / nextval 模式匹配
#
#   make         编译 build/task2（驱动）
#   make test    运行 test：自动读取 inputs/，与 outputs/ 逐行比对（忽略 PERF 行）
#   make bench   编译并运行 build/bench（最坏情况性能取证）
#   make clean   删除构建产物
#
# 共用输入解析头文件在 ../include/，通过 -I../include 引用。
#
# 跨平台：目标名带 $(EXEEXT)；每条 recipe 只用一个命令，Windows 的
# cmd.exe 与 POSIX 的 sh 都能执行；test 是 Python 脚本，统一用
# $(PYTHON) 运行（Windows 上没有执行位概念，不能写 ./test）。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -I../include -Isrc

ifeq ($(OS),Windows_NT)
EXEEXT  := .exe
PYTHON  ?= python
# MinGW 动态链接时，conda 等环境里旧的 libstdc++-6.dll 会被优先加载，
# 导致程序以 0xC0000005 退出；静态链接标准库可绕开该问题。
LDFLAGS += -static
else
EXEEXT  :=
PYTHON  ?= python3
endif
MKDIR      := mkdir -p
RM_TREE    := rm -rf
MAKEFLAGS += --no-print-directory

BUILD  := build
EXE    := $(BUILD)/task2$(EXEEXT)
BENCH  := $(BUILD)/bench$(EXEEXT)
SRCS   := src/main_task2.cpp src/match.cpp
DEPS   := src/match.hpp ../include/input_util.h

.PHONY: all test bench clean

all: $(EXE)

$(BUILD):
	@-$(MKDIR) $(BUILD)

$(EXE): $(SRCS) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(SRCS)

# bench 单独成目标：它内部的朴素算法在最坏用例上要跑 1~2 秒，
# 不适合挂进 make test 的常规路径。
$(BENCH): src/bench.cpp src/match.cpp $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ src/bench.cpp src/match.cpp

test: $(EXE)
	@$(PYTHON) test

bench: $(BENCH)
	@echo == 最坏用例：朴素 vs KMP vs nextval ==
	@"$(abspath $(BENCH))"

clean:
	-$(RM_TREE) $(BUILD)
	-$(RM_TREE) __pycache__