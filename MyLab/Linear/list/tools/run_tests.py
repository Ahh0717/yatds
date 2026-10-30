#!/usr/bin/env python3
"""示例比对器：跑一个任务程序，和期望输出逐行比对。

这是三个任务共用的工具（不属于任何单个任务）。每个任务目录下的 test
脚本会用自己的程序路径、inputs/、outputs/ 调用它：

    ../tools/run_tests.py --exe build/task1 --inputs inputs --outputs outputs

比对规则：
  - 逐行比较，行尾空白与文件末尾空行不计；
  - 以 "PERF " 开头的行（性能统计，耗时每次不同）比较时忽略；
  - 程序退出码非 0 视为 FAIL，并把 stderr 摘要打印出来。

退出码：全部 PASS 为 0，否则为 1。
"""

import argparse
import os
import subprocess
import sys

PERF_PREFIX = "PERF "


def normalize(text):
    """按行切分并去除行尾空白、丢弃末尾空行；PERF 行剔除。"""
    lines = []
    for raw in text.splitlines():
        line = raw.rstrip()
        if line.startswith(PERF_PREFIX):
            continue
        lines.append(line)
    while lines and lines[-1] == "":
        lines.pop()
    return lines


def first_difference(expected, actual):
    for i in range(max(len(expected), len(actual))):
        want = expected[i] if i < len(expected) else "<没有这一行>"
        got = actual[i] if i < len(actual) else "<没有这一行>"
        if want != got:
            return i + 1, want, got
    return None


def run_case(exe, input_path, expected_path):
    with open(input_path, encoding="utf-8") as f:
        stdin_data = f.read()
    proc = subprocess.run([exe], input=stdin_data, capture_output=True, text=True)

    with open(expected_path, encoding="utf-8") as f:
        expected = normalize(f.read())
    actual = normalize(proc.stdout)

    if proc.returncode != 0:
        detail = proc.stderr.strip().splitlines()[:3]
        return False, f"退出码 {proc.returncode}" + (
            f"; stderr: {' | '.join(detail)}" if detail else "")
    diff = first_difference(expected, actual)
    if diff is not None:
        line_no, want, got = diff
        return False, f"第 {line_no} 行不同: 期望 [{want}] 实际 [{got}]"
    return True, f"{len(expected)} 行一致"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--exe", required=True, help="被测程序")
    parser.add_argument("--inputs", default="inputs", help="输入样例目录")
    parser.add_argument("--outputs", default="outputs", help="期望输出目录")
    args = parser.parse_args()

    if not os.path.isfile(args.exe):
        print(f"找不到程序 {args.exe}，请先运行 make")
        return 1
    if not os.path.isdir(args.inputs):
        print(f"找不到输入目录 {args.inputs}")
        return 1

    cases = sorted(n for n in os.listdir(args.inputs) if n.endswith(".txt"))
    if not cases:
        print(f"{args.inputs}/ 下没有样例")
        return 1

    failures = []
    print(f"{'结果':<6}{'样例':<20}{'说明'}")
    print("-" * 64)

    for name in cases:
        expected_path = os.path.join(args.outputs, name)
        if not os.path.isfile(expected_path):
            failures.append((name, f"找不到期望输出 {expected_path}"))
            print(f"{'FAIL':<6}{name:<20}找不到期望输出")
            continue

        ok, detail = run_case(args.exe, os.path.join(args.inputs, name), expected_path)
        if ok:
            print(f"{'PASS':<6}{name:<20}{detail}")
        else:
            failures.append((name, detail))
            print(f"{'FAIL':<6}{name:<20}{detail}")

    print("-" * 64)
    total = len(cases)
    passed = total - len(failures)
    print(f"汇总: {passed}/{total} PASS, {len(failures)} FAIL")

    if failures:
        print("\n失败明细:")
        for name, detail in failures:
            print(f"  {name}: {detail}")
        return 1
    print("全部通过")
    return 0


if __name__ == "__main__":
    sys.exit(main())