# MyLab / Linear / list / task3 —— 三个经典链表算法
#
#   make        编译 build/task3
#   make test   运行 ./test：自动读取 inputs/，与 outputs/ 逐行比对
#   make clean  删除构建产物
#
# 本目录只包含任务 3 的源文件；共用的输入解析头文件在 ../include/，
# 通过 -I../include 引用。性能对比与不变量检查属于跨任务工具，见 ../tools/。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -I../include -Isrc

BUILD := build
EXE   := $(BUILD)/task3
SRCS  := src/main_task3.cpp src/list_advanced.cpp
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