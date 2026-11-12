# MyLab / Linear / Baseline / String —— 字符串实验
#
# 一键入口：
#   make           编译 tools/ 下的跨任务可编译工具（当前暂无 C++ 源码，空转）
#   make test      依次进入 task1 / task2 / task3 跑各自的 make test
#   make bench     编译并运行 task2 的 bench（最坏情况性能取证）
#   make clean     删除本目录与 task1/2/3 的构建产物、Python 缓存
#
# 三个任务各自仍是独立提交物，可以单独构建与测试：
#   cd task1 && make && make test
#   cd task2 && make && make test
#   cd task3 && make && make test
#
# 跨平台约定：
#   - 在 Windows 上一律用 mingw32-make，语义与 make 相同；
#   - 目标名统一带 $(EXEEXT)，Windows 下是 .exe，其他平台为空；
#   - recipe 尽量只用一条命令，不写 shell 变量/循环/`&&`，
#     这样 cmd.exe 与 POSIX sh 都能执行；
#   - 子 make 一律用 $(MAKE)，保证 -n / -j / 变量覆盖向下传递。
#
# 注意：MAKEFLAGS += --no-print-directory 会同时关掉 "Entering directory"
# 提示；这里**不用**它，好让 make test 的三段输出能看出在哪个任务里跑。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -Iinclude

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
MKDIR   := mkdir -p
RM_TREE := rm -rf

BUILD := build

# 每个含 .cpp 的 tools/<name> 目录产出一个 build/<name>$(EXEEXT)。
# 用 wildcard + foreach 展开，不依赖 shell 通配。
TOOL_DIRS := $(patsubst tools/%/,%,$(wildcard tools/*/))
CXX_TOOLS := $(foreach d,$(TOOL_DIRS),\
               $(if $(wildcard tools/$(d)/*.cpp),$(d),))
TOOL_BINS := $(foreach d,$(CXX_TOOLS),$(BUILD)/$(d)$(EXEEXT))

.PHONY: all tools test bench clean $(CXX_TOOLS)

all: tools

# tools/ 现在全是 Python（oracle / fuzz / 全区间穷举 / 大样例），没有可编译源码。
# 这里保留通用规则：将来往 tools/<name>/ 放 .cpp 就能自动产出 build/<name>。
tools: $(TOOL_BINS)
ifeq ($(strip $(CXX_TOOLS)),)
	@echo "tools/ 下暂无可编译的 C++ 源码（oracle/fuzz 等均为 Python，无需编译）。"
endif

# 每个工具的链接规则；$(call) 展开成静态目标，便于 -j 并行。
define TOOL_RULE
$(BUILD)/$(1)$(EXEEXT): $$(wildcard tools/$(1)/*.cpp) | $(BUILD)
	$$(CXX) $$(CXXFLAGS) $$(CPPFLAGS) $$(LDFLAGS) -o $$@ $$(wildcard tools/$(1)/*.cpp)
endef
$(foreach d,$(CXX_TOOLS),$(eval $(call TOOL_RULE,$(d))))

$(BUILD):
	@-$(MKDIR) $(BUILD)

# 逐个进入任务目录跑 make test。
# 写成多条独立 recipe，而不是 shell 的 for 循环或 `&&`：cmd.exe 与 POSIX sh
# 都能执行；任一任务返回非 0 时 make 会立即停下，后面的任务不再继续。
test:
	@echo ===== task1：顺序字符串 MyString =====
	@$(MAKE) -C task1 test
	@echo ===== task2：朴素 / KMP / nextval 模式匹配 =====
	@$(MAKE) -C task2 test
	@echo ===== task3：双链 DNA 编码片段检索 =====
	@$(MAKE) -C task3 test
	@echo ===== 三个任务全部通过 =====

bench:
	@$(MAKE) -C task2 bench

# 用 `-` 忽略尚未创建的目录，避免早期阶段报错；现在三个目录都已存在。
clean:
	-$(MAKE) -C task1 clean
	-$(MAKE) -C task2 clean
	-$(MAKE) -C task3 clean
	-$(RM_TREE) $(BUILD)
	-$(RM_TREE) tools/__pycache__