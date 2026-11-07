// stackli.hpp
// 链式栈 template<typename T> class LinkedStack。
// 题目允许用 std::forward_list<T> 或自建结点；这里用 forward_list，
// 并把它的头节点当作栈顶：push 就是 push_front，pop 就是 pop_front，
// 因此所有栈操作都是 O(1)。
//
// 链式栈没有“满”的概念——只要内存够就能一直 push，故不提供 full()。

#ifndef DSAAC_LINKED_STACK_HPP
#define DSAAC_LINKED_STACK_HPP

#include <cstddef>
#include <forward_list>
#include <ostream>
#include <utility>
#include <vector>

namespace dsaac {

template <typename T>
class LinkedStack {
public:
    // ---------------------------------------------------------------
    // 题目要求的基础接口
    // ---------------------------------------------------------------

    bool empty() const;
    void push(const T& value);  // 头插，O(1)
    void pop();                 // 空栈抛 std::out_of_range
    const T& top() const;       // 空栈抛 std::out_of_range

    // ---------------------------------------------------------------
    // 扩展功能（参考 std::stack）
    // ---------------------------------------------------------------

    // 元素个数；forward_list 在 C++11 起不保证 size() 是 O(1)，
    // 这里单独维护 size_，保证 size() 恒为 O(1)。
    std::size_t size() const { return size_; }

    // 清空并释放全部结点。
    void clear() { values_.clear(); size_ = 0; }

    // 与另一座栈交换内容。
    void swap(LinkedStack& other) noexcept;

    // 按栈底到栈顶的顺序导出（与 printTopToBottom 方向相反）。
    std::vector<T> toVector() const;

    // 用另一座栈的元素覆盖自己：清空后按栈底到栈顶逐个压入。
    void assign(const LinkedStack& other);

    // 把栈顶元素改成 value；空栈抛 std::out_of_range。
    void setTop(const T& value);

    // 逐元素比较（栈底 -> 栈顶顺序）。
    bool operator==(const LinkedStack& other) const;

    bool operator!=(const LinkedStack& other) const { return !(*this == other); }

    // 按栈顶到栈底的顺序输出，空格分隔；空栈输出 EMPTY（不带换行）。
    void printTopToBottom(std::ostream& os) const;

    // std::stack 的 emplace 语义：用参数就地构造一个 T 再压栈。
    template <typename... Args>
    void emplace(Args&&... args) {
        T value(std::forward<Args>(args)...);
        push(value);
    }

private:
    void checkNotEmpty(const char* what) const;

    std::forward_list<T> values_;
    std::size_t size_ = 0;
};

} // namespace dsaac

template <typename T>
using LinkedStack = dsaac::LinkedStack<T>;

#endif