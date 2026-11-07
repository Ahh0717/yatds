// track.cpp
// Track 的实现：双向链表 + 首尾哨兵。
//
// 所有彩球节点由本类 new/delete；两个哨兵在构造时分配、析构时释放，
// 因此 clear() 只需要回收彩球节点并把哨兵重新接起来。

#include "track.h"

#include <stdexcept>
#include <utility>

namespace zuma {

namespace {

// 造一个哨兵节点。哨兵不携带颜色（color 仅为占位），
// 外部拿不到它的指针，所以不会被误当成彩球。
Track::Node* makeSentinel() {
    return new Track::Node{'\0', nullptr, nullptr};
}

}  // namespace

Track::Track() : head_(nullptr), tail_(nullptr), size_(0) {
    try {
        head_ = makeSentinel();
        tail_ = makeSentinel();
    } catch (...) {
        // 第二个哨兵分配失败时，第一个必须回收：指针成员没有析构函数兜底。
        delete head_;
        throw;
    }
    head_->next = tail_;
    tail_->prev = head_;
}

Track::~Track() {
    clear();
    delete head_;
    delete tail_;
}

Track::Track(const Track& other) : Track() {
    // 委托构造已建好空链表；若下面分配失败，已完成的委托构造函数
    // 会让本对象的析构函数兜底，不会泄漏哨兵。
    for (Track::Node* p = other.head_->next; p != other.tail_; p = p->next) {
        insertAt(size_, p->color);
    }
}

Track& Track::operator=(const Track& other) {
    if (this != &other) {
        Track copy(other);  // 先完整复制，成功后再替换（copy-and-swap）
        swapWith(copy);
    }
    return *this;
}

void Track::swapWith(Track& other) noexcept {
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
    std::swap(size_, other.size_);
}

Track::Node* Track::insertAt(std::size_t pos, char color) {
    if (pos > size_) {
        throw std::out_of_range("Track::insertAt: pos out of range");
    }

    // 找到“插入位置之后”的节点：pos == size_ 时为尾哨兵。
    Track::Node* next = (pos == size_) ? tail_ : nodeAt(pos);
    Track::Node* prev = next->prev;

    Track::Node* node = new Track::Node{color, prev, next};
    prev->next = node;
    next->prev = node;
    ++size_;
    return node;
}

void Track::erase(Track::Node* p) {
    if (p == nullptr || p == head_ || p == tail_) {
        throw std::invalid_argument("Track::erase: invalid node");
    }
    p->prev->next = p->next;
    p->next->prev = p->prev;
    delete p;
    --size_;
}

Track::Node* Track::nodeAt(std::size_t pos) {
    // 复用 const 版本的查找逻辑；这里需要可写指针，故去掉 const。
    return const_cast<Track::Node*>(static_cast<const Track&>(*this).nodeAt(pos));
}

const Track::Node* Track::nodeAt(std::size_t pos) const {
    if (pos >= size_) {
        throw std::out_of_range("Track::nodeAt: pos out of range");
    }
    if (pos < size_ / 2) {
        const Track::Node* p = head_->next;
        for (std::size_t i = 0; i < pos; ++i) {
            p = p->next;
        }
        return p;
    }
    const Track::Node* p = tail_->prev;
    for (std::size_t i = size_ - 1; i > pos; --i) {
        p = p->prev;
    }
    return p;
}

std::vector<char> Track::toVector() const {
    std::vector<char> result;
    result.reserve(size_);
    for (Track::Node* p = head_->next; p != tail_; p = p->next) {
        result.push_back(p->color);
    }
    return result;
}

void Track::clear() {
    Track::Node* p = head_->next;
    while (p != tail_) {
        Track::Node* next = p->next;
        delete p;
        p = next;
    }
    head_->next = tail_;
    tail_->prev = head_;
    size_ = 0;
}

}  // namespace zuma