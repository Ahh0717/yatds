#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""任务 3 的大样例生成器（n ≈ 1e6，q = 2e5）。

生成方式：在随机 DNA 背景上**定点植入**已知匹配。
  - 正向链：在 0-based 起点 s 处写入 P 的密码子编码，取 frame = s % 3 + 1，
    则该帧第 j = (s-(frame-1))/3 个密码子正好从 s 开始，映射回 [s+1, s+3m]；
  - 反向链：在 s 处写入 revcomp(P 的密码子编码)，取
    frame = (n-3m-s) % 3 + 1，则反向互补链在该帧对齐，映射回同一个 [s+1, s+3m]。
植入槽按 40 碱基步长排布（匹配本身占 3m = 30 碱基），互不重叠。

期望输出的计算**不复用 C++ 的任何代码**，也刻意不用 KMP：匹配用
`str.find` 逐次查找（CPython 自己的子串搜索），区间计数用 `bisect` 在
已排序的 L 列表上数点。植入位置由生成器独立记录，最后逐条核对
"每个植入点都出现在期望匹配表里"；另外随机抽若干查询用**线性扫描**暴力
复核前缀和/二分的答案。

用法：
    python3 gen_large_task3.py                       # 写默认文件并打印统计
    python3 gen_large_task3.py --length 1000000 --queries 200000
    python3 gen_large_task3.py --out /tmp/in.txt --expected /tmp/out.txt
"""

import argparse
import os
import random
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
STRING_ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)

from oracle_task3 import CODON_TABLE  # noqa: E402  密码子表与 oracle 共用（已单独核对）

BASES = "ATCG"
COMPLEMENT = {"A": "T", "T": "A", "C": "G", "G": "C"}
AMINO = sorted(set(CODON_TABLE.values()) - {"*"})
AA_TO_CODON = {}
for _codon, _aa in CODON_TABLE.items():
    AA_TO_CODON.setdefault(_aa, _codon)


def revcomp(dna):
    return "".join(COMPLEMENT[c] for c in reversed(dna))


def encode_protein(protein):
    return "".join(AA_TO_CODON[a] for a in protein)


def translate_frame(seq, frame):
    out = []
    i = frame - 1
    while i + 2 < len(seq):
        out.append(CODON_TABLE[seq[i:i + 3]])
        i += 3
    return "".join(out)


def find_all(text, pattern):
    """用 str.find 找全部（重叠）出现，不手写 KMP。"""
    res = []
    if not pattern:
        return res
    pos = text.find(pattern)
    while pos != -1:
        res.append(pos)
        pos = text.find(pattern, pos + 1)
    return res


def build_dna_and_matches(rng, n, protein):
    """返回 (dna, planted)；planted 为 [(L, R, strand, frame)]。"""
    m = len(protein)
    span = 3 * m
    stride = span + 10
    if stride * 2 > n:
        raise SystemExit("DNA 太短，放不下足够的植入槽")

    slot_count = (n - span) // stride
    slots = list(range(slot_count))
    rng.shuffle(slots)
    forward_slots = slots[: slot_count // 2]
    reverse_slots = slots[slot_count // 2:]

    dna = [rng.choice(BASES) for _ in range(n)]
    codons = encode_protein(protein)
    rc_codons = revcomp(codons)
    planted = []

    for slot in forward_slots:
        s = slot * stride
        frame = s % 3 + 1
        j = (s - (frame - 1)) // 3
        dna[s:s + span] = list(codons)
        planted.append((s + 1, s + span, "+", frame))
        # 断言植入点确实落在该帧的密码子边界上
        assert (frame - 1) + 3 * j == s

    for slot in reverse_slots:
        s = slot * stride
        frame = (n - span - s) % 3 + 1
        j = (n - span - s - (frame - 1)) // 3
        assert j >= 0
        dna[s:s + span] = list(rc_codons)
        planted.append((s + 1, s + span, "-", frame))

    return "".join(dna), planted


def collect_matches(dna, protein):
    """期望匹配表；与 oracle_task3.py 相同的坐标公式，但匹配用 str.find。"""
    n = len(dna)
    m = len(protein)
    span = 3 * m
    rc = revcomp(dna)
    matches = []
    for frame in (1, 2, 3):
        for j in find_all(translate_frame(dna, frame), protein):
            matches.append(((frame - 1) + 3 * j + 1, (frame - 1) + 3 * j + span,
                            "+", frame))
        for j in find_all(translate_frame(rc, frame), protein):
            matches.append((n - (frame - 1) - 3 * j - span + 1,
                            n - (frame - 1) - 3 * j, "-", frame))
    matches.sort(key=lambda t: (t[0], 0 if t[2] == "+" else 1, t[3]))
    return matches


def make_queries(rng, n, planted, q):
    """1/4 围绕植入匹配、1/4 短区间、1/4 全随机、1/4 边界。"""
    queries = []
    span = planted[0][1] - planted[0][0] + 1 if planted else 1
    for i in range(q):
        mode = i % 4
        if mode == 0 and planted:
            L, R, _s, _f = rng.choice(planted)
            kind = i % 8
            if kind == 0:
                l, r = L, R
            elif kind == 2:
                l, r = L, R - 1
            elif kind == 4:
                l, r = max(1, L - 1), R
            else:
                l, r = max(1, L - rng.randint(0, 5)), min(n, R + rng.randint(0, 5))
        elif mode == 1:
            l = rng.randint(1, n)
            r = min(n, l + rng.randint(0, span - 1))
        elif mode == 2:
            l = rng.randint(1, n)
            r = rng.randint(l, n)
        else:
            l, r = rng.choice([(1, n), (1, 1), (n, n), (1, span),
                               (n - span + 1, n)])
            l = max(1, min(l, n))
            r = max(l, min(r, n))
        queries.append((l, r))
    return queries


def expected_output(dna, protein, queries, matches, verify_sample=300, rng=None):
    """用二分求全部答案；并抽样用线性扫描暴力复核。返回 (lines, 复核条数)。"""
    m = len(protein)
    span = 3 * m - 1
    lefts = [t[0] for t in matches]

    import bisect
    answers = []
    for (l, r) in queries:
        hi = r - span
        if hi < l:
            answers.append(0)
            continue
        hi = min(hi, len(dna))
        lo = max(0, l - 1)
        answers.append(bisect.bisect_right(lefts, hi) - bisect.bisect_right(lefts, lo))

    # 抽样暴力复核：直接遍历所有匹配判断 l <= L 且 R <= r。
    checked = 0
    if rng is not None:
        idxs = rng.sample(range(len(queries)), min(verify_sample, len(queries)))
        for idx in idxs:
            l, r = queries[idx]
            brute = sum(1 for (L, R, _s, _f) in matches if l <= L and R <= r)
            if brute != answers[idx]:
                raise SystemExit(f"二分答案与暴力不一致：query#{idx} "
                                 f"[{l},{r}] bisect={answers[idx]} brute={brute}")
            checked += 1

    lines = [" ".join(str(x) for x in prefix_function(protein)),
             str(len(matches))]
    lines += [f"{L} {R} {s} {f}" for (L, R, s, f) in matches]
    lines += [str(a) for a in answers]
    return lines, checked


def prefix_function(p):
    """P 的 pi 数组（输出第 1 行），O(m)。"""
    m = len(p)
    pi = [0] * m
    for i in range(1, m):
        k = pi[i - 1]
        while k > 0 and p[i] != p[k]:
            k = pi[k - 1]
        if p[i] == p[k]:
            k += 1
        pi[i] = k
    return pi


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    parser = argparse.ArgumentParser(description="任务 3 大样例生成器")
    parser.add_argument("--length", type=int, default=1000000, help="DNA 长度")
    parser.add_argument("--protein-length", type=int, default=10, help="P 长度")
    parser.add_argument("--queries", type=int, default=200000, help="查询数")
    parser.add_argument("--seed", type=int, default=20261111, help="随机种子")
    # 大样例不进 inputs/（约 3 MB，且 `make test` 每次都会重跑）；
    # 默认写到 task3/build/，由 `make large` 现场生成并校验。
    parser.add_argument("--out", default=os.path.join(STRING_ROOT, "task3",
                                                     "build", "case_large.txt"))
    parser.add_argument("--expected", default=os.path.join(
        STRING_ROOT, "task3", "build", "case_large_expected.txt"))
    args = parser.parse_args()

    t0 = time.perf_counter()
    rng = random.Random(args.seed)
    protein = "".join(rng.choice(AMINO) for _ in range(args.protein_length))
    dna, planted = build_dna_and_matches(rng, args.length, protein)
    queries = make_queries(rng, args.length, planted, args.queries)

    t1 = time.perf_counter()
    matches = collect_matches(dna, protein)
    lines, checked = expected_output(dna, protein, queries, matches,
                                     verify_sample=300, rng=random.Random(args.seed + 1))
    t2 = time.perf_counter()

    # 每个植入点都必须出现在期望匹配表里。
    match_set = set(matches)
    missing = [p for p in planted if p not in match_set]
    if missing:
        raise SystemExit(f"有 {len(missing)} 个植入点没有出现在匹配表里，"
                         f"例如 {missing[:3]}")
    extra = len(matches) - len(planted)

    for path in (args.out, args.expected):
        os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)

    with open(args.out, "w", encoding="utf-8") as f:
        f.write(dna + "\n")
        f.write(protein + "\n")
        f.write(str(len(queries)) + "\n")
        f.write("".join(f"{l} {r}\n" for (l, r) in queries))
    with open(args.expected, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")

    print(f"DNA 长度      : {len(dna)}")
    print(f"蛋白长度      : {len(protein)}（{protein}）")
    print(f"查询数        : {len(queries)}")
    print(f"植入匹配      : {len(planted)}")
    print(f"期望匹配总数 K: {len(matches)}（植入之外另有 {extra} 个偶然匹配）")
    print(f"暴力复核查询  : {checked} 条，二分答案全部一致")
    print(f"生成耗时      : DNA+查询 {t1 - t0:.2f}s，期望输出 {t2 - t1:.2f}s")
    print(f"输入文件      : {args.out}")
    print(f"期望输出      : {args.expected}")
    return 0


if __name__ == "__main__":
    sys.exit(main())