#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""任务 3：**全区间穷举**核对“前缀和 / 二分计数”与暴力计数。

做法：对每个样例的 (DNA, P)，枚举所有满足 1 <= l <= r <= N 的区间，
把它们全部塞进一次运行，再逐条与暴力结果比较。暴力侧直接遍历匹配表，
判断 `l <= L and R <= r`——不使用任何前缀和/二分。

这样 N 较小时是**完全覆盖**，而不是抽样。样例之外还可以用 --dna/--protein
临时指定，或 --random 随机生成若干组再做穷举。

用法：
    python3 check_query_task3.py                     # 跑 task3/inputs/ 全部样例
    python3 check_query_task3.py --random 100        # 再加 100 组随机小用例
    python3 check_query_task3.py --exe build/task3 --dna AAAAAAAAA --protein KK
"""

import argparse
import os
import random
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
STRING_ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)

from oracle_task3 import CODON_TABLE, collect_matches  # noqa: E402

BASES = "ATCG"
AMINO = sorted(set(CODON_TABLE.values()) - {"*"})


def resolve_exe(path):
    if os.name == "nt" and not path.lower().endswith(".exe"):
        path += ".exe"
    if not os.path.isabs(path):
        path = os.path.join(STRING_ROOT, "task3", path)
    return os.path.abspath(path)


def all_intervals(n):
    return [(l, r) for l in range(1, n + 1) for r in range(l, n + 1)]


def run_case(exe, dna, protein, queries):
    """跑一次程序，返回答案列表；失败抛 RuntimeError。"""
    lines = [dna, protein, str(len(queries))]
    lines += [f"{l} {r}" for (l, r) in queries]
    stdin_data = "\n".join(lines) + "\n"
    proc = subprocess.run([exe], input=stdin_data, capture_output=True,
                          text=True, encoding="utf-8")
    if proc.returncode != 0:
        raise RuntimeError(f"退出码 {proc.returncode}: {proc.stderr.strip()}")
    text = proc.stdout.splitlines()
    if len(text) < 2:
        raise RuntimeError("输出行数不足")
    k = int(text[1])
    if len(text) != 2 + k + len(queries):
        raise RuntimeError(f"输出行数不对：{len(text)} != {2 + k + len(queries)}")
    return [int(x) for x in text[2 + k:]]


def brute_answers(matches, queries):
    return [sum(1 for (L, R, _s, _f) in matches if l <= L and R <= r)
            for (l, r) in queries]


def check_one(exe, dna, protein, label):
    queries = all_intervals(len(dna))
    got = run_case(exe, dna, protein, queries)
    matches = collect_matches(dna, protein)
    want = brute_answers(matches, queries)
    if got != want:
        for i, (g, w) in enumerate(zip(got, want)):
            if g != w:
                l, r = queries[i]
                return (False, f"{label}: 区间 [{l},{r}] 前缀和={g} 暴力={w}"
                               f"（K={len(matches)}）")
        return (False, f"{label}: 答案条数不同 {len(got)} != {len(want)}")
    return (True, f"{label}: {len(queries)} 个区间全部一致（K={len(matches)}）")


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    parser = argparse.ArgumentParser(description="任务 3 全区间穷举核对")
    parser.add_argument("--exe", default="build/task3", help="被测程序")
    parser.add_argument("--inputs", default=os.path.join(STRING_ROOT, "task3",
                                                         "inputs"),
                        help="样例目录")
    parser.add_argument("--random", type=int, default=0, help="额外随机用例数")
    parser.add_argument("--seed", type=int, default=20261111)
    parser.add_argument("--dna", help="直接指定一条 DNA")
    parser.add_argument("--protein", help="直接指定一条 P")
    args = parser.parse_args()

    exe = resolve_exe(args.exe)
    if not os.path.isfile(exe):
        print(f"找不到程序 {exe}，请先在 task3/ 下运行 make")
        return 1

    cases = []
    if args.dna is not None:
        if args.protein is None:
            print("指定 --dna 时必须同时给出 --protein")
            return 1
        cases.append((args.dna, args.protein, "命令行"))
    # 给了 --dna 就只测这一条，否则默认跑 inputs/ 下全部样例。
    if args.dna is None and os.path.isdir(args.inputs):
        for name in sorted(n for n in os.listdir(args.inputs)
                           if n.endswith(".txt")):
            with open(os.path.join(args.inputs, name), encoding="utf-8") as f:
                tokens = f.read().split()
            if len(tokens) >= 2:
                cases.append((tokens[0], tokens[1], name))

    rng = random.Random(args.seed)
    for i in range(args.random):
        n = rng.randint(1, 18)
        dna = "".join(rng.choice(BASES) for _ in range(n))
        protein = "".join(rng.choice(AMINO) for _ in range(rng.randint(1, 3)))
        cases.append((dna, protein, f"随机#{i}"))

    failures = 0
    total_intervals = 0
    for (dna, protein, label) in cases:
        ok, detail = check_one(exe, dna, protein, label)
        total_intervals += len(all_intervals(len(dna)))
        print(("PASS " if ok else "FAIL ") + detail)
        failures += 0 if ok else 1

    print("-" * 60)
    print(f"合计 {len(cases)} 组，全区间共 {total_intervals} 条，"
          f"{len(cases) - failures} 组通过")
    if failures:
        return 1
    print("前缀和答案与暴力计数完全一致")
    return 0


if __name__ == "__main__":
    sys.exit(main())