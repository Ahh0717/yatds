#include "match.hpp"

#include <cstddef>

/* START: pattern_match implementation */

int dsaac::naiveMatch(const std::string& text, const std::string& pattern)
{
    int n = static_cast<int>(text.length());
    int m = static_cast<int>(pattern.length());
    
    if (m == 0) return 0;

    for (int i = 0; i <= n - m; i++) {
        int j = 0;
        while (j < m && text[i + j] == pattern[j]) {
            j++;
        }
        if (j == m) {
            return i; // 匹配成功，返回起点
        }
    }
    return -1; // 匹配失败
}

std::vector<int> dsaac::getNext(const std::string& pattern)
{
    int m = static_cast<int>(pattern.length());
    std::vector<int> next(m, 0);
    
    if (m == 0) return next;

    int head = 0;
    for (int tail = 1; tail < m; tail++) {
        while (head > 0 && pattern[head] != pattern[tail]) {
            head = next[head - 1];
        }
        if (pattern[head] == pattern[tail]) {
            head++;
        }
        next[tail] = head;
    }
    return next;
}

// KMP 主流程：主串指针 i 只前进不回退，失配时用 next 把模式串指针 j
// 退到“最长的、仍然是已匹配后缀的前缀”之后重新比较。
// 时间复杂度 O(n + m)，空间 O(m)；返回首次匹配起点，找不到返回 -1。
int dsaac::kmpMatch(const std::string& text, const std::string& pattern)
{
    const int n = static_cast<int>(text.length());
    const int m = static_cast<int>(pattern.length());

    if (m == 0) return 0;   // 空模式串按约定视为在 0 处匹配
    if (m > n) return -1;   // 模式串更长，不可能匹配

    const std::vector<int> next = getNext(pattern);

    int j = 0;              // 当前已匹配的模式串长度（也指向下一个待比较的模式串字符）
    for (int i = 0; i < n; i++) {
        // 失配：沿 next 链回退 j。因为 next 记录的是“已匹配前缀的最长真前后缀”，
        // 回退后 text[i] 之前的字符依然与 pattern[0..j-1] 对齐，无需重新比较。
        while (j > 0 && text[i] != pattern[j]) {
            j = next[j - 1];
        }
        if (text[i] == pattern[j]) {
            j++;
        }
        if (j == m) {
            return i - m + 1;  // 匹配完成，起点是 i - m + 1
        }
    }
    return -1;
}

// nextval（展示用）：在 next 的基础上消除“回退后仍是同一个字符”的无效比较。
//
// 原始 next 的问题是：若 pattern[i] == pattern[next[i]]，那么回退后
// 下一次比较必然再次失配，这次比较纯属浪费。nextval 把这串相同的
// 字符一次性跳过。
//
// 注意：本函数严格按题目给出的公式实现，输出的表用于 NEXTVAL 操作展示，
// 形如 "AAAAB" -> 0 0 0 0 0。该表把“连续相同字符”一律压到 0，
// 直接拿去做 0-based KMP 的跳转表会回退过度（见 getNextValTable）。
// 复杂度 O(m)。
std::vector<int> dsaac::getNextVal(const std::string& pattern)
{
    const int m = static_cast<int>(pattern.length());
    std::vector<int> nextval = getNext(pattern);

    for (int i = 1; i < m; i++) {
        const int k = nextval[i];   // 此时尚未改写，等于 next[i]
        if (k == i) {
            // pattern[0..i] 全部相同，任何位置失配都只能退回开头
            nextval[i] = 0;
        } else if (pattern[i] == pattern[k]) {
            // 回退目标字符与当前字符相同：直接沿用目标的 nextval，继续往前跳
            nextval[i] = nextval[k];
        } else {
            nextval[i] = k;         // 不同则保持不变
        }
    }
    return nextval;
}

// nextval 的可执行版本：用于真正驱动 KMP。
//
// 与展示表的关键差别是下标语义。KMP 里 j 表示“已匹配的模式串长度”，
// 在 pattern[j] 处失配时应退到 next[j-1] 而不是 next[j]，所以优化目标
// 也要按 next[j-1] 取值：
//   k = next[j-1]；若 pattern[j] == pattern[k]，则 nv[j] = nv[k]，
//   否则 nv[j] = k。nv[0] 恒为 0。
//
// 对照 "AAAAB"：next = 0 1 2 3 0，本函数得到 0 0 0 0 3，
// 最后一位保留 3，失配时不必退回开头，这才是能正确匹配的表。
std::vector<int> dsaac::getNextValTable(const std::string& pattern)
{
    const int m = static_cast<int>(pattern.length());
    const std::vector<int> next = getNext(pattern);
    std::vector<int> nv(static_cast<std::size_t>(m > 0 ? m : 0), 0);

    for (int j = 1; j < m; j++) {
        const int k = next[j - 1];
        if (pattern[j] == pattern[k]) {
            nv[j] = nv[k];   // 回退后必然再失配，直接复用它已经算好的跳转
        } else {
            nv[j] = k;
        }
    }
    return nv;
}

// 用 getNextValTable 驱动的 KMP；失配时 j = nv[j]，与表的下标语义一致。
// 时间 O(n + m)，空间 O(m)，结果与 kmpMatch 完全相同。
int dsaac::kmpMatchNextVal(const std::string& text, const std::string& pattern)
{
    const int n = static_cast<int>(text.length());
    const int m = static_cast<int>(pattern.length());

    if (m == 0) return 0;
    if (m > n) return -1;

    const std::vector<int> nv = getNextValTable(pattern);

    int j = 0;
    for (int i = 0; i < n; i++) {
        while (j > 0 && text[i] != pattern[j]) {
            j = nv[j];   // 注意是 nv[j] 而非 nv[j-1]
        }
        if (text[i] == pattern[j]) {
            j++;
        }
        if (j == m) {
            return i - m + 1;
        }
    }
    return -1;
}

/* END */