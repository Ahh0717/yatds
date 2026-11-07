// ball_queue.cpp
// BallQueue 的实现：cap_ + 1 槽的环形缓冲区，front_ / back_ 回绕。
//
// 空/满判定（不用额外标记位）：
//   front_ == back_            → 空
//   nextIndex(back_) == front_ → 满
// 因此最多能存 cap_ 个元素，缓冲区多出来的那一格永远空着。

#include "ball_queue.h"

#include <stdexcept>
#include <utility>

namespace zuma {

BallQueue::BallQueue(std::size_t capacity)
    : buf_(new char[capacity + 1]), front_(0), back_(0), size_(0), cap_(capacity) {}

BallQueue::~BallQueue() {
    delete[] buf_;
}

BallQueue::BallQueue(const BallQueue& other)
    : buf_(new char[other.cap_ + 1]),
      front_(0),
      back_(0),
      size_(0),
      cap_(other.cap_) {
    // 按队首 → 队尾顺序重新入队，复制结果的 front_ 归零，布局自然紧凑。
    for (char c : other.toVector()) {
        push(c);
    }
}

BallQueue& BallQueue::operator=(const BallQueue& other) {
    if (this != &other) {
        BallQueue copy(other);  // copy-and-swap：构造失败时原对象不受影响
        swapWith(copy);
    }
    return *this;
}

void BallQueue::swapWith(BallQueue& other) noexcept {
    std::swap(buf_, other.buf_);
    std::swap(front_, other.front_);
    std::swap(back_, other.back_);
    std::swap(size_, other.size_);
    std::swap(cap_, other.cap_);
}

bool BallQueue::push(char c) {
    const std::size_t next = nextIndex(back_);
    if (next == front_) {
        return false;  // 满：多留的那一格也被占了
    }
    buf_[back_] = c;
    back_ = next;
    ++size_;
    return true;
}

char BallQueue::front() const {
    if (empty()) {
        throw std::out_of_range("BallQueue::front: queue is empty");
    }
    return buf_[front_];
}

char BallQueue::pop() {
    if (empty()) {
        throw std::out_of_range("BallQueue::pop: queue is empty");
    }
    const char c = buf_[front_];
    front_ = nextIndex(front_);
    --size_;
    return c;
}

void BallQueue::setFront(char c) {
    if (empty()) {
        throw std::out_of_range("BallQueue::setFront: queue is empty");
    }
    buf_[front_] = c;
}

std::vector<char> BallQueue::toVector() const {
    std::vector<char> result;
    result.reserve(size_);
    std::size_t i = front_;
    for (std::size_t k = 0; k < size_; ++k) {
        result.push_back(buf_[i]);
        i = nextIndex(i);
    }
    return result;
}

void BallQueue::clear() {
    // 只重置下标；缓冲区与容量保留，清空后可继续复用。
    front_ = 0;
    back_ = 0;
    size_ = 0;
}

}  // namespace zuma