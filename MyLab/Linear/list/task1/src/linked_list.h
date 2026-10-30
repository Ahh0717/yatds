// linked_list.h
// 单链表模板类：手写节点 + 手动 new/delete，不依赖任何 STL 容器。
// 满足 src/Linear/Baseline/List/README.md 中 Baseline 任务 1 的接口与语义要求。

#ifndef MYLAB_LINKED_LIST_H
#define MYLAB_LINKED_LIST_H

#include <cstddef>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

namespace mylab {

template <typename T>
class LinkedList {
public:
    // 节点：对外只读可见，便于测试程序检查结构（如遍历、判环）。
    struct Node {
        T val;
        Node *next;
        explicit Node(const T &value, Node *nextNode = nullptr)
            : val(value), next(nextNode) {}
    };

    LinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}

    // 深拷贝：逐个 new 节点，保证两条链表互不共享内存。
    LinkedList(const LinkedList &other) : LinkedList() {
        for (Node *p = other.head_; p != nullptr; p = p->next) {
            pushBack(p->val);
        }
    }

    LinkedList &operator=(const LinkedList &other);

    // 析构：逐个释放节点，避免内存泄漏。
    ~LinkedList() { clear(); }

    // ---------------------------------------------------------------
    // 基础操作
    // ---------------------------------------------------------------

    void pushFront(const T &x);
    void pushBack(const T &x);

    // 在下标 i（0 <= i <= size）处插入；i 越界返回 false 且不修改链表。
    bool insertAt(std::size_t i, const T &x);

    // 删除下标 i 处元素；i 越界返回 false 且不修改链表。
    bool eraseAt(std::size_t i);

    // 访问下标 i 处元素；i 越界返回 false 且不写入 out。
    bool access(std::size_t i, T &out) const;

    // 返回值为 x 的第一个元素下标；不存在返回 -1。
    long long find(const T &x) const;

    bool contains(const T &x) const { return find(x) != -1; }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    void clear();

    // 打印：元素以单个空格分隔；空链表输出 EMPTY（不带换行）。
    void print(std::ostream &os) const;

    // ---------------------------------------------------------------
    // 以下为超出题目要求的扩展功能
    // ---------------------------------------------------------------

    // 下标 i 处元素的引用；越界返回 false 且不写入 out。
    bool get(std::size_t i, T &out) const { return access(i, out); }

    // 修改下标 i 处元素；越界返回 false。
    bool set(std::size_t i, const T &x);

    bool popFront(T &out);  // 弹出头部；空链表返回 false
    bool popBack(T &out);   // 弹出尾部；空链表返回 false

    // 删除所有值等于 x 的节点，返回删除个数（保留相对顺序）。
    std::size_t removeAll(const T &x);

    // 删除重复值：每个值只保留第一次出现的位置，返回删除个数。
    std::size_t unique();

    // 在尾部追加另一条链表的全部元素（不修改 other）。
    void append(const LinkedList &other);

    // 反转链表（迭代实现，O(n) 时间、O(1) 额外空间）。
    void reverse();

    // 判断是否有环（快慢指针，O(n) 时间、O(1) 空间）。
    bool hasCycle() const;

    // 环入口节点下标；无环返回 -1。
    long long cycleEntry() const;

    // 返回 head 指针，供测试程序做结构级检查（只读用途）。
    const Node *head() const { return head_; }

private:
    Node *head_;
    Node *tail_;
    std::size_t size_;

    // 返回下标 i 处节点的前驱；i == 0 时返回 nullptr。要求调用方保证 i <= size。
    Node *nodeBefore(std::size_t i) const;
};

// ===================================================================
// 实现（模板类，定义必须放在头文件中）
// ===================================================================

template <typename T>
LinkedList<T> &LinkedList<T>::operator=(const LinkedList &other) {
    if (this == &other) {
        return *this;
    }
    // 先拷贝再释放，保证自赋值之外也能获得强异常安全。
    LinkedList tmp(other);
    std::swap(head_, tmp.head_);
    std::swap(tail_, tmp.tail_);
    std::swap(size_, tmp.size_);
    return *this;
}

template <typename T>
void LinkedList<T>::pushFront(const T &x) {
    Node *node = new Node(x, head_);
    head_ = node;
    if (tail_ == nullptr) {
        tail_ = node;
    }
    ++size_;
}

template <typename T>
void LinkedList<T>::pushBack(const T &x) {
    Node *node = new Node(x, nullptr);
    if (tail_ == nullptr) {
        head_ = node;
    } else {
        tail_->next = node;
    }
    tail_ = node;
    ++size_;
}

template <typename T>
typename LinkedList<T>::Node *LinkedList<T>::nodeBefore(std::size_t i) const {
    Node *prev = nullptr;
    Node *cur = head_;
    for (std::size_t k = 0; k < i; ++k) {
        prev = cur;
        cur = cur->next;
    }
    return prev;
}

template <typename T>
bool LinkedList<T>::insertAt(std::size_t i, const T &x) {
    if (i > size_) {
        return false;
    }
    if (i == 0) {
        pushFront(x);
        return true;
    }
    if (i == size_) {
        pushBack(x);
        return true;
    }
    // 中间插入：找到 i-1 号节点，在其后挂上新节点。
    Node *prev = nodeBefore(i);
    prev->next = new Node(x, prev->next);
    ++size_;
    return true;
}

template <typename T>
bool LinkedList<T>::eraseAt(std::size_t i) {
    if (i >= size_) {
        return false;
    }
    Node *target = nullptr;
    if (i == 0) {
        target = head_;
        head_ = head_->next;
        if (head_ == nullptr) {
            tail_ = nullptr;
        }
    } else {
        Node *prev = nodeBefore(i);
        target = prev->next;
        prev->next = target->next;
        if (target == tail_) {
            tail_ = prev;
        }
    }
    delete target;
    --size_;
    return true;
}

template <typename T>
bool LinkedList<T>::access(std::size_t i, T &out) const {
    if (i >= size_) {
        return false;
    }
    const Node *cur = head_;
    for (std::size_t k = 0; k < i; ++k) {
        cur = cur->next;
    }
    out = cur->val;
    return true;
}

template <typename T>
long long LinkedList<T>::find(const T &x) const {
    long long index = 0;
    for (const Node *cur = head_; cur != nullptr; cur = cur->next, ++index) {
        if (cur->val == x) {
            return index;
        }
    }
    return -1;
}

template <typename T>
void LinkedList<T>::clear() {
    Node *cur = head_;
    while (cur != nullptr) {
        Node *next = cur->next;
        delete cur;
        cur = next;
    }
    head_ = tail_ = nullptr;
    size_ = 0;
}

template <typename T>
void LinkedList<T>::print(std::ostream &os) const {
    if (head_ == nullptr) {
        os << "EMPTY";
        return;
    }
    bool first = true;
    for (const Node *cur = head_; cur != nullptr; cur = cur->next) {
        if (!first) {
            os << ' ';
        }
        os << cur->val;
        first = false;
    }
}

// -------------------------------------------------------------------
// 扩展功能实现
// -------------------------------------------------------------------

template <typename T>
bool LinkedList<T>::set(std::size_t i, const T &x) {
    if (i >= size_) {
        return false;
    }
    Node *cur = head_;
    for (std::size_t k = 0; k < i; ++k) {
        cur = cur->next;
    }
    cur->val = x;
    return true;
}

template <typename T>
bool LinkedList<T>::popFront(T &out) {
    if (empty()) {
        return false;
    }
    out = head_->val;
    return eraseAt(0);
}

template <typename T>
bool LinkedList<T>::popBack(T &out) {
    if (empty()) {
        return false;
    }
    out = tail_->val;
    return eraseAt(size_ - 1);
}

template <typename T>
std::size_t LinkedList<T>::removeAll(const T &x) {
    std::size_t removed = 0;
    // 先处理头部连续命中的情况，避免陷入死循环。
    while (head_ != nullptr && head_->val == x) {
        Node *victim = head_;
        head_ = head_->next;
        delete victim;
        --size_;
        ++removed;
    }
    if (head_ == nullptr) {
        tail_ = nullptr;
        return removed;
    }
    Node *cur = head_;
    while (cur->next != nullptr) {
        if (cur->next->val == x) {
            Node *victim = cur->next;
            cur->next = victim->next;
            if (victim == tail_) {
                tail_ = cur;
            }
            delete victim;
            --size_;
            ++removed;
        } else {
            cur = cur->next;
        }
    }
    return removed;
}

template <typename T>
std::size_t LinkedList<T>::unique() {
    // O(n^2) 去重：对每个节点向后扫描，删除值相同的后继节点。
    // 不借助哈希表，符合“只用链表”的约束。
    std::size_t removed = 0;
    for (Node *cur = head_; cur != nullptr; cur = cur->next) {
        Node *scan = cur;
        while (scan->next != nullptr) {
            if (scan->next->val == cur->val) {
                Node *victim = scan->next;
                scan->next = victim->next;
                if (victim == tail_) {
                    tail_ = scan;
                }
                delete victim;
                --size_;
                ++removed;
            } else {
                scan = scan->next;
            }
        }
    }
    return removed;
}

template <typename T>
void LinkedList<T>::append(const LinkedList &other) {
    for (const Node *cur = other.head_; cur != nullptr; cur = cur->next) {
        pushBack(cur->val);
    }
}

template <typename T>
void LinkedList<T>::reverse() {
    Node *prev = nullptr;
    Node *cur = head_;
    tail_ = head_;  // 反转后原头节点变尾节点（空链表时为 nullptr）
    while (cur != nullptr) {
        Node *next = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next;
    }
    head_ = prev;
}

template <typename T>
bool LinkedList<T>::hasCycle() const {
    const Node *slow = head_;
    const Node *fast = head_;
    while (fast != nullptr && fast->next != nullptr) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            return true;
        }
    }
    return false;
}

template <typename T>
long long LinkedList<T>::cycleEntry() const {
    const Node *slow = head_;
    const Node *fast = head_;
    while (fast != nullptr && fast->next != nullptr) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            // 一个指针回到起点，二者同速前进，再次相遇处即入口。
            const Node *entry = head_;
            const Node *meet = slow;
            long long index = 0;
            while (entry != meet) {
                entry = entry->next;
                meet = meet->next;
                ++index;
            }
            return index;
        }
    }
    return -1;
}

}  // namespace mylab

#endif  // MYLAB_LINKED_LIST_H