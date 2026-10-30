#!/usr/bin/env python3
"""任务 2 的随机差分测试：生成随机输入，比较 C++ 程序与 oracle 的输出。

除了逐行比对结果，还会单独统计“容量报告是否与翻倍策略一致”——
这是任务 2 最容易写错、又最难从功能输出里看出来的部分。

用法：python3 tools/fuzz_task2.py <可执行文件> [轮数]
"""

import random
import subprocess
import sys

import oracle_task2

OPS = ["PUSH_BACK", "INSERT", "ERASE", "GET", "SET",
       "SIZE", "CAPACITY", "EMPTY", "PRINT", "CLEAR"]


def gen(rng, numeric):
    values = [str(rng.randint(-9, 9))] if numeric else \
        rng.sample(["a", "b", "c", "ab", "bb", "0", "-1"], 5)

    capacity = rng.randint(0, 8)
    n = rng.randint(0, capacity)
    init = [rng.choice(values) for _ in range(n)]

    lines = [f"{capacity} {n}", " ".join(init)]
    m = rng.randint(1, 50)
    lines.append(str(m))
    size = n
    for _ in range(m):
        op = rng.choice(OPS)
        if op == "PUSH_BACK":
            lines.append(f"PUSH_BACK {rng.choice(values)}")
            size += 1
        elif op == "INSERT":
            i = rng.randint(0, size + 1)  # 偶尔越界
            lines.append(f"INSERT {i} {rng.choice(values)}")
            if i <= size:
                size += 1
        elif op == "ERASE":
            i = rng.randint(0, size)      # 偶尔越界
            lines.append(f"ERASE {i}")
            if i < size:
                size -= 1
        elif op == "GET":
            lines.append(f"GET {rng.randint(0, max(size, 1))}")
        elif op == "SET":
            lines.append(f"SET {rng.randint(0, max(size, 1))} {rng.choice(values)}")
        elif op == "CLEAR":
            lines.append("CLEAR")
            size = 0
        else:
            lines.append(op)
    return "\n".join(lines) + "\n"


def expected(text):
    """复用 oracle 的 Array 类，避免复制一份容量逻辑。"""
    tokens = text.split()
    pos = 0

    def take():
        nonlocal pos
        tok = tokens[pos]
        pos += 1
        return tok

    capacity = int(take())
    n = int(take())
    arr = oracle_task2.Array(capacity, [take() for _ in range(n)])
    m = int(take())

    out = []
    for _ in range(m):
        op = take()
        if op == "PUSH_BACK":
            arr.push_back(take())
        elif op == "INSERT":
            i, x = int(take()), take()
            arr.insert(i, x)
        elif op == "ERASE":
            arr.erase(int(take()))
        elif op == "GET":
            i = int(take())
            out.append(arr.data[i] if 0 <= i < len(arr.data) else "ERROR")
        elif op == "SET":
            i, x = int(take()), take()
            if 0 <= i < len(arr.data):
                arr.data[i] = x
        elif op == "SIZE":
            out.append(str(len(arr.data)))
        elif op == "CAPACITY":
            out.append(str(arr.cap))
        elif op == "EMPTY":
            out.append("true" if not arr.data else "false")
        elif op == "PRINT":
            out.append(" ".join(arr.data) if arr.data else "EMPTY")
        elif op == "CLEAR":
            arr.clear()
    return "\n".join(out) + ("\n" if out else "")


def main():
    exe = sys.argv[1]
    rounds = int(sys.argv[2]) if len(sys.argv) > 2 else 300
    rng = random.Random(20261030)
    cap_checks = 0
    for k in range(rounds):
        numeric = k % 2 == 0
        text = gen(rng, numeric)
        want = expected(text)
        args = [exe] if numeric else [exe, "--string"]
        got = subprocess.run(args, input=text, capture_output=True,
                             text=True).stdout
        if got != want:
            print(f"第 {k + 1} 轮不一致（numeric={numeric}）")
            print("--- 输入 ---")
            print(text)
            print("--- 期望 ---")
            print(want)
            print("--- 实际 ---")
            print(got)
            return 1
        cap_checks += sum(1 for line in want.splitlines())
    print(f"{rounds} 轮随机差分测试全部一致（共比对 {cap_checks} 行输出）")
    return 0


if __name__ == "__main__":
    sys.exit(main())