#!/usr/bin/env python3
"""任务 3 的随机差分测试：生成随机场景，比较 C++ 程序与 oracle 的输出。

重点覆盖：各种环的位置（自环、整链成环、中间成环）、prefix 为 0 的相交、
公共段长度为 1 的反转、空链表、值重复但不相交（防止“值相等即相交”的实现）。

用法：python3 tools/fuzz_task3.py <可执行文件> [轮数]
"""

import random
import subprocess
import sys

import oracle_task3


def gen_case(rng):
    kind = rng.choice(["CYCLE", "CYCLE", "NO_INTERSECTION", "INTERSECTION", "INTERSECTION"])
    lines = [kind]

    if kind == "CYCLE":
        n = rng.randint(0, 8)
        if n == 0:
            entry = -1
        else:
            entry = rng.choice([-1, -1, 0, n - 1, rng.randrange(n)])
        lines.append(f"{n} {entry}")
        lines.append(" ".join(str(rng.randint(1, 5)) for _ in range(n)))

    elif kind == "NO_INTERSECTION":
        nA = rng.randint(0, 6)
        lines.append(str(nA))
        lines.append(" ".join(str(rng.randint(1, 5)) for _ in range(nA)))
        nB = rng.randint(0, 6)
        lines.append(str(nB))
        lines.append(" ".join(str(rng.randint(1, 5)) for _ in range(nB)))

    else:
        nA = rng.randint(0, 5)
        lines.append(str(nA))
        lines.append(" ".join(str(rng.randint(1, 5)) for _ in range(nA)))
        nB = rng.randint(0, 5)
        lines.append(str(nB))
        lines.append(" ".join(str(rng.randint(1, 5)) for _ in range(nB)))
        k = rng.randint(1, 5)
        lines.append(str(k))
        lines.append(" ".join(str(rng.randint(1, 5)) for _ in range(k)))

    return lines


def main():
    exe = sys.argv[1]
    rounds = int(sys.argv[2]) if len(sys.argv) > 2 else 200
    rng = random.Random(20261030)

    for r in range(rounds):
        t = rng.randint(1, 4)
        lines = [str(t)]
        for _ in range(t):
            lines.extend(gen_case(rng))
        text = "\n".join(lines) + "\n"

        with open("tools/.fuzz_task3.txt", "w", encoding="utf-8") as f:
            f.write(text)

        import io
        import contextlib
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            oracle_task3.main("tools/.fuzz_task3.txt")
        want = buf.getvalue()

        got = subprocess.run([exe], input=text, capture_output=True,
                             text=True).stdout
        if got != want:
            print(f"第 {r + 1} 轮不一致")
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