# MyLab / Linear / Baseline / String / task1 —— 顺序字符串 MyString
#
#   make        编译 build/task1
#   make test   运行 test：自动读取 inputs/，与 outputs/ 逐行比对（忽略 PERF 行）
#   make clean  删除构建产物
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

BUILD := build
EXE   := $(BUILD)/task1$(EXEEXT)
SRCS  := src/main_task1.cpp src/mystring.cpp
DEPS  := src/mystring.hpp ../include/input_util.h

.PHONY: all test clean

all: $(EXE)

$(BUILD):
	@-$(MKDIR) $(BUILD)

$(EXE): $(SRCS) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(SRCS)

test: $(EXE)
	@$(PYTHON) test

clean:
	-$(RM_TREE) $(BUILD)
	-$(RM_TREE) __pycache__