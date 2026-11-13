// forest-impl.hpp
// 森林与二叉树转换、森林遍历的实现。
//
// 对应 README 的「任务内容」四个函数：
//   1. forest_to_binary_tree()  —— 森林转二叉树（左孩子右兄弟）
//   2. binary_tree_to_forest()  —— 二叉树还原为森林
//   3. forest_preorder()        —— 森林前序遍历
//   4. forest_postorder()       —— 森林后序遍历
// 另有两个二叉树遍历，只用于演示时打印对照，不属于任务要求。
//
// 四个函数的时间复杂度都是 O(n)，n 为森林（或二叉树）的节点数，见 README。

#ifndef MYLAB_TREE_FOREST_IMPL_HPP
#define MYLAB_TREE_FOREST_IMPL_HPP

#include <cstddef>
#include <utility>
#include <vector>

#include "forest.h"

namespace dsaac {
namespace detail {

// 一般树 -> 二叉树（左孩子右兄弟）。
//   根      -> 新节点
//   第一个孩子        -> 新节点的 left
//   其余孩子按次序    -> 左孩子 right 链上的后续节点
// 每个节点恰好被创建一次，因此整棵树 O(n)。
template <typename T>
std::unique_ptr<BinaryTreeNode<T>> tree_to_binary(const ForestNode<T> &node) {
    auto out = BinaryTreeNode<T>::get_new_node(node.data);
    if (node.children.empty()) {
        return out;
    }

    out->left = tree_to_binary(*node.children.front());
    BinaryTreeNode<T> *tail = out->left.get();
    for (std::size_t i = 1; i < node.children.size(); ++i) {
        tail->right = tree_to_binary(*node.children[i]);
        tail = tail->right.get();
    }
    return out;
}

// 二叉树的一个节点 -> 一般树的一个节点（只处理该节点及其下方，不碰 right 兄弟）。
//   left        -> 第一个孩子
//   left->right -> 第二个孩子，依此类推
template <typename T>
std::unique_ptr<ForestNode<T>> binary_node_to_tree(const BinaryTreeNode<T> &node) {
    auto out = ForestNode<T>::get_new_node(node.data);
    for (const BinaryTreeNode<T> *child = node.left.get(); child != nullptr;
         child = child->right.get()) {
        // 注意：递归只消费 child 的 left 链，child->right 由本层循环接管，
        // 所以同一个兄弟节点不会被重复创建。
        out->children.push_back(binary_node_to_tree(*child));
    }
    return out;
}

// 前序遍历一棵一般树，把节点数据依次追加到 out。
template <typename T>
void preorder_tree(const ForestNode<T> &node, std::vector<T> &out) {
    out.push_back(node.data);
    for (const auto &child : node.children) {
        preorder_tree(*child, out);
    }
}

// 后序遍历一棵一般树。
template <typename T>
void postorder_tree(const ForestNode<T> &node, std::vector<T> &out) {
    for (const auto &child : node.children) {
        postorder_tree(*child, out);
    }
    out.push_back(node.data);
}

// 二叉树前序 / 中序。
template <typename T>
void preorder_binary(const BinaryTreeNode<T> *node, std::vector<T> &out) {
    if (node == nullptr) {
        return;
    }
    out.push_back(node->data);
    preorder_binary(node->left.get(), out);
    preorder_binary(node->right.get(), out);
}

template <typename T>
void inorder_binary(const BinaryTreeNode<T> *node, std::vector<T> &out) {
    if (node == nullptr) {
        return;
    }
    inorder_binary(node->left.get(), out);
    out.push_back(node->data);
    inorder_binary(node->right.get(), out);
}

} // namespace detail

// ---------------------------------------------------------------------------
// 演示用的二叉树遍历（不属于题面的四个必做函数）
// ---------------------------------------------------------------------------

template <typename T>
std::vector<T> binary_preorder(const BinaryTreeNode<T> *root) {
    std::vector<T> out;
    detail::preorder_binary(root, out);
    return out;
}

template <typename T>
std::vector<T> binary_inorder(const BinaryTreeNode<T> *root) {
    std::vector<T> out;
    detail::inorder_binary(root, out);
    return out;
}

// ---------------------------------------------------------------------------
// 四个必做函数
// ---------------------------------------------------------------------------

// 森林 -> 二叉树：第一棵树的根是二叉树的根，其余各树的根依次挂在它的 right 链上。
template <typename T>
std::unique_ptr<BinaryTreeNode<T>>
forest_to_binary_tree(const Forest<T> &forest) {
    if (forest.empty()) {
        return nullptr;
    }

    std::unique_ptr<BinaryTreeNode<T>> root =
        detail::tree_to_binary(*forest.front());
    BinaryTreeNode<T> *tail = root.get();
    for (std::size_t i = 1; i < forest.size(); ++i) {
        tail->right = detail::tree_to_binary(*forest[i]);
        tail = tail->right.get();
    }
    return root;
}

// 二叉树 -> 森林：沿根的 right 链，每个节点对应森林中一棵树的根。
template <typename T>
Forest<T> binary_tree_to_forest(const std::unique_ptr<BinaryTreeNode<T>> &root) {
    Forest<T> forest;
    for (const BinaryTreeNode<T> *tree_root = root.get(); tree_root != nullptr;
         tree_root = tree_root->right.get()) {
        forest.push_back(detail::binary_node_to_tree(*tree_root));
    }
    return forest;
}

// 森林前序遍历：逐棵树，先根后子树。结果与对应二叉树的前序相同。
template <typename T>
std::vector<T> forest_preorder(const Forest<T> &forest) {
    std::vector<T> out;
    for (const auto &root : forest) {
        detail::preorder_tree(*root, out);
    }
    return out;
}

// 森林后序遍历：逐棵树，先子树后根。结果与对应二叉树的中序相同。
template <typename T>
std::vector<T> forest_postorder(const Forest<T> &forest) {
    std::vector<T> out;
    for (const auto &root : forest) {
        detail::postorder_tree(*root, out);
    }
    return out;
}

} // namespace dsaac

#endif // MYLAB_TREE_FOREST_IMPL_HPP