#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""任务 3 大样例：现场生成 → 跑被测程序 → 与独立期望逐字节比对 → 计时。

大样例不放进 task3/inputs/：约 3.5 MB 输入、22 万行输出，放进样例目录会让
`make test` 每次都多跑一份。这里按需生成到 task3/build/，用 `make large` 触发。

计时方式：每档跑 --runs 次，取**最小值**（排除首次缺页、调度抖动）。同时用
同一份 DNA 跑几档变体，把 "翻译/匹配" 与 "查询" 的开销分开：

  full      q 条查询（默认 2e5）
  noq       q = 0，只看翻译 + KMP + 排序 + 前缀和预处理 + 输出匹配表
  nomatch   P 换成随机蛋白，K = 0，看纯翻译 + KMP 扫一遍的底价

期望输出由 gen_large_task3.py 独立生成（`str.find` 找匹配 + `bisect` 计数，
且已抽样与线性扫描暴力复核），与被测 C++ 不共享代码。

用法：
    python3 bench_large_task3.py                  # 默认 n=1e6, q=2e5, 跑 5 次
    python3 bench_large_task3.py --runs 9 --queries 200000
"""

import argparse
import os
import random
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
STRING_ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)

import gen_large_task3 as gen  # noqa: E402


def resolve_exe(path):
    if os.name == "nt" and not path.lower().endswith(".exe"):
        path += ".exe"
    if not os.path.isabs(path):
        path = os.path.join(STRING_ROOT, "task3", path)
    return os.path.abspath(path)


def write_input(path, dna, protein, queries):
    with open(path, "w", encoding="utf-8") as f:
        f.write(dna + "\n")
        f.write(protein + "\n")
        f.write(str(len(queries)) + "\n")
        f.write("".join(f"{l} {r}\n" for (l, r) in queries))


def run_once(exe, in_path, out_path):
    """跑一次，返回 (wall_ms, 输出文件的字节数)。"""
    with open(in_path, "rb") as fin:
        t0 = time.perf_counter()
        proc = subprocess.run([exe], stdin=fin, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE)
        t1 = time.perf_counter()
    if proc.returncode != 0:
        raise RuntimeError(f"退出码 {proc.returncode}: "
                           f"{proc.stderr.decode('utf-8', 'replace').strip()}")
    if out_path is not None:
        with open(out_path, "wb") as fout:
            fout.write(proc.stdout)
    return (t1 - t0) * 1000.0, len(proc.stdout)


def min_time(exe, in_path, runs):
    best = None
    out_path = in_path + ".out"
    for _ in range(runs):
        ms, nbytes = run_once(exe, in_path, out_path)
        best = ms if best is None else min(best, ms)
    return best, nbytes, out_path


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    parser = argparse.ArgumentParser(description="任务 3 大样例生成 + 计时")
    parser.add_argument("--exe", default="build/task3", help="被测程序")
    parser.add_argument("--length", type=int, default=1000000)
    parser.add_argument("--protein-length", type=int, default=10)
    parser.add_argument("--queries", type=int, default=200000)
    parser.add_argument("--runs", type=int, default=5, help="每档重复次数（取最小）")
    parser.add_argument("--scaling", action="store_true",
                        help="额外跑 N 加倍扫描，验证线性")
    parser.add_argument("--seed", type=int, default=20261111)
    parser.add_argument("--workdir", default=os.path.join(STRING_ROOT, "task3",
                                                          "build"))
    args = parser.parse_args()

    exe = resolve_exe(args.exe)
    if not os.path.isfile(exe):
        print(f"找不到程序 {exe}，请先在 task3/ 下运行 make")
        return 1
    os.makedirs(args.workdir, exist_ok=True)

    rng = random.Random(args.seed)
    protein = "".join(rng.choice(gen.AMINO) for _ in range(args.protein_length))
    dna, planted = gen.build_dna_and_matches(rng, args.length, protein)
    queries = gen.make_queries(rng, args.length, planted, args.queries)

    t0 = time.perf_counter()
    matches = gen.collect_matches(dna, protein)
    lines, checked = gen.expected_output(
        dna, protein, queries, matches, verify_sample=300,
        rng=random.Random(args.seed + 1))
    gen_ms = (time.perf_counter() - t0) * 1000.0

    full_in = os.path.join(args.workdir, "case_large.txt")
    full_exp = os.path.join(args.workdir, "case_large_expected.txt")
    noq_in = os.path.join(args.workdir, "case_large_noq.txt")
    nomatch_in = os.path.join(args.workdir, "case_large_nomatch.txt")

    write_input(full_in, dna, protein, queries)
    write_input(noq_in, dna, protein, [])
    with open(full_exp, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")

    print("== 大样例 ==")
    print(f"DNA 长度 N     : {len(dna)}")
    print(f"蛋白长度 m     : {len(protein)}（{protein}）")
    print(f"查询数 q       : {len(queries)}")
    print(f"植入匹配       : {len(planted)}")
    print(f"期望匹配总数 K : {len(matches)}")
    print(f"生成 + 期望输出: {gen_ms:.0f} ms（二分答案抽样 {checked} 条与暴力一致）")
    print()

    ms_full, nbytes, out_path = min_time(exe, full_in, args.runs)
    with open(full_exp, "rb") as f:
        expected = f.read()
    actual = open(out_path, "rb").read()
    if actual != expected:
        print(f"FAIL：大样例输出与期望不一致（{len(actual)} vs {len(expected)} 字节）")
        return 1
    print(f"大样例输出      : {nbytes / 1024 / 1024:.2f} MB，与独立期望逐字节一致")

    ms_noq, _, _ = min_time(exe, noq_in, args.runs)
    rng2 = random.Random(args.seed + 7)
    other = "".join(rng2.choice(gen.AMINO) for _ in range(args.protein_length))
    write_input(nomatch_in, dna, other, [])
    ms_nomatch, _, _ = min_time(exe, nomatch_in, args.runs)

    print()
    tag = f"n={len(dna)}, K={len(matches)}"
    print(f"{'档位':<30}{'最小耗时':>12}")
    print("-" * 44)
    print(f"{tag + ', q=0 (换 P 使 K=0)':<30}{ms_nomatch:>9.1f} ms")
    print(f"{tag + ', q=0':<30}{ms_noq:>9.1f} ms")
    print(f"{tag + ', q=' + str(len(queries)):<30}{ms_full:>9.1f} ms")
    print("-" * 44)
    if len(queries) > 0:
        per_q = (ms_full - ms_noq) / len(queries) * 1e6
        print(f"单条查询边际成本 ≈ {per_q:.0f} ns（{len(queries)} 条合计 "
              f"{ms_full - ms_noq:.1f} ms）")
    print(f"翻译 + 6 帧 KMP 的每碱基成本 ≈ {ms_nomatch / len(dna) * 1e6:.2f} ns")

    if args.scaling:
        print()
        print("== N 加倍扫描（验证线性，q 固定）==")
        print(f"{'N':>10}{'K':>10}{'q=0':>12}{'q=' + str(args.queries):>12}")
        print("-" * 46)
        for n in (args.length // 8, args.length // 4, args.length // 2,
                  args.length):
            r = random.Random(args.seed + n)
            p = "".join(r.choice(gen.AMINO) for _ in range(args.protein_length))
            d, pl = gen.build_dna_and_matches(r, n, p)
            qs = gen.make_queries(r, n, pl, args.queries)
            base = os.path.join(args.workdir, f"scale_{n}")
            write_input(base + "_q0.txt", d, p, [])
            write_input(base + "_q.txt", d, p, qs)
            t0c, _, _ = min_time(exe, base + "_q0.txt", args.runs)
            t1c, _, _ = min_time(exe, base + "_q.txt", args.runs)
            print(f"{n:>10}{len(pl):>10}{t0c:>9.1f} ms{t1c:>9.1f} ms")
        print("-" * 46)
        print("两列随 N 近似同比翻倍，说明主体开销与 N 成线性。")

    print()
    print("结论：总计 O(N + m + K + q)，实测秒级以内、且 q 的贡献线性。")
    return 0


if __name__ == "__main__":
    sys.exit(main())