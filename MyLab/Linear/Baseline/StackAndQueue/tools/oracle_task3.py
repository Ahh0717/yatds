#!/usr/bin/env python3
"""任务 3 的独立参考实现：用 Python list 模拟栈/队列，生成期望输出。

与 C++ 实现完全无关，避免“用被测程序证明自己正确”。
输入输出协议见 ../task3/README_task3.md。

用法：
    python3 oracle_task3.py < input.txt
    python3 oracle_task3.py --write
    python3 oracle_task3.py --write --inputs task3/string_inputs \
        --outputs task3/string_outputs

CONVERT 的两种方向结果都是把输入序列反转输出：
  QUEUE_TO_STACK  输入按队首 -> 队尾，输出按栈顶 -> 栈底；
  STACK_TO_QUEUE  输入按栈底 -> 栈顶，输出按队首 -> 队尾。
"""

import argparse
import os
import sys
from collections import deque

HERE = os.path.dirname(os.path.abspath(__file__))
TASK_DIR = os.path.join(HERE, "..", "task3")


def simulate(text):
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

    cases = int(take())
    for t in range(1, cases + 1):
        out.append(f"CASE {t}")
        kind = take()
        if kind == "STACK_BY_QUEUE":
            n = int(take())
            stack = []  # 尾部是栈顶
            # 初始元素按栈底 -> 栈顶依次 push
            for _ in range(n):
                stack.append(take())
            m = int(take())
            for _ in range(m):
                op = take()
                if op == "PUSH":
                    stack.append(take())
                elif op == "POP":
                    out.append(stack.pop() if stack else "ERROR")
                elif op == "TOP":
                    out.append(stack[-1] if stack else "ERROR")
                elif op == "PRINT":
                    out.append(" ".join(reversed(stack)) if stack else "EMPTY")
                else:
                    raise ValueError(f"未知栈操作 {op}")
        elif kind == "QUEUE_BY_STACK":
            n = int(take())
            queue = deque()
            for _ in range(n):
                queue.append(take())
            m = int(take())
            for _ in range(m):
                op = take()
                if op == "PUSH":
                    queue.append(take())
                elif op == "POP":
                    out.append(queue.popleft() if queue else "ERROR")
                elif op == "FRONT":
                    out.append(queue[0] if queue else "ERROR")
                elif op == "PRINT":
                    out.append(" ".join(queue) if queue else "EMPTY")
                else:
                    raise ValueError(f"未知队列操作 {op}")
        elif kind == "CONVERT":
            n = int(take())
            elements = [take() for _ in range(n)]
            direction = take()
            if direction not in ("QUEUE_TO_STACK", "STACK_TO_QUEUE"):
                raise ValueError(f"未知转换方向 {direction}")
            # 两种方向都是把输入反转后输出
            reversed_seq = list(reversed(elements))
            out.append(" ".join(reversed_seq) if reversed_seq else "EMPTY")
        else:
            raise ValueError(f"未知场景类型 {kind}")
    if pos != len(tokens):
        raise ValueError(f"多余 token: {tokens[pos:]}")
    return "\n".join(out) + ("\n" if out else "")


def write_all(names, inputs, outputs):
    os.makedirs(outputs, exist_ok=True)
    for name in names:
        with open(os.path.join(inputs, name), encoding="utf-8") as f:
            text = f.read()
        result = simulate(text)
        with open(os.path.join(outputs, name), "w", encoding="utf-8") as f:
            f.write(result)
        print(f"{name}: {len(result.splitlines())} 行")


def main():
    parser = argparse.ArgumentParser(description="任务 3 参考实现")
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--inputs", default=os.path.join(TASK_DIR, "inputs"))
    parser.add_argument("--outputs", default=os.path.join(TASK_DIR, "outputs"))
    args = parser.parse_args()

    if not args.write:
        sys.stdout.write(simulate(sys.stdin.read()))
        return 0

    names = sorted(n for n in os.listdir(args.inputs) if n.endswith(".txt"))
    write_all(names, args.inputs, args.outputs)
    return 0


if __name__ == "__main__":
    sys.exit(main())