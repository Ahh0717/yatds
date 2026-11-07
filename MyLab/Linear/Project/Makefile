# MyLab / Linear / Project —— 祖玛（双向链表 + 循环队列 + 撤销栈）
#
#   make / mingw32-make    编译 build/zuma（Windows 下为 build/zuma.exe）
#   make test              运行 test，逐个跑 inputs/case*.txt 并比对 outputs/
#   make clean             删除 build/
#
# 跨平台约定：
#   - 目标名带 $(EXEEXT)：Windows 下 g++ 生成 build/zuma.exe，Makefile 必须
#     用同一个名字记账，否则目标永远「不存在」，每次 make 都会重编；
#   - 每条 recipe 只用一个命令，不写 shell 变量 / 管道 / `&&`，这样 Windows
#     的 cmd.exe 和 POSIX 的 sh 都能执行；
#   - test 是 Python 脚本，统一用 $(PYTHON) 运行（Windows 上没有执行位的
#     概念，不能写 ./test）。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g

ifeq ($(OS),Windows_NT)
EXEEXT  := .exe
MKDIR   := mkdir
RM_TREE := rmdir /s /q
PYTHON  ?= python
else
EXEEXT  :=
MKDIR   := mkdir -p
RM_TREE := rm -rf
PYTHON  ?= python3
endif

BUILD := build
EXE   := $(BUILD)/zuma$(EXEEXT)
SRCS  := src/main.cpp src/game.cpp src/track.cpp src/ball_queue.cpp src/undo_stack.cpp src/interactive.cpp
DEPS  := $(wildcard src/*.h)

.PHONY: all test clean

all: $(EXE)

$(BUILD):
	@-$(MKDIR) $(BUILD)

# 目录用 order-only 依赖：目录时间戳变化不会触发重编。
$(EXE): $(SRCS) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

test: $(EXE)
	@$(PYTHON) test

clean:
	-$(RM_TREE) $(BUILD)