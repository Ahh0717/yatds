// vector_queue.h
// 基于 std::vector 的队列 template<typename T> class VectorQueue。
// 题目要求它和 VectorStack 能相互转换：VectorQueue::toStack() 返回一个
// VectorStack，VectorStack::toQueue() 返回一个 VectorQueue。
//
// 这里用队首下标 front_ 避免出队时搬移：
//   pop()   —— 只把 front_ 前移，O(1)；vector 里的旧元素暂时留着占位。
//   push()  —— 尾部追加；当 front_ 前移过多时做一次压缩（compact），
//              均摊 O(1)。
// 与循环缓冲区的区别：缓冲区不会扩容，这里则依赖 vector 的翻倍增长。
//
// 复杂度：push 均摊 O(1)（压缩 + 扩容都摊还到每次 push 上），pop/front O(1)。

#ifndef DSAAC_VECTOR_QUEUE_H
#define DSAAC_VECTOR_QUEUE_H

#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "vector_stack.h"

namespace dsaac {

template <typename T>
class VectorQueue {
public:
    // ---------------------------------------------------------------
    // 队列语义
    // ---------------------------------------------------------------
    bool empty() const { return size() == 0; }
    std::size_t size() const { return data_.size() - front_; }

    // 均摊 O(1)。
    void push(const T& value);

    // O(1)：只前移 front_。
    void pop();

    // O(1)。空队列抛 std::out_of_range。
    const T& front() const;

    // ---------------------------------------------------------------
    // 扩展功能
    // ---------------------------------------------------------------
    const T& back() const;  // 队尾；空队列抛 std::out_of_range

    void clear() {
        data_.clear();
        front_ = 0;
    }

    void swap(VectorQueue& other) noexcept {
        data_.swap(other.data_);
        std::swap(front_, other.front_);
    }

    // 按队首 -> 队尾导出。
    std::vector<T> toVector() const;

    bool operator==(const VectorQueue& other) const {
        return toVector() == other.toVector();
    }
    bool operator!=(const VectorQueue& other) const { return !(*this == other); }

    // 按队首到队尾输出，空格分隔；空队列输出 EMPTY（不带换行）。
    void printFrontToBack(std::ostream& os) const;

    // 转换成栈：从队首逐个出队、依次压栈（等价于按队首 -> 队尾的顺序
    // 压栈）。则队尾成为栈顶，栈顶到栈底输出正好是原队列的逆序，
    // 与 VectorStack::toQueue() 互为镜像。复杂度 O(n)。
    VectorStack<T> toStack() const;

    template <typename... Args>
    void emplace(Args&&... args) {
        data_.emplace_back(std::forward<Args>(args)...);
    }

private:
    void checkNotEmpty(const char* what) const;
    // front_ 之后没剩多少空间时，把有效区搬回下标 0。
    void compactIfNeeded();

    std::vector<T> data_;
    std::size_t front_ = 0;  // 队首元素的下标
};

} // namespace dsaac

template <typename T>
using VectorQueue = dsaac::VectorQueue<T>;

#endif