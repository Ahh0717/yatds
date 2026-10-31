// queue.hpp
// 循环队列 template<typename T> class CircularQueue。
// 保留题目骨架给定的构造函数与 empty/full/push/pop/front 签名，
// 在此基础上参考 std::queue 补齐 size/clear/swap/toVector 等功能。
//
// 解决“front/rear 走到缓冲区末尾”的办法（方法一：下标回绕）
// ------------------------------------------------------------
// 把缓冲区看成一个首尾相接的环，front_ 指向队首元素，back_ 指向
// “下一个入队位置”。两个下标都只增不减、到达 buffer_.size() 时
// 取模回到 0，因此出队永远不用搬动任何元素，push/pop 都是 O(1)。
//
// 空 / 满的区分（额外一个空槽）
// ------------------------------------------------------------
// 对外语义：构造参数 capacity 就是“最多能放多少个元素”。
// 内部缓冲区开 capacity + 1 个槽，始终留一个是空的，于是：
//   空：front_ == back_
//   满：back_ 再走一格就撞上 front_
// 不用额外维护 count_，也不浪费对外可见的容量。
// 代价是内部多存一个 T 的空间（对 int 来说通常就是 4~8 字节）。

#ifndef DSAAC_QUEUE_HPP
#define DSAAC_QUEUE_HPP

#include <cstddef>
#include <ostream>
#include <utility>
#include <vector>

namespace dsaac {

template <typename T>
class CircularQueue {
public:
    // 对外容量 = capacity 个元素；内部缓冲区多开一个空槽。
    explicit CircularQueue(std::size_t capacity);

    // ---------------------------------------------------------------
    // 题目要求的基础接口
    // ---------------------------------------------------------------

    bool empty() const;
    bool full() const;
    void push(const T& value);  // 满队列抛 std::overflow_error
    void pop();                 // 空队列抛 std::out_of_range
    const T& front() const;     // 空队列抛 std::out_of_range

    // ---------------------------------------------------------------
    // 扩展功能（参考 std::queue）
    // ---------------------------------------------------------------

    std::size_t size() const { return count_; }

    // 对外容量（等于构造参数；内部缓冲区是 capacity + 1 个槽）。
    std::size_t capacity() const { return buffer_.empty() ? 0 : buffer_.size() - 1; }

    // 队尾元素（std::queue 的 back()）；空队列抛 std::out_of_range。
    const T& back() const;

    // 清空全部元素；容量不变。
    void clear();

    // 与另一座队列交换内容（缓冲区与两组下标一起交换）。
    void swap(CircularQueue& other) noexcept;

    // 按队首 -> 队尾的顺序导出。
    std::vector<T> toVector() const;

    // 逐元素比较（队首 -> 队尾顺序）。
    bool operator==(const CircularQueue& other) const;
    bool operator!=(const CircularQueue& other) const { return !(*this == other); }

    // 按队首到队尾输出，空格分隔；空队列输出 EMPTY（不带换行）。
    void printFrontToBack(std::ostream& os) const;

    // 就地构造后入队；成员模板定义在头文件里。
    template <typename... Args>
    void emplace(Args&&... args) {
        T value(std::forward<Args>(args)...);
        push(value);
    }

private:
    void checkNotEmpty(const char* what) const;

    // 环上的下一个下标。
    std::size_t next(std::size_t index) const {
        return index + 1 == buffer_.size() ? 0 : index + 1;
    }

    std::vector<T> buffer_;  // capacity + 1 个槽
    std::size_t front_ = 0;  // 队首元素的下标
    std::size_t back_ = 0;   // 下一个入队位置
    std::size_t count_ = 0;  // 当前元素个数（O(1) 的 size()）
};

} // namespace dsaac

template <typename T>
using CircularQueue = dsaac::CircularQueue<T>;

#endif