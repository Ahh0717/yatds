// list_advanced.h
// 任务 3：三个经典链表算法的实现接口。
//
// 三个算法都要求 O(n) 时间、O(1) 额外空间，不允许使用哈希表、数组或标记位，
// 因此全部只靠指针移动完成。
//
// 关于“相交”的定义：两条链表相交，是指它们**共享同一批节点对象**
// （同一内存地址），而不是节点里的值相等。返回的也是节点指针本身，
// 调用方可以通过指针比较来验证。

#ifndef MYLAB_LIST_ADVANCED_H
#define MYLAB_LIST_ADVANCED_H

#include <cstddef>
#include <ostream>

namespace mylab {
namespace adv {

// 单链表节点。任务 3 的题目只涉及整数，这里不模板化，保持算法实现直观。
struct Node {
    int val;
    Node *next;
    explicit Node(int v, Node *n = nullptr) : val(v), next(n) {}
};

// 求链表长度（不含环的情况下）；用于 getIntersectionNode 的长度对齐。
std::size_t length(const Node *head);

// 判断链表是否有环；有环时找到入口节点，把“入口的前驱”的 next 置空以解环，
// 返回入口节点；无环返回 nullptr。
//
// 复杂度：O(n) 时间，O(1) 额外空间。快慢指针相遇后再从头同步走，相遇处即入口。
Node *detectAndRemoveCycle(Node *head);

// 返回两条无环链表的第一个公共节点；不相交返回 nullptr。
// 前置条件：a 与 b 都没有环（调用 detectAndRemoveCycle 之后即可满足）。
//
// 复杂度：O(n + m) 时间，O(1) 额外空间。先求长度差，让长链先走差值再同步前进。
Node *getIntersectionNode(Node *a, Node *b);

// 反转两条链表的公共部分后，两条链表各自的新头节点。
struct Heads {
    Node *a;
    Node *b;
};

// 只反转“交汇点及其之后”的公共部分，各自独立的前缀保持原顺序与内容。
// intersection 为 nullptr 时原样返回。
//
// 复杂度：O(n + m) 时间，O(1) 额外空间。公共段就地反转一次，两条链表的
// 前缀随后各自接到新的公共段头部。
Heads reverseSharedPath(Node *a, Node *b, Node *intersection);

// 打印链表：元素以单个空格分隔，空链表输出 EMPTY（不带换行）。
// expectedNodes 是调用方已知的节点数，用作遍历上限——这样即使链表仍带环，
// 打印也不会死循环。返回值表示实际走到的节点数是否与预期一致。
bool printList(std::ostream &os, const Node *head, std::size_t expectedNodes);

}  // namespace adv
}  // namespace mylab

#endif  // MYLAB_LIST_ADVANCED_H