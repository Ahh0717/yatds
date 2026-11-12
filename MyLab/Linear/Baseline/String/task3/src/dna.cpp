#include "dna.hpp"

#include <cstddef>

namespace dsaac {

namespace {

// 标准遗传密码表（DNA，用 T），按“碱基编号 = A:0 C:1 G:2 T:3”的三位下标
// b1*16 + b2*4 + b3 排列，共 64 项。
// 逐条与题面表格核对过；'*' 为终止密码子，可以参与字符串匹配。
const char kCodonTable[64] = {
    // 第一位 A
    'K', 'N', 'K', 'N', /* AAA AAC AAG AAT */
    'T', 'T', 'T', 'T', /* ACA ACC ACG ACT */
    'R', 'S', 'R', 'S', /* AGA AGC AGG AGT */
    'I', 'I', 'M', 'I', /* ATA ATC ATG ATT */
    // 第一位 C
    'Q', 'H', 'Q', 'H', /* CAA CAC CAG CAT */
    'P', 'P', 'P', 'P', /* CCA CCC CCG CCT */
    'R', 'R', 'R', 'R', /* CGA CGC CGG CGT */
    'L', 'L', 'L', 'L', /* CTA CTC CTG CTT */
    // 第一位 G
    'E', 'D', 'E', 'D', /* GAA GAC GAG GAT */
    'A', 'A', 'A', 'A', /* GCA GCC GCG GCT */
    'G', 'G', 'G', 'G', /* GGA GGC GGG GGT */
    'V', 'V', 'V', 'V', /* GTA GTC GTG GTT */
    // 第一位 T
    '*', 'Y', '*', 'Y', /* TAA TAC TAG TAT */
    'S', 'S', 'S', 'S', /* TCA TCC TCG TCT */
    '*', 'C', 'W', 'C', /* TGA TGC TGG TGT */
    'L', 'F', 'L', 'F', /* TTA TTC TTG TTT */
};

// 碱基编号：A:0 C:1 G:2 T:3；非法字符返回 -1。
int baseIndex(char c) {
    switch (c) {
    case 'A': return 0;
    case 'C': return 1;
    case 'G': return 2;
    case 'T': return 3;
    default: return -1;
    }
}

// 按 L 做稳定计数排序，O(N + K)（定义在 collectMatches 之后）。
std::vector<Match> sortByLeft(const std::vector<Match> &matches, int n);

// 从 seq 的第 frame 个碱基（1-based）开始，每 3 个一读，末尾不足 3 个丢弃。
std::string readFrame(const std::string &seq, int frame) {
    std::string protein;
    const std::size_t n = seq.size();
    const std::size_t start = static_cast<std::size_t>(frame - 1);
    protein.reserve(n / 3 + 1);
    for (std::size_t i = start; i + 2 < n; i += 3) {
        protein.push_back(translateCodon(seq.c_str() + i));
    }
    return protein;
}

} // namespace

char translateCodon(const char *codon) {
    const int a = baseIndex(codon[0]);
    const int b = baseIndex(codon[1]);
    const int c = baseIndex(codon[2]);
    if (a < 0 || b < 0 || c < 0) {
        return '\0';
    }
    return kCodonTable[a * 16 + b * 4 + c];
}

std::string reverseComplement(const std::string &dna) {
    std::string rc;
    rc.resize(dna.size());
    const std::size_t n = dna.size();
    for (std::size_t i = 0; i < n; ++i) {
        char c = dna[n - 1 - i];
        switch (c) {
        case 'A': rc[i] = 'T'; break;
        case 'T': rc[i] = 'A'; break;
        case 'C': rc[i] = 'G'; break;
        case 'G': rc[i] = 'C'; break;
        default: rc[i] = c; break; // 非法字符原样保留，交由翻译阶段报错
        }
    }
    return rc;
}

std::vector<std::string> translateSixFrames(const std::string &dna) {
    const std::string rc = reverseComplement(dna);
    std::vector<std::string> frames(6);

    for (int f = 1; f <= 3; ++f) {
        // 下标 0..2 放正向链 frame 1..3，3..5 放反向互补链 frame 1..3。
        frames[static_cast<std::size_t>(f - 1)] = readFrame(dna, f);
        frames[static_cast<std::size_t>(f + 2)] = readFrame(rc, f);
    }
    return frames;
}

std::vector<int> computePi(const std::string &pattern) {
    const int m = static_cast<int>(pattern.size());
    std::vector<int> pi(static_cast<std::size_t>(m > 0 ? m : 0), 0);
    int head = 0;
    for (int tail = 1; tail < m; ++tail) {
        while (head > 0 && pattern[static_cast<std::size_t>(head)] !=
                               pattern[static_cast<std::size_t>(tail)]) {
            head = pi[static_cast<std::size_t>(head - 1)];
        }
        if (pattern[static_cast<std::size_t>(head)] ==
            pattern[static_cast<std::size_t>(tail)]) {
            ++head;
        }
        pi[static_cast<std::size_t>(tail)] = head;
    }
    return pi;
}

void findAllOccurrences(const std::string &text, const std::string &pattern,
                        const std::vector<int> &pi,
                        std::vector<int> &starts) {
    const int n = static_cast<int>(text.size());
    const int m = static_cast<int>(pattern.size());
    if (m == 0 || m > n) {
        return;
    }
    int j = 0;
    for (int i = 0; i < n; ++i) {
        while (j > 0 && text[static_cast<std::size_t>(i)] !=
                               pattern[static_cast<std::size_t>(j)]) {
            j = pi[static_cast<std::size_t>(j - 1)];
        }
        if (text[static_cast<std::size_t>(i)] ==
            pattern[static_cast<std::size_t>(j)]) {
            ++j;
        }
        if (j == m) {
            starts.push_back(i - m + 1);
            // 继续找重叠出现：把 j 退到最长真前后缀，而不是清零重来。
            j = pi[static_cast<std::size_t>(j - 1)];
        }
    }
}

std::vector<Match> collectMatches(const std::string &dna,
                                  const std::string &protein) {
    std::vector<Match> matches;
    const int m = static_cast<int>(protein.size());
    if (m == 0 || dna.empty()) {
        return matches;
    }

    const std::vector<std::string> frames = translateSixFrames(dna);
    const std::vector<int> pi = computePi(protein);
    const int n = static_cast<int>(dna.size());
    const int span = 3 * m; // 一条匹配覆盖的碱基数

    // 先按 "正链 frame 1,2,3；反链 frame 1,2,3" 的顺序生成。
    // 这个顺序恰好就是 L 相同时要求的先后（'+' 在前、frame 升序），
    // 所以后面按 L 做**稳定**计数排序时，同一 L 内部的次序不用再比较。
    std::vector<int> starts;
    for (int pass = 0; pass < 2; ++pass) {
        const bool forward = (pass == 0);
        for (int f = 1; f <= 3; ++f) {
            const std::size_t frameIndex =
                static_cast<std::size_t>(forward ? f - 1 : f + 2);
            starts.clear();
            findAllOccurrences(frames[frameIndex], protein, pi, starts);
            for (std::size_t k = 0; k < starts.size(); ++k) {
                const int j = starts[k];
                Match mt;
                if (forward) {
                    // 正向链：碱基下标 = (f-1) + 3j + 0 .. + 3m-1，换成 1-based 即
                    // L = (f-1) + 3j + 1，R = (f-1) + 3j + 3m。
                    mt.left = (f - 1) + 3 * j + 1;
                    mt.right = (f - 1) + 3 * j + span;
                    mt.strand = '+';
                } else {
                    // 反向互补链：RC 下标 (f-1)+3j 对应原串下标 N-1-((f-1)+3j)，
                    // 该密码子占原串 [N-(f-1)-3j-3m+1, N-(f-1)-3j]。
                    mt.left = n - (f - 1) - 3 * j - span + 1;
                    mt.right = n - (f - 1) - 3 * j;
                    mt.strand = '-';
                }
                mt.frame = f;
                matches.push_back(mt);
            }
        }
    }

    return sortByLeft(matches, n);
}

namespace {

// 按 L 做稳定计数排序，O(N + K)。值域是 1..n，所以不需要比较排序的 log 因子；
// 稳定 + 生成顺序（+1,+2,+3,-1,-2,-3）= 题面要求的完整次序。
// K <= 2N（6 条蛋白序列总长约 2N，每条上最多每个位置命中一次），故总计 O(N)。
std::vector<Match> sortByLeft(const std::vector<Match> &matches, int n) {
    std::vector<Match> sorted(matches.size());
    if (matches.empty()) {
        return sorted;
    }
    std::vector<int> offset(static_cast<std::size_t>(n) + 2, 0);
    for (std::size_t i = 0; i < matches.size(); ++i) {
        ++offset[static_cast<std::size_t>(matches[i].left)];
    }
    int acc = 0;
    for (int L = 1; L <= n; ++L) {
        const int count = offset[static_cast<std::size_t>(L)];
        offset[static_cast<std::size_t>(L)] = acc;
        acc += count;
    }
    for (std::size_t i = 0; i < matches.size(); ++i) {
        const int L = matches[i].left;
        sorted[static_cast<std::size_t>(offset[static_cast<std::size_t>(L)]++)] =
            matches[i];
    }
    return sorted;
}

} // namespace

MatchCounter::MatchCounter(const std::vector<Match> &matches,
                           std::size_t dnaLength, std::size_t proteinLength)
    : prefix_(dnaLength + 1, 0),
      span_(proteinLength == 0 ? -1
                               : static_cast<long long>(3 * proteinLength) - 1) {
    // matches 已按 L 升序；用差分累加填充前缀和。
    for (std::size_t i = 0; i < matches.size(); ++i) {
        const long long l = matches[i].left;
        if (l >= 1 && static_cast<std::size_t>(l) <= dnaLength) {
            ++prefix_[static_cast<std::size_t>(l)];
        }
    }
    for (std::size_t x = 1; x < prefix_.size(); ++x) {
        prefix_[x] += prefix_[x - 1];
    }
}

long long MatchCounter::countContained(long long left, long long right) const {
    if (span_ < 0) {
        return 0; // P 为空，没有匹配
    }
    const long long n = static_cast<long long>(prefix_.size()) - 1;
    // 完整包含 <=> L >= left 且 L + span_ <= right，即 left <= L <= right - span_。
    // 上界 right - span_ 就是规范里的 r - (3m - 1) = r - 3m + 1。
    // 区间装不下 3m 个碱基（r - 3m + 1 < l）时不可能有完整匹配，直接 0。
    if (right - span_ < left) {
        return 0;
    }
    long long hi = right - span_;
    if (hi > n) {
        hi = n; // 上界最多到 N，超出部分没有匹配
    }
    long long lo = left - 1; // prefix_[lo] = #(L <= l-1)，减掉它只留下 L >= l
    if (lo < 0) {
        lo = 0;
    }
    if (lo > n) {
        lo = n;
    }
    return prefix_[static_cast<std::size_t>(hi)] -
           prefix_[static_cast<std::size_t>(lo)];
}

} // namespace dsaac