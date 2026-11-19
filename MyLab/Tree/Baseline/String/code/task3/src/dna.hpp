// dna.hpp —— 任务 3：双链 DNA 目标蛋白编码片段检索
//
// 本模块只做“序列到匹配”的部分，不做 I/O；I/O 在 main_task3.cpp 里。
// 三件事：
//   1. 遗传密码翻译：把 DNA 读成 6 条蛋白序列（正/反 × 3 阅读框）；
//   2. 匹配：在每条蛋白序列上用 KMP 找出目标蛋白 P 的所有出现（含重叠）；
//   3. 坐标映射：把蛋白下标 j 换回原 DNA 上的 1-based 闭区间 [L, R]。
//
// 任务之间不互相 include，所以 KMP 在这里独立实现一份，不引用 task2。

#ifndef DSAAC_DNA_MATCH_HPP
#define DSAAC_DNA_MATCH_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace dsaac {

// 单个密码子翻译成氨基酸字符；终止密码子返回 '*'。
// codon 至少可读 3 个字符；含非 A/T/C/G 字符时返回 '\0'，调用方据此报错。
char translateCodon(const char* codon);

// 反向互补链：先反转，再把 A<->T、C<->G 互换。
std::string reverseComplement(const std::string &dna);

// 6 条蛋白序列，固定顺序为：
//   [0..2] = 正向链 frame 1,2,3；[3..5] = 反向互补链 frame 1,2,3。
// 阅读框 f 从下标 f-1 开始，每 3 个碱基一个密码子，末尾不足 3 个的丢弃。
std::vector<std::string> translateSixFrames(const std::string &dna);

// 0-based 前缀函数（KMP 的 pi / next）。
std::vector<int> computePi(const std::string &pattern);

// 找出 pattern 在 text 中的所有出现起点（0-based 蛋白下标），不跳过重叠。
// pi 必须是 computePi(pattern) 的结果。
void findAllOccurrences(const std::string &text, const std::string &pattern,
                        const std::vector<int> &pi,
                        std::vector<int> &starts);

// 一次匹配在原 DNA 上的位置。left/right 为 1-based 闭区间。
struct Match {
    int left;
    int right;
    char strand; // '+' 或 '-'
    int frame;   // 1..3
};

// 收集全部匹配并排好序：L 升序；L 相同按 strand（'+' 在 '-' 前）；再按 frame 升序。
// 实现用稳定计数排序（值域 1..N），所以整体是 O(N + K) 而不是 O(K log K)。
// protein 为空时返回空表（长度 0 的出现没有意义，见 README）。
std::vector<Match> collectMatches(const std::string &dna,
                                  const std::string &protein);

// 区间计数：预先按“匹配左端点”做前缀和，之后每次查询 O(1)。
//
// 关键性质：同一条 P 的所有匹配长度相同，都是 3m 个碱基，即 R = L + 3m - 1。
// 于是“匹配被 [l, r] 完整包含” <=> “l <= L <= r - (3m - 1)”，
// 只需在按 L 排序的坐标轴上数点，前缀和即可。
class MatchCounter {
public:
    MatchCounter(const std::vector<Match> &matches, std::size_t dnaLength,
                 std::size_t proteinLength);

    // 查询 [l, r]（1-based 闭区间）内被完整包含的匹配数。
    long long countContained(long long left, long long right) const;

private:
    std::vector<long long> prefix_; // prefix_[x] = L <= x 的匹配数，长度 N+1
    long long span_;                // 3m - 1；protein 为空时为 -1
};

} // namespace dsaac

#endif // DSAAC_DNA_MATCH_HPP