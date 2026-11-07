// stackli.cpp
// LinkedStack<T> 的实现 + int / std::string 的显式实例化（编译期自检）。
//
// forward_list 的 begin() 就是栈顶，因此：
//   push -> push_front，pop -> pop_front，top -> front，全部 O(1)。
// 打印“栈顶到栈底”只需从头顺着 next 走一遍；反过来（toVector）需要
// 先整体读出再逆序，代价 O(n)。

#include "stackli.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace dsaac {

/* START: linked_stack implementation */

template <typename T>
bool LinkedStack<T>::empty() const {
    return values_.empty();
}

template <typename T>
void LinkedStack<T>::push(const T& value) {
    values_.push_front(value);
    ++size_;
}

template <typename T>
void LinkedStack<T>::pop() {
    checkNotEmpty("pop");
    values_.pop_front();
    --size_;
}

template <typename T>
const T& LinkedStack<T>::top() const {
    checkNotEmpty("top");
    return values_.front();
}

template <typename T>
void LinkedStack<T>::checkNotEmpty(const char* what) const {
    if (values_.empty()) {
        throw std::out_of_range(std::string("LinkedStack::") + what +
                                ": stack is empty");
    }
}

/* END */

// -------------------------------------------------------------------
// 扩展功能
// -------------------------------------------------------------------

template <typename T>
void LinkedStack<T>::swap(LinkedStack& other) noexcept {
    values_.swap(other.values_);
    std::swap(size_, other.size_);
}

template <typename T>
std::vector<T> LinkedStack<T>::toVector() const {
    // 头部是栈顶，直接读出来是“栈顶 -> 栈底”，逆序后才是“栈底 -> 栈顶”。
    std::vector<T> out;
    out.reserve(size_);
    for (const T& value : values_) {
        out.push_back(value);
    }
    std::reverse(out.begin(), out.end());
    return out;
}

template <typename T>
void LinkedStack<T>::assign(const LinkedStack& other) {
    if (this == &other) {
        return;
    }
    clear();
    // 先把 other 读成“栈底 -> 栈顶”，再按这个顺序压入，压完形状与 other 一致。
    const std::vector<T> bottomToTop = other.toVector();
    for (const T& value : bottomToTop) {
        push(value);
    }
}

template <typename T>
void LinkedStack<T>::setTop(const T& value) {
    checkNotEmpty("setTop");
    values_.front() = value;
}

template <typename T>
bool LinkedStack<T>::operator==(const LinkedStack& other) const {
    return size_ == other.size_ && std::equal(values_.begin(), values_.end(),
                                              other.values_.begin());
}

template <typename T>
void LinkedStack<T>::printTopToBottom(std::ostream& os) const {
    if (values_.empty()) {
        os << "EMPTY";
        return;
    }
    bool first = true;
    for (const T& value : values_) {
        if (!first) {
            os << ' ';
        }
        os << value;
        first = false;
    }
}

// 显式实例化：如果模板实现写错，这里会直接编译失败。
template class LinkedStack<int>;
template class LinkedStack<std::string>;

} // namespace dsaac