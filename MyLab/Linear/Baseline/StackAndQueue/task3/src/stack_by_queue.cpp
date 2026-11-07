// stack_by_queue.cpp
// StackByQueue<T> 的实现 + int / std::string 的显式实例化。

#include "stack_by_queue.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace dsaac {

/* START: stack_by_queue implementation */

template <typename T>
void StackByQueue<T>::checkNotEmpty(const char* what) const {
    if (data_.empty()) {
        throw std::out_of_range(std::string("StackByQueue::") + what +
                                ": stack is empty");
    }
}

template <typename T>
void StackByQueue<T>::rotateOnce() {
    T value = data_.front();
    data_.pop();
    data_.push(value);
}

template <typename T>
void StackByQueue<T>::rotate() {
    // 队列 [a b c d]（a 是栈底、d 是栈顶），size-1 = 3 次轮转：
    //   [b c d a] -> [c d a b] -> [d a b c]，front 变成 d（栈顶）。
    const std::size_t count = data_.size() - 1;
    for (std::size_t k = 0; k < count; ++k) {
        T value = data_.front();
        data_.pop();
        data_.push(value);
    }
}

template <typename T>
void StackByQueue<T>::pop() {
    checkNotEmpty("pop");
    rotate();      // 栈顶来到队首
    data_.pop();   // 丢掉栈顶
}

template <typename T>
T StackByQueue<T>::top() {
    checkNotEmpty("top");
    rotate();  // 栈顶来到队首
    const T value = data_.front();

    // 恢复原顺序只需再轮转 1 次，而不是 n-1 次：
    // 轮转 n-1 次等价于“向后转 1 格”，它的逆操作就是“向前转 1 格”。
    // 也就是说 rotate() + rotateOnce() 合计轮转 n 次，恰好回到原状态。
    rotateOnce();
    return value;
}

/* END */

// -------------------------------------------------------------------
// 扩展功能
// -------------------------------------------------------------------

template <typename T>
std::vector<T> StackByQueue<T>::toVector() const {
    std::vector<T> out;
    out.reserve(data_.size());
    std::queue<T> copy = data_;
    while (!copy.empty()) {
        out.push_back(copy.front());
        copy.pop();
    }
    return out;
}

template <typename T>
void StackByQueue<T>::printTopToBottom(std::ostream& os) const {
    // 队列顺序是栈底 -> 栈顶，打印要反过来；先转成 vector 再倒序列出。
    const std::vector<T> bottomToTop = toVector();
    if (bottomToTop.empty()) {
        os << "EMPTY";
        return;
    }
    for (auto it = bottomToTop.rbegin(); it != bottomToTop.rend(); ++it) {
        if (it != bottomToTop.rbegin()) {
            os << ' ';
        }
        os << *it;
    }
}

// 显式实例化：如果模板实现写错，这里会直接编译失败。
template class StackByQueue<int>;
template class StackByQueue<std::string>;

} // namespace dsaac