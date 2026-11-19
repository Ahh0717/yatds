#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""任务 3 的随机差分测试：C++ 程序 vs tools/oracle_task3.py。

两组用例：
  1. **从翻译结果反向构造**（高命中率）：随机生成 DNA 与"阅读框内的一段蛋白"作为 P，
     这样 P 一定能在某条链上命中，能真正压到坐标映射与区间计数；
  2. **纯随机 P**：字母表随机取氨基酸，多数会 0 命中，用来压实 K == 0 与 answer == 0
     这类边界，并顺带检查 pi 数组。

每一例都让 C++ 与 oracle 各自产出完整输出，逐行逐字段比较（L/R/strand/frame/K/answer）。
另外内置一个**字段级**校验：解析两边输出后，分别比较 pi、K、match 列表、answer 列表，
这样即使行数不同也能指出是哪一类不一致。

用法：
    python3 fuzz_task3.py                 # 默认 300 组
    python3 fuzz_task3.py --cases 1000
    python3 fuzz_task3.py --exe build/task3 --seed 42
"""

import argparse
import os
import random
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
STRING_ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)

from oracle_task3 import CODON_TABLE, solve  # noqa: E402

BASES = "ATCG"
# 题面保证 P 不含终止符 '*'，随机用例也遵守；含 '*' 的健壮性另有专门用例。
AMINO = sorted(set(CODON_TABLE.values()) - {"*"})
AMINO_WITH_STAR = sorted(set(CODON_TABLE.values()))


def resolve_exe(path):
    if os.name == "nt" and not path.lower().endswith(".exe"):
        path += ".exe"
    if not os.path.isabs(path):
        path = os.path.join(STRING_ROOT, "task3", path)
    return os.path.abspath(path)


def random_dna(rng, n):
    return "".join(rng.choice(BASES) for _ in range(n))


def translate_frame(seq, frame):
    out = []
    i = frame - 1
    while i + 2 < len(seq):
        out.append(CODON_TABLE[seq[i:i + 3]])
        i += 3
    return "".join(out)


def revcomp(dna):
    comp = {"A": "T", "T": "A", "C": "G", "G": "C"}
    return "".join(comp[c] for c in reversed(dna))


def build_hitting_case(rng):
    """构造一个必然命中的用例：从某条链的某个阅读框里截一段蛋白当 P。"""
    n = rng.randint(3, 60)
    dna = random_dna(rng, n)
    seq = dna if rng.random() < 0.5 else revcomp(dna)
    frame = rng.randint(1, 3)
    protein_all = translate_frame(seq, frame)
    if len(protein_all) == 0:
        # 太短，退回纯随机 P
        protein = "".join(rng.choice(AMINO) for _ in range(rng.randint(1, 3)))
        return dna, protein
    # 从蛋白序列里截一段（允许重叠出现的可能）
    start = rng.randint(0, len(protein_all) - 1)
    max_len = len(protein_all) - start
    length = rng.randint(1, max_len)
    protein = protein_all[start:start + length]
    return dna, protein


def build_random_case(rng):
    n = rng.randint(3, 60)
    dna = random_dna(rng, n)
    protein = "".join(rng.choice(AMINO) for _ in range(rng.randint(1, 4)))
    return dna, protein


# 反向：氨基酸 -> 一个能翻译成它的密码子（同一氨基酸取第一个即可）。
AA_TO_CODON = {}
for _codon, _aa in CODON_TABLE.items():
    AA_TO_CODON.setdefault(_aa, _codon)


def build_overlap_case(rng):
    """同一条链、同一阅读框里连续出现多次：把 P 的密码子串首尾相接地拼起来。

    DNA = codon(P) 重复 k 次，蛋白序列就是 P 重复 k 次，于是匹配起点相差 3m，
    相邻匹配在碱基层面**首尾相接但不重叠**；若要真正重叠，需要 P 自身有前后缀
    关系，所以这里再从 P 的重复里取一段作为新 P（如 P="AA" 时 "AAAA" 有 3 个出现）。
    """
    codon_seq = "".join(AA_TO_CODON[rng.choice(AMINO)]
                        for _ in range(rng.randint(1, 3)))
    repeat = rng.randint(2, 6)
    core = codon_seq * repeat
    # 叠加一个随机前缀/后缀，让坐标不总是从 1 开始
    pre = random_dna(rng, rng.randint(0, 6))
    suf = random_dna(rng, rng.randint(0, 6))
    dna = pre + core + suf
    protein = translate_frame(dna, 1)  # 从第 1 帧读，保证至少一次命中
    if not protein:
        return dna, random_dna(rng, 3)
    # 取蛋白序列里连续的一段，制造更多重叠/多次出现
    start = rng.randrange(len(protein))
    length = rng.randint(1, max(1, len(protein) - start))
    return dna, protein[start:start + length]


def build_same_position_case(rng):
    """全同碱基长串 + 长度 1 的蛋白：**高密度重叠命中**。

    同一条链上不同阅读框的 L 对 3 取余互不相同（L = (f-1)+3j+1），
    所以**同链内不可能有相同 L**；能共享 L 的只有正反两条链。这里构造的
    `base × n` 让三个帧在相邻位置各命中一次，是压“重叠”与批量排序的用例。

    真正压“同 L 不同链”的是 build_palindrome_case（回文串让两链镜像命中）。
    """
    # 选一个全同碱基的片段：AAAAAA 在三个阅读框里都读成 KKKK
    base = rng.choice(BASES)
    run = base * rng.randint(6, 18)
    dna = random_dna(rng, rng.randint(0, 6)) + run + random_dna(rng, rng.randint(0, 6))
    aa_run = translate_frame(base * 3, 1)[0]  # 该碱基三元组对应的氨基酸
    return dna, aa_run


def build_tiny_case(rng):
    """极小规模：N 从 1 到 6，覆盖“没有完整密码子”“P 比任何蛋白序列都长”。"""
    n = rng.randint(1, 6)
    dna = random_dna(rng, n)
    if rng.random() < 0.5:
        protein = "".join(rng.choice(AMINO) for _ in range(rng.randint(1, 3)))
    else:
        # 从翻译结果里取，可能为空（N < 3 时所有帧都是空串）
        seq = dna
        frame = rng.randint(1, 3)
        t = translate_frame(seq, frame)
        protein = t if t else rng.choice(AMINO)
    return dna, protein


def build_star_case(rng):
    """P 含终止符 '*'：题面说 P 不含，但翻译结果里会出现 '*'，
    这里验证实现把 '*' 当普通字符处理、不会提前截断。"""
    n = rng.randint(9, 40)
    dna = random_dna(rng, n)
    seq = dna if rng.random() < 0.5 else revcomp(dna)
    frame = rng.randint(1, 3)
    t = translate_frame(seq, frame)
    if not t:
        return dna, "*"
    start = rng.randrange(len(t))
    length = rng.randint(1, len(t) - start)
    return dna, t[start:start + length]


def build_palindrome_case(rng):
    """构造正反两条链都能命中的近似回文，压到“同 L 时 '+' 必须排在 '-' 前”。

    DNA 取 X + revcomp(X) 的形式，则反向互补链等于它自己的镜像，两条链的
    翻译结果互为镜像，容易在同一 L 上同时命中。
    """
    half = random_dna(rng, rng.randint(3, 12))
    dna = half + revcomp(half)
    seq = dna
    frame = rng.randint(1, 3)
    protein_all = translate_frame(seq, frame)
    if not protein_all:
        return dna, random_dna(rng, 3)
    start = rng.randrange(len(protein_all))
    length = rng.randint(1, max(1, len(protein_all) - start))
    return dna, protein_all[start:start + length]


def make_queries(rng, n):
    q = rng.randint(0, 8)
    queries = []
    for _ in range(q):
        l = rng.randint(1, n)
        r = rng.randint(l, n)
        queries.append((l, r))
    return queries


def format_input(dna, protein, queries):
    lines = [dna, protein, str(len(queries))]
    lines += [f"{l} {r}" for (l, r) in queries]
    return "\n".join(lines) + "\n"


def parse_output(text):
    """把输出解析成 (pi, K, matches, answers)。"""
    lines = text.splitlines()
    if not lines:
        return None
    pi = [int(x) for x in lines[0].split()] if lines[0].strip() else []
    k = int(lines[1])
    matches = []
    idx = 2
    for _ in range(k):
        parts = lines[idx].split()
        matches.append((int(parts[0]), int(parts[1]), parts[2], int(parts[3])))
        idx += 1
    answers = [int(lines[idx + i]) for i in range(len(lines) - idx)]
    return pi, k, matches, answers


def compare(expected_lines, actual_lines):
    """返回 None 表示一致，否则返回一段人类可读的差异描述。"""
    exp = parse_output("\n".join(expected_lines))
    act = parse_output("\n".join(actual_lines))
    if act is None:
        return "程序没有输出"
    if exp[0] != act[0]:
        for i in range(min(len(exp[0]), len(act[0]))):
            if exp[0][i] != act[0][i]:
                return f"pi 第 {i} 项：oracle={exp[0][i]} 程序={act[0][i]}"
        return f"pi 长度不同：oracle={len(exp[0])} 程序={len(act[0])}"
    if exp[1] != act[1]:
        return f"K 不同：oracle={exp[1]} 程序={act[1]}"
    if exp[2] != act[2]:
        for i in range(min(len(exp[2]), len(act[2]))):
            if exp[2][i] != act[2][i]:
                return (f"第 {i} 个匹配不同：oracle={exp[2][i]} 程序={act[2][i]}")
        return f"匹配数不同：oracle={len(exp[2])} 程序={len(act[2])}"
    if exp[3] != act[3]:
        for i in range(min(len(exp[3]), len(act[3]))):
            if exp[3][i] != act[3][i]:
                return f"第 {i} 个 answer 不同：oracle={exp[3][i]} 程序={act[3][i]}"
        return f"answer 个数不同：oracle={len(exp[3])} 程序={len(act[3])}"
    return None


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    parser = argparse.ArgumentParser(description="任务 3 随机差分测试")
    parser.add_argument("--exe", default="build/task3", help="被测程序")
    parser.add_argument("--cases", type=int, default=300, help="用例数")
    parser.add_argument("--seed", type=int, default=20261111, help="随机种子")
    parser.add_argument("--fail-fast", action="store_true", help="首个失败即停")
    args = parser.parse_args()

    exe = resolve_exe(args.exe)
    if not os.path.isfile(exe):
        print(f"找不到程序 {exe}，请先在 task3/ 下运行 make")
        return 1

    rng = random.Random(args.seed)
    failures = []
    hit_cases = 0
    zero_hit_cases = 0

    for case_id in range(args.cases):
        kind = case_id % 7
        if kind == 0:
            dna, protein = build_hitting_case(rng)
        elif kind == 1:
            dna, protein = build_random_case(rng)
        elif kind == 2:
            dna, protein = build_overlap_case(rng)
        elif kind == 3:
            dna, protein = build_same_position_case(rng)
        elif kind == 4:
            dna, protein = build_palindrome_case(rng)
        elif kind == 5:
            dna, protein = build_tiny_case(rng)
        else:
            dna, protein = build_star_case(rng)
        queries = make_queries(rng, len(dna))
        stdin_data = format_input(dna, protein, queries)

        expected = solve(stdin_data.split())
        if int(expected[1]) > 0:
            hit_cases += 1
        else:
            zero_hit_cases += 1

        proc = subprocess.run([exe], input=stdin_data, capture_output=True,
                              text=True, encoding="utf-8")
        if proc.returncode != 0:
            failures.append((case_id, dna, protein, queries,
                             f"退出码 {proc.returncode}；stderr={proc.stderr.strip()}"))
            if args.fail_fast:
                break
            continue

        diff = compare(expected, proc.stdout.splitlines())
        if diff is not None:
            failures.append((case_id, dna, protein, queries, diff))
            if args.fail_fast:
                break

    total = args.cases
    print(f"随机差分：{total - len(failures)}/{total} 一致"
          f"（命中用例 {hit_cases}，零命中用例 {zero_hit_cases}）")

    if failures:
        print(f"\n失败 {len(failures)} 例，前 5 例明细：")
        for (case_id, dna, protein, queries, detail) in failures[:5]:
            print(f"\n  case {case_id}: S={dna} P={protein} q={len(queries)}")
            print(f"    queries={queries}")
            print(f"    {detail}")
        return 1

    print("全部一致")
    return 0


if __name__ == "__main__":
    sys.exit(main())