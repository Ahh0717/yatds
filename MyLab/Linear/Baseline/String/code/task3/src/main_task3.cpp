// main_task3.cpp —— 任务 3 驱动：双链 DNA 目标蛋白编码片段检索
//
// 输入：
//   第 1 行  S   DNA 序列（A/T/C/G）
//   第 2 行  P   目标蛋白序列（氨基酸字母，可含非终止字符）
//   第 3 行  q   查询次数
//   之后 q 行   每行 "l r"（1-based 闭区间，保证 1 <= l <= r <= N）
//
// 输出：
//   第 1 行      P 的 pi 数组（KMP 前缀函数）
//   第 2 行      K（匹配总数）
//   之后 K 行    "L R strand frame"
//   最后 q 行    每个区间的 answer
//
// 本任务不输出 PERF。

#include "dna.hpp"
#include "input_util.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace {

using mylab::io::Cursor;

// 打印 pi 数组，空格分隔；P 为空时输出空行。
void printPi(const std::vector<int> &pi) {
    for (std::size_t i = 0; i < pi.size(); ++i) {
        if (i != 0) {
            std::cout << ' ';
        }
        std::cout << pi[i];
    }
    std::cout << '\n';
}

} // namespace

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    const std::vector<std::string> tokens = mylab::io::readAllTokens();
    Cursor cur(tokens);

    std::string dna;
    std::string protein;
    std::size_t queryCount = 0;
    if (!mylab::io::readString(cur, dna, "DNA 序列") ||
        !mylab::io::readString(cur, protein, "目标蛋白序列") ||
        !mylab::io::readSize(cur, queryCount, "查询次数")) {
        return 1;
    }

    // DNA 只允许 A/T/C/G；否则翻译结果里会出现 '\0'，坐标也无法解释。
    for (std::size_t i = 0; i < dna.size(); ++i) {
        const char c = dna[i];
        if (c != 'A' && c != 'T' && c != 'C' && c != 'G') {
            std::cerr << "输入错误：DNA 序列第 " << i + 1 << " 个字符 \"" << c
                      << "\" 不是 A/T/C/G\n";
            return 1;
        }
    }

    // 先读入全部查询再计算，这样畸形的查询不会留下半截输出。
    std::vector<std::pair<std::size_t, std::size_t>> queries(queryCount);
    for (std::size_t i = 0; i < queryCount; ++i) {
        std::size_t left = 0;
        std::size_t right = 0;
        if (!mylab::io::readSize(cur, left, "查询左端点", i + 1) ||
            !mylab::io::readSize(cur, right, "查询右端点", i + 1)) {
            return 1;
        }
        if (left < 1 || left > right || right > dna.size()) {
            std::cerr << "输入错误：第 " << i + 1 << " 步的查询 [" << left << ", "
                      << right << "] 不在 1.." << dna.size() << " 内或不满足 l <= r\n";
            return 1;
        }
        queries[i] = {left, right};
    }

    const std::vector<int> pi = dsaac::computePi(protein);
    const std::vector<dsaac::Match> matches = dsaac::collectMatches(dna, protein);
    const dsaac::MatchCounter counter(matches, dna.size(), protein.size());

    printPi(pi);
    std::cout << matches.size() << '\n';
    for (std::size_t i = 0; i < matches.size(); ++i) {
        std::cout << matches[i].left << ' ' << matches[i].right << ' '
                  << matches[i].strand << ' ' << matches[i].frame << '\n';
    }
    for (std::size_t i = 0; i < queries.size(); ++i) {
        std::cout << counter.countContained(static_cast<long long>(queries[i].first),
                                           static_cast<long long>(queries[i].second))
                  << '\n';
    }
    return 0;
}