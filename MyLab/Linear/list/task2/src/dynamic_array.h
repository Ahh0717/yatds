// dynamic_array.h
// 手写的可变数组模板类，语义参考 std::vector：
//   - 内部只有 T *data_ / std::size_t size_ / std::size_t cap_ 三个成员；
//   - 扩容时申请新块、逐个搬迁、释放旧块，不借助任何 STL 容器；
//   - 容量按 2 倍增长，并保证任何时刻 cap_ >= kMinCapacity（不会出现容量为 0）。
//
// 关于 CLEAR 的语义（题目要求明确说明）：
//   clear() 只把 size_ 置 0 并析构其中的元素，**不释放也不缩小** data_。
//   这样紧随其后的 pushBack 不必重新分配，适合“清空后马上重用”的循环场景；
//   若确实需要把内存还给系统，请显式调用 shrinkToFit()。
//   注意 std::vector::clear() 同样不保证缩容，此处的选择与之一致，
//   差别在于本类保证容量一字不变（std::vector 允许但通常不会缩减）。

#ifndef MYLAB_DYNAMIC_ARRAY_H
#define MYLAB_DYNAMIC_ARRAY_H

#include <cstddef>
#include <new>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace mylab {

template <typename T>
class DynamicArray {
public:
    // 空数组新建时给的最小容量，同时也是扩容下限，避免容量为 0 时的除零。
    static constexpr std::size_t kMinCapacity = 4;

    // 默认构造也立刻分配最小容量：题目要求“空数组新建时给一个最小容量”，
    // 这样 cap_ 恒不为 0，任何“按 cap_ 取模/相除”的扩展逻辑都不会除零。
    DynamicArray() : data_(nullptr), size_(0), cap_(0) {
        reserve(kMinCapacity);
    }

    // 按指定初始容量分配；n 与请求容量分别对应输入格式第一行的 capacity n。
    explicit DynamicArray(std::size_t initialCapacity);

    DynamicArray(const DynamicArray &other);
    DynamicArray &operator=(const DynamicArray &other);
    DynamicArray(DynamicArray &&other) noexcept;
    DynamicArray &operator=(DynamicArray &&other) noexcept;

    ~DynamicArray();

    // ---------------------------------------------------------------
    // 题目要求的接口
    // ---------------------------------------------------------------

    void pushBack(const T &x);

    // 在下标 i（0 <= i <= size）处插入；i 越界返回 false 且不修改数组。
    bool insertAt(std::size_t i, const T &x);

    // 删除下标 i 处元素；i 越界返回 false 且不修改数组。
    bool eraseAt(std::size_t i);

    // 读取下标 i 处元素；越界返回 false 且不写入 out。
    bool get(std::size_t i, T &out) const;

    // 修改下标 i 处元素；越界返回 false。
    bool set(std::size_t i, const T &x);

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return cap_; }
    bool empty() const { return size_ == 0; }

    // 清空元素但不缩容，详见文件头注释。
    void clear();

    // 元素以单个空格分隔；空数组输出 EMPTY（不带换行）。
    void print(std::ostream &os) const;

    // ---------------------------------------------------------------
    // vector 式的扩展接口
    // ---------------------------------------------------------------

    // 预留至少 newCap 的容量；只增不减。返回最终容量。
    std::size_t reserve(std::size_t newCap) {
        if (newCap > cap_) {
            reallocate(newCap);
        }
        return cap_;
    }

    // 调整元素个数：变大时用 T() 补齐，变小时析构多余元素（不缩容）。
    bool resize(std::size_t newSize) {
        if (newSize > cap_) {
            reserve(newSize);
        }
        while (size_ > newSize) {
            data_[--size_].~T();
        }
        while (size_ < newSize) {
            new (data_ + size_) T();
            ++size_;
        }
        return true;
    }

    // 带边界检查的访问；越界抛 std::out_of_range（与 std::vector::at 一致）。
    T &at(std::size_t i) {
        if (i >= size_) {
            throw std::out_of_range("DynamicArray::at: 下标越界");
        }
        return data_[i];
    }
    const T &at(std::size_t i) const {
        if (i >= size_) {
            throw std::out_of_range("DynamicArray::at: 下标越界");
        }
        return data_[i];
    }

    // 不检查边界的访问；调用方自行保证 i < size。空数组调用 front/back 是未定义行为。
    T &operator[](std::size_t i) { return data_[i]; }
    const T &operator[](std::size_t i) const { return data_[i]; }

    T &front() { return data_[0]; }
    const T &front() const { return data_[0]; }
    T &back() { return data_[size_ - 1]; }
    const T &back() const { return data_[size_ - 1]; }

    // 弹出尾部元素；空数组返回 false。
    bool popBack(T &out) {
        if (empty()) {
            return false;
        }
        out = std::move(data_[--size_]);
        data_[size_].~T();
        return true;
    }

    // 把容量收缩到当前元素个数（不低于 kMinCapacity）。
    void shrinkToFit() {
        const std::size_t target = size_ < kMinCapacity ? kMinCapacity : size_;
        if (target < cap_) {
            reallocate(target);
        }
    }

    // 在末尾就地构造元素，返回引用。
    // 注意：若参数是*本数组内部*元素的引用（如 a.emplaceBack(a[0])），扩容会让
    // 该引用悬垂，本函数无法察觉。需要这种用法时请走 pushBack / insertAt，
    // 它们会先把值拷贝出来再扩容。
    template <typename... Args>
    T &emplaceBack(Args &&...args) {
        if (size_ == cap_) {
            // 默认构造的数组 cap_ 为 0，这里不能直接乘 2，否则会得到 0 容量。
            reallocate(cap_ == 0 ? kMinCapacity : cap_ * 2);
        }
        new (data_ + size_) T(std::forward<Args>(args)...);
        return data_[size_++];
    }

private:
    T *data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t cap_ = 0;

    // 把容量调整到 newCap（newCap 不应小于 size_），逐个搬迁后释放旧块。
    void reallocate(std::size_t newCap);

    // 确保还能再放 perSize 个元素，不够就按 2 倍增长。
    void ensureCapacity(std::size_t need) {
        if (need <= cap_) {
            return;
        }
        std::size_t newCap = cap_ == 0 ? kMinCapacity : cap_;
        while (newCap < need) {
            newCap *= 2;
        }
        reserve(newCap);
    }
};

// ===================================================================
// 实现
// ===================================================================

template <typename T>
DynamicArray<T>::DynamicArray(std::size_t initialCapacity)
    : data_(nullptr), size_(0), cap_(0) {
    reserve(initialCapacity < kMinCapacity ? kMinCapacity : initialCapacity);
}

template <typename T>
DynamicArray<T>::DynamicArray(const DynamicArray &other)
    : data_(nullptr), size_(0), cap_(0) {
    reserve(other.size_);
    for (std::size_t i = 0; i < other.size_; ++i) {
        new (data_ + i) T(other.data_[i]);
        ++size_;
    }
}

template <typename T>
DynamicArray<T> &DynamicArray<T>::operator=(const DynamicArray &other) {
    if (this == &other) {
        return *this;
    }
    // 先拷贝再交换：既保证自赋值安全，也提供强异常安全。
    DynamicArray tmp(other);
    std::swap(data_, tmp.data_);
    std::swap(size_, tmp.size_);
    std::swap(cap_, tmp.cap_);
    return *this;
}

template <typename T>
DynamicArray<T>::DynamicArray(DynamicArray &&other) noexcept
    : data_(other.data_), size_(other.size_), cap_(other.cap_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.cap_ = 0;
}

template <typename T>
DynamicArray<T> &DynamicArray<T>::operator=(DynamicArray &&other) noexcept {
    if (this == &other) {
        return *this;
    }
    // 释放自身资源后接管 other 的缓冲区。
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i].~T();
    }
    ::operator delete(data_);
    data_ = other.data_;
    size_ = other.size_;
    cap_ = other.cap_;
    other.data_ = nullptr;
    other.size_ = 0;
    other.cap_ = 0;
    return *this;
}

template <typename T>
DynamicArray<T>::~DynamicArray() {
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i].~T();
    }
    // 缓冲区由 ::operator new 申请，因此也必须用 ::operator delete 释放。
    ::operator delete(data_);
}

// 元素不会与缓冲区用同一条释放路径混用：构造只走 placement new，
// 结束只走显式析构，缓冲区本身只走 ::operator new / ::operator delete。
template <typename T>
void DynamicArray<T>::reallocate(std::size_t newCap) {
    if (newCap < size_) {
        newCap = size_;
    }
    // 申请未初始化的原始存储，元素个数由 size_ / cap_ 管理。
    T *fresh = static_cast<T *>(::operator new(newCap * sizeof(T)));
    for (std::size_t i = 0; i < size_; ++i) {
        new (fresh + i) T(std::move_if_noexcept(data_[i]));
        data_[i].~T();
    }
    ::operator delete(data_);
    data_ = fresh;
    cap_ = newCap;
}

template <typename T>
void DynamicArray<T>::pushBack(const T &x) {
    // 先把值拷到局部变量：若 x 就是本数组内的元素，重新分配会让它悬垂。
    T copy(x);
    emplaceBack(std::move(copy));
}

template <typename T>
bool DynamicArray<T>::insertAt(std::size_t i, const T &x) {
    if (i > size_) {
        return false;
    }
    // 同理，先把要插入的值拷出来，避免 x 指向本数组且随后的扩容使其悬垂。
    T copy(x);
    ensureCapacity(size_ + 1);
    if (i == size_) {
        new (data_ + size_) T(std::move(copy));
    } else {
        // 先把末元素挪到 i 处，再从 i 开始整体后移一位。为避免与自身重叠
        // 时出错，从尾部向前逐个构造。
        new (data_ + size_) T(std::move(data_[size_ - 1]));
        for (std::size_t k = size_ - 1; k > i; --k) {
            data_[k] = std::move(data_[k - 1]);
        }
        data_[i] = std::move(copy);
    }
    ++size_;
    return true;
}

template <typename T>
bool DynamicArray<T>::eraseAt(std::size_t i) {
    if (i >= size_) {
        return false;
    }
    for (std::size_t k = i; k + 1 < size_; ++k) {
        data_[k] = std::move(data_[k + 1]);
    }
    --size_;
    data_[size_].~T();
    return true;
}

template <typename T>
bool DynamicArray<T>::get(std::size_t i, T &out) const {
    if (i >= size_) {
        return false;
    }
    out = data_[i];
    return true;
}

template <typename T>
bool DynamicArray<T>::set(std::size_t i, const T &x) {
    if (i >= size_) {
        return false;
    }
    data_[i] = x;
    return true;
}

template <typename T>
void DynamicArray<T>::clear() {
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i].~T();
    }
    size_ = 0;
    // 刻意不释放 data_、不改动 cap_：见文件头注释。
}

template <typename T>
void DynamicArray<T>::print(std::ostream &os) const {
    if (size_ == 0) {
        os << "EMPTY";
        return;
    }
    for (std::size_t i = 0; i < size_; ++i) {
        if (i != 0) {
            os << ' ';
        }
        os << data_[i];
    }
}

}  // namespace mylab

#endif  // MYLAB_DYNAMIC_ARRAY_H