#ifndef DSAAC_PATTERN_MATCH_HPP
#define DSAAC_PATTERN_MATCH_HPP

#include <string>
#include <vector>

namespace dsaac {

int naiveMatch(const std::string& text, const std::string& pattern);
std::vector<int> getNext(const std::string& pattern);
int kmpMatch(const std::string& text, const std::string& pattern);

// Step 3 进阶优化要求：
std::vector<int> getNextVal(const std::string& pattern);

// 与 getNextVal 的展示表配套：返回可直接用于 0-based KMP 的跳转表。
// nv[j] 表示“已匹配 j 个字符、在 pattern[j] 处失配”时应退回的已匹配长度。
// 用法：while (j > 0 && text[i] != pattern[j]) j = nv[j];
std::vector<int> getNextValTable(const std::string& pattern);

// 用 getNextValTable 驱动的 KMP，等价于 kmpMatch 的结果。
int kmpMatchNextVal(const std::string& text, const std::string& pattern);

} // namespace dsaac

// 为了方便使用，将方法暴露在全局（也可按需不暴露）
using dsaac::naiveMatch;
using dsaac::getNext;
using dsaac::kmpMatch;
using dsaac::getNextVal;
using dsaac::getNextValTable;
using dsaac::kmpMatchNextVal;

#endif // DSAAC_PATTERN_MATCH_HPP