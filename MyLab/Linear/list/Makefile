# MyLab / Linear / list —— 跨任务工具
#
# 三个任务各自独立，各有自己的 Makefile：
#
#   cd task1 && make && make test
#   cd task2 && make && make test
#   cd task3 && make && make test
#
# 本 Makefile 只构建**不属于任何单个任务**的跨任务工具：
#
#   make           编译 build/bench 与 build/invariants
#   make test      依次构建三个任务并各跑一遍各自的 test，最后跑不变量检查
#   make bench     性能对比：DynamicArray / std::vector / LinkedList
#   make fuzz      随机差分测试（需要 Python 3）
#   make sanitize  ASan/UBSan 版本
#   make clean     删除本目录及三个任务目录的构建产物

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -Iinclude -Itask1/src -Itask2/src -Itask3/src
PYTHON   ?= python3

ifeq ($(OS),Windows_NT)
EXEEXT := .exe
else
EXEEXT :=
endif

BUILD := build
BENCH := $(BUILD)/bench$(EXEEXT)
INVAR := $(BUILD)/invariants$(EXEEXT)

TASK_DIRS := task1 task2 task3
TASK1_SRCS := task1/src/linked_list.cpp
TASK2_SRCS := task2/src/dynamic_array.cpp
TASK3_SRCS := task3/src/list_advanced.cpp
HEADERS := include/input_util.h $(wildcard task1/src/*.h) \
           $(wildcard task2/src/*.h) $(wildcard task3/src/*.h)

.PHONY: all test bench fuzz sanitize clean

all: $(BENCH) $(INVAR)

$(BUILD):
	@mkdir -p $(BUILD)

$(BENCH): tools/bench.cpp $(TASK1_SRCS) $(TASK2_SRCS) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ tools/bench.cpp $(TASK1_SRCS) $(TASK2_SRCS)

$(INVAR): tools/invariants.cpp $(TASK1_SRCS) $(TASK2_SRCS) $(TASK3_SRCS) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ tools/invariants.cpp \
		$(TASK1_SRCS) $(TASK2_SRCS) $(TASK3_SRCS)

# ---------------------------------------------------------------------
# 测试：三个任务各按自己的 Makefile 构建并测试，再跑跨任务的不变量检查
# ---------------------------------------------------------------------
test:
	@status=0; \
	for d in $(TASK_DIRS); do \
		echo "== $$d =="; \
		$(MAKE) -C $$d test || status=1; \
		echo; \
	done; \
	echo "== 算法级不变量检查（跨任务） =="; \
	$(MAKE) $(INVAR) >/dev/null && $(PYTHON) tools/run_invariants.py || status=1; \
	exit $$status

bench: $(BENCH)
	@./$(BENCH)

fuzz:
	@for d in $(TASK_DIRS); do $(MAKE) -C $$d >/dev/null; done
	@$(PYTHON) tools/fuzz_task1.py ./task1/build/task1 200
	@$(PYTHON) tools/fuzz_task2.py ./task2/build/task2 200
	@$(PYTHON) tools/fuzz_task3.py ./task3/build/task3 200

# ---------------------------------------------------------------------
# Sanitizer
# ---------------------------------------------------------------------
SANFLAGS := -std=c++17 -g -fsanitize=address,undefined -fno-omit-frame-pointer

$(BUILD)/task1_san$(EXEEXT): task1/src/main_task1.cpp $(TASK1_SRCS) $(HEADERS) | $(BUILD)
	$(CXX) $(SANFLAGS) $(CPPFLAGS) -o $@ task1/src/main_task1.cpp $(TASK1_SRCS)

$(BUILD)/task2_san$(EXEEXT): task2/src/main_task2.cpp $(TASK2_SRCS) $(HEADERS) | $(BUILD)
	$(CXX) $(SANFLAGS) $(CPPFLAGS) -o $@ task2/src/main_task2.cpp $(TASK2_SRCS)

$(BUILD)/task3_san$(EXEEXT): task3/src/main_task3.cpp $(TASK3_SRCS) $(HEADERS) | $(BUILD)
	$(CXX) $(SANFLAGS) $(CPPFLAGS) -o $@ task3/src/main_task3.cpp $(TASK3_SRCS)

sanitize: $(BUILD)/task1_san$(EXEEXT) $(BUILD)/task2_san$(EXEEXT) $(BUILD)/task3_san$(EXEEXT) $(INVAR)
	@for d in $(TASK_DIRS); do \
		$(PYTHON) tools/run_tests.py --exe ./$(BUILD)/$${d}_san$(EXEEXT) \
			--inputs $$d/inputs --outputs $$d/outputs || exit 1; \
	done
	@./$(INVAR)
	@ASAN_OPTIONS=detect_leaks=1 $(PYTHON) tools/fuzz_task1.py ./$(BUILD)/task1_san 100
	@ASAN_OPTIONS=detect_leaks=1 $(PYTHON) tools/fuzz_task2.py ./$(BUILD)/task2_san 100
	@ASAN_OPTIONS=detect_leaks=1 $(PYTHON) tools/fuzz_task3.py ./$(BUILD)/task3_san 100

# ---------------------------------------------------------------------
clean:
	rm -rf $(BUILD)
	@for d in $(TASK_DIRS); do $(MAKE) -C $$d clean; done
	rm -rf tools/__pycache__
	rm -f tools/.fuzz_case.txt tools/.fuzz_task3.txt