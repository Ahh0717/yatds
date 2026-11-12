// bench.cpp —— 任务 2 的最坏情况性能取证（Step 1 / 2 / 3 的分析依据）
//
// 两组用例，都通过 match.hpp 暴露的公共接口调用被测函数：
//
//   case A（Step 1/2）：经典最坏情况
//       text    = 'A'*200000 + 'B'
//       pattern = 'A'*20000  + 'B'
//       朴素算法在每个起点都“几乎匹配成功”后才失配，比较次数约 (n-m)*m ≈ 3.6e9，
//       退化为 O(n*m)；KMP 只比较约 2n 次，保持 O(n+m)。
//
//   case B（Step 3）：nextval 能体现差别的情况
//       text    = ('A'*20000 + 'C') 重复 20 次
//       pattern = 'A'*20000 + 'B'
//       每次在块尾 'C' 处失配。原始 next 只退一格，随后还要拿 'A' 和 'C'
//       再比一次（必然失败）；nextval 一次就把整段相同前缀跳过去。
//       比较次数约减半，这是 nextval 相对 next 的真实收益。
//
// 每行 PERF 同时给出耗时与字符比较次数：比较次数是复杂度的直接证据，
// 且不受缓存、分支预测影响，比耗时更稳定。

#include "match.hpp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

// 每组用例重复次数。朴素算法单次就要数百毫秒，重复多次会太慢，故分开设置。
const int kFastRepeats = 5;
const int kNaiveRepeats = 2;

template <typename Fn>
double timeMinMs(Fn fn, int repeats) {
    double best = 0.0;
    for (int r = 0; r < repeats; ++r) {
        const auto begin = Clock::now();
        fn();
        const auto end = Clock::now();
        const double ms =
            std::chrono::duration<double, std::milli>(end - begin).count();
        if (r == 0 || ms < best) {
            best = ms;
        }
    }
    return best;
}

// ---------------------------------------------------------------------
// 计数版：只用于统计比较次数，逻辑与被测函数逐句对应。
// ---------------------------------------------------------------------

int naiveCounted(const std::string& text, const std::string& pattern,
                 std::size_t& cmp) {
    const int n = static_cast<int>(text.size());
    const int m = static_cast<int>(pattern.size());
    cmp = 0;
    if (m == 0) return 0;
    for (int i = 0; i <= n - m; ++i) {
        int j = 0;
        while (j < m) {
            ++cmp;
            if (text[i + j] != pattern[j]) break;
            ++j;
        }
        if (j == m) return i;
    }
    return -1;
}

int kmpCounted(const std::string& text, const std::string& pattern,
               std::size_t& cmp) {
    const int n = static_cast<int>(text.size());
    const int m = static_cast<int>(pattern.size());
    cmp = 0;
    if (m == 0) return 0;
    if (m > n) return -1;
    const std::vector<int> next = dsaac::getNext(pattern);
    int j = 0;
    for (int i = 0; i < n; ++i) {
        while (j > 0 && text[i] != pattern[j]) {
            ++cmp;
            j = next[j - 1];
        }
        if (j == 0 && text[i] != pattern[0]) {
            ++cmp;
            continue;
        }
        ++cmp;
        ++j;
        if (j == m) return i - m + 1;
    }
    return -1;
}

int kmpNextvalCounted(const std::string& text, const std::string& pattern,
                      std::size_t& cmp) {
    const int n = static_cast<int>(text.size());
    const int m = static_cast<int>(pattern.size());
    cmp = 0;
    if (m == 0) return 0;
    if (m > n) return -1;
    const std::vector<int> nv = dsaac::getNextValTable(pattern);
    int j = 0;
    for (int i = 0; i < n; ++i) {
        while (j > 0 && text[i] != pattern[j]) {
            ++cmp;
            j = nv[j];
        }
        if (j == 0 && text[i] != pattern[0]) {
            ++cmp;
            continue;
        }
        ++cmp;
        ++j;
        if (j == m) return i - m + 1;
    }
    return -1;
}

// 打印一张表的前 10 项，用于直观展示 next 与 nextval 的差别。
void printTableHead(const char* tag, const std::vector<int>& table) {
    std::printf("  %-10s:", tag);
    const std::size_t shown = table.size() < 10 ? table.size() : 10;
    for (std::size_t i = 0; i < shown; ++i) {
        std::printf(" %d", table[i]);
    }
    std::printf("%s\n", table.size() > shown ? " ..." : "");
}

// 跑一组用例：计时 + 计数 + 一致性校验。
// 返回 false 表示三个算法给出了不同答案（bench 视为失败）。
bool runCase(const char* label, const std::string& text,
             const std::string& pattern, int naiveRepeats) {
    const std::size_t n = text.size();
    const std::size_t m = pattern.size();

    std::printf("\n%s  n=%zu m=%zu\n", label, n, m);
    printTableHead("next", dsaac::getNext(pattern));
    printTableHead("nextval", dsaac::getNextVal(pattern));
    printTableHead("nv table", dsaac::getNextValTable(pattern));

    volatile int sink = 0;
    const double tNaive =
        timeMinMs([&] { sink += dsaac::naiveMatch(text, pattern); }, naiveRepeats);
    const double tKmp =
        timeMinMs([&] { sink += dsaac::kmpMatch(text, pattern); }, kFastRepeats);
    const double tNv =
        timeMinMs([&] { sink += dsaac::kmpMatchNextVal(text, pattern); }, kFastRepeats);
    (void)sink;

    std::size_t cNaive = 0, cKmp = 0, cNv = 0;
    const int iNaive = naiveCounted(text, pattern, cNaive);
    const int iKmp = kmpCounted(text, pattern, cKmp);
    const int iNv = kmpNextvalCounted(text, pattern, cNv);

    std::printf("  matched index: naive=%d kmp=%d kmp_nextval=%d\n",
                iNaive, iKmp, iNv);
    std::printf("  PERF naive         n=%zu m=%zu time_ms=%.3f comparisons=%zu\n",
                n, m, tNaive, cNaive);
    std::printf("  PERF kmp           n=%zu m=%zu time_ms=%.3f comparisons=%zu\n",
                n, m, tKmp, cKmp);
    std::printf("  PERF kmp_nextval   n=%zu m=%zu time_ms=%.3f comparisons=%zu\n",
                n, m, tNv, cNv);

    if (iNaive != iKmp || iKmp != iNv) {
        std::printf("  错误：三个算法结果不一致\n");
        return false;
    }
    return true;
}

}  // namespace

int main() {
    bool ok = true;

    // case A：经典最坏情况，主串末尾才失配，并且能匹配成功。
    // 朴素约 (n-m)*m = 3.6e9 次比较，本机约 1~2 秒。
    {
        const std::size_t textRun = 200000;
        const std::size_t patRun = 20000;
        const std::string text = std::string(textRun, 'A') + 'B';
        const std::string pattern = std::string(patRun, 'A') + 'B';
        ok = runCase("case A: 'A'*200000+'B' vs 'A'*20000+'B'",
                     text, pattern, kNaiveRepeats) && ok;
    }

    // case B：nextval 的收益用例。每块末尾都失配，next 只退一格后
    // 还要再比较一次 'A' vs 'C'，nextval 直接跳过整段相同前缀。
    {
        const std::size_t patRun = 20000;
        const int blocks = 20;
        std::string text;
        text.reserve(static_cast<std::size_t>(blocks) * (patRun + 1));
        for (int b = 0; b < blocks; ++b) {
            text.append(patRun, 'A');
            text.push_back('C');
        }
        const std::string pattern = std::string(patRun, 'A') + 'B';
        ok = runCase("case B: ('A'*20000+'C')x20 vs 'A'*20000+'B'",
                     text, pattern, kNaiveRepeats) && ok;
    }

    return ok ? 0 : 1;
}