// stack_by_queue.h
// 用队列实现的栈 template<typename T> class StackByQueue。
//
// 核心思路：内部只有一个 std::queue，约定 front 是栈底、back 是栈顶。
//   push(x) —— 直接入队，O(1)。
//   pop()   —— 把前 n-1 个元素依次“出队再入队”，它们会挪到队尾，
//              于是队首变成原来的队尾（栈顶），直接出队丢掉。O(n)。
//   top()   —— 同样先转到栈顶在队首，取走这个值后，再把队列转回去，
//              保持数据不变。两次各转 n-1 个元素，O(n)。
//
// 复杂度（均摊分析）
//   push 单次 O(1)；pop/top 单次 O(n)，n 为当前元素个数。
//   这里没有把代价摊到 push 上的技巧（那是“两个队列、push 时搬家”的做法），
//   每一次 pop/top 都实打实地转一圈，所以是严格的 O(n)。之所以写“均摊
//   O(n)”是因为题目以此命名：连续 k 次操作的代价是 k·O(n)，不存在某一次
//   特别贵、其余特别便宜的摊还效应——它被队列的 FIFO 顺序逼出了这个下界。
//
// 空间 O(n)，不需要第二个队列：转圈时被弹出的元素立刻回到同一个队列。
// top() 返回 by value，因为转圈过程中队首元素会被弹出再压入，
// 返回引用可能悬垂（deque 的 pop_front 会让指向该元素的引用失效）。

#ifndef DSAAC_STACK_BY_QUEUE_H
#define DSAAC_STACK_BY_QUEUE_H

#include <cstddef>
#include <ostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace dsaac {

template <typename T>
class StackByQueue {
public:
    // ---------------------------------------------------------------
    // 栈语义
    // ---------------------------------------------------------------

    bool empty() const { return data_.empty(); }
    std::size_t size() const { return data_.size(); }

    // O(1)：直接入队。
    void push(const T& value) { data_.push(value); }

    // O(n)：转到栈顶在队首后出队。
    void pop();

    // O(n)：转到栈顶在队首取值，再转回来。空栈抛 std::out_of_range。
    T top();

    // ---------------------------------------------------------------
    // 扩展功能（参考 std::stack）
    // ---------------------------------------------------------------

    void clear() {
        std::queue<T> replaced;
        std::swap(data_, replaced);
    }

    void swap(StackByQueue& other) noexcept { std::swap(data_, other.data_); }

    // 按栈底 -> 栈顶导出：队列的 front -> back 正是栈底 -> 栈顶。
    std::vector<T> toVector() const;

    bool operator==(const StackByQueue& other) const { return toVector() == other.toVector(); }
    bool operator!=(const StackByQueue& other) const { return !(*this == other); }

    // 按栈顶到栈底输出，空格分隔；空栈输出 EMPTY（不带换行）。
    void printTopToBottom(std::ostream& os) const;

    template <typename... Args>
    void emplace(Args&&... args) {
        data_.emplace(std::forward<Args>(args)...);
    }

private:
    void checkNotEmpty(const char* what) const;
    // 轮转 1 格：队首移到队尾。
    void rotateOnce();
    // 把 back（栈顶）转到 front：前 size()-1 个元素依次出队再入队。
    void rotate();

    std::queue<T> data_;  // front = 栈底，back = 栈顶
};

} // namespace dsaac

template <typename T>
using StackByQueue = dsaac::StackByQueue<T>;

#endif