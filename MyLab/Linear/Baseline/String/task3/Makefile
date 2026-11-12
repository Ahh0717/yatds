# MyLab / Linear / Baseline / String / task3 —— 双链 DNA 目标蛋白编码片段检索
#
#   make         编译 build/task3（驱动）
#   make test    运行 test：自动读取 inputs/，与 outputs/ 逐行比对
#   make fuzz    随机差分：build/task3 vs ../tools/oracle_task3.py
#   make check   全区间穷举：前缀和答案 vs 暴力计数（13 个样例全覆盖）
#   make large   n=1e6、q=2e5 大样例：现场生成、逐字节比对、计时
#   make clean   删除构建产物
#
# 共用输入解析头文件在 ../include/，通过 -I../include 引用。
#
# 跨平台：目标名带 $(EXEEXT)；每条 recipe 只用一个命令，Windows 的
# cmd.exe 与 POSIX 的 sh 都能执行；test / fuzz 是 Python 脚本，统一用
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

BUILD  := build
EXE    := $(BUILD)/task3$(EXEEXT)
SRCS   := src/main_task3.cpp src/dna.cpp
DEPS   := src/dna.hpp ../include/input_util.h

.PHONY: all test fuzz check large clean

all: $(EXE)

$(BUILD):
	@-$(MKDIR) $(BUILD)

$(EXE): $(SRCS) $(DEPS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(SRCS)

test: $(EXE)
	@$(PYTHON) test

# 差分测试的 oracle 与 fuzz 放在 ../tools/，不复制进本目录，避免两份实现漂移。
fuzz: $(EXE)
	@$(PYTHON) ../tools/fuzz_task3.py --exe $(EXE)

# 全区间穷举：把每个样例的 N(N+1)/2 个区间一次喂进去，逐条与暴力对比。
check: $(EXE)
	@$(PYTHON) ../tools/check_query_task3.py --exe $(EXE)

# 大样例现场生成到 build/，跑完与被测程序逐字节比对，并打印分档耗时。
large: $(EXE)
	@$(PYTHON) ../tools/bench_large_task3.py --exe $(EXE)

clean:
	-$(RM_TREE) $(BUILD)
	-$(RM_TREE) __pycache__
	-$(RM_TREE) ../tools/__pycache__