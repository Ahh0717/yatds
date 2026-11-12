#ifndef DSAAC_MYSTRING_HPP
#define DSAAC_MYSTRING_HPP

#include <cstddef>

namespace dsaac {

class MyString {
public:
    // 长度查找失败时的返回值（std::string 同款约定）。
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    MyString();
    explicit MyString(const char* str);
    ~MyString();

    MyString(const MyString& other);
    MyString& operator=(const MyString& other);

    std::size_t length() const;
    MyString substr(std::size_t pos, std::size_t n) const;
    void append(const MyString& other);

    // 返回以 '\0' 结尾的内部缓冲区；O(1)，调用方不得改写。
    const char* c_str() const;
    // 是否为空串；O(1)。
    bool empty() const;
    // 首次出现 sub 的下标，找不到返回 npos；空模式返回 0。最坏 O((n-m+1)*m)。
    std::size_t find(const MyString& sub) const;
    // 把首次出现的 old 替换为 new；old 为空或不存在时不做任何改动。O(n+m)。
    void replace(const MyString& old_sub, const MyString& new_sub);
    // 字典序比较：小于返回 -1，相等返回 0，大于返回 1。O(min(n,m))。
    int compare(const MyString& other) const;
    // 清空为 ""，保留已分配容量不缩容；O(1)。
    void clear();
    // 预分配至少 n+1 字节容量，已有容量更大时不缩容；O(n)。
    void reserve(std::size_t n);
    // 等价于 append(other)，支持链式调用；复杂度同 append。
    MyString& operator+=(const MyString& other);

private:
    // 确保容量至少为 need（含结尾 '\0'），不足时按倍增策略扩容。
    void ensureCapacity(std::size_t need);

    char* data_;
    std::size_t len_;
    std::size_t cap_;
};

} // namespace dsaac

using MyString = dsaac::MyString;

#endif // DSAAC_MYSTRING_HPP