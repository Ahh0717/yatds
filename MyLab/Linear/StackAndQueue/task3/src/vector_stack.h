// vector_stack.h
// 基于 std::vector 的栈 template<typename T> class VectorStack。
// 题目要求它和 VectorQueue 能相互转换：VectorStack::toQueue() 返回一个
// VectorQueue，VectorQueue::toStack() 返回一个 VectorStack。
//
// 栈顶固定在 vector 尾部，push/pop/top 都是 O(1)。
//   toQueue() —— 从栈顶逐个弹出、依次入队（等价于按栈顶 -> 栈底的顺序
//                入队）。于是队首是原栈顶，队首到队尾输出就是原栈的逆序，
//                与 VectorQueue::toStack() 互为镜像。复杂度 O(n)。

#ifndef DSAAC_VECTOR_STACK_H
#define DSAAC_VECTOR_STACK_H

#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace dsaac {

template <typename T>
class VectorQueue;  // 前置声明：toQueue() 返回它

template <typename T>
class VectorStack {
public:
    // ---------------------------------------------------------------
    // 栈语义
    // ---------------------------------------------------------------
    bool empty() const { return data_.empty(); }
    std::size_t size() const { return data_.size(); }

    // O(1)。
    void push(const T& value) { data_.push_back(value); }

    // O(1)。
    void pop();

    // O(1)。空栈抛 std::out_of_range。
    const T& top() const;

    // ---------------------------------------------------------------
    // 扩展功能
    // ---------------------------------------------------------------
    void clear() { data_.clear(); }

    void swap(VectorStack& other) noexcept { data_.swap(other.data_); }

    // 按栈底 -> 栈顶导出。
    std::vector<T> toVector() const { return data_; }

    bool operator==(const VectorStack& other) const { return data_ == other.data_; }
    bool operator!=(const VectorStack& other) const { return !(*this == other); }

    // 按栈顶到栈底输出，空格分隔；空栈输出 EMPTY（不带换行）。
    void printTopToBottom(std::ostream& os) const;

    // 转换成队列：按栈顶 -> 栈底依次入队，等价于把序列反转。
    // 定义在 vector_stack.cpp（那里包含 vector_queue.h，VectorQueue<T>
    // 在显式实例化的位置是完整类型）。
    VectorQueue<T> toQueue() const;

    template <typename... Args>
    void emplace(Args&&... args) {
        data_.emplace_back(std::forward<Args>(args)...);
    }

private:
    void checkNotEmpty(const char* what) const;

    std::vector<T> data_;
};

} // namespace dsaac

template <typename T>
using VectorStack = dsaac::VectorStack<T>;

// 注意：VectorStack::toQueue() 的实现在 vector_stack.cpp，不在本头文件里，
// 因为它的返回类型 VectorQueue<T> 必须在定义处是完整类型。
// 调用 toQueue() 的翻译单元需要包含 vector_queue.h
// （它本身已经包含本文件，不会出现重复定义问题）。

#endif