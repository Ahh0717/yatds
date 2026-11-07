# MyLab / Linear / StackAndQueue / task1 —— 数组栈 + 链式栈
#
#   make        编译 build/task1
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
MKDIR   := mkdir
RM_TREE := rmdir /s /q
ECHO_BLANK := echo.
PYTHON  ?= python
else
EXEEXT  :=
MKDIR   := mkdir -p
RM_TREE := rm -rf
ECHO_BLANK := echo
PYTHON  ?= python3
endif
MAKEFLAGS += --no-print-directory

BUILD := build
EXE   := $(BUILD)/task1$(EXEEXT)
SRCS  := src/main_task1.cpp src/stackar.cpp src/stackli.cpp
DEPS  := $(wildcard src/*.hpp) ../include/input_util.h

.PHONY: all test clean

all: $(EXE)

$(BUILD):
	@-$(MKDIR) $(BUILD)

$(EXE): $(SRCS) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(SRCS)

# 两套样例：inputs/ 用数组栈跑，linked_inputs/ 用链式栈跑（--linked）。
test: $(EXE)
	@echo == array_stack ==
	@$(PYTHON) test
	@$(ECHO_BLANK)
	@echo == linked_stack ==
	@$(PYTHON) test --inputs linked_inputs --outputs linked_outputs --arg=--linked

clean:
	-$(RM_TREE) $(BUILD)