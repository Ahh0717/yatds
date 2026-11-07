#!/usr/bin/env python3
"""任务 3 的独立参考实现（oracle），用于生成 outputs/task3/ 下的期望输出。

用“下标 + 链接表”模拟节点指针：相交用同一批下标表示，因此“共享节点对象”
在参考实现里也是真实共享，而不是靠值相等蒙混过去。

用法：python3 tools/oracle_task3.py inputs/task3/case02.txt
"""

import sys


class LinkedList:
    """用节点下标描述的一条链表：nodes[i] = (值, 后继下标或 None)。"""

    def __init__(self, nodes, head):
        self.nodes = nodes  # 整个场景共享的节点表
        self.head = head

    def values(self):
        """返回从 head 出发（假设无环）的值序列。"""
        out = []
        cur = self.head
        while cur is not None:
            out.append(self.nodes[cur][0])
            cur = self.nodes[cur][1]
        return out


def detect_and_remove_cycle(nodes, head):
    """返回 (入口下标或 None, 新的 head)。有环则把环入口的前驱 next 置空。"""
    slow = fast = head
    while fast is not None and nodes[fast][1] is not None:
        slow = nodes[slow][1]
        fast = nodes[nodes[fast][1]][1]
        if slow == fast:
            break
    else:
        return None, head
    if fast is None or nodes[fast][1] is None:
        return None, head

    entry = head
    meet = slow
    while entry != meet:
        entry = nodes[entry][1]
        meet = nodes[meet][1]

    # 找入口的前驱：从入口出发绕一圈，最后一个能回到入口的节点就是尾节点。
    tail = entry
    while nodes[tail][1] != entry:
        tail = nodes[tail][1]
    nodes[tail] = (nodes[tail][0], None)
    return entry, head


def get_intersection(nodes, a, b):
    """返回第一个公共节点下标；不相交返回 None。"""
    lenA = len(LinkedList(nodes, a).values())
    lenB = len(LinkedList(nodes, b).values())
    pa, pb = a, b
    while lenA > lenB:
        pa = nodes[pa][1]
        lenA -= 1
    while lenB > lenA:
        pb = nodes[pb][1]
        lenB -= 1
    while pa != pb:
        pa = nodes[pa][1]
        pb = nodes[pb][1]
    return pa


def reverse_shared(nodes, a, b, intersection):
    """只反转公共段，返回 (新 headA, 新 headB)。"""
    if intersection is None:
        return a, b

    # 记录各自交汇点的前驱。
    prevA = None
    cur = a
    while cur != intersection:
        prevA = cur
        cur = nodes[cur][1]
    prevB = None
    cur = b
    while cur != intersection:
        prevB = cur
        cur = nodes[cur][1]

    # 反转公共段。
    prev = None
    cur = intersection
    while cur is not None:
        nxt = nodes[cur][1]
        nodes[cur] = (nodes[cur][0], prev)
        prev = cur
        cur = nxt
    new_head = prev

    if prevA is not None:
        nodes[prevA] = (nodes[prevA][0], new_head)
    if prevB is not None:
        nodes[prevB] = (nodes[prevB][0], new_head)
    return (a if prevA is not None else new_head,
            b if prevB is not None else new_head)


def fmt(values):
    return " ".join(str(v) for v in values) if values else "EMPTY"


def main(path):
    with open(path, encoding="utf-8") as f:
        tokens = f.read().split()

    pos = 0

    def take():
        nonlocal pos
        if pos >= len(tokens):
            raise SystemExit(f"{path}: 输入提前结束")
        tok = tokens[pos]
        pos += 1
        return tok

    def take_int():
        return int(take())

    t = take_int()
    out = []

    for case in range(1, t + 1):
        kind = take()
        nodes = []

        def add(value):
            nodes.append([value, None])
            return len(nodes) - 1

        def build_chain(count):
            if count == 0:
                return None
            head = add(take_int())
            last = head
            for _ in range(count - 1):
                nxt = add(take_int())
                nodes[last][1] = nxt
                last = nxt
            return head

        out.append(f"CASE {case}")

        if kind == "CYCLE":
            n, entry = take_int(), take_int()
            head = build_chain(n)
            if entry >= 0 and n > 0:
                nodes[n - 1][1] = entry
            found, head = detect_and_remove_cycle(nodes, head)
            out.append(f"CYCLE_ENTRY {nodes[found][0] if found is not None else 'NONE'}")
            out.append("LIST " + fmt(LinkedList(nodes, head).values()))

        elif kind == "NO_INTERSECTION":
            nA = take_int()
            a = build_chain(nA)
            nB = take_int()
            b = build_chain(nB)
            common = get_intersection(nodes, a, b)
            if common is None:
                out.append("INTERSECTION NONE")
            else:
                a, b = reverse_shared(nodes, a, b, common)
                out.append(f"INTERSECTION {nodes[common][0]}")
                out.append("LIST_A " + fmt(LinkedList(nodes, a).values()))
                out.append("LIST_B " + fmt(LinkedList(nodes, b).values()))

        elif kind == "INTERSECTION":
            nA = take_int()
            a_prefix = build_chain(nA)
            nB = take_int()
            b_prefix = build_chain(nB)
            k = take_int()
            shared = build_chain(k)

            def attach(head, target):
                if head is None:
                    return target
                cur = head
                while nodes[cur][1] is not None:
                    cur = nodes[cur][1]
                nodes[cur][1] = target
                return head

            a = attach(a_prefix, shared)
            b = attach(b_prefix, shared)
            common = get_intersection(nodes, a, b)
            if common is None:
                out.append("INTERSECTION NONE")
            else:
                a, b = reverse_shared(nodes, a, b, common)
                out.append(f"INTERSECTION {nodes[common][0]}")
                out.append("LIST_A " + fmt(LinkedList(nodes, a).values()))
                out.append("LIST_B " + fmt(LinkedList(nodes, b).values()))
        else:
            raise SystemExit(f"{path}: 未知场景 {kind}")

    if out:
        print("\n".join(out))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    main(sys.argv[1])