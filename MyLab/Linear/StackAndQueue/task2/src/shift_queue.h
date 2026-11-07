// shift_queue.h
// 平移式数组队列 template<typename T> class ShiftQueue。
//
// 解决“front/rear 走到缓冲区末尾”的办法（方法二：出队后整体左移）
// ------------------------------------------------------------
// 不维护队首下标：front 恒为下标 0，元素永远紧凑地排在数组前部。
// 入队写在 size_ 处，出队后把 [1, size_) 的整体左移一格。
// 这样根本不存在“队首/队尾下标越界”的问题——位置永远从 0 重新算起。
//
// 与循环队列的复杂度差异
// ------------------------------------------------------------
//   push : 二者都是 O(1)（这里只是尾部赋值）
//   pop  : 循环队列 O(1)（只挪下标）；本实现 O(n)（搬 n-1 个元素）
//   front: 二者都是 O(1)（这里永远读 index 0，命中率更好）
// 所以 ShiftQueue 只适合“出队很少、或队列很短”的场景。它的价值在于
// 用最少的逻辑说明：把下标复位也能绕开越界问题，代价是搬移。

#ifndef DSAAC_SHIFT_QUEUE_H
#define DSAAC_SHIFT_QUEUE_H

#include <cstddef>
#include <ostream>
#include <utility>
#include <vector>

namespace dsaac {

template <typename T>
class ShiftQueue {
public:
    // 对外容量 = capacity 个元素；内部正好开 capacity 个槽（无空槽）。
    explicit ShiftQueue(std::size_t capacity);

    // ---------------------------------------------------------------
    // 基础接口（与 CircularQueue 对齐）
    // ---------------------------------------------------------------

    bool empty() const;
    bool full() const;
    void push(const T& value);  // 满队列抛 std::overflow_error
    void pop();                 // 空队列抛 std::out_of_range
    const T& front() const;     // 空队列抛 std::out_of_range

    // ---------------------------------------------------------------
    // 扩展功能（参考 std::queue）
    // ---------------------------------------------------------------

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }

    const T& back() const;  // 队尾；空队列抛 std::out_of_range

    void clear() { size_ = 0; }

    void swap(ShiftQueue& other) noexcept;

    std::vector<T> toVector() const;

    bool operator==(const ShiftQueue& other) const;
    bool operator!=(const ShiftQueue& other) const { return !(*this == other); }

    // 按队首到队尾输出，空格分隔；空队列输出 EMPTY（不带换行）。
    void printFrontToBack(std::ostream& os) const;

    template <typename... Args>
    void emplace(Args&&... args) {
        T value(std::forward<Args>(args)...);
        push(value);
    }

private:
    void checkNotEmpty(const char* what) const;

    std::vector<T> buffer_;  // capacity_ 个槽，有效元素在 [0, size_)
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};

} // namespace dsaac

template <typename T>
using ShiftQueue = dsaac::ShiftQueue<T>;

#endif