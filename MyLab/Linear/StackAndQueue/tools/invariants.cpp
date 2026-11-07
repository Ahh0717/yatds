// invariants.cpp
// 三个阶段的算法级不变量检查：直接构造对象并断言接口语义，
// 覆盖样例与随机测试不容易表达的路径（异常类型、容量边界、回绕、
// 扩展功能、两种结构的相互转换）。
//
// 用法：make && ./build/invariants
// 成功时输出「通过 N 项，失败 0 项」并返回 0；有失败则逐条打印到
// stderr，最后输出「通过 N 项，失败 M 项」并返回 1。

#include <cstddef>
#include <iostream>
#include <queue>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "linked_queue.h"
#include "queue.hpp"
#include "queue_by_stack.h"
#include "shift_queue.h"
#include "stack_by_queue.h"
#include "stackar.hpp"
#include "stackli.hpp"
#include "vector_queue.h"
#include "vector_stack.h"

namespace {

int g_failed = 0;
int g_passed = 0;

// 单条断言：成功只累加计数，失败把标签与说明打到 stderr。
void check(bool ok, const char* what, const char* detail) {
    if (ok) {
        ++g_passed;
        return;
    }
    std::cerr << "FAIL [" << what << "] " << detail << '\n';
    ++g_failed;
}

template <typename Fn>
void checkThrowsOutOfRange(Fn fn, const char* what, const char* detail) {
    try {
        fn();
        check(false, what,
              (std::string(detail) + "：应当抛 std::out_of_range").c_str());
    } catch (const std::out_of_range&) {
        check(true, what, detail);  // 期望的异常
    } catch (...) {
        check(false, what,
              (std::string(detail) + "：抛的不是 std::out_of_range").c_str());
    }
}

template <typename Fn>
void checkThrowsOverflow(Fn fn, const char* what, const char* detail) {
    try {
        fn();
        check(false, what,
              (std::string(detail) + "：满结构 push 应当抛 std::overflow_error")
                  .c_str());
    } catch (const std::overflow_error&) {
        check(true, what, detail);  // 期望的异常
    } catch (...) {
        check(false, what,
              (std::string(detail) + "：满结构 push 抛的不是 std::overflow_error")
                  .c_str());
    }
}

// ---------------------------------------------------------------------
// 队列（阶段 2）共用的接口约定。
// ---------------------------------------------------------------------

// 三种队列共用的语义；queue 由调用方构造好，newQueue 用于 swap 的一方。
template <typename Queue, typename Factory>
void checkQueueCommon(Queue& queue, Factory newQueue, const char* name) {
    check(queue.empty(), name, "初始为空");
    check(queue.size() == 0, name, "初始 size 为 0");
    checkThrowsOutOfRange([&] { queue.front(); }, name, "空队列 front");
    checkThrowsOutOfRange([&] { queue.pop(); }, name, "空队列 pop");
    checkThrowsOutOfRange([&] { queue.back(); }, name, "空队列 back");

    // 先进先出；每次 push 后 size 恰好多一
    for (int value = 1; value <= 3; ++value) {
        const std::size_t before = queue.size();
        queue.push(value);
        check(queue.size() == before + 1, name, "push 后 size 加一");
    }
    check(!queue.empty(), name, "push 后非空");
    check(queue.size() == 3, name, "push 后 size 为 3");
    check(queue.front() == 1, name, "队首是最早入队的 1");
    check(queue.back() == 3, name, "队尾是最后入队的 3");

    // front/back 不改变内容
    check(queue.front() == 1 && queue.size() == 3, name, "front 不改变队列");

    const std::size_t beforePop = queue.size();
    queue.pop();
    check(queue.size() + 1 == beforePop, name, "pop 后 size 减一");
    check(queue.front() == 2, name, "pop 后队首是 2");

    // printFrontToBack：队首 -> 队尾
    std::ostringstream os;
    queue.printFrontToBack(os);
    check(os.str() == "2 3", name, "printFrontToBack 按队首到队尾输出");

    // toVector：队首 -> 队尾
    const std::vector<int> v = queue.toVector();
    check(v.size() == 2 && v[0] == 2 && v[1] == 3, name, "toVector 按队首到队尾");

    // 拷贝 + operator== / operator!=
    const Queue copy = queue;
    check(copy == queue, name, "拷贝后相等");
    Queue modified = queue;
    modified.push(4);
    check(modified != queue, name, "多一个元素后不相等");
    check(queue.size() == 2, name, "修改拷贝不影响原队列");

    // swap
    Queue other = newQueue();
    other.push(9);
    queue.swap(other);
    check(queue.size() == 1 && queue.front() == 9, name, "swap 后自身内容");
    check(other.size() == 2 && other.front() == 2, name, "swap 后对方内容");

    // clear
    queue.clear();
    check(queue.empty() && queue.size() == 0, name, "clear 后为空");
    os.str("");
    queue.printFrontToBack(os);
    check(os.str() == "EMPTY", name, "空队列打印 EMPTY");

    // clear 之后仍可复用
    queue.push(5);
    check(queue.front() == 5, name, "clear 后可继续 push");
}

// 两种栈共用的接口约定。stack 由调用方构造好（两种栈的构造方式不同），
// newStack 返回一座空栈，用于 swap 的一方。
template <typename Stack, typename Factory>
void checkCommon(Stack& stack, Factory newStack, const char* name) {
    check(stack.empty(), name, "初始为空");
    check(stack.size() == 0, name, "初始 size 为 0");
    checkThrowsOutOfRange([&] { stack.top(); }, name, "空栈 top");
    checkThrowsOutOfRange([&] { stack.pop(); }, name, "空栈 pop");
    checkThrowsOutOfRange([&] { stack.setTop(0); }, name, "空栈 setTop");

    // 先进后出；每次 push 后 top 必须正是刚压入的值，size 恰好多一
    for (int value = 1; value <= 3; ++value) {
        const std::size_t before = stack.size();
        stack.push(value);
        check(stack.top() == value, name, "push 后 top 等于刚压入的值");
        check(stack.size() == before + 1, name, "push 后 size 加一");
    }
    check(!stack.empty(), name, "push 后非空");
    check(stack.size() == 3, name, "push 后 size 为 3");
    check(stack.top() == 3, name, "栈顶是最后 push 的 3");

    // top 不改变内容
    check(stack.top() == 3 && stack.size() == 3, name, "top 不改变栈");

    const std::size_t beforePop = stack.size();
    stack.pop();
    check(stack.size() + 1 == beforePop, name, "pop 后 size 减一");
    check(stack.top() == 2, name, "pop 后栈顶是 2");

    // printTopToBottom：栈顶 -> 栈底
    std::ostringstream os;
    stack.printTopToBottom(os);
    check(os.str() == "2 1", name, "printTopToBottom 输出栈顶到栈底");

    // toVector：栈底 -> 栈顶
    const std::vector<int> v = stack.toVector();
    check(v.size() == 2 && v[0] == 1 && v[1] == 2, name,
          "toVector 输出栈底到栈顶");

    // setTop 只改栈顶
    stack.setTop(20);
    check(stack.top() == 20 && stack.size() == 2, name, "setTop 只改栈顶");

    // 拷贝 + operator== / operator!=
    const Stack copy = stack;
    check(copy == stack, name, "拷贝后相等");
    Stack modified = stack;
    modified.push(30);
    check(modified != stack, name, "多一个元素后不相等");
    check(stack.size() == 2, name, "修改拷贝不影响原栈");

    // assign：覆盖成另一座栈的形状
    stack.assign(modified);
    check(stack == modified, name, "assign 后相等");
    check(stack.size() == 3, name, "assign 后 size 正确");

    // swap
    Stack other = newStack();
    other.push(9);
    stack.swap(other);
    check(stack.size() == 1 && stack.top() == 9, name, "swap 后自身内容");
    check(other.size() == 3 && other.top() == 30, name, "swap 后对方内容");

    // clear
    stack.clear();
    check(stack.empty() && stack.size() == 0, name, "clear 后为空");
    os.str("");
    stack.printTopToBottom(os);
    check(os.str() == "EMPTY", name, "空栈打印 EMPTY");

    // clear 之后仍可复用
    stack.push(5);
    check(stack.top() == 5, name, "clear 后可继续 push");
}

void checkArraySpecific() {
    // capacity = 0：一个也不能放
    {
        dsaac::ArrayStack<int> stack(0);
        check(stack.empty() && stack.full(), "array", "容量 0 视为已满");
        checkThrowsOverflow([&] { stack.push(1); }, "array", "容量 0 时 push");
    }

    // 容量 1：push -> 满 -> pop -> push
    {
        dsaac::ArrayStack<int> stack(1);
        stack.push(7);
        check(stack.full(), "array", "容量 1 一个元素即满");
        checkThrowsOverflow([&] { stack.push(8); }, "array", "容量 1 已满时 push");
        check(stack.top() == 7, "array", "满栈 push 失败后栈顶不变");
        stack.pop();
        check(stack.empty() && !stack.full(), "array", "pop 后不再满");
        stack.push(9);
        check(stack.top() == 9, "array", "pop 后可再 push");
    }

    // 连续 push 到满再逐个 pop，顺序必须是逆序
    {
        dsaac::ArrayStack<int> stack(5);
        for (int i = 1; i <= 5; ++i) {
            stack.push(i);
        }
        check(stack.full() && stack.size() == 5, "array", "压满容量");
        checkThrowsOverflow([&] { stack.push(6); }, "array", "压满后 push");
        for (int i = 5; i >= 1; --i) {
            check(stack.top() == i, "array", "满栈弹出顺序为逆序");
            stack.pop();
        }
        check(stack.empty(), "array", "弹空");
    }

    // 容量固定：clear 不改变 capacity
    {
        dsaac::ArrayStack<int> stack(4);
        stack.push(1);
        stack.clear();
        check(stack.capacity() == 4, "array", "clear 不改变容量");
        check(!stack.full(), "array", "clear 后不再满");
    }

    // assign 超出容量必须失败且不破坏自己
    {
        dsaac::ArrayStack<int> small(2);
        small.push(1);
        dsaac::ArrayStack<int> big(4);
        for (int i = 1; i <= 4; ++i) {
            big.push(i);
        }
        checkThrowsOverflow([&] { small.assign(big); }, "array", "assign 超出容量");
        check(small.size() == 1 && small.top() == 1, "array",
              "assign 失败后原栈不变");
    }

    // swap 时容量也一起交换，两边都保持 size <= capacity
    {
        dsaac::ArrayStack<int> a(2);
        a.push(1);
        dsaac::ArrayStack<int> b(5);
        for (int i = 1; i <= 4; ++i) {
            b.push(i);
        }
        a.swap(b);
        check(a.capacity() == 5 && a.size() == 4, "array", "swap 交换容量");
        check(b.capacity() == 2 && b.size() == 1, "array", "swap 交换容量");
        a.push(5);
        check(a.full(), "array", "交换后可继续用到新容量");
    }

    // emplace 走同一套容量检查
    {
        dsaac::ArrayStack<std::string> stack(1);
        stack.emplace("hi");
        check(stack.top() == "hi", "array", "emplace 构造元素");
        checkThrowsOverflow([&] { stack.emplace("yo"); }, "array", "emplace 满栈");
    }
}

void checkLinkedSpecific() {
    // 链式栈没有容量上限：连续 push 远超任何“capacity”
    {
        dsaac::LinkedStack<int> stack;
        for (int i = 0; i < 1000; ++i) {
            stack.push(i);
        }
        check(stack.size() == 1000, "linked", "可以压入 1000 个元素");
        check(stack.top() == 999, "linked", "栈顶是最后压入的元素");
        for (int i = 999; i >= 0; --i) {
            if (stack.top() != i) {
                check(false, "linked", "弹出顺序为逆序");
                break;
            }
            stack.pop();
        }
        check(stack.empty(), "linked", "全部弹出后为空");
    }

    // 反复 push/pop 不应让 size_ 与链表实际长度脱节
    {
        dsaac::LinkedStack<int> stack;
        for (int round = 0; round < 50; ++round) {
            stack.push(round);
            stack.push(round + 1);
            stack.pop();
        }
        check(stack.size() == 50, "linked", "size 与实际结点数一致");
        check(stack.toVector().size() == 50, "linked", "toVector 长度一致");
    }

    // 自赋值不应清空自己
    {
        dsaac::LinkedStack<int> stack;
        stack.push(1);
        stack.push(2);
        dsaac::LinkedStack<int>& self = stack;
        stack.assign(self);
        check(stack.size() == 2 && stack.top() == 2, "linked", "自赋值不变");
    }
}

void checkStringSupport() {
    dsaac::ArrayStack<std::string> array(3);
    dsaac::LinkedStack<std::string> linked;
    for (const char* word : {"a", "bb", "ccc"}) {
        array.push(word);
        linked.push(word);
    }
    std::ostringstream a;
    std::ostringstream l;
    array.printTopToBottom(a);
    linked.printTopToBottom(l);
    check(a.str() == "ccc bb a", "string", "数组栈 string 打印");
    check(l.str() == "ccc bb a", "string", "链式栈 string 打印");
}

// ---------------------------------------------------------------------
// 队列的专项检查
// ---------------------------------------------------------------------

void checkCircularSpecific() {
    // 对外容量就是构造参数：capacity = 3 能装 3 个，第 4 个才满。
    {
        dsaac::CircularQueue<int> queue(3);
        for (int i = 1; i <= 3; ++i) {
            queue.push(i);
        }
        check(queue.full() && queue.size() == 3, "circular", "容量 3 装满 3 个");
        check(queue.capacity() == 3, "circular", "对外容量等于构造参数");
        checkThrowsOverflow([&] { queue.push(4); }, "circular", "满队列 push");
        check(queue.size() == 3 && queue.front() == 1, "circular",
              "满队列 push 失败后内容不变");
    }

    // 回绕：给 capacity 3 的缓冲区喂 500 次操作，让 front_/back_
    // 反复越过末尾（每次越过都会让下标从 2 绕回 0）。
    {
        const std::size_t capacity = 3;
        dsaac::CircularQueue<int> queue(capacity);
        std::vector<int> expected;  // 逻辑内容（队首 -> 队尾）
        std::size_t popped = 0;
        for (int k = 0; k < 500; ++k) {
            if (queue.full()) {
                // 满时先出队，腾出空间；这正是下标跨过末尾的时机。
                check(queue.front() == expected[popped], "circular",
                      "回绕过程中队首正确");
                queue.pop();
                ++popped;
            }
            queue.push(k);
            expected.push_back(k);

            const std::vector<int> got = queue.toVector();
            const std::vector<int> want(
                expected.begin() + static_cast<std::ptrdiff_t>(popped),
                expected.end());
            if (got != want) {
                check(false, "circular", "回绕过程中逻辑顺序正确");
                break;
            }
        }
        check(queue.size() == expected.size() - popped, "circular",
              "回绕过程中 size 正确");
        check(popped > 2 * capacity, "circular", "确实多次越过缓冲区末尾");
    }

    // 容量 1：push/pop 交替，front_ 与 back_ 每次都绕回 0。
    {
        dsaac::CircularQueue<int> queue(1);
        for (int k = 0; k < 10; ++k) {
            queue.push(k);
            check(queue.full() && queue.front() == k, "circular", "容量 1 入队");
            queue.pop();
            check(queue.empty(), "circular", "容量 1 出队");
        }
    }

    // 容量 0：一个也放不下。
    {
        dsaac::CircularQueue<int> queue(0);
        check(queue.empty() && queue.full(), "circular", "容量 0 视为已满");
        checkThrowsOverflow([&] { queue.push(1); }, "circular", "容量 0 时 push");
    }

    // clear 后容量不变，且下标复位后仍能绕圈工作。
    {
        dsaac::CircularQueue<int> queue(4);
        queue.push(1);
        queue.push(2);
        queue.pop();
        queue.clear();
        check(queue.capacity() == 4 && queue.empty(), "circular", "clear 不缩容");
        for (int i = 0; i < 4; ++i) {
            queue.push(i);
        }
        check(queue.full(), "circular", "clear 后仍可装满");
    }
}

void checkShiftSpecific() {
    // 平移式队列的 front 恒为下标 0：出队后整体左移。
    {
        dsaac::ShiftQueue<int> queue(4);
        for (int i = 1; i <= 4; ++i) {
            queue.push(i);
        }
        check(queue.full() && queue.size() == 4, "shift", "容量 4 装满 4 个");
        checkThrowsOverflow([&] { queue.push(5); }, "shift", "满队列 push");

        // 反复出队后剩下的元素仍然正确，且顺序不变
        for (int k = 1; k <= 3; ++k) {
            check(queue.front() == k, "shift", "出队顺序为先进先出");
            queue.pop();
        }
        check(queue.size() == 1 && queue.front() == 4, "shift", "左移后队首正确");

        // 出队腾出的空间可以继续使用（位置从 0 重新算起）
        queue.push(5);
        queue.push(6);
        const std::vector<int> want = {4, 5, 6};
        check(queue.toVector() == want, "shift", "左移后继续入队");
    }

    // 与循环队列的行为一致：容量 0 / 容量 1。
    {
        dsaac::ShiftQueue<int> queue(0);
        check(queue.empty() && queue.full(), "shift", "容量 0 视为已满");
        checkThrowsOverflow([&] { queue.push(1); }, "shift", "容量 0 时 push");
    }
    {
        dsaac::ShiftQueue<int> queue(1);
        queue.push(1);
        check(queue.full() && queue.front() == 1, "shift", "容量 1 入队");
        checkThrowsOverflow([&] { queue.push(2); }, "shift", "容量 1 满时 push");
        queue.pop();
        check(queue.empty(), "shift", "容量 1 出队");
    }

    // 连续出入队：先攒到接近满，再隔一个出一个，内容必须与参考序列一致。
    {
        const std::size_t capacity = 8;
        dsaac::ShiftQueue<int> queue(capacity);
        std::vector<int> expected;
        std::size_t popped = 0;
        for (int k = 0; k < 1000; ++k) {
            if (queue.full()) {
                queue.pop();
                ++popped;
            }
            queue.push(k);
            expected.push_back(k);
            if (k % 3 == 0) {
                // 出队后剩余元素整体左移，队首必须仍是 expected[popped]。
                check(queue.front() == expected[popped], "shift",
                      "左移后队首正确");
                queue.pop();
                ++popped;
            }
            const std::vector<int> got = queue.toVector();
            const std::vector<int> want(
                expected.begin() + static_cast<std::ptrdiff_t>(popped),
                expected.end());
            if (got != want) {
                check(false, "shift", "连续出入队后内容正确");
                break;
            }
        }
        check(true, "shift", "连续出入队检查完成");
    }
}

void checkLinkedQueueSpecific() {
    // 无容量上限：远超过任何“capacity”都能装下。
    {
        dsaac::LinkedQueue<int> queue;
        for (int i = 0; i < 1000; ++i) {
            queue.push(i);
        }
        check(queue.size() == 1000, "linked_queue", "可以入队 1000 个元素");
        check(queue.front() == 0 && queue.back() == 999, "linked_queue",
              "队首/队尾正确");
        for (int i = 0; i < 1000; ++i) {
            if (queue.front() != i) {
                check(false, "linked_queue", "出队顺序为先进先出");
                break;
            }
            queue.pop();
        }
        check(queue.empty(), "linked_queue", "全部出队后为空");
    }

    // 删空后再 push：tail_ 必须被正确重置，不能写到已释放的结点。
    {
        dsaac::LinkedQueue<int> queue;
        queue.push(1);
        queue.pop();
        queue.push(2);
        check(queue.front() == 2 && queue.back() == 2 && queue.size() == 1,
              "linked_queue", "删空后可继续入队");
    }

    // 深拷贝：两条队列不共享结点。
    {
        dsaac::LinkedQueue<int> a;
        a.push(1);
        a.push(2);
        dsaac::LinkedQueue<int> b = a;
        b.pop();
        check(a.size() == 2 && a.front() == 1, "linked_queue",
              "拷贝后修改副本不影响原队列");
        check(b.size() == 1 && b.front() == 2, "linked_queue", "副本内容正确");
    }

    // 移动后源对象回到空状态，可以继续使用。
    {
        dsaac::LinkedQueue<int> a;
        a.push(1);
        a.push(2);
        dsaac::LinkedQueue<int> b = std::move(a);
        check(b.size() == 2 && b.front() == 1, "linked_queue", "移动后内容转移");
        check(a.empty(), "linked_queue", "移动后源对象为空");
        a.push(3);
        check(a.front() == 3, "linked_queue", "移动后的源对象可复用");
    }

    // 自赋值不应清空自己。
    {
        dsaac::LinkedQueue<int> queue;
        queue.push(1);
        queue.push(2);
        dsaac::LinkedQueue<int>& self = queue;
        queue = self;
        check(queue.size() == 2 && queue.front() == 1, "linked_queue",
              "自赋值后内容不变");
    }
}

void checkQueueStringSupport() {
    dsaac::CircularQueue<std::string> circular(3);
    dsaac::ShiftQueue<std::string> shift(3);
    dsaac::LinkedQueue<std::string> linked;
    for (const char* word : {"a", "bb", "ccc"}) {
        circular.push(word);
        shift.push(word);
        linked.push(word);
    }
    std::ostringstream c;
    std::ostringstream s;
    std::ostringstream l;
    circular.printFrontToBack(c);
    shift.printFrontToBack(s);
    linked.printFrontToBack(l);
    check(c.str() == "a bb ccc", "string", "循环队列 string 打印");
    check(s.str() == "a bb ccc", "string", "平移队列 string 打印");
    check(l.str() == "a bb ccc", "string", "链式队列 string 打印");
}

// ---------------------------------------------------------------------
// 阶段 3：栈/队列相互实现 + 转换
// ---------------------------------------------------------------------

void checkStackByQueue() {
    dsaac::StackByQueue<int> stack;
    check(stack.empty() && stack.size() == 0, "stack_by_queue", "初始为空");
    checkThrowsOutOfRange([&] { stack.pop(); }, "stack_by_queue", "空栈 pop");
    checkThrowsOutOfRange([&] { stack.top(); }, "stack_by_queue", "空栈 top");

    for (int i = 1; i <= 5; ++i) {
        const std::size_t before = stack.size();
        stack.push(i);
        check(stack.top() == i, "stack_by_queue", "push 后 top 等于刚 push 的值");
        check(stack.size() == before + 1, "stack_by_queue", "push 后 size 加一");
    }
    check(stack.size() == 5, "stack_by_queue", "size 正确");
    check(stack.top() == 5, "stack_by_queue", "栈顶是最后 push 的元素");

    // top 不得改变内容：连续两次 top 结果相同、size 不变
    for (int k = 0; k < 3; ++k) {
        check(stack.top() == 5, "stack_by_queue", "重复 top 结果一致");
        check(stack.size() == 5, "stack_by_queue", "top 不改变 size");
    }

    // 后进先出：pop 顺序必须是 5 4 3 2 1，且每弹一个 size 减一
    for (int i = 5; i >= 1; --i) {
        const std::size_t before = stack.size();
        check(stack.top() == i, "stack_by_queue", "后进先出顺序");
        stack.pop();
        check(stack.size() + 1 == before, "stack_by_queue", "pop 后 size 减一");
    }
    check(stack.empty(), "stack_by_queue", "全部弹出后为空");

    // 弹空后再 push，队列内部状态必须干净
    stack.push(42);
    check(stack.size() == 1 && stack.top() == 42, "stack_by_queue",
          "弹空后可继续使用");

    // 与 std::stack 做差分：大量随机 push/pop/top 后结果必须一致
    {
        dsaac::StackByQueue<int> mine;
        std::stack<int> ref;
        unsigned state = 12345u;
        for (int k = 0; k < 200; ++k) {
            state = state * 1103515245u + 12345u;
            const unsigned action = (state >> 16) % 3;
            if (action == 0 || ref.empty()) {
                const int value = static_cast<int>((state >> 8) % 100);
                mine.push(value);
                ref.push(value);
            } else if (action == 1) {
                mine.pop();
                ref.pop();
            } else {
                if (mine.top() != ref.top()) {
                    check(false, "stack_by_queue", "与 std::stack 差分一致");
                    break;
                }
            }
            if (mine.size() != ref.size()) {
                check(false, "stack_by_queue", "与 std::stack 的 size 一致");
                break;
            }
        }
        check(true, "stack_by_queue", "差分测试完成");
    }

    // toVector 按栈底 -> 栈顶
    {
        dsaac::StackByQueue<int> s;
        for (int i = 1; i <= 4; ++i) {
            s.push(i);
        }
        check(s.toVector() == std::vector<int>{1, 2, 3, 4}, "stack_by_queue",
              "toVector 栈底到栈顶");
        std::ostringstream os;
        s.printTopToBottom(os);
        check(os.str() == "4 3 2 1", "stack_by_queue", "printTopToBottom");
    }
}

void checkQueueByStack() {
    dsaac::QueueByStack<int> queue;
    check(queue.empty() && queue.size() == 0, "queue_by_stack", "初始为空");
    checkThrowsOutOfRange([&] { queue.pop(); }, "queue_by_stack", "空队列 pop");
    checkThrowsOutOfRange([&] { queue.front(); }, "queue_by_stack", "空队列 front");

    for (int i = 1; i <= 5; ++i) {
        const std::size_t before = queue.size();
        queue.push(i);
        check(queue.size() == before + 1, "queue_by_stack", "push 后 size 加一");
    }
    check(queue.size() == 5, "queue_by_stack", "size 正确");
    check(queue.front() == 1, "queue_by_stack", "队首是最早入队的元素");

    // 先进先出，且每出一个 size 减一
    for (int i = 1; i <= 5; ++i) {
        const std::size_t before = queue.size();
        check(queue.front() == i, "queue_by_stack", "先进先出顺序");
        queue.pop();
        check(queue.size() + 1 == before, "queue_by_stack", "pop 后 size 减一");
    }
    check(queue.empty(), "queue_by_stack", "全部出队后为空");

    // 交错的 push/pop：in_/out_ 之间来回倒
    {
        for (int round = 0; round < 50; ++round) {
            queue.push(round);
            if (round % 2 == 1) {
                check(queue.front() == round / 2, "queue_by_stack",
                      "交错出入队顺序");
                queue.pop();
            }
        }
        check(queue.size() == 25, "queue_by_stack", "交错操作后 size 正确");
    }

    // 与 std::queue 做差分
    {
        dsaac::QueueByStack<int> mine;
        std::queue<int> ref;
        unsigned state = 54321u;
        for (int k = 0; k < 200; ++k) {
            state = state * 1103515245u + 12345u;
            const unsigned action = (state >> 16) % 3;
            if (action == 0 || ref.empty()) {
                const int value = static_cast<int>((state >> 8) % 100);
                mine.push(value);
                ref.push(value);
            } else if (action == 1) {
                mine.pop();
                ref.pop();
            } else {
                if (mine.front() != ref.front()) {
                    check(false, "queue_by_stack", "与 std::queue 差分一致");
                    break;
                }
            }
        }
        check(mine.size() == ref.size(), "queue_by_stack", "与 std::queue 的 size 一致");
    }

    // toVector 按队首 -> 队尾
    {
        dsaac::QueueByStack<int> q;
        q.push(1);
        q.push(2);
        q.push(3);
        q.pop();
        q.push(4);
        check(q.toVector() == std::vector<int>{2, 3, 4}, "queue_by_stack",
              "toVector 队首到队尾");
    }
}

void checkVectorConversions() {
    // Queue -> Stack：反转
    {
        dsaac::VectorQueue<int> queue;
        for (int i = 1; i <= 4; ++i) {
            queue.push(i);
        }
        const dsaac::VectorStack<int> stack = queue.toStack();
        check(stack.size() == 4, "convert", "toStack 元素个数");
        check(stack.top() == 4, "convert", "toStack 后栈顶是原队尾");
        std::ostringstream os;
        stack.printTopToBottom(os);
        check(os.str() == "4 3 2 1", "convert", "QUEUE_TO_STACK 输出为反转");
    }

    // Stack -> Queue：反转
    {
        dsaac::VectorStack<int> stack;
        for (int i = 1; i <= 4; ++i) {
            stack.push(i);
        }
        const dsaac::VectorQueue<int> queue = stack.toQueue();
        check(queue.size() == 4, "convert", "toQueue 元素个数");
        check(queue.front() == 4, "convert", "toQueue 后队首是原栈顶");
        std::ostringstream os;
        queue.printFrontToBack(os);
        check(os.str() == "4 3 2 1", "convert", "STACK_TO_QUEUE 输出为反转");
    }

    // toStack 保持自然顺序（栈底 -> 栈顶 == 队首 -> 队尾），
    // toQueue 则把自然顺序倒过来（队首 -> 队尾 == 栈顶 -> 栈底）。
    // 所以连续两次转换的净效果是一次反转——这与“两个方向的输出行
    // 都是输入行逆序”的规定一致：第一次反转体现在栈的打印方向上，
    // 第二次反转体现在队列本身的顺序上。
    {
        dsaac::VectorQueue<int> original;
        for (int i = 1; i <= 5; ++i) {
            original.push(i);
        }
        const dsaac::VectorQueue<int> roundTrip = original.toStack().toQueue();
        check(roundTrip.toVector() == std::vector<int>{5, 4, 3, 2, 1}, "convert",
              "两次转换的净效果是一次反转");
    }

    // 空结构转换
    {
        dsaac::VectorQueue<int> emptyQueue;
        check(emptyQueue.toStack().empty(), "convert", "空队列转栈仍为空");
        dsaac::VectorStack<int> emptyStack;
        check(emptyStack.toQueue().empty(), "convert", "空栈转队列仍为空");
    }

    // 单元素转换：反转与否看不出区别，专门钉住 size 与取值
    {
        dsaac::VectorQueue<int> queue;
        queue.push(42);
        const dsaac::VectorStack<int> stack = queue.toStack();
        check(stack.size() == 1 && stack.top() == 42, "convert",
              "单元素队列转栈");

        dsaac::VectorStack<int> s;
        s.push(42);
        const dsaac::VectorQueue<int> q = s.toQueue();
        check(q.size() == 1 && q.front() == 42, "convert", "单元素栈转队列");
    }

    // 两个方向都用 string，确认转换不依赖算术类型
    {
        dsaac::VectorQueue<std::string> queue;
        for (const char* word : {"a", "bb", "ccc"}) {
            queue.push(word);
        }
        std::ostringstream os;
        queue.toStack().printTopToBottom(os);
        check(os.str() == "ccc bb a", "convert", "string 队列转栈");

        dsaac::VectorStack<std::string> stack;
        for (const char* word : {"a", "bb", "ccc"}) {
            stack.push(word);
        }
        std::ostringstream os2;
        stack.toQueue().printFrontToBack(os2);
        check(os2.str() == "ccc bb a", "convert", "string 栈转队列");
    }

    // VectorQueue 的 front_ 压缩：大量 pop 后仍保持正确
    {
        dsaac::VectorQueue<int> queue;
        std::vector<int> expected;
        for (int i = 0; i < 100; ++i) {
            queue.push(i);
            expected.push_back(i);
        }
        std::size_t head = 0;
        for (int i = 0; i < 100; ++i) {
            if (queue.front() != expected[head]) {
                check(false, "vector_queue", "大量 pop 后队首正确");
                break;
            }
            queue.pop();
            ++head;
        }
        check(queue.empty(), "vector_queue", "全部出队后为空");
        queue.push(7);
        check(queue.front() == 7 && queue.size() == 1, "vector_queue",
              "弹空后可继续使用");
    }
}

}  // namespace

int main() {
    {
        dsaac::ArrayStack<int> stack(8);
        checkCommon(stack, [] { return dsaac::ArrayStack<int>(8); }, "array");
    }
    {
        dsaac::LinkedStack<int> stack;
        checkCommon(stack, [] { return dsaac::LinkedStack<int>(); }, "linked");
    }
    checkArraySpecific();
    checkLinkedSpecific();
    checkStringSupport();

    {
        dsaac::CircularQueue<int> queue(8);
        checkQueueCommon(queue, [] { return dsaac::CircularQueue<int>(8); },
                         "circular");
    }
    {
        dsaac::ShiftQueue<int> queue(8);
        checkQueueCommon(queue, [] { return dsaac::ShiftQueue<int>(8); }, "shift");
    }
    {
        dsaac::LinkedQueue<int> queue;
        checkQueueCommon(queue, [] { return dsaac::LinkedQueue<int>(); },
                         "linked_queue");
    }
    checkCircularSpecific();
    checkShiftSpecific();
    checkLinkedQueueSpecific();
    checkQueueStringSupport();

    checkStackByQueue();
    checkQueueByStack();
    checkVectorConversions();

    std::cout << "通过 " << g_passed << " 项，失败 " << g_failed << " 项\n";
    return g_failed == 0 ? 0 : 1;
}