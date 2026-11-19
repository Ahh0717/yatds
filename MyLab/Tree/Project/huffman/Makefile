# huffman —— 基于哈夫曼树的简化 Deflate（甘锦珊_24325083_Tree / Project）
#
#   make        编译到 build/huffman$(EXEEXT)
#   make test   编译并跑 inputs/ 与 outputs/ 的比对
#   make clean  删除构建产物
#
# 跨平台约定：
#   - 目标名统一带 $(EXEEXT)，Windows 下是 .exe；
#   - Windows 分支静态链接，绕开 conda 等环境里旧 libstdc++ 的 0xC0000005；
#   - 每条 recipe 只写一条命令，不用 shell 循环、变量或 &&（cmd.exe 不支持）；
#   - mkdir 不带 -p（cmd.exe 会把 -p 当成目录名）；目录已存在时由 - 忽略报错；
#   - 删目录交给 Python 的 shutil.rmtree，cmd.exe 与 sh 下行为一致，
#     不会出现 rm 与 rmdir 对不上的问题。

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -Isrc

ifeq ($(OS),Windows_NT)
EXEEXT  := .exe
PYTHON  ?= python
LDFLAGS += -static
else
EXEEXT  :=
PYTHON  ?= python3
endif

MKDIR := mkdir
BUILD := build
TARGET := $(BUILD)/huffman$(EXEEXT)
SOURCES := src/huffman.cpp src/deflate.cpp src/main.cpp
HEADERS := src/huffman.h src/deflate.h

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(SOURCES)

$(BUILD):
	@-$(MKDIR) $(BUILD)

# test 是 Python 脚本，统一用 $(PYTHON) 运行（Windows 上没有执行位，
# 所以不写成 ./test）。脚本自己负责建/删 build/tmp。
test: $(TARGET)
	@$(PYTHON) test --exe $(TARGET)

# 用 Python 删目录，不依赖 shell 里有没有 rm 或 rmdir。
clean:
	-@$(PYTHON) -c "import shutil; shutil.rmtree('build', ignore_errors=True)"