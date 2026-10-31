# MyLab / Linear / list / task1 —— 单链表模板类
#
#   make        编译 build/task1
#   make test   运行 ./test：自动读取 inputs/，与 outputs/ 逐行比对
#   make clean  删除构建产物
#
# 本目录只包含任务 1 的源文件；共用的输入解析头文件在 ../include/，
# 通过 -I../include 引用。性能对比与不变量检查属于跨任务工具，见 ../tools/。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -I../include -Isrc

ifeq ($(OS),Windows_NT)
EXEEXT := .exe
else
EXEEXT :=
endif

BUILD := build
EXE   := $(BUILD)/task1$(EXEEXT)
SRCS  := src/main_task1.cpp src/linked_list.cpp
DEPS  := $(wildcard src/*.h) ../include/input_util.h

.PHONY: all test clean

all: $(EXE)

$(BUILD):
	@mkdir -p $(BUILD)

$(EXE): $(SRCS) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(SRCS)

test: $(EXE)
	@./test

clean:
	rm -rf $(BUILD)