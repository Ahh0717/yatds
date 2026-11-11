// queue_by_stack.h
// 用栈实现的队列 template<typename T> class QueueByStack。
//
// 核心思路：两个栈 in_ / out_。
//   push(x)      —— 压入 in_，O(1)。
//   front()/pop() —— 若 out_ 为空，就把 in_ 全部弹出并压进 out_：
//                   两次逆序抵消，out_ 的栈顶正好是最早入队的元素。
//                   之后从 out_ 顶部取/弹即可。
//
// 复杂度（均摊分析）
//   每个元素一生只会在 in_ 里被压入一次、在倒入 out_ 时被弹出推送各一次、
//   最终从 out_ 被弹出至多一次，总共 O(1) 次栈操作。因此 m 次混合操作的
//   总代价是 O(m)，单次 front/pop 均摊 O(1)，最坏情况（恰好触发一次倒入）
//   是 O(n)。
//   空间 O(n)：元素只存在两个栈中的某一个里。

#ifndef DSAAC_QUEUE_BY_STACK_H
#define DSAAC_QUEUE_BY_STACK_H

#include <cstddef>
#include <ostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace dsaac {

template <typename T>
class QueueByStack {
public:
    // ---------------------------------------------------------------
    // 队列语义
    // ---------------------------------------------------------------

    bool empty() const { return in_.empty() && out_.empty(); }
    std::size_t size() const { return in_.size() + out_.size(); }

    // O(1)：直接压 in_。
    void push(const T& value) { in_.push(value); }

    // 均摊 O(1)，最坏 O(n)：必要时把 in_ 倒入 out_ 后弹出 out_ 顶部。
    void pop();

    // 同 pop()，只取值不删除。空队列抛 std::out_of_range。
    const T& front() const;

    // ---------------------------------------------------------------
    // 扩展功能（参考 std::queue）
    // ---------------------------------------------------------------

    void clear() {
        std::stack<T> in;
        std::stack<T> out;
        std::swap(in_, in);
        std::swap(out_, out);
    }

    void swap(QueueByStack& other) noexcept {
        std::swap(in_, other.in_);
        std::swap(out_, other.out_);
    }

    // 按队首 -> 队尾导出。
    std::vector<T> toVector() const;

    bool operator==(const QueueByStack& other) const {
        return toVector() == other.toVector();
    }
    bool operator!=(const QueueByStack& other) const { return !(*this == other); }

    // 按队首到队尾输出，空格分隔；空队列输出 EMPTY（不带换行）。
    void printFrontToBack(std::ostream& os) const;

    template <typename... Args>
    void emplace(Args&&... args) {
        in_.emplace(std::forward<Args>(args)...);
    }

private:
    void checkNotEmpty(const char* what) const;
    // 若 out_ 为空则把 in_ 整个倒入 out_。调用前需保证至少有一个元素。
    void pour() const;

    mutable std::stack<T> in_;   // 新元素从这里进
    mutable std::stack<T> out_;  // 队首在栈顶
};

} // namespace dsaac

template <typename T>
using QueueByStack = dsaac::QueueByStack<T>;

#endif