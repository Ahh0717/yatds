# MyLab / Linear / StackAndQueue / task3 —— 栈与队列的相互转换
#
#   make        编译 build/task3
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
EXE   := $(BUILD)/task3$(EXEEXT)
SRCS  := src/main_task3.cpp src/stack_by_queue.cpp src/queue_by_stack.cpp \
         src/vector_stack.cpp src/vector_queue.cpp
DEPS  := $(wildcard src/*.hpp src/*.h) ../include/input_util.h

.PHONY: all test clean

all: $(EXE)

$(BUILD):
	@-$(MKDIR) $(BUILD)

$(EXE): $(SRCS) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(SRCS)

# 两套样例：inputs/ 是 int，string_inputs/ 是 std::string（--string）。
test: $(EXE)
	@echo == int ==
	@$(PYTHON) test
	@$(ECHO_BLANK)
	@echo == string ==
	@$(PYTHON) test --inputs string_inputs --outputs string_outputs --arg=--string

clean:
	-$(RM_TREE) $(BUILD)