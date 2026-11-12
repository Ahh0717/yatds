#include "mystring.hpp"

#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>

/* START: mystring implementation */

// 默认构造：容量固定为 1（只放结尾 '\0'），保证 cap_ 永不为 0。
dsaac::MyString::MyString()
{
    cap_ = 1;
    data_ = new char[cap_];
    data_[0] = '\0';
    len_ = 0;
}

// 以 C 字符串构造；nullptr 视为空串，避免调用方传空指针时崩溃。
dsaac::MyString::MyString(const char* str)
{
    len_ = 0;
    if (str != nullptr) {
        while (str[len_] != '\0') {
            len_++;
        }
    }
    cap_ = len_ + 1;
    data_ = new char[cap_];
    for (std::size_t i = 0; i < len_; ++i) {
        data_[i] = str[i];
    }
    data_[len_] = '\0';
}

// 析构：释放字符数组，delete[] 与构造时的 new[] 一一对应。
dsaac::MyString::~MyString()
{
    delete[] data_;
}

// 拷贝构造：深拷贝，容量按源对象当前容量分配，不共享缓冲区。
dsaac::MyString::MyString(const MyString& other)
{
    len_ = other.len_;
    cap_ = other.cap_;
    data_ = new char[cap_];
    for (std::size_t i = 0; i <= len_; ++i) {
        data_[i] = other.data_[i];
    }
}

// 拷贝赋值：深拷贝，先申请新块再释放旧块，保证自赋值与异常安全。
dsaac::MyString& dsaac::MyString::operator=(const MyString& other)
{
    if (this != &other) {
        char* block = new char[other.cap_];
        for (std::size_t i = 0; i <= other.len_; ++i) {
            block[i] = other.data_[i];
        }
        delete[] data_;
        data_ = block;
        len_ = other.len_;
        cap_ = other.cap_;
    }
    return *this;
}

// 返回有效字符个数；O(1)。
std::size_t dsaac::MyString::length() const
{
    return len_;
}

// 截取子串：pos >= len_ 一律抛 out_of_range（空串也抛）；
// pos + n 超过末尾时截到末尾。复杂度 O(k)，k 为结果长度。
dsaac::MyString dsaac::MyString::substr(std::size_t pos, std::size_t n) const
{
    if (pos >= len_) {
        throw std::out_of_range("MyString::substr: pos out of range");
    }
    std::size_t subLen = (n > len_ - pos) ? (len_ - pos) : n;

    char* temp = new char[subLen + 1];
    for (std::size_t i = 0; i < subLen; i++) {
        temp[i] = data_[pos + i];
    }
    temp[subLen] = '\0';

    dsaac::MyString result(temp);
    delete[] temp;
    return result;
}

// 末尾拼接：容量不足时按倍增策略扩容，再整块搬移。均摊 O(other.len_)。
void dsaac::MyString::append(const MyString& other)
{
    if (other.len_ == 0) {
        return;
    }
    if (this == &other) {
        // 自拼接：扩容可能释放 other 所指向的旧块，先深拷贝一份再拼。
        const dsaac::MyString copy(other);
        append(copy);
        return;
    }
    // 快路径：容量够时直接拷贝，避免每次 append 都走一次扩容检查的函数调用。
    const std::size_t need = len_ + other.len_ + 1;
    if (need > cap_) {
        ensureCapacity(need);
    }
    std::memcpy(data_ + len_, other.data_, other.len_);
    len_ += other.len_;
    data_[len_] = '\0';
}

// 返回以 '\0' 结尾的缓冲区，仅用于打印与只读访问；O(1)。
const char* dsaac::MyString::c_str() const
{
    return data_;
}

// 是否为空串；O(1)。
bool dsaac::MyString::empty() const
{
    return len_ == 0;
}

// 首次出现 sub 的下标，找不到返回 npos；空模式串按约定返回 0。
// 逐位回溯比较，最坏 O((n - m + 1) * m)。
std::size_t dsaac::MyString::find(const MyString& sub) const
{
    if (sub.len_ == 0) {
        return 0;
    }
    if (sub.len_ > len_) {
        return npos;
    }
    const std::size_t last = len_ - sub.len_;
    for (std::size_t i = 0; i <= last; ++i) {
        std::size_t j = 0;
        while (j < sub.len_ && data_[i + j] == sub.data_[j]) {
            ++j;
        }
        if (j == sub.len_) {
            return i;
        }
    }
    return npos;
}

// 把首次出现的 old 替换为 new；old 为空或不存在时保持原样。
// 先定位，再扩容、搬移尾部、写入新内容，整体 O(n + m)。
void dsaac::MyString::replace(const MyString& old_sub, const MyString& new_sub)
{
    if (old_sub.len_ == 0) {
        return;
    }
    const std::size_t pos = find(old_sub);
    if (pos == npos) {
        return;
    }

    const std::size_t tail = len_ - pos - old_sub.len_;  // 被替换段之后剩余的字符数
    const std::size_t newLen = pos + new_sub.len_ + tail;

    // new_sub 可能指向本对象的缓冲区；扩容会先释放旧块，必须先固定其内容。
    // 指针大小关系用 uintptr_t 比较，避免无关指针之间的关系运算。
    // 用 unique_ptr 持有临时块，扩容抛 bad_alloc 时也不会泄漏。
    std::unique_ptr<char[]> owned;
    const char* src = new_sub.data_;
    if (new_sub.len_ > 0) {
        const auto begin = reinterpret_cast<std::uintptr_t>(data_);
        const auto end = begin + len_ + 1;
        const auto addr = reinterpret_cast<std::uintptr_t>(new_sub.data_);
        if (addr >= begin && addr + new_sub.len_ <= end) {
            owned.reset(new char[new_sub.len_]);
            std::memcpy(owned.get(), new_sub.data_, new_sub.len_);
            src = owned.get();
        }
    }

    ensureCapacity(newLen + 1);
    if (tail > 0 && new_sub.len_ != old_sub.len_) {
        std::memmove(data_ + pos + new_sub.len_, data_ + pos + old_sub.len_, tail);
    }
    if (new_sub.len_ > 0) {
        std::memcpy(data_ + pos, src, new_sub.len_);
    }

    len_ = newLen;
    data_[len_] = '\0';
}

// 字典序比较：按 ASCII 逐字符比较，前缀相同时短串更小。O(min(n, m))。
int dsaac::MyString::compare(const MyString& other) const
{
    const std::size_t common = (len_ < other.len_) ? len_ : other.len_;
    for (std::size_t i = 0; i < common; ++i) {
        const unsigned char a = static_cast<unsigned char>(data_[i]);
        const unsigned char b = static_cast<unsigned char>(other.data_[i]);
        if (a != b) {
            return (a < b) ? -1 : 1;
        }
    }
    if (len_ == other.len_) {
        return 0;
    }
    return (len_ < other.len_) ? -1 : 1;
}

// 清空为 ""，容量保持不变，便于后续复用；O(1)。
void dsaac::MyString::clear()
{
    len_ = 0;
    data_[0] = '\0';
}

// 预分配至少 n + 1 字节容量；已有容量更大时不做缩容；O(n)。
void dsaac::MyString::reserve(std::size_t n)
{
    if (n == npos) {
        // n + 1 会回绕成 0，必须显式拒绝，否则 reserve 变成空操作。
        throw std::length_error("MyString::reserve: capacity too large");
    }
    ensureCapacity(n + 1);
}

// 与 append 等价，返回自身以支持链式调用；复杂度同 append。
dsaac::MyString& dsaac::MyString::operator+=(const MyString& other)
{
    append(other);
    return *this;
}

// 确保 cap_ >= need（need 含结尾 '\0'）。
// 倍增扩容：cap_ 反复乘 2 直到够用，整体拷贝成本均摊 O(1) 每字节。
void dsaac::MyString::ensureCapacity(std::size_t need)
{
    if (need <= cap_) {
        return;
    }
    std::size_t newCap = (cap_ == 0) ? 1 : cap_;
    while (newCap < need) {
        const std::size_t doubled = newCap * 2;
        if (doubled <= newCap) {  // 乘 2 溢出，直接按所需容量分配
            newCap = need;
            break;
        }
        newCap = doubled;
    }
    char* block = new char[newCap];
    if (len_ > 0) {
        std::memcpy(block, data_, len_);
    }
    block[len_] = '\0';
    delete[] data_;
    data_ = block;
    cap_ = newCap;
}

/* END */