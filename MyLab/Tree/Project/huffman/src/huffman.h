// huffman.h
// 哈夫曼编码：273 符号空间上的编码表构造与带权路径长度。
//
// 本文件只放接口声明，实现位于 huffman.cpp。
//
// 符号空间（共 273 个，与题面一致）：
//   0   ~ 255   字面量字节
//   256 ~ 271   长度 3 ~ 18，符号 = 256 + (len - 3)
//   272         EOS（压缩数据结束）
//
// 树用数组存，不用指针：节点数固定为 2m - 1（m 为非零频次符号数，最多 273），
// 一次分配即可，省掉逐个 new 的开销；平局判定直接比下标，结果可复现。
// 每个节点四个字段：weight、left、right、symbol；叶子用 left == -1 表示。
//
// 确定性约定（编码端与解码端必须得到同一棵树）：
//   - 频次为零的符号不建叶子；非零符号按符号值升序排好；
//   - 叶子编号 0 ~ m-1，内部节点从 m 开始递增；
//   - 最小堆按 (weight, id) 弹出：weight 小的先弹，相同则 id 小的先弹；
//   - 先弹出的作左子树，后弹出的作右子树；DFS 时左 0 右 1。
//
// 退化情形：
//   - 频次表全为零：返回全空的编码表（调用方不会走到这种情况）；
//   - 只有一种符号：整棵树只有一个节点，该符号编码记作 "0"（长度 1），
//     不能是空串，否则比特流不可逆。

#ifndef MYLAB_TREE_PROJECT_HUFFMAN_H
#define MYLAB_TREE_PROJECT_HUFFMAN_H

#include <array>
#include <cstdint>
#include <string>

namespace dsaac {

// 由频次表构造每个符号的哈夫曼编码。
// 返回 273 项的字符串表，下标即符号；未出现的符号对应空串。
// 只有一种符号时，该符号的编码为 "0"。
std::array<std::string, 273>
build_codes(const std::array<std::uint32_t, 273> &freq);

// 带权路径长度：Σ(频次 × 该符号编码长度)，即编码整段数据所需的比特总数。
// 用于自检：与「所有内部节点权值之和」应完全一致。
std::uint64_t
huffman_wpl(const std::array<std::string, 273> &codes,
            const std::array<std::uint32_t, 273> &freq);

} // namespace dsaac

#endif // MYLAB_TREE_PROJECT_HUFFMAN_H