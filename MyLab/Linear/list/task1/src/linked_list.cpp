// linked_list.cpp
// 模板类的“非模板化”部分与显式实例化。
//
// 说明：模板类的成员函数必须以头文件形式对使用者可见（否则每个成员都无法在
// 通用情形下被实例化），因此完整的实现放在 linked_list.h 中。本文件承担两个
// 职责：
//   1. 显式实例化 int 与 std::string 两种类型，起到“编译期自检”的作用：
//      只要任一模板成员在两种类型下无法编译，本文件就会报错；
//   2. 提供与模板无关的辅助实现（如输出运算符），避免头文件里堆砌非模板代码。

#include "linked_list.h"

#include <ostream>

namespace mylab {

// 输出运算符：便于 std::cout << list 的写法。链表为空时不输出内容。
template <typename T>
std::ostream &operator<<(std::ostream &os, const LinkedList<T> &list) {
    list.print(os);
    return os;
}

// 显式实例化（编译期自检 + 减少使用方重复实例化带来的编译时间）。
template class LinkedList<int>;
template class LinkedList<std::string>;

template std::ostream &operator<<(std::ostream &os, const LinkedList<int> &list);
template std::ostream &operator<<(std::ostream &os, const LinkedList<std::string> &list);

}  // namespace mylab