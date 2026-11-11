#!/usr/bin/env python3
"""任务 1 的独立参考实现（oracle），用于生成 outputs/task1/ 下的期望输出。

它不追求效率，只追求语义直观，便于与 C++ 实现互相交叉验证。
用法：python3 tools/oracle_task1.py inputs/task1/case01.txt
"""

import sys


def main(path):
    with open(path, encoding="utf-8") as f:
        tokens = f.read().split()

    pos = 0

    def take():
        nonlocal pos
        if pos >= len(tokens):
            raise SystemExit(f"{path}: 输入提前结束")
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
        else:
            raise SystemExit(f"{path}: 未知操作 {op}")

    if out:
        print("\n".join(out))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    main(sys.argv[1])