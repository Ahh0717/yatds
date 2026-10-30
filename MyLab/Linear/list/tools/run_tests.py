#!/usr/bin/env python3
"""遍历 inputs/ 下的样例，按文件名前缀分派到对应任务的程序，与 outputs/ 比对。

分派规则（看文件名前缀，不猜内容）：
    task1_*.txt  ->  build/task1
    task2_*.txt  ->  build/task2
    task3_*.txt  ->  build/task3

比对规则：
  - 逐行比较，行尾空白与文件末尾空行不计；
  - 以 "PERF " 开头的行（性能统计，耗时每次不同）比较时忽略；
  - 程序退出码非 0 视为 FAIL，并把 stderr 摘要打印出来。

退出码：全部 PASS 为 0，否则为 1。
"""

import os
import re
import subprocess
import sys

TASK_PATTERN = re.compile(r"^(task[1-9]\d*)_")
PERF_PREFIX = "PERF "


def tasks_available(build_dir):
    """列出任务程序名 -> 路径。

    默认使用 build/taskN；可通过环境变量 TASK1/TASK2/... 覆盖
    （make sanitize 借此指向 ASan 版本的程序）。
    """
    exes = {}
    for name in os.listdir(build_dir):
        path = os.path.join(build_dir, name)
        if not (name.startswith("task") and os.path.isfile(path)
                and os.access(path, os.X_OK)):
            continue
        if name.endswith("_san"):
            continue
        exes[name] = os.environ.get(name.upper(), path)
    return exes


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
        return False, f"退出码 {proc.returncode}" + (f"; stderr: {' | '.join(detail)}" if detail else "")

    diff = first_difference(expected, actual)
    if diff is not None:
        line_no, want, got = diff
        return False, f"第 {line_no} 行不同: 期望 [{want}] 实际 [{got}]"
    return True, f"{len(expected)} 行一致"


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    inputs_dir = os.path.join(root, "inputs")
    outputs_dir = os.path.join(root, "outputs")
    build_dir = os.path.join(root, "build")

    if not os.path.isdir(build_dir):
        print("找不到 build/，请先运行 make")
        return 1

    exes = tasks_available(build_dir)
    if not exes:
        print("build/ 下没有任何任务程序，请先运行 make")
        return 1

    cases = sorted(n for n in os.listdir(inputs_dir) if n.endswith(".txt"))
    if not cases:
        print("inputs/ 下没有样例")
        return 1

    failures = []
    used = set()

    print(f"{'结果':<6}{'样例':<20}{'程序':<12}说明")
    print("-" * 72)

    for name in cases:
        match = TASK_PATTERN.match(name)
        task = match.group(1) if match else None
        expected_path = os.path.join(outputs_dir, name)

        if task is None:
            failures.append((name, "无法从文件名判断任务（应形如 task1_case01.txt）"))
            print(f"{'FAIL':<6}{name:<20}{'-':<12}文件名缺少 taskN_ 前缀")
            continue
        if task not in exes:
            failures.append((name, f"找不到程序 build/{task}"))
            print(f"{'FAIL':<6}{name:<20}{task:<12}找不到程序")
            continue
        if not os.path.isfile(expected_path):
            failures.append((name, f"找不到期望输出 outputs/{name}"))
            print(f"{'FAIL':<6}{name:<20}{task:<12}找不到期望输出")
            continue

        ok, detail = run_case(exes[task], os.path.join(inputs_dir, name), expected_path)
        used.add(task)
        if ok:
            print(f"{'PASS':<6}{name:<20}{task:<12}{detail}")
        else:
            failures.append((name, detail))
            print(f"{'FAIL':<6}{name:<20}{task:<12}{detail}")

    print("-" * 72)
    total = len(cases)
    passed = total - len(failures)
    print(f"汇总: {passed}/{total} PASS, {len(failures)} FAIL")

    # 有程序却没有任何样例覆盖，也提示出来（不算失败，但值得知道）。
    uncovered = sorted(set(exes) - used)
    if uncovered:
        print(f"提示: 以下程序没有对应样例: {', '.join(uncovered)}")

    if failures:
        print("\n失败明细:")
        for name, detail in failures:
            print(f"  {name}: {detail}")
        return 1
    print("全部通过")
    return 0


if __name__ == "__main__":
    sys.exit(main())