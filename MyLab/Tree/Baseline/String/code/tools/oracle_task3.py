#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""任务 3（双链 DNA 蛋白编码片段检索）的独立参考实现。

定位：**只用于生成期望输出与差分测试**，与 task3/src 的 C++ 代码没有任何共享逻辑。
刻意写得慢而直白：

  - 遗传密码表逐条手写（从题面抄），不用任何推导；
  - 6 条蛋白序列逐字符翻译；
  - 匹配用暴力滑窗，不用 KMP（这样它才能当 KMP 的对照）；
  - 区间计数直接遍历所有匹配判断是否被 [l, r] 完整包含，不用前缀和；
  - 坐标映射按题面公式直接写。

用法：
    python3 oracle_task3.py < input.txt > output.txt

输出格式与 task3/src/main_task3.cpp 完全一致（无 PERF 行）。
"""

import sys

# 题面标准遗传密码表（DNA，用 T）。逐条抄写，未做任何压缩。
CODON_TABLE = {
    "TTT": "F", "TTC": "F", "TTA": "L", "TTG": "L",
    "TCT": "S", "TCC": "S", "TCA": "S", "TCG": "S",
    "TAT": "Y", "TAC": "Y", "TAA": "*", "TAG": "*",
    "TGT": "C", "TGC": "C", "TGA": "*", "TGG": "W",
    "CTT": "L", "CTC": "L", "CTA": "L", "CTG": "L",
    "CCT": "P", "CCC": "P", "CCA": "P", "CCG": "P",
    "CAT": "H", "CAC": "H", "CAA": "Q", "CAG": "Q",
    "CGT": "R", "CGC": "R", "CGA": "R", "CGG": "R",
    "ATT": "I", "ATC": "I", "ATA": "I", "ATG": "M",
    "ACT": "T", "ACC": "T", "ACA": "T", "ACG": "T",
    "AAT": "N", "AAC": "N", "AAA": "K", "AAG": "K",
    "AGT": "S", "AGC": "S", "AGA": "R", "AGG": "R",
    "GTT": "V", "GTC": "V", "GTA": "V", "GTG": "V",
    "GCT": "A", "GCC": "A", "GCA": "A", "GCG": "A",
    "GAT": "D", "GAC": "D", "GAA": "E", "GAG": "E",
    "GGT": "G", "GGC": "G", "GGA": "G", "GGG": "G",
}
COMPLEMENT = {"A": "T", "T": "A", "C": "G", "G": "C"}

assert len(CODON_TABLE) == 64


def reverse_complement(dna):
    return "".join(COMPLEMENT[c] for c in reversed(dna))


def translate_frame(seq, frame):
    """从 1-based 的 frame 位置开始，每 3 个一读；末尾不足 3 个丢弃。"""
    out = []
    i = frame - 1
    while i + 2 < len(seq):
        out.append(CODON_TABLE[seq[i:i + 3]])
        i += 3
    return "".join(out)


def prefix_function(p):
    """KMP 前缀函数，0-based：pi[i] = p[0..i] 的最长真前后缀长度。"""
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


def brute_occurrences(text, pattern):
    """暴力枚举每个起点；支持重叠。"""
    m = len(pattern)
    n = len(text)
    if m == 0:
        return []
    res = []
    for j in range(0, n - m + 1):
        if text[j:j + m] == pattern:
            res.append(j)
    return res


def collect_matches(dna, protein):
    """返回按 (L 升序, strand '+' 前, frame 升序) 排序的 (L, R, strand, frame)。"""
    n = len(dna)
    m = len(protein)
    if m == 0:
        return []
    rc = reverse_complement(dna)
    matches = []
    for frame in (1, 2, 3):
        fwd = translate_frame(dna, frame)
        for j in brute_occurrences(fwd, protein):
            L = (frame - 1) + 3 * j + 1
            R = (frame - 1) + 3 * j + 3 * m
            matches.append((L, R, "+", frame))

        rev = translate_frame(rc, frame)
        for j in brute_occurrences(rev, protein):
            L = n - (frame - 1) - 3 * j - 3 * m + 1
            R = n - (frame - 1) - 3 * j
            matches.append((L, R, "-", frame))

    # L 升序；L 相同按 strand（'+' 在 '-' 前）；再按 frame 升序。
    matches.sort(key=lambda t: (t[0], 0 if t[2] == "+" else 1, t[3]))
    return matches


def count_in_range(matches, left, right):
    """暴力计数：遍历每个匹配，判断是否被 [left, right] 完整包含。"""
    total = 0
    for (L, R, _strand, _frame) in matches:
        if left <= L and R <= right:
            total += 1
    return total


def solve(tokens):
    """tokens 是已按空白切分的输入。返回输出行列表。"""
    pos = 0
    dna = tokens[pos]; pos += 1
    protein = tokens[pos]; pos += 1
    q = int(tokens[pos]); pos += 1

    queries = []
    for _ in range(q):
        l = int(tokens[pos]); r = int(tokens[pos + 1]); pos += 2
        queries.append((l, r))

    n = len(dna)
    pi = prefix_function(protein)
    matches = collect_matches(dna, protein)

    out = []
    out.append(" ".join(str(x) for x in pi))
    out.append(str(len(matches)))
    for (L, R, strand, frame) in matches:
        out.append(f"{L} {R} {strand} {frame}")
    for (l, r) in queries:
        out.append(str(count_in_range(matches, l, r)))
    return out


def main():
    text = sys.stdin.read()
    tokens = text.split()
    if len(tokens) < 3:
        sys.stderr.write("输入不足：需要 S、P、q\n")
        return 1
    try:
        lines = solve(tokens)
    except (ValueError, KeyError, IndexError) as exc:
        sys.stderr.write(f"输入解析失败：{exc}\n")
        return 1
    sys.stdout.write("\n".join(lines) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())