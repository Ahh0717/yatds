// linked_queue.h
// 链式队列 template<typename T> class LinkedQueue。
//
// 为什么链表队列没有“front/rear 走到末尾”的问题
// ------------------------------------------------------------
// 数组队列的下标会随着出队单调右移，最终撞到缓冲区末尾；链表没有下标，
// 队首是一个指针 head_、队尾是另一个指针 tail_，删除头结点只是让
// head_ = head_->next，不需要任何位置变量向前推进，也就不会越界。
// 代价是每个元素多一个 next 指针，以及每次入队/出队一次分配/释放。
//
// 结构：head_ 指向队首，tail_ 指向队尾，尾插头删，无容量上限。

#ifndef DSAAC_LINKED_QUEUE_H
#define DSAAC_LINKED_QUEUE_H

#include <cstddef>
#include <ostream>
#include <utility>
#include <vector>

namespace dsaac {

template <typename T>
class LinkedQueue {
public:
    LinkedQueue() = default;

    // 深拷贝：逐个 new 结点，两条队列不共享内存。
    LinkedQueue(const LinkedQueue& other);
    LinkedQueue& operator=(const LinkedQueue& other);

    // 移动：直接接管对方的头尾指针，不复制元素。
    LinkedQueue(LinkedQueue&& other) noexcept;
    LinkedQueue& operator=(LinkedQueue&& other) noexcept;

    ~LinkedQueue() { clear(); }

    // ---------------------------------------------------------------
    // 题目要求的基础接口
    // ---------------------------------------------------------------

    bool empty() const { return size_ == 0; }
    void push(const T& value);  // 尾插，O(1)
    void pop();                 // 空队列抛 std::out_of_range
    const T& front() const;     // 空队列抛 std::out_of_range

    // ---------------------------------------------------------------
    // 扩展功能（参考 std::queue）
    // ---------------------------------------------------------------

    std::size_t size() const { return size_; }

    // 链式队列没有“满”的概念，因此不提供 full()。

    const T& back() const;  // 队尾；空队列抛 std::out_of_range

    void clear();

    void swap(LinkedQueue& other) noexcept;

    std::vector<T> toVector() const;

    bool operator==(const LinkedQueue& other) const;
    bool operator!=(const LinkedQueue& other) const { return !(*this == other); }

    // 按队首到队尾输出，空格分隔；空队列输出 EMPTY（不带换行）。
    void printFrontToBack(std::ostream& os) const;

    // 就地构造后入队。
    template <typename... Args>
    void emplace(Args&&... args) {
        T value(std::forward<Args>(args)...);
        push(value);
    }

private:
    struct Node {
        T val;
        Node* next;
        explicit Node(const T& value, Node* nextNode = nullptr)
            : val(value), next(nextNode) {}
    };

    void checkNotEmpty(const char* what) const;

    Node* head_ = nullptr;  // 队首
    Node* tail_ = nullptr;  // 队尾
    std::size_t size_ = 0;
};

} // namespace dsaac

template <typename T>
using LinkedQueue = dsaac::LinkedQueue<T>;

#endif