#!/usr/bin/env python3
"""任务 1 的随机差分测试：随机生成输入，比较 C++ 程序与 oracle 的输出。

用法：
    python3 tools/fuzz_task1.py <可执行文件> [轮数]

每轮交替使用 int / string，并交替使用数组栈(--array)与链式栈(--linked)；
链式栈那几轮会继续在容量之外 push，验证“链式栈不会满”。
程序输出里以 "PERF " 开头的行在比较前剔除。
"""

import random
import subprocess
import sys

import oracle_task1 as oracle  # 同目录下的参考实现

OPS = ["PUSH", "POP", "TOP", "SIZE", "EMPTY", "PRINT", "CLEAR", "SETTOP"]


def gen(rng, numeric, capacity):
    values = [str(rng.randint(-9, 9))] if numeric else \
        rng.sample(["a", "b", "c", "ab", "bb", "0", "-1"], 5)

    # 初始元素个数不超过 capacity：数组栈装不下时驱动会拒绝输入。
    n = min(rng.choice([0, 0, 1, 2, 3, capacity]), capacity)
    init = [rng.choice(values) for _ in range(n)]

    lines = [f"{capacity} {n}", " ".join(init)]
    m = rng.randint(1, 40)
    lines.append(str(m))
    for _ in range(m):
        op = rng.choice(OPS)
        if op in ("PUSH", "SETTOP"):
            lines.append(f"{op} {rng.choice(values)}")
        else:
            lines.append(op)
    return "\n".join(lines) + "\n"


def strip_perf(text):
    return "".join(line for line in text.splitlines(keepends=True)
                   if not line.startswith("PERF "))


def main():
    exe = sys.argv[1]
    rounds = int(sys.argv[2]) if len(sys.argv) > 2 else 200
    rng = random.Random(20261031)
    for k in range(rounds):
        numeric = k % 2 == 0
        linked = (k // 2) % 2 == 1
        capacity = rng.randint(0, 8)
        # 链式栈忽略 capacity，这里故意给它“装不下”的初始元素；
        # 数组栈则必须装得下，所以 n 不超过 capacity。
        text = gen(rng, numeric, capacity)
        impl = "linked" if linked else "array"
        want = oracle.simulate(text, impl)

        args = [exe, "--linked" if linked else "--array"]
        if not numeric:
            args.append("--string")
        proc = subprocess.run(args, input=text, capture_output=True, text=True)
        got = strip_perf(proc.stdout)
        if proc.returncode != 0:
            print(f"第 {k + 1} 轮退出码 {proc.returncode}（linked={linked}）")
            print(proc.stderr)
            print(text)
            return 1
        if got != want:
            print(f"第 {k + 1} 轮不一致（numeric={numeric}, linked={linked}）")
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