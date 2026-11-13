// huffman-tree.h
// 哈夫曼树的数据结构定义与接口声明。
//
// 本文件只放数据结构与函数声明，不含任何算法实现；
// 待实现函数位于 huffman-tree-impl.hpp。
//
// 约定：所有数据结构与算法都在 namespace dsaac 中。

#ifndef MYLAB_TREE_HUFFMAN_TREE_H
#define MYLAB_TREE_HUFFMAN_TREE_H

#include <array>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace dsaac {

// 通用 N 叉树节点：N 为子节点个数（默认 2，即二叉树）。
template <typename T, std::size_t N = 2>
struct Node {
    T data;
    std::array<std::unique_ptr<Node>, N> children;

    Node(const T &value) : data(value), children() {}
    Node(T &&value) : data(std::move(value)), children() {}

    static std::unique_ptr<Node> get_new_node(const T &value) {
        return std::make_unique<Node>(value);
    }
    static std::unique_ptr<Node> get_new_node(T &&value) {
        return std::make_unique<Node>(std::move(value));
    }
};

// 哈夫曼树节点的载荷：叶子存字符，内部节点为 nullopt；weight 为该子树权值和。
template <typename data_t, typename weight_t>
struct HuffmanTreeNodeData {
    std::optional<data_t> data;
    weight_t weight;
};

template <typename data_t, typename weight_t>
using HuffmanTreeNode = Node<HuffmanTreeNodeData<data_t, weight_t>, 2>;

// 由 (字符, 权值) 列表构造哈夫曼树，返回根节点；leaves 为空时返回 nullptr。
// 算法：最小堆贪心，反复合并权值最小的两棵子树，直到只剩一棵。
// 叶子权值相同时的合并顺序会影响具体的编码形状，但不影响 WPL；
// 本实现按「权值升序，权值相同则先创建者优先」打破平局，以保证输出可复现。
//
// 单字符边界：leaves 只含一个元素时，贪心合并一次都不会发生，根即叶子。
// 此时约定该字符的编码长度为 1（见 build_code_table），因此 WPL 等于它的
// 频次，而不是 0。这样「原文仍可写出比特流」，与样例 case02 = 1、
// case03 = 6 的预期一致。
//
// 时间 O(n log n)，空间 O(n)，n 为叶子数。
template <typename data_t, typename weight_t>
std::unique_ptr<HuffmanTreeNode<data_t, weight_t>>
construct_huffman_tree(const std::vector<std::pair<data_t, weight_t>> &leaves);

// 自根做一次 DFS 生成编码表：向左为 '0'，向右为 '1'，叶子处记录完整路径。
// 返回 map<字符, 编码>；map 按 data_t 升序排列，因此打印结果与 DFS 顺序无关。
// tree 为空时返回空表。
//
// 单字符边界：树只有一个叶子（根即叶子，路径为空）时，该字符记作 "0"，
// 编码长度取 1 而不是 0——空编码会让「原文 → 比特流」不可逆。
//
// 时间 O(n log n)（含 map 插入），额外空间 O(n)。
template <typename data_t, typename weight_t>
std::map<data_t, std::string>
build_code_table(const HuffmanTreeNode<data_t, weight_t> *tree);

// 由编码表与频次计算带权路径长度：Σ(频次 × 该字符编码长度)。
// 这正是编码整段文本所需的比特总数，也是本任务的评分依据。
// frequencies 中每个字符都必须出现在 codes 里，否则返回 weight_t{}。
// 时间 O(n log n)，n 为不同字符数。
template <typename data_t, typename weight_t>
weight_t huffman_wpl(const std::map<data_t, std::string> &codes,
                     const std::vector<std::pair<data_t, weight_t>> &frequencies);

} // namespace dsaac

#endif // MYLAB_TREE_HUFFMAN_TREE_H