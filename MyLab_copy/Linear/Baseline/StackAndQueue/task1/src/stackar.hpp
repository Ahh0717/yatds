// stackar.hpp
// 固定容量的数组栈 template<typename T> class ArrayStack。
// 保留模板给出的构造函数与 empty/full/push/pop/top 签名，在此基础上
// 参考 std::stack 补齐 size/clear/swap/toVector 等功能。
//
// 注意：题目允许内部用 std::vector<T> 作顺序存储，但栈的操作只暴露
// 栈语义（只能动栈顶），不把 vector 的自由操作泄漏出去。

#ifndef DSAAC_ARRAY_STACK_HPP
#define DSAAC_ARRAY_STACK_HPP

#include <cstddef>
#include <ostream>
#include <utility>
#include <vector>

namespace dsaac {

template <typename T>
class ArrayStack {
public:
    // 构造即确定最大容量；capacity 为 0 表示一个元素也放不下。
    explicit ArrayStack(std::size_t capacity);

    // ---------------------------------------------------------------
    // 题目要求的基础接口
    // ---------------------------------------------------------------

    bool empty() const;
    bool full() const;
    void push(const T& value);  // 满栈抛 std::overflow_error
    void pop();                 // 空栈抛 std::out_of_range
    const T& top() const;       // 空栈抛 std::out_of_range

    // ---------------------------------------------------------------
    // 扩展功能（参考 std::stack）
    // ---------------------------------------------------------------

    // 当前元素个数 / 最大容量。
    std::size_t size() const { return values_.size(); }
    std::size_t capacity() const { return capacity_; }

    // 清空全部元素；容量保持不变（与固定容量数组栈的语义一致）。
    void clear();

    // 容量允许时把栈顶元素改成 value；满栈时再入栈会丢数据的场景下
    // 不能直接 push，故提供显式的“改栈顶”。空栈抛 std::out_of_range。
    void setTop(const T& value);

    // 与另一座栈交换全部内容（元素与容量一起交换）。
    void swap(ArrayStack& other);

    // 按栈底到栈顶的顺序导出（与栈的打印方向正好相反），便于外部比较。
    std::vector<T> toVector() const;

    // 复用另一个栈的元素：清空后按栈底到栈顶逐个压入。
    void assign(const ArrayStack& other);

    // 自底向上逐元素比较（两个栈的容量可以不同，只看元素）。
    bool operator==(const ArrayStack& other) const;
    bool operator!=(const ArrayStack& other) const { return !(*this == other); }

    // 按栈顶到栈底的顺序输出，空格分隔；空栈输出 EMPTY（不带换行）。
    void printTopToBottom(std::ostream& os) const;

    // std::stack 的 emplace 语义：用参数就地构造一个 T 再压栈。
    // 成员模板无法在 .cpp 里显式实例化，故定义在头文件内。
    template <typename... Args>
    void emplace(Args&&... args) {
        T value(std::forward<Args>(args)...);
        push(value);
    }

private:
    void checkNotEmpty(const char* what) const;

    std::vector<T> values_;
    std::size_t capacity_;
};

} // namespace dsaac

template <typename T>
using ArrayStack = dsaac::ArrayStack<T>;

#endif