// vector_queue.cpp
// VectorQueue<T> 的实现 + int / std::string 的显式实例化。
// 转换函数 toStack() 在本文件实现；反向的 VectorStack::toQueue() 在
// vector_stack.cpp 实现（那里需要 VectorQueue<T> 是完整类型）。

#include "vector_queue.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace dsaac {

/* START: vector_queue implementation */

template <typename T>
void VectorQueue<T>::checkNotEmpty(const char* what) const {
    if (empty()) {
        throw std::out_of_range(std::string("VectorQueue::") + what +
                                ": queue is empty");
    }
}

template <typename T>
void VectorQueue<T>::compactIfNeeded() {
    // 有效元素只剩不到一半时，把 [front_, end) 搬回下标 0。
    // 每次压缩至少腾出一半空间，摊还到之前的 pop 上是 O(1)。
    if (front_ > 0 && front_ >= data_.size() - front_) {
        data_.erase(data_.begin(), data_.begin() + static_cast<std::ptrdiff_t>(front_));
        front_ = 0;
    }
}

template <typename T>
void VectorQueue<T>::push(const T& value) {
    compactIfNeeded();  // 先腾出空间，避免 front_ 长期占着前缀
    data_.push_back(value);
}

template <typename T>
void VectorQueue<T>::pop() {
    checkNotEmpty("pop");
    ++front_;  // 只前移队首下标，不搬元素
    if (empty()) {
        clear();  // 弹空后把占位元素一并清掉，避免 vector 无限增长
    }
}

template <typename T>
const T& VectorQueue<T>::front() const {
    checkNotEmpty("front");
    return data_[front_];
}

/* END */

// -------------------------------------------------------------------
// 扩展功能
// -------------------------------------------------------------------

template <typename T>
const T& VectorQueue<T>::back() const {
    checkNotEmpty("back");
    return data_.back();
}

template <typename T>
std::vector<T> VectorQueue<T>::toVector() const {
    return std::vector<T>(data_.begin() + static_cast<std::ptrdiff_t>(front_),
                          data_.end());
}

template <typename T>
void VectorQueue<T>::printFrontToBack(std::ostream& os) const {
    if (empty()) {
        os << "EMPTY";
        return;
    }
    for (std::size_t index = front_; index < data_.size(); ++index) {
        if (index != front_) {
            os << ' ';
        }
        os << data_[index];
    }
}

template <typename T>
VectorStack<T> VectorQueue<T>::toStack() const {
    // 语义：从队首逐个出队、依次压栈。队尾最后入栈，成为栈顶，
    // 于是“栈顶 -> 栈底”就是原队列的反转。
    VectorStack<T> stack;
    for (std::size_t index = front_; index < data_.size(); ++index) {
        stack.push(data_[index]);
    }
    return stack;
}

// 显式实例化：如果模板实现写错，这里会直接编译失败。
// VectorStack 的显式实例化放在 vector_stack.cpp（同一个特化只能定义一次）。
template class VectorQueue<int>;
template class VectorQueue<std::string>;

} // namespace dsaac