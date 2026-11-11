#!/usr/bin/env python3
"""任务 3 的随机差分测试：随机生成多场景输入，比较 C++ 与 oracle 的输出。

用法：
    python3 tools/fuzz_task3.py <可执行文件> [轮数]

每轮随机 1-4 个场景，三种类型混排；int 与 string 交替。
StackByQueue 的 top/pop 是 O(n)，所以每轮元素规模刻意压得很小。
"""

import random
import subprocess
import sys

import MyLab.Linear.Baseline.StackAndQueue.tools.oracle_task3 as oracle  # 同目录下的参考实现


def gen_scene(rng, numeric, kind):
    values = [str(rng.randint(-9, 9))] if numeric else \
        rng.sample(["a", "b", "c", "ab", "bb", "0", "-1"], 5)
    n = rng.choice([0, 0, 1, 2, 3, 4])
    init = [rng.choice(values) for _ in range(n)]
    lines = [kind, str(n), " ".join(init)]

    if kind == "CONVERT":
        lines.append(rng.choice(["QUEUE_TO_STACK", "STACK_TO_QUEUE"]))
        return lines

    ops = ["PUSH", "POP", "PRINT", "PRINT"]
    ops.append("TOP" if kind == "STACK_BY_QUEUE" else "FRONT")
    m = rng.randint(1, 20)
    lines.append(str(m))
    for _ in range(m):
        op = rng.choice(ops)
        if op == "PUSH":
            lines.append(f"PUSH {rng.choice(values)}")
        else:
            lines.append(op)
    return lines


def gen(rng, numeric):
    scenes = []
    count = rng.randint(1, 4)
    for _ in range(count):
        kind = rng.choice(["STACK_BY_QUEUE", "QUEUE_BY_STACK", "CONVERT"])
        scenes.append(gen_scene(rng, numeric, kind))
    lines = [str(len(scenes))]
    for scene in scenes:
        lines.extend(scene)
    return "\n".join(lines) + "\n"


def strip_perf(text):
    return "".join(line for line in text.splitlines(keepends=True)
                   if not line.startswith("PERF "))


def main():
    exe = sys.argv[1]
    rounds = int(sys.argv[2]) if len(sys.argv) > 2 else 200
    rng = random.Random(20261033)
    for k in range(rounds):
        numeric = k % 2 == 0
        text = gen(rng, numeric)
        want = oracle.simulate(text)

        args = [exe] if numeric else [exe, "--string"]
        proc = subprocess.run(args, input=text, capture_output=True, text=True)
        got = strip_perf(proc.stdout)
        if proc.returncode != 0:
            print(f"第 {k + 1} 轮退出码 {proc.returncode}（numeric={numeric}）")
            print(proc.stderr)
            print(text)
            return 1
        if got != want:
            print(f"第 {k + 1} 轮不一致（numeric={numeric}）")
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