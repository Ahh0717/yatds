// dynamic_array.cpp
// 模板类的成员函数必须对使用者可见，因此完整实现写在 dynamic_array.h 中。
// 本文件只做两件事：
//   1. 显式实例化 int 与 std::string，充当“编译期自检”——任一成员在两种类型下
//      无法编译都会在这里立刻暴露；
//   2. 承载与外层类型无关的辅助实现。

#include "dynamic_array.h"

#include <ostream>
#include <string>

namespace mylab {

// 输出运算符：便于 std::cout << array 的写法。空数组不输出内容。
template <typename T>
std::ostream &operator<<(std::ostream &os, const DynamicArray<T> &array) {
    array.print(os);
    return os;
}

template class DynamicArray<int>;
template class DynamicArray<std::string>;

template std::ostream &operator<<(std::ostream &os, const DynamicArray<int> &array);
template std::ostream &operator<<(std::ostream &os, const DynamicArray<std::string> &array);

}  // namespace mylab