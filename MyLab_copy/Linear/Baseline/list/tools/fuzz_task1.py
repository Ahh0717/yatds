#!/usr/bin/env python3
"""随机差分测试：生成随机输入，比较 C++ 程序与 oracle 的输出。

用法：python3 tools/fuzz.py <可执行文件> [轮数]
"""

import random
import subprocess
import sys

import MyLab.Linear.Baseline.list.tools.oracle_task1 as oracle  # 同目录下的参考实现

OPS = ["PUSH_FRONT", "PUSH_BACK", "INSERT", "ERASE",
       "ACCESS", "FIND", "CONTAINS", "SIZE", "PRINT", "CLEAR"]


def gen(rng, numeric):
    values = [str(rng.randint(-9, 9))] if numeric else \
        rng.sample(["a", "b", "c", "ab", "bb", "0", "-1"], 5)

    n = rng.choice([0, 0, 1, 2, 3, 5])
    init = [rng.choice(values) for _ in range(n)]

    # 用“可能的最大下标 + 1”作为越界试探的基准。
    lines = [str(n), " ".join(init)]
    m = rng.randint(1, 40)
    lines.append(str(m))
    size = n
    for _ in range(m):
        op = rng.choice(OPS)
        if op in ("PUSH_FRONT", "PUSH_BACK"):
            lines.append(f"{op} {rng.choice(values)}")
            size += 1
        elif op == "INSERT":
            i = rng.randint(0, max(size, 0) + 1)  # 偶尔越界
            lines.append(f"INSERT {i} {rng.choice(values)}")
            if i <= size:
                size += 1
        elif op == "ERASE":
            i = rng.randint(0, max(size, 0))      # 偶尔越界
            lines.append(f"ERASE {i}")
            if i < size:
                size -= 1
        elif op == "ACCESS":
            lines.append(f"ACCESS {rng.randint(0, max(size, 1))}")
        elif op == "FIND":
            lines.append(f"FIND {rng.choice(values)}")
        elif op == "CONTAINS":
            lines.append(f"CONTAINS {rng.choice(values)}")
        elif op == "CLEAR":
            lines.append("CLEAR")
            size = 0
        else:
            lines.append(op)
    return "\n".join(lines) + "\n"


def expected(text, tmp="tools/.fuzz_case.txt"):
    with open(tmp, "w", encoding="utf-8") as f:
        f.write(text)
    tokens = text.split()
    pos = 0

    def take():
        nonlocal pos
        tok = tokens[pos]
        pos += 1
        return tok

    n = int(take())
    data = [take() for _ in range(n)]
    m = int(take())
    out = []
    for _ in range(m):
        op = take()
        if op == "PUSH_FRONT":
            data.insert(0, take())
        elif op == "PUSH_BACK":
            data.append(take())
        elif op == "INSERT":
            i, x = int(take()), take()
            if 0 <= i <= len(data):
                data.insert(i, x)
        elif op == "ERASE":
            i = int(take())
            if 0 <= i < len(data):
                del data[i]
        elif op == "ACCESS":
            i = int(take())
            out.append(data[i] if 0 <= i < len(data) else "ERROR")
        elif op == "FIND":
            x = take()
            out.append(str(data.index(x)) if x in data else "-1")
        elif op == "CONTAINS":
            out.append("true" if take() in data else "false")
        elif op == "SIZE":
            out.append(str(len(data)))
        elif op == "PRINT":
            out.append(" ".join(data) if data else "EMPTY")
        elif op == "CLEAR":
            data.clear()
    return "\n".join(out) + ("\n" if out else "")


def main():
    exe = sys.argv[1]
    rounds = int(sys.argv[2]) if len(sys.argv) > 2 else 300
    rng = random.Random(20261030)
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
    print(f"{rounds} 轮随机差分测试全部一致")
    return 0


if __name__ == "__main__":
    sys.exit(main())