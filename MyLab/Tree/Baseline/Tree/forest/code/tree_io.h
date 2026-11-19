// tree_io.h
// forest-demo 的输入解析与输出格式化，同时固定本任务的输入 / 输出格式。
//
// 本文件只做 I/O，不含森林与二叉树之间的转换、也不含遍历算法。
//
// ---------------------------------------------------------------------------
// 输入格式
// ---------------------------------------------------------------------------
// 把 stdin 全部按空白切成 token：
//
//   第一个 token：k —— 森林中树的棵数，k >= 0（k = 0 表示空森林）
//   之后 k 个 token：每棵树的括号表示法
//
// 节点标签是单个可见 ASCII 字符（0x21–0x7E），不能是空白、逗号或圆括号。
// 孩子写在紧跟标签的左括号内，以逗号分隔：
//
//   A            单个节点
//   E(F,G)       根 E，两个孩子 F、G
//   A(B(C(D)))   根 A，唯一孩子 B，唯一孩子 C，唯一孩子 D
//   A(B,C,D)     根 A，三个孩子 B、C、D
//
// 每个 token 用递归下降解析：先读一个字符作根；若紧跟 '('，则循环解析孩子，
// 逗号分隔，直到 ')'。不支持多字符标签。合法性由解析器校验，不合法的输入
// 抛 std::runtime_error，由 main 报告并以非 0 退出。
//
// ---------------------------------------------------------------------------
// 输出格式
// ---------------------------------------------------------------------------
// 固定三节、每节两组，共六组行；排版与题面 README 的「预期输出」逐字一致：
//
//   === 分节标题 ===
//   标签:             序列（元素之间单个空格）
//
// 标签左对齐到第 18 列后接序列；某一组序列为空时，该行到冒号就结束，
// 不留行尾空格。节与节之间隔一个空行，全文末尾不额外留空行。

#ifndef MYLAB_TREE_IO_H
#define MYLAB_TREE_IO_H

#include <cstddef>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "forest.h"

namespace dsaac {
namespace io {

namespace detail {

// 合法标签：可见 ASCII，且不是分隔用途的逗号或圆括号。
inline bool is_valid_label(char c) {
    const unsigned char value = static_cast<unsigned char>(c);
    if (value < 0x21 || value > 0x7E) {
        return false;
    }
    return c != ',' && c != '(' && c != ')';
}

// 递归下降：从 tok[pos] 开始解析一棵子树。
//   node := label [ '(' node (',' node)* ')' ]
// 成功时 pos 指向该子树之后的位置；失败时抛 std::runtime_error。
template <typename T>
std::unique_ptr<ForestNode<T>> parse_node(const std::string &tok,
                                          std::size_t &pos) {
    if (pos >= tok.size()) {
        throw std::runtime_error("意外的输入结尾：这里应当有一个节点标签");
    }
    const char label = tok[pos];
    if (!is_valid_label(label)) {
        throw std::runtime_error(std::string("非法节点标签 '") + label +
                                 "'：标签须是单个可见 ASCII 字符，"
                                 "且不能是逗号或圆括号");
    }
    ++pos;
    auto node = ForestNode<T>::get_new_node(T(label));

    if (pos < tok.size() && tok[pos] == '(') {
        ++pos;
        for (;;) {
            node->children.push_back(parse_node<T>(tok, pos));
            if (pos >= tok.size()) {
                throw std::runtime_error("括号未闭合：" + tok);
            }
            if (tok[pos] == ',') {
                ++pos;
                continue;
            }
            if (tok[pos] == ')') {
                ++pos;
                break;
            }
            throw std::runtime_error(
                std::string("孩子之后应为 ',' 或 ')', 实际是 '") +
                tok[pos] + "': " + tok);
        }
    }
    return node;
}

// 把 token 解析成非负十进制整数；不合法时抛 std::runtime_error。
inline int parse_count(const std::string &tok) {
    if (tok.empty()) {
        throw std::runtime_error("棵数 k 不能为空");
    }
    long long value = 0;
    for (const char c : tok) {
        if (c < '0' || c > '9') {
            throw std::runtime_error("棵数 k 必须是十进制非负整数，实际为 '" +
                                     tok + "'");
        }
        value = value * 10 + (c - '0');
        if (value > 1000000) {
            throw std::runtime_error("棵数 k 过大：" + tok);
        }
    }
    return static_cast<int>(value);
}

} // namespace detail

// 从 in 读取一片森林（格式见文件头），失败时抛 std::runtime_error。
template <typename T>
Forest<T> read_forest(std::istream &in) {
    std::vector<std::string> tokens;
    std::string token;
    while (in >> token) {
        tokens.push_back(token);
    }
    if (tokens.empty()) {
        throw std::runtime_error("输入为空：第一个 token 应当是棵数 k");
    }

    const int k = detail::parse_count(tokens.front());
    const std::size_t expected = static_cast<std::size_t>(k) + 1;
    if (tokens.size() < expected) {
        throw std::runtime_error(
            "声明了 " + std::to_string(k) + " 棵树，但只提供了 " +
            std::to_string(tokens.size() - 1) + " 个树的 token");
    }
    if (tokens.size() > expected) {
        throw std::runtime_error(
            "多余的 token：声明 " + std::to_string(k) + " 棵树，实际给了 " +
            std::to_string(tokens.size() - 1) + " 个");
    }

    Forest<T> forest;
    forest.reserve(static_cast<std::size_t>(k));
    for (std::size_t i = 0; i < static_cast<std::size_t>(k); ++i) {
        const std::string &tok = tokens[i + 1];
        std::size_t pos = 0;
        std::unique_ptr<ForestNode<T>> root = detail::parse_node<T>(tok, pos);
        if (pos != tok.size()) {
            throw std::runtime_error("树的表示之后有多余字符: " + tok);
        }
        forest.push_back(std::move(root));
    }
    return forest;
}

// 打印 "=== 标题 ===" 一行。
inline void print_section(std::ostream &out, const std::string &title) {
    out << "=== " << title << " ===\n";
}

// 打印一行 "标签 + 补齐到 18 列 + 单空格分隔的序列"。
// 序列为空时只输出标签（到冒号为止），不留行尾空格。
template <typename T>
void print_line(std::ostream &out, const std::string &label,
                const std::vector<T> &values) {
    out << label;
    if (!values.empty()) {
        const std::size_t width = 18;
        for (std::size_t i = label.size(); i < width; ++i) {
            out << ' ';
        }
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i != 0) {
                out << ' ';
            }
            out << values[i];
        }
    }
    out << '\n';
}

} // namespace io
} // namespace dsaac

#endif // MYLAB_TREE_IO_H