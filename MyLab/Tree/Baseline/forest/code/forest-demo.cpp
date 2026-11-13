// forest-demo.cpp
// 森林 / 二叉树转换与遍历的演示程序，同时是本任务的测试驱动。
//
// 从标准输入按 tree_io.h 的格式读入一片森林，输出固定三节、每节两组：
//
//   1. 原森林的前序、后序遍历；
//   2. 转换所得二叉树的前序、中序遍历
//      （用于验证「森林前序 = 二叉树前序」「森林后序 = 二叉树中序」）；
//   3. 由二叉树还原出的森林的前序、后序遍历（用于验证往返一致）。
//
// 输入不合法时打印 "ERROR: ..." 到 stderr 并返回 1，stdout 不产生任何输出。

#include <exception>
#include <ios>
#include <iostream>

#include "forest-impl.hpp"
#include "forest.h"
#include "tree_io.h"

int main() {
    std::ios::sync_with_stdio(false);

    try {
        const dsaac::Forest<char> forest = dsaac::io::read_forest<char>(std::cin);
        std::ostream &out = std::cout;

        dsaac::io::print_section(out, "Original Forest");
        dsaac::io::print_line(out, "Forest preorder:", dsaac::forest_preorder(forest));
        dsaac::io::print_line(out, "Forest postorder:", dsaac::forest_postorder(forest));
        out << '\n';

        const std::unique_ptr<dsaac::BinaryTreeNode<char>> binary =
            dsaac::forest_to_binary_tree(forest);
        dsaac::io::print_section(out, "Binary Tree (from forest)");
        dsaac::io::print_line(out, "Binary preorder:", dsaac::binary_preorder(binary.get()));
        dsaac::io::print_line(out, "Binary inorder:", dsaac::binary_inorder(binary.get()));
        out << '\n';

        const dsaac::Forest<char> recovered = dsaac::binary_tree_to_forest(binary);
        dsaac::io::print_section(out, "Recovered Forest");
        dsaac::io::print_line(out, "Forest preorder:", dsaac::forest_preorder(recovered));
        dsaac::io::print_line(out, "Forest postorder:", dsaac::forest_postorder(recovered));
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
    return 0;
}