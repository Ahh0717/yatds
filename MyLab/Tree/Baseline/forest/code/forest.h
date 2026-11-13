// forest.h
// 森林与二叉树（左孩子右兄弟表示法）的数据结构定义与接口声明。
//
// 本文件只放数据结构与函数声明，不含任何算法实现；
// 四个待实现函数位于 forest-impl.hpp。
//
// 约定：所有数据结构与算法都在 namespace dsaac 中。

#ifndef MYLAB_TREE_FOREST_H
#define MYLAB_TREE_FOREST_H

#include <memory>
#include <utility>
#include <vector>

namespace dsaac {

// 一般树节点：用 vector 存孩子，支持任意度数；所有权由 unique_ptr 管理。
template <typename T>
struct ForestNode {
    T data;
    std::vector<std::unique_ptr<ForestNode>> children;

    ForestNode(const T &value) : data(value), children() {}
    ForestNode(T &&value) : data(std::move(value)), children() {}

    static std::unique_ptr<ForestNode> get_new_node(const T &value) {
        return std::make_unique<ForestNode>(value);
    }
    static std::unique_ptr<ForestNode> get_new_node(T &&value) {
        return std::make_unique<ForestNode>(std::move(value));
    }
};

// 森林：各棵树的根节点依次排列。
template <typename T>
using Forest = std::vector<std::unique_ptr<ForestNode<T>>>;

// 二叉树节点：固定 left / right 两个子节点。
template <typename T>
struct BinaryTreeNode {
    T data;
    std::unique_ptr<BinaryTreeNode> left;
    std::unique_ptr<BinaryTreeNode> right;

    BinaryTreeNode(const T &value) : data(value), left(), right() {}
    BinaryTreeNode(T &&value) : data(std::move(value)), left(), right() {}

    static std::unique_ptr<BinaryTreeNode> get_new_node(const T &value) {
        return std::make_unique<BinaryTreeNode>(value);
    }
    static std::unique_ptr<BinaryTreeNode> get_new_node(T &&value) {
        return std::make_unique<BinaryTreeNode>(std::move(value));
    }
};

// ---------------------------------------------------------------------------
// 四个待实现函数（forest-impl.hpp）
//
// 本文件只放数据结构定义与这四个函数的声明；演示程序用到的二叉树遍历辅助
// 函数声明在 forest-impl.hpp 中，不属于题面要求的部分。
// ---------------------------------------------------------------------------

// 森林 -> 二叉树（左孩子右兄弟）。
// 第一个孩子成为左孩子，其余孩子依次挂在左孩子的 right 链上；
// 森林中第一棵树的根作为二叉树的根，后续各树的根依次挂在根节点的 right 链上。
// 空森林返回 nullptr。时间 O(森林节点数)。
template <typename T>
std::unique_ptr<BinaryTreeNode<T>>
forest_to_binary_tree(const Forest<T> &forest);

// 二叉树 -> 森林，forest_to_binary_tree 的逆过程。
// 沿二叉树的 right 链得到森林中每棵树的根；对每个根，沿 left->right 链
// 得到该根的全部孩子。空指针返回空森林。时间 O(二叉树节点数)。
template <typename T>
Forest<T> binary_tree_to_forest(const std::unique_ptr<BinaryTreeNode<T>> &root);

// 森林前序遍历，按「逐棵树、先根后子树」的顺序把节点数据依次写入结果。
// 与对应二叉树的前序遍历结果相同。时间 O(森林节点数)，额外空间 O(树高)。
template <typename T>
std::vector<T> forest_preorder(const Forest<T> &forest);

// 森林后序遍历，按「逐棵树、先子树后根」的顺序把节点数据依次写入结果。
// 与对应二叉树的中序遍历结果相同。时间 O(森林节点数)，额外空间 O(树高)。
template <typename T>
std::vector<T> forest_postorder(const Forest<T> &forest);

} // namespace dsaac

#endif // MYLAB_TREE_FOREST_H