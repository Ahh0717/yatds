# MyLab / Linear / StackAndQueue —— 栈与队列实验
#
# 三个阶段各自是一个独立提交物，各自带 Makefile、test 与 README：
#
#   cd task1 && make && make test      # 数组栈 + 链式栈
#   cd task2 && make && make test      # 循环队列 / 平移队列 / 链式队列
#   cd task3 && make && make test      # 栈队列互转 + <vector> 版转换
#
# 本 Makefile 只构建**不属于单个任务**的跨任务工具：
#
#   make           编译 build/invariants（不变量检查）
#   make test      依次 make -C task1/2/3 test，再跑不变量检查
#   make fuzz      随机差分测试（需要 Python 3，非必需）
#   make sanitize  ASan/UBSan 下重跑样例与模糊测试（仅 Linux/macOS）
#   make clean     删除本目录与 task1/2/3 的构建产物
#
# 跨平台约定：
#   - 目标名统一带 $(EXEEXT)，Windows 下是 .exe，其他平台为空；
#   - 每个 recipe 只用一条命令，不写 shell 变量/循环/`&&`，
#     这样 Windows 的 cmd.exe 与 POSIX 的 sh 都能正确执行；
#   - 子 make 一律用 $(MAKE)，保证 -n / -j 等选项向下传递。
#
# 性能统计不在这里做：各 main_taskN.cpp 的 benchmark() 会在功能输出后
# 追加 PERF 行，所以没有 make bench 目标。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -Iinclude -Itask1/src -Itask2/src -Itask3/src

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
MAKEFLAGS += --no-print-directory

BUILD := build
INVAR := $(BUILD)/invariants$(EXEEXT)

TASK_DIRS := task1 task2 task3
TASK1_SRCS := task1/src/stackar.cpp task1/src/stackli.cpp
TASK2_SRCS := task2/src/queue.cpp task2/src/shift_queue.cpp task2/src/linked_queue.cpp
TASK3_SRCS := task3/src/stack_by_queue.cpp task3/src/queue_by_stack.cpp \
              task3/src/vector_stack.cpp task3/src/vector_queue.cpp
TASK_SRCS := $(TASK1_SRCS) $(TASK2_SRCS) $(TASK3_SRCS)
HEADERS := include/input_util.h \
           $(wildcard task1/src/*.hpp) \
           $(wildcard task2/src/*.hpp) $(wildcard task2/src/*.h) \
           $(wildcard task3/src/*.hpp) $(wildcard task3/src/*.h)

.PHONY: all test fuzz sanitize clean

all: $(INVAR)

$(BUILD):
	@-$(MKDIR) $(BUILD)

$(INVAR): tools/invariants.cpp $(TASK_SRCS) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ tools/invariants.cpp $(TASK_SRCS)

# ---------------------------------------------------------------------
# 测试：三个任务各按自己的 Makefile 构建并测试，最后跑跨任务的不变量检查。
#
# 不用 for 循环把三行写死，是为了让 Windows 的 cmd.exe 也能逐条执行；
# 每条都是独立 recipe 行，任何一行失败就停下（这是 make 的默认行为）。
# ---------------------------------------------------------------------
test:
	$(MAKE) -C task1 test
	$(MAKE) -C task2 test
	$(MAKE) -C task3 test
	@echo == 算法级不变量检查 ==
	$(MAKE) $(INVAR)
	$(abspath $(INVAR))

# ---------------------------------------------------------------------
# 随机差分测试：生成随机输入，与 Python 参考实现逐行比对。
# ---------------------------------------------------------------------
fuzz:
	$(MAKE) -C task1
	$(MAKE) -C task2
	$(MAKE) -C task3
	$(PYTHON) tools/fuzz_task1.py ./task1/$(BUILD)/task1$(EXEEXT) 200
	$(PYTHON) tools/fuzz_task2.py ./task2/$(BUILD)/task2$(EXEEXT) 200
	$(PYTHON) tools/fuzz_task3.py ./task3/$(BUILD)/task3$(EXEEXT) 200

# ---------------------------------------------------------------------
# Sanitizer：只有 GCC/Clang 且非 Windows 才有意义。
# ---------------------------------------------------------------------
SANFLAGS := -std=c++17 -g -fsanitize=address,undefined -fno-omit-frame-pointer

$(BUILD)/task1_san$(EXEEXT): task1/src/main_task1.cpp $(TASK1_SRCS) $(HEADERS) | $(BUILD)
	$(CXX) $(SANFLAGS) $(CPPFLAGS) -o $@ task1/src/main_task1.cpp $(TASK1_SRCS)

$(BUILD)/task2_san$(EXEEXT): task2/src/main_task2.cpp $(TASK2_SRCS) $(HEADERS) | $(BUILD)
	$(CXX) $(SANFLAGS) $(CPPFLAGS) -o $@ task2/src/main_task2.cpp $(TASK2_SRCS)

$(BUILD)/task3_san$(EXEEXT): task3/src/main_task3.cpp $(TASK3_SRCS) $(HEADERS) | $(BUILD)
	$(CXX) $(SANFLAGS) $(CPPFLAGS) -o $@ task3/src/main_task3.cpp $(TASK3_SRCS)

$(BUILD)/invariants_san$(EXEEXT): tools/invariants.cpp $(TASK_SRCS) $(HEADERS) | $(BUILD)
	$(CXX) $(SANFLAGS) $(CPPFLAGS) -o $@ tools/invariants.cpp $(TASK_SRCS)

# taskN/test 会 chdir 到 taskN/，所以 --exe 与样例目录都传绝对路径。
sanitize: $(BUILD)/task1_san$(EXEEXT) $(BUILD)/task2_san$(EXEEXT) \
          $(BUILD)/task3_san$(EXEEXT) $(BUILD)/invariants_san$(EXEEXT)
	@echo == array_stack ==
	$(PYTHON) task1/test --exe $(abspath $(BUILD)/task1_san$(EXEEXT)) \
		--inputs $(abspath task1/inputs) --outputs $(abspath task1/outputs)
	@echo == linked_stack ==
	$(PYTHON) task1/test --exe $(abspath $(BUILD)/task1_san$(EXEEXT)) \
		--inputs $(abspath task1/linked_inputs) \
		--outputs $(abspath task1/linked_outputs) --arg=--linked
	@echo == circular_queue ==
	$(PYTHON) task2/test --exe $(abspath $(BUILD)/task2_san$(EXEEXT)) \
		--inputs $(abspath task2/inputs) --outputs $(abspath task2/outputs)
	@echo == shift_queue ==
	$(PYTHON) task2/test --exe $(abspath $(BUILD)/task2_san$(EXEEXT)) \
		--inputs $(abspath task2/inputs) --outputs $(abspath task2/outputs) \
		--arg=--shift
	@echo == capacity_edges ==
	$(PYTHON) task2/test --exe $(abspath $(BUILD)/task2_san$(EXEEXT)) \
		--inputs $(abspath task2/capacity_inputs) \
		--outputs $(abspath task2/capacity_outputs)
	@echo == linked_queue ==
	$(PYTHON) task2/test --exe $(abspath $(BUILD)/task2_san$(EXEEXT)) \
		--inputs $(abspath task2/linked_inputs) \
		--outputs $(abspath task2/linked_outputs) --arg=--linked
	@echo == task3_int ==
	$(PYTHON) task3/test --exe $(abspath $(BUILD)/task3_san$(EXEEXT)) \
		--inputs $(abspath task3/inputs) --outputs $(abspath task3/outputs)
	@echo == task3_string ==
	$(PYTHON) task3/test --exe $(abspath $(BUILD)/task3_san$(EXEEXT)) \
		--inputs $(abspath task3/string_inputs) \
		--outputs $(abspath task3/string_outputs) --arg=--string
	@echo == invariants ==
	./$(BUILD)/invariants_san$(EXEEXT)
	$(PYTHON) tools/fuzz_task1.py ./$(BUILD)/task1_san$(EXEEXT) 100
	$(PYTHON) tools/fuzz_task2.py ./$(BUILD)/task2_san$(EXEEXT) 100
	$(PYTHON) tools/fuzz_task3.py ./$(BUILD)/task3_san$(EXEEXT) 60

# ---------------------------------------------------------------------
clean:
	$(MAKE) -C task1 clean
	$(MAKE) -C task2 clean
	$(MAKE) -C task3 clean
	-$(RM_TREE) $(BUILD)
	-$(RM_TREE) tools/__pycache__