// main_task3.cpp
// 任务 3 的驱动程序：读取若干场景，依次执行三个经典链表算法并按要求输出。
//
// 输入格式：
//   第 1 行      T                场景数
//   每个场景：
//     CYCLE                         n entry / 下一行 n 个值
//                                    （entry 是尾节点指向的下标，-1 表示无环）
//     NO_INTERSECTION               nA + nA 个值；nB + nB 个值
//     INTERSECTION                  nA + A 独有前缀；nB + B 独有前缀；k + 公共尾部
//
// 输出格式：
//   每个场景先输出 CASE t（t 从 1 开始），再输出该场景的结果。
//
// 内存：每个场景构造的所有节点都登记在 NodePool 中，场景处理完后统一释放。
// 这一点在链表被解环、被反转之后尤其重要——此时沿着 next 指针遍历不再能保证
// 覆盖全部节点（公共段可能被两条链表共享），只有登记表能给出“构造过哪些节点”
// 的完整答案，从而保证每个节点恰好释放一次。

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include "input_util.h"
#include "list_advanced.h"

namespace {

using mylab::adv::Node;
using mylab::io::Cursor;
using mylab::io::readIndex;
using mylab::io::readValue;

bool gOk = true;

// 读取一个允许为 -1 的下标（CYCLE 的 entry）。
bool readSignedIndex(Cursor &cur, long long &out, const char *what, std::size_t step) {
    const std::string &token = cur.next();
    if (cur.error()) {
        std::cerr << "输入错误：第 " << step << " 个场景缺少" << what << '\n';
        return false;
    }
    std::size_t used = 0;
    try {
        out = std::stoll(token, &used);
    } catch (const std::exception &) {
        used = 0;
    }
    if (used != token.size()) {
        std::cerr << "输入错误：第 " << step << " 个场景的" << what << " \"" << token
                  << "\" 不是整数\n";
        return false;
    }
    return true;
}

// 一个场景构造出来的全部节点。析构时逐个 delete，保证不泄漏、也不重复释放。
class NodePool {
public:
    NodePool() = default;
    NodePool(const NodePool &) = delete;
    NodePool &operator=(const NodePool &) = delete;

    ~NodePool() {
        for (Node *node : nodes_) {
            delete node;
        }
    }

    // 读取 count 个值，new 出节点串成一条链，返回头节点；count = 0 时返回 nullptr。
    // 失败时返回 nullptr 并把 gOk 置 false。
    Node *buildChain(Cursor &cur, std::size_t count, std::size_t step) {
        Node *head = nullptr;
        Node *tail = nullptr;
        for (std::size_t k = 0; k < count; ++k) {
            int value = 0;
            if (!readValue(cur, "链表元素", step, value)) {
                gOk = false;
                return nullptr;
            }
            Node *node = new Node(value);
            nodes_.push_back(node);
            if (head == nullptr) {
                head = node;
            } else {
                tail->next = node;
            }
            tail = node;
        }
        return head;
    }

    // 取出下标为 index 的节点（按构造顺序），用于 CYCLE 场景结环。
    Node *at(std::size_t index) const { return nodes_[index]; }

private:
    std::vector<Node *> nodes_;
};

// 把链表的尾节点接到 target 上；链为空时直接返回 target。
Node *appendChain(Node *chainHead, Node *target) {
    if (chainHead == nullptr) {
        return target;
    }
    Node *last = chainHead;
    while (last->next != nullptr) {
        last = last->next;
    }
    last->next = target;
    return chainHead;
}

// CYCLE：n entry + n 个值。entry = -1 表示无环，否则尾节点指向下标 entry 的节点。
void runCycle(Cursor &cur, std::size_t caseIndex, std::size_t step, std::ostream &out) {
    const std::size_t n = [&] {
        std::size_t value = 0;
        if (!readIndex(cur, value, "CYCLE 的 n", step)) {
            gOk = false;
        }
        return value;
    }();
    long long entryIndex = -1;
    if (!readSignedIndex(cur, entryIndex, "entry", step)) {
        gOk = false;
    }
    if (!gOk) {
        return;
    }
    if (entryIndex >= 0 && static_cast<std::size_t>(entryIndex) >= n) {
        std::cerr << "输入错误：第 " << step << " 个场景的 entry = " << entryIndex
                  << " 超出节点总数 " << n << '\n';
        gOk = false;
        return;
    }

    NodePool pool;
    Node *head = pool.buildChain(cur, n, step);
    if (!gOk) {
        return;
    }

    // 结环：尾节点指向下标 entry 的节点。n = 0 时没有节点，天然无环。
    if (entryIndex >= 0 && n > 0) {
        Node *tail = pool.at(n - 1);
        tail->next = pool.at(static_cast<std::size_t>(entryIndex));
    }

    out << "CASE " << caseIndex << '\n';
    Node *entry = mylab::adv::detectAndRemoveCycle(head);
    if (entry == nullptr) {
        out << "CYCLE_ENTRY NONE\n";
    } else {
        out << "CYCLE_ENTRY " << entry->val << '\n';
    }
    out << "LIST ";
    mylab::adv::printList(out, head, n);
    out << '\n';
}

// NO_INTERSECTION：nA + nA 个值，nB + nB 个值。两条链表互不相交。
void runNoIntersection(Cursor &cur, std::size_t caseIndex, std::size_t step,
                       std::ostream &out) {
    const std::size_t nA = [&] {
        std::size_t value = 0;
        if (!readIndex(cur, value, "NO_INTERSECTION 的 nA", step)) {
            gOk = false;
        }
        return value;
    }();
    NodePool poolA;
    Node *a = poolA.buildChain(cur, nA, step);
    if (!gOk) {
        return;
    }
    const std::size_t nB = [&] {
        std::size_t value = 0;
        if (!readIndex(cur, value, "NO_INTERSECTION 的 nB", step)) {
            gOk = false;
        }
        return value;
    }();
    NodePool poolB;
    Node *b = poolB.buildChain(cur, nB, step);
    if (!gOk) {
        return;
    }

    out << "CASE " << caseIndex << '\n';
    Node *common = mylab::adv::getIntersectionNode(a, b);
    if (common == nullptr) {
        out << "INTERSECTION NONE\n";
    } else {
        // 场景声明为“不相交”却找到了公共节点，说明构造数据有问题；
        // 仍然如实输出算法结果，便于发现用例错误。
        out << "INTERSECTION " << common->val << '\n';
        const mylab::adv::Heads heads = mylab::adv::reverseSharedPath(a, b, common);
        out << "LIST_A ";
        mylab::adv::printList(out, heads.a, nA);
        out << "\nLIST_B ";
        mylab::adv::printList(out, heads.b, nB);
        out << '\n';
    }
}

// INTERSECTION：nA + A 独有前缀；nB + B 独有前缀；k + 公共尾部。
// 公共尾部只 new 一次，两条链表的尾部指针指向同一批节点。
void runIntersection(Cursor &cur, std::size_t caseIndex, std::size_t step,
                     std::ostream &out) {
    const std::size_t nA = [&] {
        std::size_t value = 0;
        if (!readIndex(cur, value, "INTERSECTION 的 nA", step)) {
            gOk = false;
        }
        return value;
    }();
    if (!gOk) {
        return;
    }
    // 注意：前缀与公共尾部必须放在同一个池子里按构造顺序登记，
    // 这样最终链条的节点顺序与下标才对得上。
    NodePool pool;

    // 先按输入顺序把 A 前缀、B 前缀、公共尾部各自建好，再拼接。
    // A 前缀
    Node *aPrefix = pool.buildChain(cur, nA, step);
    if (!gOk) {
        return;
    }
    const std::size_t nB = [&] {
        std::size_t value = 0;
        if (!readIndex(cur, value, "INTERSECTION 的 nB", step)) {
            gOk = false;
        }
        return value;
    }();
    if (!gOk) {
        return;
    }
    Node *bPrefix = pool.buildChain(cur, nB, step);
    if (!gOk) {
        return;
    }

    const std::size_t k = [&] {
        std::size_t value = 0;
        if (!readIndex(cur, value, "INTERSECTION 的 k", step)) {
            gOk = false;
        }
        return value;
    }();
    if (!gOk) {
        return;
    }
    Node *shared = pool.buildChain(cur, k, step);
    if (!gOk) {
        return;
    }

    // 拼接：两条链表共用同一批公共节点。
    Node *a = appendChain(aPrefix, shared);
    Node *b = appendChain(bPrefix, shared);

    out << "CASE " << caseIndex << '\n';
    Node *common = mylab::adv::getIntersectionNode(a, b);
    if (common == nullptr) {
        out << "INTERSECTION NONE\n";
        return;
    }
    out << "INTERSECTION " << common->val << '\n';
    const mylab::adv::Heads heads = mylab::adv::reverseSharedPath(a, b, common);
    out << "LIST_A ";
    mylab::adv::printList(out, heads.a, nA + k);
    out << "\nLIST_B ";
    mylab::adv::printList(out, heads.b, nB + k);
    out << '\n';
}

}  // namespace

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    const std::vector<std::string> tokens = mylab::io::readAllTokens();
    Cursor cur(tokens);

    std::size_t t = 0;
    if (!readIndex(cur, t, "场景数 T")) {
        return 1;
    }

    for (std::size_t step = 1; step <= t; ++step) {
        const std::string &type = cur.next();
        if (cur.error()) {
            std::cerr << "输入错误：读取第 " << step << " 个场景类型时意外结束\n";
            return 1;
        }
        if (type == "CYCLE") {
            runCycle(cur, step, step, std::cout);
        } else if (type == "NO_INTERSECTION") {
            runNoIntersection(cur, step, step, std::cout);
        } else if (type == "INTERSECTION") {
            runIntersection(cur, step, step, std::cout);
        } else {
            std::cerr << "输入错误：第 " << step << " 个场景类型未知：" << type << '\n';
            return 1;
        }
        if (!gOk) {
            return 1;
        }
    }
    return 0;
}