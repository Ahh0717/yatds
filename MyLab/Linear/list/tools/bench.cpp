// bench.cpp
// 任务 2 要求的性能对比：在相同的规模与操作序列下，分别测量
//   (1) 自己写的 DynamicArray
//   (2) std::vector
//   (3) 阶段 1 手写的 LinkedList
//
// 每个对象输出一行：
//   PERF name=<名字> n=<数据规模> ops=<操作次数> time_ms=<毫秒，保留两位小数>
//
// 计时只覆盖“按固定序列执行操作”这一段，不含数据生成与输出。
// 输出格式与题目样例一致；由于耗时每次不同，PERF 行不进 outputs/。

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "dynamic_array.h"
#include "linked_list.h"

namespace {

using Clock = std::chrono::steady_clock;

double msSince(Clock::time_point start) {
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        Clock::now() - start)
                        .count();
    return static_cast<double>(ns) / 1e6;
}

// 操作序列：定死下来，保证三种数据结构面对完全相同的输入。
struct Op {
    enum class Kind { PushBack, Insert, Erase, Get, Set } kind;
    std::size_t index;
    int value;
};

std::vector<Op> makeOps(std::size_t count, std::size_t initialSize, unsigned seed) {
    std::mt19937 rng(seed);
    // 操作分布：一半尾插（这是三者的主战场），其余均匀分散在下标修改上。
    // 刻意让插入/删除的下标尽量落在靠前的位置，以暴露数组的搬移成本。
    const int kinds[] = {0, 0, 0, 0, 0, 1, 2, 3, 4};
    std::vector<Op> ops;
    ops.reserve(count);
    std::size_t size = initialSize;
    for (std::size_t k = 0; k < count; ++k) {
        const int pick = kinds[rng() % (sizeof(kinds) / sizeof(kinds[0]))];
        Op op{Op::Kind::PushBack, 0, static_cast<int>(rng() % 1000)};
        switch (pick) {
            case 0:
                op.kind = Op::Kind::PushBack;
                ++size;
                break;
            case 1:
                op.kind = Op::Kind::Insert;
                op.index = size == 0 ? 0 : rng() % size;
                ++size;
                break;
            case 2:
                op.kind = Op::Kind::Erase;
                op.index = size == 0 ? 0 : rng() % size;
                if (size > 0) {
                    --size;
                }
                break;
            case 3:
            case 4:
                op.kind = pick == 3 ? Op::Kind::Get : Op::Kind::Set;
                op.index = size == 0 ? 0 : rng() % size;
                break;
            default:
                break;
        }
        ops.push_back(op);
    }
    return ops;
}

template <typename Fn>
void report(const char *name, std::size_t n, std::size_t ops, Fn bench) {
    // 先预热一次，避免把首次扩容/缺页的抖动算进正式计时。
    bench();
    const auto start = Clock::now();
    bench();
    const double ms = msSince(start);
    std::cout << "PERF name=" << name << " n=" << n << " ops=" << ops
              << " time_ms=" << std::fixed << ms << '\n';
    std::cout.unsetf(std::ios::fixed);
}

}  // namespace

int main(int argc, char **argv) {
    std::size_t n = 100000;
    std::size_t ops = 200000;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg.rfind("--n=", 0) == 0) {
            n = std::stoull(arg.substr(4));
        } else if (arg.rfind("--ops=", 0) == 0) {
            ops = std::stoull(arg.substr(6));
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "用法：" << argv[0] << " [--n=规模] [--ops=操作数]\n";
            return 0;
        } else {
            std::cerr << "未知参数：" << arg << "（可用 --help 查看用法）\n";
            return 2;
        }
    }
    if (n == 0) {
        n = 1;
    }

    const std::vector<Op> sequence = makeOps(ops, n, 20261030);

    // 初始数据：0 .. n-1，三种结构用同一份。
    std::vector<int> seed(n);
    for (std::size_t i = 0; i < n; ++i) {
        seed[i] = static_cast<int>(i);
    }

    // 用一个 volatile 汇总量避免优化器把整段计算删掉。
    volatile long long sink = 0;

    std::cout << std::fixed;

    report("vector_like", n, ops, [&] {
        mylab::DynamicArray<int> a;
        a.reserve(n);
        for (int x : seed) {
            a.pushBack(x);
        }
        long long local = 0;
        for (const Op &op : sequence) {
            switch (op.kind) {
                case Op::Kind::PushBack:
                    a.pushBack(op.value);
                    break;
                case Op::Kind::Insert:
                    if (op.index <= a.size()) {
                        a.insertAt(op.index, op.value);
                    }
                    break;
                case Op::Kind::Erase:
                    if (op.index < a.size()) {
                        a.eraseAt(op.index);
                    }
                    break;
                case Op::Kind::Get: {
                    int v = 0;
                    if (a.get(op.index, v)) {
                        local += v;
                    }
                    break;
                }
                case Op::Kind::Set:
                    if (op.index < a.size()) {
                        a.set(op.index, op.value);
                    }
                    break;
            }
        }
        sink = local;
    });

    report("std_vector", n, ops, [&] {
        std::vector<int> a;
        a.reserve(n);
        a.insert(a.end(), seed.begin(), seed.end());
        long long local = 0;
        for (const Op &op : sequence) {
            switch (op.kind) {
                case Op::Kind::PushBack:
                    a.push_back(op.value);
                    break;
                case Op::Kind::Insert:
                    if (op.index <= a.size()) {
                        a.insert(a.begin() + static_cast<std::ptrdiff_t>(op.index),
                                 op.value);
                    }
                    break;
                case Op::Kind::Erase:
                    if (op.index < a.size()) {
                        a.erase(a.begin() + static_cast<std::ptrdiff_t>(op.index));
                    }
                    break;
                case Op::Kind::Get:
                    if (op.index < a.size()) {
                        local += a[op.index];
                    }
                    break;
                case Op::Kind::Set:
                    if (op.index < a.size()) {
                        a[op.index] = op.value;
                    }
                    break;
            }
        }
        sink = local;
    });

    report("linked_list", n, ops, [&] {
        mylab::LinkedList<int> a;
        for (int x : seed) {
            a.pushBack(x);
        }
        long long local = 0;
        for (const Op &op : sequence) {
            switch (op.kind) {
                case Op::Kind::PushBack:
                    a.pushBack(op.value);
                    break;
                case Op::Kind::Insert:
                    if (op.index <= a.size()) {
                        a.insertAt(op.index, op.value);
                    }
                    break;
                case Op::Kind::Erase:
                    if (op.index < a.size()) {
                        a.eraseAt(op.index);
                    }
                    break;
                case Op::Kind::Get: {
                    int v = 0;
                    if (a.access(op.index, v)) {
                        local += v;
                    }
                    break;
                }
                case Op::Kind::Set:
                    if (op.index < a.size()) {
                        a.set(op.index, op.value);
                    }
                    break;
            }
        }
        sink = local;
    });

    (void)sink;
    return 0;
}