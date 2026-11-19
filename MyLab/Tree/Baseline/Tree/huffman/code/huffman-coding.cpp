// huffman-coding.cpp
// 哈夫曼编码的演示程序，同时是本任务的测试驱动。
//
// 非交互式：从标准输入读**第一行**整行文本，统计每个字节（char）的出现次数，
// 构造哈夫曼树，输出每个字符的编码与该文本编码后的总比特数（WPL）。
//
// 输入：
//   - 只读第一行，用 std::getline 读整行，可以含空格，不做 trim；
//   - 空行、空文件都当作空串处理（没有字符）。
//
// 输出（三段，顺序固定）：
//
//   char <c> cnt <n>       每字符一行，按 (次数升序, 字符升序) 排列
//   char <c> code <bits>   每字符一行，顺序与第一段完全一致
//   WPL <total>            编码整段文本所需的比特总数 = Σ(次数 × 码长)
//
//   字符位置直接打印该字符本身（空格就原样打一个空格），不做转义。
//   因此空格那一行看起来是 "char   cnt 1"。
//   空输入时只输出两行：一行 "EMPTY"，一行 "WPL 0"。
//
// 编码的具体形状取决于叶子权值相同时的合并顺序。题面明确「不要求编码与参考
// 实现完全一致，只要 WPL 相同即可」；本实现按 huffman-tree.h 注释里写明的
// 平局规则（权值相同则先创建者优先）产生唯一、可复现的结果，因此样例可以
// 严格逐行比对。

#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "huffman-tree-impl.hpp"
#include "huffman-tree.h"

namespace {

using Data = char;
using Weight = int;
using CodeTable = std::map<Data, std::string>;

// 字符与其出现次数。
struct CharCount {
    Data ch;
    Weight count;
};

// 按 (次数升序, 字符升序) 排序，与题面样例的输出顺序一致。
// 比较时先转成 unsigned char，避免高位为 1 的字节参与有符号比较而乱序。
bool char_count_less(const CharCount &lhs, const CharCount &rhs) {
    if (lhs.count != rhs.count) {
        return lhs.count < rhs.count;
    }
    return static_cast<unsigned char>(lhs.ch) <
           static_cast<unsigned char>(rhs.ch);
}

} // namespace

int main() {
    std::ios::sync_with_stdio(false);

    // 只读第一行；空行得到空串，读不到（空文件）也按空串处理。
    std::string line;
    if (!std::getline(std::cin, line)) {
        line.clear();
    }

    // 字节频次：用 256 个桶覆盖全部 char 取值（含高位为 1 的字节）。
    std::array<Weight, 256> freq{};
    for (const char raw : line) {
        ++freq[static_cast<unsigned char>(raw)];
    }

    std::vector<CharCount> items;
    for (std::size_t i = 0; i < freq.size(); ++i) {
        if (freq[i] > 0) {
            items.push_back({static_cast<char>(i), freq[i]});
        }
    }
    std::sort(items.begin(), items.end(), char_count_less);

    // 空输入不提前返回：仍然走完整的构造 → 编码 → WPL 链路，
    // 让实现必须正确处理「没有字符」这一边界，而不是靠调用方特判绕过。
    try {
        std::vector<std::pair<Data, Weight>> leaves;
        leaves.reserve(items.size());
        for (const CharCount &item : items) {
            leaves.push_back({item.ch, item.count});
        }

        const std::unique_ptr<dsaac::HuffmanTreeNode<Data, Weight>> tree =
            dsaac::construct_huffman_tree<Data, Weight>(leaves);
        const CodeTable codes =
            dsaac::build_code_table<Data, Weight>(tree.get());
        const Weight wpl =
            dsaac::huffman_wpl<Data, Weight>(codes, leaves);

        // 空字符集：一行 EMPTY + 一行 WPL 0。
        if (items.empty()) {
            std::cout << "EMPTY\n";
            std::cout << "WPL 0\n";
            return 0;
        }

        // 第一段：频次。
        for (const CharCount &item : items) {
            std::cout << "char " << item.ch << " cnt " << item.count << '\n';
        }
        // 第二段：编码，顺序与上一段完全一致。
        for (const CharCount &item : items) {
            const auto found = codes.find(item.ch);
            if (found == codes.end()) {
                throw std::logic_error("编码表缺少字符");
            }
            std::cout << "char " << item.ch << " code " << found->second
                      << '\n';
        }
        // 第三段：总比特数。
        std::cout << "WPL " << wpl << '\n';
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
    return 0;
}