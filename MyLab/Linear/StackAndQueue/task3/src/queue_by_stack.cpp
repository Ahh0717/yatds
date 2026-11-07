// queue_by_stack.cpp
// QueueByStack<T> 的实现 + int / std::string 的显式实例化。

#include "queue_by_stack.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace dsaac {

/* START: queue_by_stack implementation */

template <typename T>
void QueueByStack<T>::checkNotEmpty(const char* what) const {
    if (empty()) {
        throw std::out_of_range(std::string("QueueByStack::") + what +
                                ": queue is empty");
    }
}

template <typename T>
void QueueByStack<T>::pour() const {
    // 倒入只在 out_ 为空时发生；in_ 的栈顶（最新元素）先弹出，
    // 压进 out_ 后就落到底部，于是最早入队的元素最终位于 out_ 的顶部。
    while (!in_.empty()) {
        out_.push(in_.top());
        in_.pop();
    }
}

template <typename T>
void QueueByStack<T>::pop() {
    checkNotEmpty("pop");
    if (out_.empty()) {
        pour();  // 均摊 O(1)：每个元素一生只被搬这一次
    }
    out_.pop();
}

template <typename T>
const T& QueueByStack<T>::front() const {
    checkNotEmpty("front");
    if (out_.empty()) {
        pour();
    }
    return out_.top();
}

/* END */

// -------------------------------------------------------------------
// 扩展功能
// -------------------------------------------------------------------

template <typename T>
std::vector<T> QueueByStack<T>::toVector() const {
    // 队首 -> 队尾 = out_ 从顶到底，再接 in_ 从底到顶。
    std::vector<T> out;
    out.reserve(size());

    std::stack<T> outCopy = out_;
    while (!outCopy.empty()) {
        out.push_back(outCopy.top());
        outCopy.pop();
    }

    std::vector<T> inCopy;
    inCopy.reserve(in_.size());
    std::stack<T> inTmp = in_;
    while (!inTmp.empty()) {
        inCopy.push_back(inTmp.top());
        inTmp.pop();
    }
    out.insert(out.end(), inCopy.rbegin(), inCopy.rend());
    return out;
}

template <typename T>
void QueueByStack<T>::printFrontToBack(std::ostream& os) const {
    const std::vector<T> items = toVector();
    if (items.empty()) {
        os << "EMPTY";
        return;
    }
    for (std::size_t k = 0; k < items.size(); ++k) {
        if (k != 0) {
            os << ' ';
        }
        os << items[k];
    }
}

// 显式实例化：如果模板实现写错，这里会直接编译失败。
template class QueueByStack<int>;
template class QueueByStack<std::string>;

} // namespace dsaac