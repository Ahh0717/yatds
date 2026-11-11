#!/usr/bin/env python3
"""任务 2 的随机差分测试：随机生成输入，比较 C++ 程序与 oracle 的输出。

用法：
    python3 tools/fuzz_task2.py <可执行文件> [轮数]

每轮交替使用 int / string，并轮流跑 circular / shift / linked 三种实现。
循环队列那几轮刻意用很小的容量、大量的 PUSH/POP 交替，尽量把
front_/back_ 推过缓冲区末尾，触发回绕。
"""

import random
import subprocess
import sys

import MyLab.Linear.Baseline.StackAndQueue.tools.oracle_task2 as oracle  # 同目录下的参考实现

OPS = ["PUSH", "POP", "FRONT", "BACK", "SIZE", "EMPTY", "PRINT", "CLEAR"]
IMPLS = ["circular", "shift", "linked"]


def gen(rng, numeric, capacity):
    values = [str(rng.randint(-9, 9))] if numeric else \
        rng.sample(["a", "b", "c", "ab", "bb", "0", "-1"], 5)

    # 初始元素个数不超过 capacity：数组实现装不下时驱动会拒绝输入。
    n = min(rng.choice([0, 0, 1, 2, 3, capacity]), capacity)
    init = [rng.choice(values) for _ in range(n)]

    lines = [f"{capacity} {n}", " ".join(init)]
    m = rng.randint(1, 50)
    lines.append(str(m))
    for _ in range(m):
        op = rng.choice(OPS)
        if op == "PUSH":
            lines.append(f"PUSH {rng.choice(values)}")
        else:
            lines.append(op)
    return "\n".join(lines) + "\n"


def strip_perf(text):
    return "".join(line for line in text.splitlines(keepends=True)
                   if not line.startswith("PERF "))


def main():
    exe = sys.argv[1]
    rounds = int(sys.argv[2]) if len(sys.argv) > 2 else 200
    rng = random.Random(20261032)
    for k in range(rounds):
        numeric = k % 2 == 0
        impl = IMPLS[(k // 2) % len(IMPLS)]
        # 小容量能更快把下标推到末尾，从而提高回绕覆盖率。
        capacity = rng.randint(0, 6)
        text = gen(rng, numeric, capacity)
        want = oracle.simulate(text, impl)

        args = [exe, f"--{impl}"]
        if not numeric:
            args.append("--string")
        proc = subprocess.run(args, input=text, capture_output=True, text=True)
        got = strip_perf(proc.stdout)
        if proc.returncode != 0:
            print(f"第 {k + 1} 轮退出码 {proc.returncode}（impl={impl}）")
            print(proc.stderr)
            print(text)
            return 1
        if got != want:
            print(f"第 {k + 1} 轮不一致（numeric={numeric}, impl={impl}）")
            print("--- 输入 ---")
            print(text)
            print("--- 期望 ---")
            print(want)
            print("--- 实际 ---")
            print(got)
            return 1
    print(f"{rounds} 轮随机差分测试全部一致")
    return 0


if __name__ == "__main__":
    sys.exit(main())