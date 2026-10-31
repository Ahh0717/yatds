// linked_queue.cpp
// LinkedQueue<T> 的实现 + int / std::string 的显式实例化。
//
// tail_ 的维护是这类实现最容易出错的地方：头删、清空之后必须保证
// 队列真的空了再 push 时不会写到已释放的内存，因此 pop 里删掉最后一个
// 结点时把 tail_ 一并置空。

#include "linked_queue.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace dsaac {

/* START: linked_queue implementation */

template <typename T>
LinkedQueue<T>::LinkedQueue(const LinkedQueue& other) {
    for (const T& value : other.toVector()) {
        push(value);
    }
}

template <typename T>
LinkedQueue<T>& LinkedQueue<T>::operator=(const LinkedQueue& other) {
    if (this != &other) {
        LinkedQueue copy(other);  // 先复制成功，再交换，异常安全
        swap(copy);
    }
    return *this;
}

template <typename T>
LinkedQueue<T>::LinkedQueue(LinkedQueue&& other) noexcept
    : head_(other.head_), tail_(other.tail_), size_(other.size_) {
    other.head_ = nullptr;
    other.tail_ = nullptr;
    other.size_ = 0;
}

template <typename T>
LinkedQueue<T>& LinkedQueue<T>::operator=(LinkedQueue&& other) noexcept {
    if (this != &other) {
        clear();
        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}

template <typename T>
void LinkedQueue<T>::push(const T& value) {
    Node* node = new Node(value);
    if (tail_ == nullptr) {  // 空队列：头尾都指向新结点
        head_ = node;
        tail_ = node;
    } else {
        tail_->next = node;
        tail_ = node;
    }
    ++size_;
}

template <typename T>
void LinkedQueue<T>::pop() {
    checkNotEmpty("pop");
    Node* old = head_;
    head_ = head_->next;
    delete old;
    --size_;
    if (head_ == nullptr) {
        // 删空了：tail_ 必须一起置空，否则下次 push 会写到已释放的结点。
        tail_ = nullptr;
    }
}

template <typename T>
const T& LinkedQueue<T>::front() const {
    checkNotEmpty("front");
    return head_->val;
}

template <typename T>
void LinkedQueue<T>::checkNotEmpty(const char* what) const {
    if (empty()) {
        throw std::out_of_range(std::string("LinkedQueue::") + what +
                                ": queue is empty");
    }
}

/* END */

// -------------------------------------------------------------------
// 扩展功能
// -------------------------------------------------------------------

template <typename T>
const T& LinkedQueue<T>::back() const {
    checkNotEmpty("back");
    return tail_->val;
}

template <typename T>
void LinkedQueue<T>::clear() {
    while (head_ != nullptr) {
        Node* next = head_->next;
        delete head_;
        head_ = next;
    }
    tail_ = nullptr;
    size_ = 0;
}

template <typename T>
void LinkedQueue<T>::swap(LinkedQueue& other) noexcept {
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
    std::swap(size_, other.size_);
}

template <typename T>
std::vector<T> LinkedQueue<T>::toVector() const {
    std::vector<T> out;
    out.reserve(size_);
    for (const Node* cur = head_; cur != nullptr; cur = cur->next) {
        out.push_back(cur->val);
    }
    return out;
}

template <typename T>
bool LinkedQueue<T>::operator==(const LinkedQueue& other) const {
    if (size_ != other.size_) {
        return false;
    }
    const Node* a = head_;
    const Node* b = other.head_;
    while (a != nullptr && b != nullptr) {
        if (!(a->val == b->val)) {
            return false;
        }
        a = a->next;
        b = b->next;
    }
    return a == nullptr && b == nullptr;
}

template <typename T>
void LinkedQueue<T>::printFrontToBack(std::ostream& os) const {
    if (empty()) {
        os << "EMPTY";
        return;
    }
    bool first = true;
    for (const Node* cur = head_; cur != nullptr; cur = cur->next) {
        if (!first) {
            os << ' ';
        }
        os << cur->val;
        first = false;
    }
}

// 显式实例化：如果模板实现写错，这里会直接编译失败。
template class LinkedQueue<int>;
template class LinkedQueue<std::string>;

} // namespace dsaac