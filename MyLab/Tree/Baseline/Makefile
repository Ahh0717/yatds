# Tree Baseline —— 交付目录（甘锦珊_24325083_Tree）
#
# 两个任务各自独立，可以单独构建与测试：
#   cd forest  && make && make test
#   cd huffman && make && make test
#
# 一键入口：
#   make        构建两个任务的演示程序
#   make test   依次进入 forest / huffman 跑各自的 make test
#   make clean  递归清理两个任务的构建产物
#
# 跨平台约定（与各子目录一致）：
#   - 每条 recipe 只写一条命令，不用 shell 循环、变量或 &&，cmd.exe 也能跑；
#   - 子 make 一律用 $(MAKE)，保证 -n / -j / 变量覆盖向下传递。

.PHONY: all test test-forest test-huffman clean forest huffman

# 两个任务都要读写各自的 build/，且 test 必须按 forest → huffman 的次序执行，
# 因此整个文件不做并行（即使调用方传了 -j 也串行）。
.NOTPARALLEL:

all:
	@$(MAKE) -C forest
	@$(MAKE) -C huffman

forest:
	@$(MAKE) -C forest

huffman:
	@$(MAKE) -C huffman

# test 依赖两个独立目标，make 按顺序执行；forest 失败则立刻停止，不会跑到 huffman。
test: test-forest test-huffman

test-forest:
	@echo ===== forest: 森林与二叉树转换 =====
	@$(MAKE) -C forest test

test-huffman:
	@echo ===== huffman: 哈夫曼树与编码 =====
	@$(MAKE) -C huffman test

# 先清 huffman 再清 forest；前导 - 让某一个目录清理失败时仍继续清另一个。
clean:
	-@$(MAKE) -C huffman clean
	-@$(MAKE) -C forest clean