// invariants.cpp
// 算法级不变量检查（I/O 样例覆盖不到的那部分）：
//   1. LinkedList：下标语义、tail_ 维护、删空后复用、深拷贝；
//   2. DynamicArray：容量翻倍、clear 不缩容、自引用插入、拷贝/移动；
//   3. list_advanced：用指针相等断言三个算法返回的是同一节点对象，验证
//      自环/整链成环/普通环的解环、公共段共享、零前缀反转等性质。
//
// 样例的逐行比对由 tools/run_tests.py（make test）负责，本程序只做单元检查。
//
// 用法：./build/invariants

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "dynamic_array.h"
#include "linked_list.h"
#include "list_advanced.h"

#include <initializer_list>

namespace {

int g_passed = 0;
int g_failed = 0;

void check(bool ok, const std::string &name, const std::string &detail = "") {
    if (ok) {
        ++g_passed;
        std::cout << "  [ok]   " << name << '\n';
    } else {
        ++g_failed;
        std::cout << "  [FAIL] " << name;
        if (!detail.empty()) {
            std::cout << "  (" << detail << ')';
        }
        std::cout << '\n';
    }
}

template <typename Container>
std::string dump(const Container &c) {
    std::ostringstream os;
    c.print(os);
    return os.str();
}

// ===================== 任务 1：链表不变量 =====================

void testList() {
    std::cout << "== 任务 1：LinkedList 不变量 ==\n";
    mylab::LinkedList<int> list;

    for (int x : {1, 3, 5}) {
        list.pushBack(x);
    }
    check(dump(list) == "1 3 5", "pushBack 保持顺序", dump(list));
    list.pushFront(0);
    check(dump(list) == "0 1 3 5", "pushFront 插到头部", dump(list));

    // 关键语义：INSERT i x 是“插到下标 i 处”。
    check(list.insertAt(2, 2), "insertAt 返回 true");
    check(dump(list) == "0 1 2 3 5", "insertAt 插在下标 i 处", dump(list));

    check(list.insertAt(list.size(), 6), "insertAt(size) 等价 pushBack");
    check(dump(list) == "0 1 2 3 5 6", "尾部插入结果正确", dump(list));

    const std::string snapshot = dump(list);
    check(!list.insertAt(list.size() + 1, 9), "insertAt 越界返回 false");
    check(!list.insertAt(999, 9), "insertAt 远越界返回 false");
    check(dump(list) == snapshot && list.size() == 6, "越界插入不修改链表");

    mylab::LinkedList<int> emptyList;
    check(emptyList.insertAt(0, 42), "空链表 insertAt(0) 成功");
    check(dump(emptyList) == "42", "空链表成为单元素链表", dump(emptyList));

    check(list.eraseAt(5), "eraseAt 尾元素返回 true");
    check(dump(list) == "0 1 2 3 5", "删除尾元素后内容正确", dump(list));
    list.pushBack(7);
    check(dump(list) == "0 1 2 3 5 7", "删尾后 pushBack 仍接在尾部", dump(list));

    check(list.eraseAt(0), "eraseAt 头元素返回 true");
    check(dump(list) == "1 2 3 5 7", "删除头元素后内容正确", dump(list));
    list.pushFront(-1);
    check(dump(list) == "-1 1 2 3 5 7", "删头后 pushFront 仍正确", dump(list));

    const std::size_t before = list.size();
    check(!list.eraseAt(before), "eraseAt(size) 越界返回 false");
    check(!list.eraseAt(999), "eraseAt 远越界返回 false");
    check(list.size() == before, "越界删除不改变 size");

    int value = 0;
    check(list.access(1, value) && value == 1, "access 读出下标 1 的元素");
    check(!list.access(before, value), "access 越界返回 false");
    check(list.find(3) == 3, "find 返回首个匹配下标", std::to_string(list.find(3)));
    check(list.find(100) == -1, "find 不存在返回 -1");
    check(list.contains(7) && !list.contains(100), "contains 正确");

    mylab::LinkedList<int> drain;
    for (int i = 0; i < 5; ++i) {
        drain.pushBack(i);
    }
    for (int i = 4; i >= 0; --i) {
        drain.eraseAt(static_cast<std::size_t>(i));
    }
    check(drain.empty() && dump(drain) == "EMPTY", "逐个删除后为空且打印 EMPTY");
    drain.pushBack(99);
    check(dump(drain) == "99", "删空后再 pushBack 正确", dump(drain));

    drain.clear();
    check(drain.empty(), "clear 后为空");
    drain.pushFront(8);
    check(dump(drain) == "8", "clear 后仍可 pushFront", dump(drain));

    mylab::LinkedList<int> dup;
    for (int x : {4, 9, 4, 4}) {
        dup.pushBack(x);
    }
    check(dup.find(4) == 0, "有重复值时 find 取第一个", std::to_string(dup.find(4)));

    mylab::LinkedList<int> copy(dup);
    copy.pushBack(1);
    check(dump(dup) == "4 9 4 4" && dump(copy) == "4 9 4 4 1", "深拷贝后互相独立");
    mylab::LinkedList<int> &self = copy;
    copy = self;
    check(dump(copy) == "4 9 4 4 1", "自赋值后内容不变", dump(copy));

    mylab::LinkedList<int> q;
    for (int x : {1, 2, 3, 4}) {
        q.pushBack(x);
    }
    q.reverse();
    check(dump(q) == "4 3 2 1", "reverse 结果正确", dump(q));
    q.pushBack(0);
    check(dump(q) == "4 3 2 1 0", "reverse 后 pushBack 仍正确", dump(q));
    check(!q.hasCycle() && q.cycleEntry() == -1, "无环链表判定正确");
}

// ===================== 任务 2：动态数组不变量 =====================

void testDynamicArray() {
    std::cout << "== 任务 2：DynamicArray 不变量 ==\n";

    // 空数组也要有非零容量，否则后续扩容会除零/死循环。
    {
        mylab::DynamicArray<int> a;
        check(a.capacity() >= mylab::DynamicArray<int>::kMinCapacity,
              "默认构造的容量不低于最小容量",
              std::to_string(a.capacity()));
        a.pushBack(1);
        check(dump(a) == "1", "默认构造后 pushBack 正确", dump(a));
    }

    // 初始容量与扩容：4 -> 8 -> 16。
    {
        mylab::DynamicArray<int> a(4);
        check(a.capacity() == 4 && a.size() == 0, "构造时容量为 4");
        for (int i = 0; i < 4; ++i) {
            a.pushBack(i);
        }
        check(a.capacity() == 4 && a.size() == 4, "装满 4 个时容量不变");
        a.pushBack(4);
        check(a.capacity() == 8 && a.size() == 5, "第 5 个触发翻倍到 8",
              std::to_string(a.capacity()));
        for (int i = 5; i < 8; ++i) {
            a.pushBack(i);
        }
        check(a.capacity() == 8, "再次装满 8 个时容量仍为 8");
        a.pushBack(8);
        check(a.capacity() == 16, "第 9 个触发翻倍到 16", std::to_string(a.capacity()));
        check(dump(a) == "0 1 2 3 4 5 6 7 8", "扩容后元素顺序完整", dump(a));
    }

    // 初始容量小于 n 时必须先扩容再装入。
    {
        mylab::DynamicArray<int> a(1);
        for (int i = 0; i < 10; ++i) {
            a.pushBack(i);
        }
        check(a.capacity() >= 10 && a.size() == 10, "初始容量过小也能装下",
              "cap=" + std::to_string(a.capacity()));
        check(dump(a) == "0 1 2 3 4 5 6 7 8 9", "装下后元素完整", dump(a));
    }

    // INSERT 语义与插入时的扩容。
    {
        mylab::DynamicArray<int> a(4);
        for (int x : {10, 20, 30}) {
            a.pushBack(x);
        }
        check(a.insertAt(1, 15), "insertAt 返回 true");
        check(dump(a) == "10 15 20 30", "insertAt 插在下标 i 处", dump(a));
        check(a.insertAt(a.size(), 40), "insertAt(size) 等价 pushBack");
        check(a.size() == 5 && a.capacity() == 8, "插入触发扩容到 8",
              "cap=" + std::to_string(a.capacity()));
        check(dump(a) == "10 15 20 30 40", "插入后顺序正确", dump(a));

        const std::string snapshot = dump(a);
        const std::size_t cap = a.capacity();
        check(!a.insertAt(a.size() + 1, 1), "insertAt 越界返回 false");
        check(!a.insertAt(999, 1), "insertAt 远越界返回 false");
        check(dump(a) == snapshot && a.size() == 5 && a.capacity() == cap,
              "越界插入不修改数组也不扩容");
    }

    // ERASE、GET、SET 的边界行为。
    {
        mylab::DynamicArray<int> a(4);
        for (int x : {1, 2, 3}) {
            a.pushBack(x);
        }
        check(a.eraseAt(1), "eraseAt 返回 true");
        check(dump(a) == "1 3", "eraseAt 结果正确", dump(a));
        check(!a.eraseAt(2), "eraseAt(size) 越界返回 false");
        check(!a.eraseAt(999), "eraseAt 远越界返回 false");
        check(dump(a) == "1 3" && a.size() == 2, "越界删除不修改数组");

        int v = 0;
        check(a.get(1, v) && v == 3, "get 读出下标 1 的元素");
        check(!a.get(2, v), "get 越界返回 false");
        check(a.set(0, 9) && dump(a) == "9 3", "set 修改指定下标", dump(a));
        check(!a.set(2, 0), "set 越界返回 false");

        check(a.front() == 9 && a.back() == 3, "front / back 正确");
        check(a.at(1) == 3, "at 越界以外可用");
        bool threw = false;
        try {
            a.at(5);
        } catch (const std::out_of_range &) {
            threw = true;
        }
        check(threw, "at 越界抛 std::out_of_range");
    }

    // CLEAR 只清元素、不缩容；之后 pushBack 不再扩容。
    {
        mylab::DynamicArray<int> a(4);
        for (int i = 0; i < 9; ++i) {
            a.pushBack(i);
        }
        const std::size_t cap = a.capacity();
        a.clear();
        check(a.empty() && a.size() == 0 && dump(a) == "EMPTY",
              "clear 后为空且打印 EMPTY");
        check(a.capacity() == cap, "clear 不缩容", "cap=" + std::to_string(a.capacity()));
        a.pushBack(7);
        check(a.capacity() == cap && dump(a) == "7", "clear 后 pushBack 不扩容", dump(a));
    }

    // reserve / resize / popBack / shrinkToFit / emplaceBack。
    {
        mylab::DynamicArray<int> a;
        check(a.reserve(50) == 50 && a.size() == 0, "reserve 提升容量");
        check(a.reserve(10) == 50, "reserve 小于当前容量时不缩容");
        a.resize(3);
        check(a.size() == 3 && dump(a) == "0 0 0", "resize 变大用默认值补齐", dump(a));
        a.resize(1);
        check(a.size() == 1 && dump(a) == "0", "resize 变小截断", dump(a));
        a.shrinkToFit();
        check(a.capacity() == mylab::DynamicArray<int>::kMinCapacity,
              "shrinkToFit 缩到最小容量", std::to_string(a.capacity()));
        a.emplaceBack(5);
        a.emplaceBack(6);
        int v = 0;
        check(a.popBack(v) && v == 6, "popBack 返回并移除尾元素");
        check(dump(a) == "0 5", "popBack 后内容正确", dump(a));
        a.resize(0);
        check(!a.popBack(v), "空数组 popBack 返回 false");
    }

    // 深拷贝与移动。
    {
        mylab::DynamicArray<int> a(4);
        for (int x : {1, 2, 3}) {
            a.pushBack(x);
        }
        mylab::DynamicArray<int> b(a);
        b.pushBack(4);
        check(dump(a) == "1 2 3" && dump(b) == "1 2 3 4", "拷贝后互不影响");
        mylab::DynamicArray<int> c(std::move(b));
        check(dump(c) == "1 2 3 4" && b.empty(), "移动后目标持有数据、源为空");
        c = a;
        check(dump(c) == "1 2 3", "拷贝赋值结果正确");
        mylab::DynamicArray<int> &self = c;
        c = self;
        check(dump(c) == "1 2 3", "自赋值后内容不变", dump(c));
    }

    // 字符串实例：检查元素真的被逐个构造/析构，长字符串不会被截断。
    {
        mylab::DynamicArray<std::string> a(2);
        const std::string long1(200, 'x');
        a.pushBack("apple");
        a.pushBack(long1);
        a.pushBack("cherry");
        check(a.size() == 3 && a.capacity() == 4, "string: 扩容后容量为 4");
        check(a.back() == "cherry" && a.front() == "apple", "string: front/back 正确");
        a.insertAt(1, "banana");
        check(dump(a) == "apple banana " + long1 + " cherry", "string: 插入后内容完整");
        // 自引用：值来自本数组内部，插入/追加时的扩容不能让它悬垂。
        mylab::DynamicArray<std::string> alias(4);
        alias.pushBack("first");
        alias.pushBack("second");
        alias.pushBack("third");
        alias.pushBack("fourth");
        alias.pushBack(alias.at(0));  // 此时容量恰好用满，会触发扩容
        check(alias.at(4) == "first" && alias.at(0) == "first",
              "string: pushBack 自引用在扩容后仍正确", alias.at(4));
        alias.insertAt(0, alias.at(4));
        check(alias.at(0) == "first" && alias.at(5) == "first",
              "string: insertAt 自引用在扩容后仍正确", alias.at(0));
        a.eraseAt(1);
        check(a.at(1) == long1, "string: 长字符串完整保存");
    }
}

// ===================== 任务 3：进阶链表算法 =====================

namespace adv = mylab::adv;

// 用初始值列表 new 一条链，节点登记在 pool 里（场景结束时统一释放）。
std::vector<adv::Node *> makeChain(std::vector<adv::Node *> &pool,
                                   std::initializer_list<int> values) {
    std::vector<adv::Node *> chain;
    for (int v : values) {
        adv::Node *node = new adv::Node(v);
        pool.push_back(node);
        chain.push_back(node);
    }
    for (std::size_t i = 1; i < chain.size(); ++i) {
        chain[i - 1]->next = chain[i];
    }
    return chain;
}

std::string dumpAdv(adv::Node *head, std::size_t expected) {
    std::ostringstream os;
    adv::printList(os, head, expected);
    return os.str();
}

void testCycleDetection() {
    std::cout << "== 任务 3：detectAndRemoveCycle ==\n";

    // null 与单节点无环。
    check(adv::detectAndRemoveCycle(nullptr) == nullptr, "空链表返回 nullptr");

    {
        std::vector<adv::Node *> pool;
        auto chain = makeChain(pool, {7});
        check(adv::detectAndRemoveCycle(chain[0]) == nullptr, "单节点无环返回 nullptr");
        check(chain[0]->next == nullptr, "无环链表未被修改");
        for (auto *n : pool) delete n;
    }

    // 普通环：尾节点回到下标 2。
    {
        std::vector<adv::Node *> pool;
        auto chain = makeChain(pool, {1, 2, 3, 4, 5});
        chain.back()->next = chain[2];
        adv::Node *entry = adv::detectAndRemoveCycle(chain[0]);
        check(entry == chain[2], "普通环入口为该节点本身（指针相同）");
        check(entry->val == 3, "普通环入口值正确");
        check(chain.back()->next == nullptr, "解环后尾节点 next 为 nullptr");
        check(dumpAdv(chain[0], 5) == "1 2 3 4 5", "解环后链表完整", dumpAdv(chain[0], 5));
        check(adv::detectAndRemoveCycle(chain[0]) == nullptr, "再次检测确认已无环");
        for (auto *n : pool) delete n;
    }

    // 整链成环：尾节点回到 head，入口就是头节点，没有“入口之前的节点”。
    {
        std::vector<adv::Node *> pool;
        auto chain = makeChain(pool, {9, 8, 7});
        chain.back()->next = chain[0];
        adv::Node *entry = adv::detectAndRemoveCycle(chain[0]);
        check(entry == chain[0], "整链成环时入口是头节点");
        check(chain[2]->next == nullptr, "整链成环时尾节点被正确断开");
        check(dumpAdv(chain[0], 3) == "9 8 7", "整链成环解环后完整", dumpAdv(chain[0], 3));
        for (auto *n : pool) delete n;
    }

    // 自环。
    {
        std::vector<adv::Node *> pool;
        auto chain = makeChain(pool, {5});
        chain[0]->next = chain[0];
        adv::Node *entry = adv::detectAndRemoveCycle(chain[0]);
        check(entry == chain[0], "自环入口是自身");
        check(chain[0]->next == nullptr, "自环被解开");
        for (auto *n : pool) delete n;
    }

    // 三节点环回到中间。
    {
        std::vector<adv::Node *> pool;
        auto chain = makeChain(pool, {1, 2, 3, 4});
        chain.back()->next = chain[1];
        adv::Node *entry = adv::detectAndRemoveCycle(chain[0]);
        check(entry == chain[1] && entry->val == 2, "环入口为下标 1 的节点");
        check(dumpAdv(chain[0], 4) == "1 2 3 4", "解环后顺序正确", dumpAdv(chain[0], 4));
        for (auto *n : pool) delete n;
    }
}

void testIntersection() {
    std::cout << "== 任务 3：getIntersectionNode ==\n";

    check(adv::getIntersectionNode(nullptr, nullptr) == nullptr,
          "两条空链表返回 nullptr");

    // 值相同但不是同一批节点：必须判定为不相交。
    {
        std::vector<adv::Node *> pool;
        auto a = makeChain(pool, {1, 2, 3});
        auto b = makeChain(pool, {1, 2, 3});
        check(adv::getIntersectionNode(a[0], b[0]) == nullptr,
              "值相同但节点不同 → 不相交");
        for (auto *n : pool) delete n;
    }

    // 等长的公共尾部。
    {
        std::vector<adv::Node *> pool;
        auto a = makeChain(pool, {10, 20});
        auto shared = makeChain(pool, {7, 8, 9});
        a.back()->next = shared[0];
        auto b = makeChain(pool, {30});
        b.back()->next = shared[0];
        adv::Node *common = adv::getIntersectionNode(a[0], b[0]);
        check(common == shared[0], "等长公共尾部时返回首个公共节点");
        check(common->val == 7, "公共节点值正确");
        for (auto *n : pool) delete n;
    }

    // 长度差很大。
    {
        std::vector<adv::Node *> pool;
        auto a = makeChain(pool, {1, 2, 3, 4, 5, 6});
        auto shared = makeChain(pool, {99});
        a.back()->next = shared[0];
        auto b = makeChain(pool, {50});
        b.back()->next = shared[0];
        adv::Node *common = adv::getIntersectionNode(a[0], b[0]);
        check(common == shared[0], "长度差异大时仍能找到公共节点");
        for (auto *n : pool) delete n;
    }

    // 其中一条完全包含在公共段里（前缀长度 0）。
    {
        std::vector<adv::Node *> pool;
        auto a = makeChain(pool, {1, 2});
        auto shared = makeChain(pool, {4, 5, 6});
        a.back()->next = shared[0];
        adv::Node *common = adv::getIntersectionNode(a[0], shared[0]);
        check(common == shared[0], "零前缀时公共节点就是另一条链的头");
        for (auto *n : pool) delete n;
    }
}

void testReverseSharedPath() {
    std::cout << "== 任务 3：reverseSharedPath ==\n";

    // 题目样例：A = 10 20 7 8 9，B = 30 7 8 9。
    {
        std::vector<adv::Node *> pool;
        auto aPrefix = makeChain(pool, {10, 20});
        auto shared = makeChain(pool, {7, 8, 9});
        aPrefix.back()->next = shared[0];
        auto bPrefix = makeChain(pool, {30});
        bPrefix.back()->next = shared[0];

        adv::Node *common = adv::getIntersectionNode(aPrefix[0], bPrefix[0]);
        const adv::Heads heads = adv::reverseSharedPath(aPrefix[0], bPrefix[0], common);
        check(dumpAdv(heads.a, 5) == "10 20 9 8 7", "A 的独立前缀保持原序",
              dumpAdv(heads.a, 5));
        check(dumpAdv(heads.b, 4) == "30 9 8 7", "B 的独立前缀保持原序",
              dumpAdv(heads.b, 4));
        // 公共段仍是同一批对象：两条链表在反转后依然共享。
        adv::Node *pa = heads.a;
        while (pa != nullptr && pa->val != 9) pa = pa->next;
        adv::Node *pb = heads.b;
        while (pb != nullptr && pb->val != 9) pb = pb->next;
        check(pa != nullptr && pa == pb, "反转后两条链表仍共享公共段节点");
        for (auto *n : pool) delete n;
    }

    // 两条链表都从公共段开始：反转后头指针应更新。
    {
        std::vector<adv::Node *> pool;
        auto shared = makeChain(pool, {1, 2, 3});
        adv::Node *common = adv::getIntersectionNode(shared[0], shared[0]);
        const adv::Heads heads = adv::reverseSharedPath(shared[0], shared[0], common);
        check(heads.a == shared[2] && heads.b == shared[2], "零前缀时头节点更新为原尾节点");
        check(dumpAdv(heads.a, 3) == "3 2 1", "整链反转结果正确", dumpAdv(heads.a, 3));
        for (auto *n : pool) delete n;
    }

    // 公共段只有一个节点：反转等于不变。
    {
        std::vector<adv::Node *> pool;
        auto a = makeChain(pool, {1, 2});
        auto b = makeChain(pool, {3});
        adv::Node *common = adv::getIntersectionNode(a[0], b[0]);
        check(common == nullptr, "单节点链不构成相交");
        for (auto *n : pool) delete n;
    }

    // intersection 为 nullptr 时原样返回。
    {
        std::vector<adv::Node *> pool;
        auto a = makeChain(pool, {1, 2});
        auto b = makeChain(pool, {3, 4});
        const adv::Heads heads = adv::reverseSharedPath(a[0], b[0], nullptr);
        check(heads.a == a[0] && heads.b == b[0], "无交汇点时原样返回");
        check(dumpAdv(heads.a, 2) == "1 2" && dumpAdv(heads.b, 2) == "3 4",
              "无交汇点时内容不变");
        for (auto *n : pool) delete n;
    }
}

}  // namespace

int main() {
    testList();
    testDynamicArray();
    testCycleDetection();
    testIntersection();
    testReverseSharedPath();

    std::cout << "\n通过 " << g_passed << " 项，失败 " << g_failed << " 项\n";
    return g_failed == 0 ? 0 : 1;
}