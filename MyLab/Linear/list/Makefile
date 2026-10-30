# MyLab / Linear / list —— 线性表基础实验（三个阶段的整合构建）
#
#   make          编译全部程序（build/task1、task2、task3、bench 与 test）
#   make test     遍历 inputs/ 分派到对应任务程序，与 outputs/ 逐行比对，
#                 再跑算法级不变量检查，最后打印 PASS/FAIL 汇总
#   make clean    删除全部构建产物
#
# 其他目标：
#   make bench    性能对比：DynamicArray / std::vector / LinkedList
#   make fuzz     随机差分测试（需要 Python 3）
#   make sanitize ASan/UBSan 版本的服务于样例检查

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -Isrc
PYTHON   ?= python3

BUILD := build
SRC   := src

# 三个任务程序 + 性能对比 + 测试程序
TASKS  := $(BUILD)/task1 $(BUILD)/task2 $(BUILD)/task3
BENCH  := $(BUILD)/bench
TESTEXE  := test
TESTSTAMP:= $(BUILD)/.test-script

HEADERS := $(wildcard $(SRC)/*.h)

.PHONY: all test bench fuzz sanitize clean

# ---------------------------------------------------------------------
# 默认目标：编译全部程序，并把测试程序放在根目录名为 test
# ---------------------------------------------------------------------
all: $(TASKS) $(BENCH) $(BUILD)/invariants $(TESTSTAMP)

$(BUILD):
	@mkdir -p $(BUILD)

# 任务 1 / 2 / 3 的程序
$(BUILD)/task1: $(SRC)/main_task1.cpp $(SRC)/linked_list.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(SRC)/main_task1.cpp $(SRC)/linked_list.cpp

$(BUILD)/task2: $(SRC)/main_task2.cpp $(SRC)/dynamic_array.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(SRC)/main_task2.cpp $(SRC)/dynamic_array.cpp

$(BUILD)/task3: $(SRC)/main_task3.cpp $(SRC)/list_advanced.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(SRC)/main_task3.cpp $(SRC)/list_advanced.cpp

# 性能对比
$(BENCH): $(SRC)/bench.cpp $(SRC)/linked_list.cpp $(SRC)/dynamic_array.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(SRC)/bench.cpp $(SRC)/linked_list.cpp \
		$(SRC)/dynamic_array.cpp

# 不变量检查程序（C++，由 make test 一并调用）
$(BUILD)/invariants: $(SRC)/invariants.cpp $(SRC)/linked_list.cpp \
                     $(SRC)/dynamic_array.cpp $(SRC)/list_advanced.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(SRC)/invariants.cpp $(SRC)/linked_list.cpp \
		$(SRC)/dynamic_array.cpp $(SRC)/list_advanced.cpp

# 测试程序：一个 shell 包装，真实逻辑在 tools/run_tests.py。
# 用 stamp 文件作为目标名——目标若直接叫 test，会和下面的 phony 目标 test 冲突
# （make 会把它们当成同一个规则的两份配方而报警）。
$(TESTSTAMP): tools/run_tests.py tools/run_invariants.py | $(BUILD)
	@printf '%s\n' '#!/bin/sh' \
		'# 由 make 生成：分派 inputs/ 中的样例并逐行比对 outputs/。' \
		'cd "$$(dirname "$$0")"' \
		'status=0' \
		'$(PYTHON) tools/run_tests.py "$$@" || status=1' \
		'echo' \
		'echo "== 算法级不变量检查 =="' \
		'$(PYTHON) tools/run_invariants.py || status=1' \
		'exit $$status' > $(TESTEXE)
	@chmod +x $(TESTEXE)
	@touch $@

# ---------------------------------------------------------------------
# 测试
# ---------------------------------------------------------------------
test: all
	@./$(TESTEXE)

bench: $(BENCH)
	@./$(BENCH)

fuzz: all
	@$(PYTHON) tools/fuzz_task1.py ./$(BUILD)/task1 200
	@$(PYTHON) tools/fuzz_task2.py ./$(BUILD)/task2 200
	@$(PYTHON) tools/fuzz_task3.py ./$(BUILD)/task3 200

# ---------------------------------------------------------------------
# Sanitizer
# ---------------------------------------------------------------------
SANFLAGS := -std=c++17 -g -fsanitize=address,undefined -fno-omit-frame-pointer

$(BUILD)/task1_san: $(SRC)/main_task1.cpp $(SRC)/linked_list.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(SANFLAGS) $(CPPFLAGS) -o $@ $(SRC)/main_task1.cpp $(SRC)/linked_list.cpp

$(BUILD)/task2_san: $(SRC)/main_task2.cpp $(SRC)/dynamic_array.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(SANFLAGS) $(CPPFLAGS) -o $@ $(SRC)/main_task2.cpp $(SRC)/dynamic_array.cpp

$(BUILD)/task3_san: $(SRC)/main_task3.cpp $(SRC)/list_advanced.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(SANFLAGS) $(CPPFLAGS) -o $@ $(SRC)/main_task3.cpp $(SRC)/list_advanced.cpp

sanitize: $(BUILD)/task1_san $(BUILD)/task2_san $(BUILD)/task3_san
	@TASK1=./$(BUILD)/task1_san TASK2=./$(BUILD)/task2_san TASK3=./$(BUILD)/task3_san \
		$(PYTHON) tools/run_tests.py
	@$(BUILD)/invariants
	@ASAN_OPTIONS=detect_leaks=1 $(PYTHON) tools/fuzz_task1.py ./$(BUILD)/task1_san 100
	@ASAN_OPTIONS=detect_leaks=1 $(PYTHON) tools/fuzz_task2.py ./$(BUILD)/task2_san 100
	@ASAN_OPTIONS=detect_leaks=1 $(PYTHON) tools/fuzz_task3.py ./$(BUILD)/task3_san 100

# ---------------------------------------------------------------------
clean:
	rm -rf $(BUILD) $(TESTEXE)
	rm -rf tools/__pycache__
	rm -f tools/.fuzz_case.txt tools/.fuzz_task3.txt