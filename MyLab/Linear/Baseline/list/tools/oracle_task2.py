#!/usr/bin/env python3
"""任务 2 的独立参考实现（oracle），用于生成 outputs/task2/ 下的期望输出。

关键点：容量策略必须由这里独立模拟，才能验证被测程序真的做了“翻倍扩容”
与“CLEAR 不缩容”，而不是碰巧输出对了。

用法：python3 tools/oracle_task2.py inputs/task2/case01.txt
"""

import sys

MIN_CAPACITY = 4


class Array:
    def __init__(self, capacity, values):
        # 初始容量不低于最小容量，装载初始元素时容量不足就翻倍。
        self.cap = max(capacity, MIN_CAPACITY)
        self.data = list(values)
        while len(self.data) > self.cap:
            self.cap *= 2

    def grow(self, need):
        while need > self.cap:
            self.cap *= 2

    def push_back(self, x):
        self.grow(len(self.data) + 1)
        self.data.append(x)

    def insert(self, i, x):
        if not 0 <= i <= len(self.data):
            return False
        self.grow(len(self.data) + 1)
        self.data.insert(i, x)
        return True

    def erase(self, i):
        if not 0 <= i < len(self.data):
            return False
        del self.data[i]
        return True

    def clear(self):
        self.data.clear()  # 只清元素，cap 不变


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

    capacity = int(take())
    n = int(take())
    values = [take() for _ in range(n)]
    arr = Array(capacity, values)
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
        else:
            raise SystemExit(f"{path}: 未知操作 {op}")

    if out:
        print("\n".join(out))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    main(sys.argv[1])