# forest —— 森林与二叉树转换（甘锦珊_24325083_Tree / Baseline）
#
#   make        编译到 build/forest-demo$(EXEEXT)
#   make test   编译并跑 inputs/ 与 outputs/ 的逐行比对
#   make clean  删除构建产物
#
# 跨平台约定：
#   - 目标名统一带 $(EXEEXT)，Windows 下是 .exe；
#   - Windows 分支静态链接，绕开 conda 等环境里旧 libstdc++ 的 0xC0000005；
#   - 每条 recipe 只写一条命令，不用 shell 循环、变量或 &&（cmd.exe 不支持）；
#   - mkdir 不带 -p（cmd.exe 会把 -p 当成目录名）；目录已存在时由 - 忽略报错；
#   - 删目录按平台取命令：Windows 用 rmdir /s /q，其他平台用 rm -rf。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -Icode
PYTHON   ?= python3

ifeq ($(OS),Windows_NT)
EXEEXT  := .exe
PYTHON  ?= python
LDFLAGS += -static
# cmd.exe 没有 rm，删目录用 rmdir /s /q。
RM_TREE := rmdir /s /q
else
EXEEXT  :=
PYTHON  ?= python3
RM_TREE := rm -rf
endif

MKDIR := mkdir
BUILD := build
TARGET := $(BUILD)/forest-demo$(EXEEXT)
SOURCES := code/forest-demo.cpp
HEADERS := code/forest.h code/forest-impl.hpp code/tree_io.h

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(SOURCES)

$(BUILD):
	@-$(MKDIR) $(BUILD)

test: $(TARGET)
	$(PYTHON) test --exe $(TARGET)

clean:
	-$(RM_TREE) $(BUILD)