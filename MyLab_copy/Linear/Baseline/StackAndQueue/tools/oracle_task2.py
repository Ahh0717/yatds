#!/usr/bin/env python3
"""任务 2 的独立参考实现：用 Python collections.deque 模拟队列，生成期望输出。

与 C++ 实现完全无关，避免“用被测程序证明自己正确”。
输入输出协议见 ../task2/README_task2.md。

用法：
    python3 oracle_task2.py < input.txt
    python3 oracle_task2.py --write                          # inputs/  -> outputs/
    python3 oracle_task2.py --write --impl linked \
        --inputs task2/capacity_inputs --outputs task2/capacity_outputs

impl 只影响容量语义：
  circular / shift —— capacity 是硬上限，满了继续 PUSH 就丢弃；
  linked           —— 忽略 capacity，永远压不满。
"""

import argparse
import os
import sys
from collections import deque

HERE = os.path.dirname(os.path.abspath(__file__))
TASK_DIR = os.path.join(HERE, "..", "task2")


def simulate(text, impl):
    tokens = text.split()
    pos = 0
    out = []

    def take():
        nonlocal pos
        if pos >= len(tokens):
            raise ValueError("输入意外结束")
        value = tokens[pos]
        pos += 1
        return value

    capacity = int(take())
    n = int(take())
    data = deque(take() for _ in range(n))
    limited = impl in ("circular", "shift")
    if limited and n > capacity:
        raise ValueError("初始元素个数超过 capacity，数组实现装不下")

    m = int(take())
    for _ in range(m):
        op = take()
        if op == "PUSH":
            value = take()
            if limited and len(data) >= capacity:
                continue
            data.append(value)
        elif op == "POP":
            out.append(data.popleft() if data else "ERROR")
        elif op == "FRONT":
            out.append(data[0] if data else "ERROR")
        elif op == "BACK":
            out.append(data[-1] if data else "ERROR")
        elif op == "SIZE":
            out.append(str(len(data)))
        elif op == "EMPTY":
            out.append("true" if not data else "false")
        elif op == "PRINT":
            out.append(" ".join(data) if data else "EMPTY")
        elif op == "CLEAR":
            data.clear()
        else:
            raise ValueError(f"未知操作 {op}")
    if pos != len(tokens):
        raise ValueError(f"多余 token: {tokens[pos:]}")
    return "\n".join(out) + ("\n" if out else "")


def write_all(names, inputs, outputs, impl):
    os.makedirs(outputs, exist_ok=True)
    for name in names:
        with open(os.path.join(inputs, name), encoding="utf-8") as f:
            text = f.read()
        result = simulate(text, impl)
        with open(os.path.join(outputs, name), "w", encoding="utf-8") as f:
            f.write(result)
        print(f"{name}: {len(result.splitlines())} 行")


def main():
    parser = argparse.ArgumentParser(description="任务 2 参考实现")
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--impl", choices=("circular", "shift", "linked"),
                        default="circular")
    parser.add_argument("--inputs", default=os.path.join(TASK_DIR, "inputs"))
    parser.add_argument("--outputs", default=os.path.join(TASK_DIR, "outputs"))
    args = parser.parse_args()

    if not args.write:
        sys.stdout.write(simulate(sys.stdin.read(), args.impl))
        return 0

    names = sorted(n for n in os.listdir(args.inputs) if n.endswith(".txt"))
    write_all(names, args.inputs, args.outputs, args.impl)
    return 0


if __name__ == "__main__":
    sys.exit(main())