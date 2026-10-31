// vector_stack.cpp
// VectorStack<T> 的实现 + int / std::string 的显式实例化。
//
// 这里包含 vector_queue.h 而不是 vector_stack.h：显式实例化会实例化
// toQueue()，而它的返回类型 VectorQueue<T> 在定义处必须是完整类型。

#include "vector_queue.h"

#include <stdexcept>
#include <string>

namespace dsaac {

/* START: vector_stack implementation */

template <typename T>
void VectorStack<T>::checkNotEmpty(const char* what) const {
    if (data_.empty()) {
        throw std::out_of_range(std::string("VectorStack::") + what +
                                ": stack is empty");
    }
}

template <typename T>
void VectorStack<T>::pop() {
    checkNotEmpty("pop");
    data_.pop_back();  // 栈顶就在尾部
}

template <typename T>
const T& VectorStack<T>::top() const {
    checkNotEmpty("top");
    return data_.back();
}

/* END */

// -------------------------------------------------------------------
// 扩展功能
// -------------------------------------------------------------------

template <typename T>
void VectorStack<T>::printTopToBottom(std::ostream& os) const {
    if (data_.empty()) {
        os << "EMPTY";
        return;
    }
    for (auto it = data_.rbegin(); it != data_.rend(); ++it) {
        if (it != data_.rbegin()) {
            os << ' ';
        }
        os << *it;
    }
}

// VectorStack::toQueue() —— 定义在这里而不是 vector_queue.cpp，是因为
// 显式实例化 VectorStack<T> 时编译器必须看得到 toQueue 的函数体，
// 否则只会得到一个没有定义的符号（链接期未定义引用）。
// 本文件已包含 vector_queue.h，VectorQueue<T> 在此处是完整类型。
template <typename T>
VectorQueue<T> VectorStack<T>::toQueue() const {
    // 语义：从栈顶逐个弹出、依次入队。栈顶先进入队首，于是
    // 队首 -> 队尾正好是原栈的“栈顶 -> 栈底”，即输入序列的反转。
    // 这与 VectorQueue::toStack()（从队首逐个出队再压栈）互为镜像。
    VectorQueue<T> queue;
    for (auto it = data_.rbegin(); it != data_.rend(); ++it) {
        queue.push(*it);
    }
    return queue;
}

// 显式实例化：如果模板实现写错，这里会直接编译失败。
template class VectorStack<int>;
template class VectorStack<std::string>;

} // namespace dsaac