// huffman.cpp
// 哈夫曼编码的实现：数组存树 + 最小堆贪心 + 一次 DFS 生成编码表。
//
// 树节点按数组下标组织，节点数固定为 2m - 1（m 为非零频次符号数）：
//   - 叶子编号 0 ~ m-1，内部节点编号 m ~ 2m-2，编号同时用作平局判定的 id；
//   - 每个节点四个字段：weight、left、right、symbol，叶子用 left == -1 表示。
// 这样不需要逐节点 new，平局时直接比较下标即可，同一份频次表永远得到同一棵树。

#include "huffman.h"

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace dsaac {
namespace {

// 树节点。叶子：left == -1，symbol 为 0..272；
// 内部节点：left/right 为子节点下标，symbol 填 0。
struct Node {
    std::uint64_t weight; // 叶子是频次，内部节点是两棵子树的权值和
    int left;
    int right;
    int symbol;
};

// std::make_heap 建的是大顶堆：comp(a, b) == true 表示 a 应排在 b 之后。
// 要「weight 小的先弹、weight 相同下标小的先弹」，比较器就得反着写。
struct HeapGreater {
    const std::vector<Node> *nodes;

    bool operator()(int lhs, int rhs) const {
        const std::uint64_t lw = (*nodes)[static_cast<std::size_t>(lhs)].weight;
        const std::uint64_t rw = (*nodes)[static_cast<std::size_t>(rhs)].weight;
        if (lw != rw) {
            return lw > rw;
        }
        return lhs > rhs;
    }
};

// 自根 DFS：向左加 '0'、向右加 '1'，到叶子把当前路径写进对应符号。
void collect_codes(const std::vector<Node> &nodes, int node, std::string &path,
                   std::array<std::string, 273> &codes) {
    const Node &current = nodes[static_cast<std::size_t>(node)];
    if (current.left == -1) {
        codes[static_cast<std::size_t>(current.symbol)] = path;
        return;
    }

    path.push_back('0');
    collect_codes(nodes, current.left, path, codes);
    path.back() = '1';
    collect_codes(nodes, current.right, path, codes);
    path.pop_back();
}

} // namespace

std::array<std::string, 273>
build_codes(const std::array<std::uint32_t, 273> &freq) {
    std::array<std::string, 273> codes{};

    // 频次为零的符号不建叶子；非零符号天然按符号值升序收集。
    std::vector<int> symbols;
    symbols.reserve(273);
    for (std::size_t symbol = 0; symbol < freq.size(); ++symbol) {
        if (freq[symbol] > 0) {
            symbols.push_back(static_cast<int>(symbol));
        }
    }

    const std::size_t count = symbols.size();
    if (count == 0) {
        // 调用方不会走到这里；按约定返回全空表。
        return codes;
    }
    if (count == 1) {
        // 整棵树只有一个节点，路径为空；记作 "0" 才能写出比特流。
        codes[static_cast<std::size_t>(symbols[0])] = "0";
        return codes;
    }

    // 叶子 0 ~ m-1，内部节点 m ~ 2m-2，共 2m-1 个节点，一次分配。
    std::vector<Node> nodes(2 * count - 1);
    for (std::size_t i = 0; i < count; ++i) {
        nodes[i].weight = freq[static_cast<std::size_t>(symbols[i])];
        nodes[i].left = -1;
        nodes[i].right = -1;
        nodes[i].symbol = symbols[i];
    }

    // 堆里放节点下标；编号小的优先，保证平局规则可复现。
    std::vector<int> heap(count);
    for (std::size_t i = 0; i < count; ++i) {
        heap[i] = static_cast<int>(i);
    }
    const HeapGreater greater{&nodes};
    std::make_heap(heap.begin(), heap.end(), greater);

    int next_id = static_cast<int>(count);
    while (heap.size() > 1) {
        // 先弹出的作左孩子。
        std::pop_heap(heap.begin(), heap.end(), greater);
        const int left = heap.back();
        heap.pop_back();

        // 后弹出的作右孩子。
        std::pop_heap(heap.begin(), heap.end(), greater);
        const int right = heap.back();
        heap.pop_back();

        Node &merged = nodes[static_cast<std::size_t>(next_id)];
        merged.weight = nodes[static_cast<std::size_t>(left)].weight +
                        nodes[static_cast<std::size_t>(right)].weight;
        merged.left = left;
        merged.right = right;
        merged.symbol = 0;

        heap.push_back(next_id);
        std::push_heap(heap.begin(), heap.end(), greater);
        ++next_id;
    }

    std::string path;
    collect_codes(nodes, heap.front(), path, codes);
    return codes;
}

std::uint64_t
huffman_wpl(const std::array<std::string, 273> &codes,
            const std::array<std::uint32_t, 273> &freq) {
    std::uint64_t total = 0;
    for (std::size_t symbol = 0; symbol < freq.size(); ++symbol) {
        total += static_cast<std::uint64_t>(freq[symbol]) *
                 static_cast<std::uint64_t>(codes[symbol].size());
    }
    return total;
}

} // namespace dsaac