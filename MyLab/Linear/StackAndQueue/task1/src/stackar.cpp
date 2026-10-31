// stackar.cpp
// ArrayStack<T> 的实现 + int / std::string 的显式实例化（编译期自检）。
//
// 存储选择：values_ 用 std::vector<T>，构造时 reserve(capacity)，
// 之后 push/pop 都只是尾插/尾删，不会触发扩容，因此 capacity_ 一经设定
// 就固定不变——这正是“固定容量数组栈”的语义。

#include "stackar.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace dsaac {

/* START: array_stack implementation */

template <typename T>
ArrayStack<T>::ArrayStack(std::size_t capacity) : capacity_(capacity) {
    // 提前拿到全部存储，避免运行期 reallocation；容量为 0 时什么也不做。
    values_.reserve(capacity_);
}

template <typename T>
bool ArrayStack<T>::empty() const {
    return values_.empty();
}

template <typename T>
bool ArrayStack<T>::full() const {
    return values_.size() >= capacity_;
}

template <typename T>
void ArrayStack<T>::push(const T& value) {
    if (full()) {
        throw std::overflow_error("ArrayStack::push: stack is full");
    }
    values_.push_back(value);
}

template <typename T>
void ArrayStack<T>::pop() {
    checkNotEmpty("pop");
    values_.pop_back();
}

template <typename T>
const T& ArrayStack<T>::top() const {
    checkNotEmpty("top");
    return values_.back();
}

template <typename T>
void ArrayStack<T>::checkNotEmpty(const char* what) const {
    if (values_.empty()) {
        throw std::out_of_range(std::string("ArrayStack::") + what +
                                ": stack is empty");
    }
}

/* END */

// -------------------------------------------------------------------
// 扩展功能
// -------------------------------------------------------------------

template <typename T>
void ArrayStack<T>::clear() {
    // 只清元素，不归还存储：容量是构造时定死的，clear 不改变它。
    values_.clear();
}

template <typename T>
void ArrayStack<T>::setTop(const T& value) {
    checkNotEmpty("setTop");
    values_.back() = value;
}

template <typename T>
void ArrayStack<T>::swap(ArrayStack& other) {
    // 容量随元素一起交换，交换后两座栈都仍是“size <= capacity”的合法状态。
    values_.swap(other.values_);
    std::swap(capacity_, other.capacity_);
}

template <typename T>
std::vector<T> ArrayStack<T>::toVector() const {
    return values_;  // 顺序即栈底 -> 栈顶
}

template <typename T>
void ArrayStack<T>::assign(const ArrayStack& other) {
    if (this == &other) {
        return;
    }
    if (other.size() > capacity_) {
        throw std::overflow_error("ArrayStack::assign: not enough capacity");
    }
    values_ = other.values_;
}

template <typename T>
bool ArrayStack<T>::operator==(const ArrayStack& other) const {
    return values_ == other.values_;
}

template <typename T>
void ArrayStack<T>::printTopToBottom(std::ostream& os) const {
    if (values_.empty()) {
        os << "EMPTY";
        return;
    }
    bool first = true;
    for (auto it = values_.rbegin(); it != values_.rend(); ++it) {
        if (!first) {
            os << ' ';
        }
        os << *it;
        first = false;
    }
}

// 显式实例化：如果模板实现写错，这里会直接编译失败。
template class ArrayStack<int>;
template class ArrayStack<std::string>;

} // namespace dsaac