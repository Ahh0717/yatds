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
#     概念，不能写 ./test）；
#   - Windows 下加 -static 静态链接 C++ 运行时（见下方说明）。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
LDFLAGS  ?=

ifeq ($(OS),Windows_NT)
EXEEXT  := .exe
MKDIR   := mkdir
RM_TREE := rmdir /s /q
PYTHON  ?= python
# MinGW 默认动态链接 libstdc++-6.dll / libgcc_s_seh-1.dll / libwinpthread-1.dll。
# 这几个 DLL 不会随 exe 出现在 build/ 里，Windows 会按「exe 目录 → 系统目录 →
# PATH」的顺序去找。只要 PATH 上有另一套 MinGW（典型如 conda 自带的
# Library\mingw-w64\bin），就会加载到 ABI 不兼容的同名 DLL：程序能启动，
# 但在第一次 std::cin / std::string 操作时访问冲突退出（0xC0000005，
# 即退出码 3221225477），表现为 test 里「所有 case 都 FAIL」。
# 静态链接后 exe 只依赖系统自带的 KERNEL32.dll / msvcrt.dll，与 PATH 无关。
LDFLAGS += -static
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
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(LDFLAGS)

test: $(EXE)
	@$(PYTHON) test

clean:
	-$(RM_TREE) $(BUILD)