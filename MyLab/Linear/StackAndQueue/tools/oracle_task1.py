#!/usr/bin/env python3
"""任务 1 的独立参考实现：用 Python list 模拟栈，生成期望输出。

与 C++ 实现完全无关，避免“用被测程序证明自己正确”。
输入输出协议见 ../task1/README_task1.md。

用法：
    python3 oracle_task1.py < input.txt        # 输出期望结果（不含 PERF 行）
    python3 oracle_task1.py --write           # 重算 inputs/ -> outputs/
    python3 oracle_task1.py --write --impl linked \
        --inputs task1/linked_inputs --outputs task1/linked_outputs

样例分两套：`inputs/` 对应数组栈（容量是硬约束），`linked_inputs/`
对应链式栈（忽略 capacity，永远压不满）。两套目录各自有同名期望输出。
"""

import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TASK_DIR = os.path.join(HERE, "..", "task1")


def simulate(text, impl):
    """impl 为 'array' 时执行固定容量语义（满栈 push 静默丢弃）；
    'linked' 时忽略容量，永不溢出。"""
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
    data = [take() for _ in range(n)]
    limited = impl == "array"

    m = int(take())
    for _ in range(m):
        op = take()
        if op == "PUSH":
            value = take()
            if limited and len(data) >= capacity:
                continue
            data.append(value)
        elif op == "POP":
            out.append(data.pop() if data else "ERROR")
        elif op == "TOP":
            out.append(data[-1] if data else "ERROR")
        elif op == "SIZE":
            out.append(str(len(data)))
        elif op == "EMPTY":
            out.append("true" if not data else "false")
        elif op == "PRINT":
            out.append(" ".join(reversed(data)) if data else "EMPTY")
        elif op == "CLEAR":
            data.clear()
        elif op == "SETTOP":
            value = take()
            if data:
                data[-1] = value
        else:
            raise ValueError(f"未知操作 {op}")
    if pos != len(tokens):
        raise ValueError(f"多余 token: {tokens[pos:]}")
    return "\n".join(out) + ("\n" if out else "")


def check_initial_fits(text, impl):
    """数组栈语义下初始元素必须装得下；否则 C++ 驱动会拒绝输入。"""
    tokens = text.split()
    if impl == "array" and int(tokens[1]) > int(tokens[0]):
        raise ValueError("初始元素个数超过 capacity，数组栈无法装入")


def write_all(names, inputs, outputs, impl):
    os.makedirs(outputs, exist_ok=True)
    for name in names:
        path = os.path.join(inputs, name)
        with open(path, encoding="utf-8") as f:
            text = f.read()
        check_initial_fits(text, impl)
        result = simulate(text, impl)
        with open(os.path.join(outputs, name), "w", encoding="utf-8") as f:
            f.write(result)
        print(f"{name}: {len(result.splitlines())} 行")


def main():
    parser = argparse.ArgumentParser(description="任务 1 参考实现")
    parser.add_argument("--write", action="store_true",
                        help="重算样例并写入 outputs 目录")
    parser.add_argument("--impl", choices=("array", "linked"), default="array")
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