// queue.cpp
// CircularQueue<T> 的实现 + int / std::string 的显式实例化（编译期自检）。

#include "queue.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace dsaac {

/* START: queue implementation */

template <typename T>
CircularQueue<T>::CircularQueue(std::size_t capacity)
    : buffer_(capacity + 1) {  // 多开一个空槽，用来区分空和满
    front_ = 0;
    back_ = 0;
    count_ = 0;
}

template <typename T>
bool CircularQueue<T>::empty() const {
    return count_ == 0;
}

template <typename T>
bool CircularQueue<T>::full() const {
    return count_ == capacity();
}

template <typename T>
void CircularQueue<T>::push(const T& value) {
    if (full()) {
        throw std::overflow_error("CircularQueue::push: queue is full");
    }
    buffer_[back_] = value;
    back_ = next(back_);  // 到了缓冲区末尾就回绕到 0
    ++count_;
}

template <typename T>
void CircularQueue<T>::pop() {
    checkNotEmpty("pop");
    front_ = next(front_);  // 只移动队首下标，不搬动元素
    --count_;
}

template <typename T>
const T& CircularQueue<T>::front() const {
    checkNotEmpty("front");
    return buffer_[front_];
}

template <typename T>
void CircularQueue<T>::checkNotEmpty(const char* what) const {
    if (empty()) {
        throw std::out_of_range(std::string("CircularQueue::") + what +
                                ": queue is empty");
    }
}

/* END */

// -------------------------------------------------------------------
// 扩展功能
// -------------------------------------------------------------------

template <typename T>
const T& CircularQueue<T>::back() const {
    checkNotEmpty("back");
    // back_ 指向“下一个入队位置”，它前一格才是队尾；下标 0 的前一格要回绕。
    return buffer_[back_ == 0 ? buffer_.size() - 1 : back_ - 1];
}

template <typename T>
void CircularQueue<T>::clear() {
    // 只把逻辑长度清零，缓冲区与容量保持不变，便于复用。
    front_ = 0;
    back_ = 0;
    count_ = 0;
}

template <typename T>
void CircularQueue<T>::swap(CircularQueue& other) noexcept {
    buffer_.swap(other.buffer_);
    std::swap(front_, other.front_);
    std::swap(back_, other.back_);
    std::swap(count_, other.count_);
}

template <typename T>
std::vector<T> CircularQueue<T>::toVector() const {
    // 从队首沿环走 count_ 步；缓冲区里的物理顺序不等于逻辑顺序。
    std::vector<T> out;
    out.reserve(count_);
    std::size_t index = front_;
    for (std::size_t k = 0; k < count_; ++k) {
        out.push_back(buffer_[index]);
        index = next(index);
    }
    return out;
}

template <typename T>
bool CircularQueue<T>::operator==(const CircularQueue& other) const {
    return toVector() == other.toVector();
}

template <typename T>
void CircularQueue<T>::printFrontToBack(std::ostream& os) const {
    if (empty()) {
        os << "EMPTY";
        return;
    }
    bool first = true;
    std::size_t index = front_;
    for (std::size_t k = 0; k < count_; ++k) {
        if (!first) {
            os << ' ';
        }
        os << buffer_[index];
        first = false;
        index = next(index);
    }
}

// 显式实例化：如果模板实现写错，这里会直接编译失败。
template class CircularQueue<int>;
template class CircularQueue<std::string>;

} // namespace dsaac