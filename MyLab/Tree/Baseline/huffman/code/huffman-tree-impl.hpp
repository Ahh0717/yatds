// huffman-tree-impl.hpp
// 哈夫曼树构造、编码表与 WPL 的实现。
//
// 对应 README 的「任务内容」三个函数：
//   1. construct_huffman_tree() —— 最小堆贪心构造哈夫曼树
//   2. build_code_table()      —— DFS 生成 map<字符, 编码>
//   3. huffman_wpl()           —— 用编码表算 Σ(频次 × 码长)，即评分依据
//
// 确定性约定（样例逐行比对依赖它）：
//   每个节点带一个 id。叶子按 leaves 的输入顺序取 0, 1, 2, ...；
//   内部节点用递增计数器继续编号。堆按「weight 小的优先；weight 相同则
//   id 小的优先」弹出，因此同一份输入永远得到同一棵树、同一张编码表。

#ifndef MYLAB_TREE_HUFFMAN_TREE_IMPL_HPP
#define MYLAB_TREE_HUFFMAN_TREE_IMPL_HPP

#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "huffman-tree.h"

namespace dsaac {
namespace detail {

// 堆里的一项：节点本体 + 决定平局次序的 id。
// 用 unique_ptr 持有节点，合并时整体移动，不做拷贝。
template <typename data_t, typename weight_t>
struct HeapEntry {
    std::unique_ptr<HuffmanTreeNode<data_t, weight_t>> node;
    std::size_t id;
};

// std::make_heap 建的是大顶堆：comp(a, b) == true 表示 a 应排在 b 之后。
// 要「weight 小的先弹、weight 相同 id 小的先弹」，比较器就得反着写：
//   当 a.weight > b.weight，或两者相等且 a.id > b.id 时，a 排在 b 后面。
template <typename data_t, typename weight_t>
struct HeapGreater {
    bool operator()(const HeapEntry<data_t, weight_t> &a,
                    const HeapEntry<data_t, weight_t> &b) const {
        const weight_t wa = a.node->data.weight;
        const weight_t wb = b.node->data.weight;
        if (wa != wb) {
            return wa > wb;
        }
        return a.id > b.id;
    }
};

// 叶子载荷：data 有值，weight 为频次。
template <typename data_t, typename weight_t>
HuffmanTreeNodeData<data_t, weight_t> make_leaf_payload(const data_t &value,
                                                        weight_t weight) {
    return HuffmanTreeNodeData<data_t, weight_t>{std::optional<data_t>(value),
                                                 weight};
}

// 内部节点载荷：data 为 nullopt，weight 为两棵子树权值之和。
template <typename data_t, typename weight_t>
HuffmanTreeNodeData<data_t, weight_t> make_internal_payload(weight_t weight) {
    return HuffmanTreeNodeData<data_t, weight_t>{std::nullopt, weight};
}

// DFS 收集编码：左孩子记 '0'，右孩子记 '1'，到叶子时把当前路径写入 codes。
// 调用前保证 tree 不是叶子（单叶子的特殊情况由 build_code_table 直接处理）。
template <typename data_t, typename weight_t>
void collect_codes(const HuffmanTreeNode<data_t, weight_t> *node,
                   std::string &path, std::map<data_t, std::string> &codes) {
    const HuffmanTreeNodeData<data_t, weight_t> &payload = node->data;

    if (payload.data.has_value()) {
        codes.emplace(*payload.data, path);
        return;
    }

    path.push_back('0');
    collect_codes(node->children[0].get(), path, codes);
    path.back() = '1';
    collect_codes(node->children[1].get(), path, codes);
    path.pop_back();
}

} // namespace detail

// ---------------------------------------------------------------------------
// 最小堆贪心构造
// ---------------------------------------------------------------------------

template <typename data_t, typename weight_t>
std::unique_ptr<HuffmanTreeNode<data_t, weight_t>>
construct_huffman_tree(const std::vector<std::pair<data_t, weight_t>> &leaves) {
    if (leaves.empty()) {
        return nullptr;
    }

    // 第 i 个叶子的 id 就是 i，与 leaves 的输入顺序一致。
    std::vector<detail::HeapEntry<data_t, weight_t>> heap;
    heap.reserve(leaves.size());
    std::size_t next_id = 0;
    for (const std::pair<data_t, weight_t> &leaf : leaves) {
        heap.push_back({HuffmanTreeNode<data_t, weight_t>::get_new_node(
                            detail::make_leaf_payload(leaf.first, leaf.second)),
                        next_id});
        ++next_id;
    }

    detail::HeapGreater<data_t, weight_t> greater;
    std::make_heap(heap.begin(), heap.end(), greater);

    while (heap.size() > 1) {
        // 第一次弹出 weight/id 最小的，作为左孩子。
        std::pop_heap(heap.begin(), heap.end(), greater);
        std::unique_ptr<HuffmanTreeNode<data_t, weight_t>> node1 =
            std::move(heap.back().node);
        heap.pop_back();

        // 第二次弹出的作为右孩子。
        std::pop_heap(heap.begin(), heap.end(), greater);
        std::unique_ptr<HuffmanTreeNode<data_t, weight_t>> node2 =
            std::move(heap.back().node);
        heap.pop_back();

        // 合并成一个内部节点：data 为 nullopt，weight 为两者之和。
        std::unique_ptr<HuffmanTreeNode<data_t, weight_t>> merged =
            HuffmanTreeNode<data_t, weight_t>::get_new_node(
                detail::make_internal_payload<data_t, weight_t>(
                    node1->data.weight + node2->data.weight));
        merged->children[0] = std::move(node1);
        merged->children[1] = std::move(node2);

        heap.push_back({std::move(merged), next_id});
        ++next_id;
        std::push_heap(heap.begin(), heap.end(), greater);
    }

    // 堆里只剩下根。
    return std::move(heap.front().node);
}

// ---------------------------------------------------------------------------
// 编码表
// ---------------------------------------------------------------------------

template <typename data_t, typename weight_t>
std::map<data_t, std::string>
build_code_table(const HuffmanTreeNode<data_t, weight_t> *tree) {
    std::map<data_t, std::string> codes;
    if (tree == nullptr) {
        return codes;
    }

    // 单叶子的退化情形：根即叶子，路径为空。空编码无法写出比特流，
    // 因此约定记作 "0"（长度为 1）。
    if (tree->data.data.has_value()) {
        codes.emplace(*tree->data.data, std::string("0"));
        return codes;
    }

    std::string path;
    detail::collect_codes(tree, path, codes);
    return codes;
}

// ---------------------------------------------------------------------------
// 带权路径长度
// ---------------------------------------------------------------------------

template <typename data_t, typename weight_t>
weight_t huffman_wpl(const std::map<data_t, std::string> &codes,
                     const std::vector<std::pair<data_t, weight_t>> &frequencies) {
    weight_t total{};
    for (const std::pair<data_t, weight_t> &entry : frequencies) {
        const auto found = codes.find(entry.first);
        if (found == codes.end()) {
            // 编码表不完整：无法给出有意义的 WPL。
            return weight_t{};
        }
        total += entry.second * static_cast<weight_t>(found->second.size());
    }
    return total;
}

} // namespace dsaac

#endif // MYLAB_TREE_HUFFMAN_TREE_IMPL_HPP