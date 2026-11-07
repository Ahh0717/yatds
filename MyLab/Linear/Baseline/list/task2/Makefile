# MyLab / Linear / list / task2 —— 动态数组
#
#   make        编译 build/task2
#   make test   运行 ./test：自动读取 inputs/，与 outputs/ 逐行比对
#   make clean  删除构建产物
#
# 任务 2 的功能输出后需追加 PERF（性能对比），对比对象包含阶段 1 的链表，
# 因此编译时引入 ../task1/src/linked_list.{h,cpp}（README 任务 2 明确要求
# 分析「链表 vs 可变数组」的性能差异）。共用的输入解析头文件在 ../include/。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -I../include -Isrc -I../task1/src

ifeq ($(OS),Windows_NT)
EXEEXT := .exe
else
EXEEXT :=
endif

BUILD := build
EXE   := $(BUILD)/task2$(EXEEXT)
SRCS  := src/main_task2.cpp src/dynamic_array.cpp ../task1/src/linked_list.cpp
DEPS  := $(wildcard src/*.h) ../include/input_util.h ../task1/src/linked_list.h

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