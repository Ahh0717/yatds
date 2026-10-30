# MyLab / Linear / list / task2 —— 动态数组
#
#   make        编译 build/task2
#   make test   运行 ./test：自动读取 inputs/，与 outputs/ 逐行比对
#   make clean  删除构建产物
#
# 本目录只包含任务 2 的源文件；共用的输入解析头文件在 ../include/，
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
EXE   := $(BUILD)/task2$(EXEEXT)
SRCS  := src/main_task2.cpp src/dynamic_array.cpp
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