// list_advanced.cpp
// 任务 3 的三个经典链表算法实现。
//
// 共同约束：O(n) 时间、O(1) 额外空间；不使用哈希表、数组、标记位，
// 也不修改节点的值（值可能重复，用值做标记不可靠）。

#include "list_advanced.h"

namespace mylab {
namespace adv {

std::size_t length(const Node *head) {
    std::size_t n = 0;
    for (const Node *cur = head; cur != nullptr; cur = cur->next) {
        ++n;
    }
    return n;
}

Node *detectAndRemoveCycle(Node *head) {
    // 复杂度：O(n) 时间，O(1) 额外空间。
    //
    // 第一阶段：快慢指针。若无环，快指针先到达 nullptr；若有环，两者必在环内相遇。
    Node *slow = head;
    Node *fast = head;
    while (fast != nullptr && fast->next != nullptr) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            break;
        }
    }
    if (fast == nullptr || fast->next == nullptr) {
        return nullptr;  // 无环
    }

    // 第二阶段：设头到入口的距离为 a，入口到相遇点为 b，环长为 L。
    // 相遇时慢指针走了 a + b，快指针走了 a + b + kL，而快是慢的两倍，
    // 于是 a + b = kL，即 a = kL - b。也就是说：从相遇点继续走 a 步，
    // 与从头节点走 a 步，会同时到达入口节点。
    Node *entry = head;
    Node *meet = slow;
    while (entry != meet) {
        entry = entry->next;
        meet = meet->next;
    }

    // 第三阶段：解环。入口节点可能恰好是头节点（整条链表成环），
    // 此时没有“入口之前的节点”，需要沿环走一圈找到它真正的尾节点。
    //
    // 入口是头节点时，环覆盖了从 head 开始的全部节点；沿环从入口往后走
    // 直到 next 重新指回入口，最后停下的就是尾节点。
    Node *tail = entry;
    while (tail->next != entry) {
        tail = tail->next;
    }
    tail->next = nullptr;
    return entry;
}

Node *getIntersectionNode(Node *a, Node *b) {
    // 复杂度：O(n + m) 时间，O(1) 额外空间。
    //
    // 两条链表在交汇点之后共享同一批节点，所以公共部分长度相同。
    // 让较长的链表先走长度差，之后两者到交汇点的距离相等，同步前进即可相遇。
    const std::size_t lenA = length(a);
    const std::size_t lenB = length(b);

    Node *pa = a;
    Node *pb = b;
    if (lenA > lenB) {
        for (std::size_t k = 0; k < lenA - lenB; ++k) {
            pa = pa->next;
        }
    } else {
        for (std::size_t k = 0; k < lenB - lenA; ++k) {
            pb = pb->next;
        }
    }

    while (pa != pb) {
        pa = pa->next;
        pb = pb->next;
    }
    return pa;  // 不相交时两者同时走到 nullptr，返回 nullptr
}

Heads reverseSharedPath(Node *a, Node *b, Node *intersection) {
    // 复杂度：O(n + m) 时间，O(1) 额外空间。
    //
    // 思路分三步：
    //   1. 从交汇点开始就地反转公共段，得到新的公共段头部 newShared；
    //   2. 记住两条链各自“交汇点的前驱”；
    //   3. 把各自前缀的尾部接到新头部，再把新的公共段尾部接到 nullptr。
    // 注意公共段只反转一次，因此最终两条链表共享的是同一批（已被反转的）节点。
    if (intersection == nullptr) {
        return Heads{a, b};
    }

    // 记录 a、b 中交汇点的前驱；没有前缀时为 nullptr。
    Node *prevA = nullptr;
    for (Node *cur = a; cur != nullptr && cur != intersection; cur = cur->next) {
        prevA = cur;
    }
    Node *prevB = nullptr;
    for (Node *cur = b; cur != nullptr && cur != intersection; cur = cur->next) {
        prevB = cur;
    }

    // 反转公共段：intersection -> ... -> tail  变成  tail -> ... -> intersection。
    Node *prev = nullptr;
    Node *cur = intersection;
    Node *newTail = intersection;  // 反转后，原交汇点成为公共段的新尾巴
    while (cur != nullptr) {
        Node *next = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next;
    }
    Node *newSharedHead = prev;
    newTail->next = nullptr;

    // 前缀接上新的公共段；没有前缀时链表头就是公共段的新头。
    if (prevA != nullptr) {
        prevA->next = newSharedHead;
    }
    if (prevB != nullptr) {
        prevB->next = newSharedHead;
    }

    Heads heads;
    heads.a = prevA != nullptr ? a : newSharedHead;
    heads.b = prevB != nullptr ? b : newSharedHead;
    return heads;
}

bool printList(std::ostream &os, const Node *head, std::size_t expectedNodes) {
    if (head == nullptr || expectedNodes == 0) {
        os << "EMPTY";
        return head == nullptr;
    }
    const Node *cur = head;
    std::size_t printed = 0;
    while (cur != nullptr && printed < expectedNodes) {
        if (printed != 0) {
            os << ' ';
        }
        os << cur->val;
        cur = cur->next;
        ++printed;
    }
    // 预期 n 个节点，却提前遇到 nullptr → 链表比预期短。
    return printed == expectedNodes;
}

}  // namespace adv
}  // namespace mylab