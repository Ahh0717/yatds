#!/usr/bin/env python3
"""通用测试驱动：运行可执行文件并比对 stdout 与期望输出。

用法:
    check.py <exe> <test.in> <test.out>

- Windows 下若可执行文件缺少 .exe 后缀则先补上，再转绝对路径。
- 用 subprocess 运行 exe，argv 传入 test.in 路径，捕获它的 stdout。
- stdout 按行 split，与 test.out 逐行比对，忽略行尾空白和文件末尾空行。
- 全部一致返回 0；否则打印第一个不同行的行号、期望值、实际值并返回 1。
"""

import os
import subprocess
import sys


def normalize(lines):
    """去掉每行行尾空白，再去掉文件末尾的空行。"""
    stripped = [line.rstrip() for line in lines]
    while stripped and stripped[-1] == "":
        stripped.pop()
    return stripped


def main(argv):
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")

    if len(argv) != 4:
        print("usage: check.py <exe> <test.in> <test.out>", file=sys.stderr)
        return 2

    exe, test_in, test_out = argv[1], argv[2], argv[3]

    if os.name == "nt":
        if not exe.lower().endswith(".exe"):
            exe += ".exe"
        exe = os.path.abspath(exe)

    result = subprocess.run(
        [exe, test_in],
        capture_output=True,
        text=True,
        encoding="utf-8",
    )

    actual = normalize(result.stdout.splitlines())

    with open(test_out, "r", encoding="utf-8") as handle:
        expected = normalize(handle.read().splitlines())

    total = max(len(expected), len(actual))
    for index in range(total):
        want = expected[index] if index < len(expected) else "<no line>"
        got = actual[index] if index < len(actual) else "<no line>"
        if want != got:
            print("MISMATCH at line {}".format(index + 1))
            print("  expected: {!r}".format(want))
            print("  actual  : {!r}".format(got))
            return 1

    print("PASS: {}".format(test_out))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))