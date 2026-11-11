# MyLab / Linear / StackAndQueue / task2 —— 循环队列 / 平移队列 / 链式队列
#
#   make        编译 build/task2
#   make test   运行 test：自动读取 inputs/，与 outputs/ 逐行比对
#   make clean  删除构建产物
#
# 共用的输入解析头文件在 ../include/，通过 -I../include 引用。
#
# 跨平台：目标名带 $(EXEEXT)；每条 recipe 只用一个命令，Windows 的
# cmd.exe 与 POSIX 的 sh 都能执行；test 是 Python 脚本，统一用
# $(PYTHON) 运行（Windows 上没有执行位的概念，不能写 ./test）。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -I../include -Isrc

ifeq ($(OS),Windows_NT)
EXEEXT  := .exe
PYTHON  ?= python
else
EXEEXT  :=
PYTHON  ?= python3
endif
MKDIR      := mkdir -p
RM_TREE    := rm -rf
ECHO_BLANK := echo
MAKEFLAGS += --no-print-directory

BUILD := build
EXE   := $(BUILD)/task2$(EXEEXT)
SRCS  := src/main_task2.cpp src/queue.cpp src/shift_queue.cpp src/linked_queue.cpp
DEPS  := $(wildcard src/*.hpp src/*.h) ../include/input_util.h

.PHONY: all test clean

all: $(EXE)

$(BUILD):
	@-$(MKDIR) $(BUILD)

$(EXE): $(SRCS) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(SRCS)

# 三套样例：
#   inputs/           两种数组队列共用（容量语义相同）
#   capacity_inputs/  容量 0/1/刚好装满等边界，只对数组实现有意义
#   linked_inputs/    链式队列专属（忽略 capacity，样例本身超过容量）
test: $(EXE)
	@echo == circular_queue ==
	@$(PYTHON) test
	@$(ECHO_BLANK)
	@echo == shift_queue ==
	@$(PYTHON) test --arg=--shift
	@$(ECHO_BLANK)
	@echo == capacity_edges ==
	@$(PYTHON) test --inputs capacity_inputs --outputs capacity_outputs
	@$(PYTHON) test --inputs capacity_inputs --outputs capacity_outputs --arg=--shift
	@$(ECHO_BLANK)
	@echo == linked_queue ==
	@$(PYTHON) test --inputs linked_inputs --outputs linked_outputs --arg=--linked

clean:
	-$(RM_TREE) $(BUILD)