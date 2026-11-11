// shift_queue.cpp
// ShiftQueue<T> 的实现 + int / std::string 的显式实例化。

#include "shift_queue.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace dsaac {

/* START: shift_queue implementation */

template <typename T>
ShiftQueue<T>::ShiftQueue(std::size_t capacity)
    : buffer_(capacity), size_(0), capacity_(capacity) {}

template <typename T>
bool ShiftQueue<T>::empty() const {
    return size_ == 0;
}

template <typename T>
bool ShiftQueue<T>::full() const {
    return size_ == capacity_;
}

template <typename T>
void ShiftQueue<T>::push(const T& value) {
    if (full()) {
        throw std::overflow_error("ShiftQueue::push: queue is full");
    }
    buffer_[size_] = value;  // 队尾永远是下标 size_，不会越界
    ++size_;
}

template <typename T>
void ShiftQueue<T>::pop() {
    checkNotEmpty("pop");
    // 关键的一步：队首移除后，剩余元素整体左移一位，front 重新回到 0。
    // 这就是 O(n) 的来源；循环队列在这里只做一次下标自增。
    std::move(buffer_.begin() + 1, buffer_.begin() + static_cast<std::ptrdiff_t>(size_),
              buffer_.begin());
    --size_;
}

template <typename T>
const T& ShiftQueue<T>::front() const {
    checkNotEmpty("front");
    return buffer_[0];  // front 恒为 0
}

template <typename T>
void ShiftQueue<T>::checkNotEmpty(const char* what) const {
    if (empty()) {
        throw std::out_of_range(std::string("ShiftQueue::") + what +
                                ": queue is empty");
    }
}

/* END */

// -------------------------------------------------------------------
// 扩展功能
// -------------------------------------------------------------------

template <typename T>
const T& ShiftQueue<T>::back() const {
    checkNotEmpty("back");
    return buffer_[size_ - 1];
}

template <typename T>
void ShiftQueue<T>::swap(ShiftQueue& other) noexcept {
    buffer_.swap(other.buffer_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
}

template <typename T>
std::vector<T> ShiftQueue<T>::toVector() const {
    // 元素本来就紧凑排列在 [0, size_)，直接拷贝即可。
    return std::vector<T>(buffer_.begin(),
                          buffer_.begin() + static_cast<std::ptrdiff_t>(size_));
}

template <typename T>
bool ShiftQueue<T>::operator==(const ShiftQueue& other) const {
    return size_ == other.size_ &&
           std::equal(buffer_.begin(),
                      buffer_.begin() + static_cast<std::ptrdiff_t>(size_),
                      other.buffer_.begin());
}

template <typename T>
void ShiftQueue<T>::printFrontToBack(std::ostream& os) const {
    if (empty()) {
        os << "EMPTY";
        return;
    }
    for (std::size_t k = 0; k < size_; ++k) {
        if (k != 0) {
            os << ' ';
        }
        os << buffer_[k];
    }
}

// 显式实例化：如果模板实现写错，这里会直接编译失败。
template class ShiftQueue<int>;
template class ShiftQueue<std::string>;

} // namespace dsaac