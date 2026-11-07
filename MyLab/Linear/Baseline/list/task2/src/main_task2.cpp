// main_task2.cpp
// Baseline 任务 2 的驱动程序：从 stdin 读入初始容量、初始元素与 m 条操作。
//
// 输入格式：
//   第 1 行        capacity n   初始容量与初始元素个数（capacity >= n）
//   第 2 行        a1 ... an    初始元素（n = 0 时为空行）
//   第 3 行        m            操作次数
//   接下来 m 行    op args...   每行一个操作
//
// 输出：只有 GET / SIZE / CAPACITY / EMPTY / PRINT 产生输出，
//       越界输出 ERROR 且不修改数组，PRINT 空数组输出 EMPTY。
//
// 模板实例的选择：默认看初始元素，全为整数用 int，否则用 std::string；
// 也可以用 --int / --string 显式指定（初始为空但要插入字符串时很有用）。

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "dynamic_array.h"
#include "input_util.h"
#include "linked_list.h"

namespace {

void usage(const char *prog) {
    std::cout << "用法：" << prog << " [--int | --string]\n"
              << "  从标准输入读取数据，把结果写到标准输出。\n"
              << "  默认按初始元素自动选择模板实例（全为整数则用 int，否则用 string）。\n";
}

template <typename T>
int run(const std::vector<std::string> &tokens, std::ostream &out) {
    using mylab::io::Cursor;
    using mylab::io::readIndex;

    Cursor cur(tokens);

    std::size_t capacity = 0;
    std::size_t n = 0;
    if (!readIndex(cur, capacity, "初始容量 capacity") ||
        !readIndex(cur, n, "初始长度 n")) {
        return 1;
    }
    if (capacity < n) {
        std::cerr << "输入错误：初始容量 " << capacity << " 小于初始元素个数 " << n
                  << '\n';
        return 1;
    }

    mylab::DynamicArray<T> array(capacity);
    for (std::size_t k = 0; k < n; ++k) {
        T x;
        if (!mylab::io::readValue(cur, "初始元素", 0, x)) {
            return 1;
        }
        array.pushBack(x);
    }

    std::size_t m = 0;
    if (!readIndex(cur, m, "操作次数 m")) {
        return 1;
    }

    for (std::size_t k = 0; k < m; ++k) {
        const std::size_t step = k + 1;
        const std::string &op = cur.next();
        if (cur.error()) {
            std::cerr << "输入错误：在读取第 " << step << " 条操作时意外结束\n";
            return 1;
        }

        if (op == "PUSH_BACK") {
            T x;
            if (!mylab::io::readValue(cur, op.c_str(), step, x)) {
                return 1;
            }
            array.pushBack(x);
        } else if (op == "INSERT") {
            std::size_t i = 0;
            if (!readIndex(cur, i, "INSERT 的下标", step)) {
                return 1;
            }
            T x;
            if (!mylab::io::readValue(cur, op.c_str(), step, x)) {
                return 1;
            }
            // 越界插入不输出任何内容，也不修改数组。
            array.insertAt(i, x);
        } else if (op == "ERASE") {
            std::size_t i = 0;
            if (!readIndex(cur, i, "ERASE 的下标", step)) {
                return 1;
            }
            array.eraseAt(i);
        } else if (op == "GET") {
            std::size_t i = 0;
            if (!readIndex(cur, i, "GET 的下标", step)) {
                return 1;
            }
            T value;
            if (array.get(i, value)) {
                out << value << '\n';
            } else {
                out << "ERROR\n";
            }
        } else if (op == "SET") {
            std::size_t i = 0;
            if (!readIndex(cur, i, "SET 的下标", step)) {
                return 1;
            }
            T x;
            if (!mylab::io::readValue(cur, op.c_str(), step, x)) {
                return 1;
            }
            array.set(i, x);  // 越界时不修改，也不输出
        } else if (op == "SIZE") {
            out << array.size() << '\n';
        } else if (op == "CAPACITY") {
            out << array.capacity() << '\n';
        } else if (op == "EMPTY") {
            out << (array.empty() ? "true" : "false") << '\n';
        } else if (op == "PRINT") {
            array.print(out);
            out << '\n';
        } else if (op == "CLEAR") {
            array.clear();  // 只清元素，不缩容
        } else {
            // 未知操作：只报告到 stderr，不影响已产生的标准输出。
            std::cerr << "警告：忽略第 " << step << " 条未知操作 " << op << '\n';
        }
    }
    return 0;
}

}  // namespace

// ---------------------------------------------------------------------------
// 性能统计（README 任务 2 要求：功能输出后追加 PERF 行）
// 在相同规模与固定操作序列下对比三种结构：
//   vector_like = mylab::DynamicArray<int>
//   std_vector  = std::vector<int>
//   linked_list = mylab::LinkedList<int>（阶段 1 的链表，O(n) 随机访问）
// 规模取题目样例的 n=100000、ops=200000；耗时每次不同，PERF 行不进 outputs/。
// ---------------------------------------------------------------------------
namespace {

using PerfClock = std::chrono::steady_clock;

double perfMsSince(PerfClock::time_point start) {
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        PerfClock::now() - start)
                        .count();
    return static_cast<double>(ns) / 1e6;
}

struct PerfOp {
    enum class Kind { PushBack, Insert, Erase, Get, Set } kind;
    std::size_t index;
    int value;
};

std::vector<PerfOp> makePerfOps(std::size_t count, std::size_t initialSize,
                                unsigned seed) {
    std::mt19937 rng(seed);
    // 一半尾插（三者的主战场），其余均匀分散在下标修改上，
    // 刻意让插入/删除下标尽量靠前，以暴露数组的搬移成本与链表的遍历成本。
    const int kinds[] = {0, 0, 0, 0, 0, 1, 2, 3, 4};
    std::vector<PerfOp> ops;
    ops.reserve(count);
    std::size_t size = initialSize;
    for (std::size_t k = 0; k < count; ++k) {
        const int pick = kinds[rng() % (sizeof(kinds) / sizeof(kinds[0]))];
        PerfOp op{PerfOp::Kind::PushBack, 0, static_cast<int>(rng() % 1000)};
        switch (pick) {
            case 0:
                op.kind = PerfOp::Kind::PushBack;
                ++size;
                break;
            case 1:
                op.kind = PerfOp::Kind::Insert;
                op.index = size == 0 ? 0 : rng() % size;
                ++size;
                break;
            case 2:
                op.kind = PerfOp::Kind::Erase;
                op.index = size == 0 ? 0 : rng() % size;
                if (size > 0) {
                    --size;
                }
                break;
            case 3:
            case 4:
                op.kind = pick == 3 ? PerfOp::Kind::Get : PerfOp::Kind::Set;
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
void reportPerf(const char *name, std::size_t n, std::size_t ops, Fn bench) {
    bench();  // 预热一次，避免首次扩容/缺页抖动计入正式计时
    const auto start = PerfClock::now();
    bench();
    const double ms = perfMsSince(start);
    std::cout << "PERF " << name << " n=" << n << " ops=" << ops
              << " time_ms=" << std::fixed << std::setprecision(2) << ms << '\n';
    std::cout.unsetf(std::ios::fixed);
}

void benchmark() {
    const std::size_t n = 100000;
    const std::size_t ops = 200000;

    const std::vector<PerfOp> sequence = makePerfOps(ops, n, 20261030);
    std::vector<int> seed(n);
    for (std::size_t i = 0; i < n; ++i) {
        seed[i] = static_cast<int>(i);
    }
    // 用一个 volatile 汇总量避免优化器把整段计算删掉。
    volatile long long sink = 0;

    reportPerf("vector_like", n, ops, [&] {
        mylab::DynamicArray<int> a;
        a.reserve(n);
        for (int x : seed) {
            a.pushBack(x);
        }
        long long local = 0;
        for (const PerfOp &op : sequence) {
            switch (op.kind) {
                case PerfOp::Kind::PushBack:
                    a.pushBack(op.value);
                    break;
                case PerfOp::Kind::Insert:
                    if (op.index <= a.size()) {
                        a.insertAt(op.index, op.value);
                    }
                    break;
                case PerfOp::Kind::Erase:
                    if (op.index < a.size()) {
                        a.eraseAt(op.index);
                    }
                    break;
                case PerfOp::Kind::Get: {
                    int v = 0;
                    if (a.get(op.index, v)) {
                        local += v;
                    }
                    break;
                }
                case PerfOp::Kind::Set:
                    if (op.index < a.size()) {
                        a.set(op.index, op.value);
                    }
                    break;
            }
        }
        sink = local;
    });

    reportPerf("std_vector", n, ops, [&] {
        std::vector<int> a;
        a.reserve(n);
        a.insert(a.end(), seed.begin(), seed.end());
        long long local = 0;
        for (const PerfOp &op : sequence) {
            switch (op.kind) {
                case PerfOp::Kind::PushBack:
                    a.push_back(op.value);
                    break;
                case PerfOp::Kind::Insert:
                    if (op.index <= a.size()) {
                        a.insert(a.begin() + static_cast<std::ptrdiff_t>(op.index),
                                 op.value);
                    }
                    break;
                case PerfOp::Kind::Erase:
                    if (op.index < a.size()) {
                        a.erase(a.begin() + static_cast<std::ptrdiff_t>(op.index));
                    }
                    break;
                case PerfOp::Kind::Get:
                    if (op.index < a.size()) {
                        local += a[op.index];
                    }
                    break;
                case PerfOp::Kind::Set:
                    if (op.index < a.size()) {
                        a[op.index] = op.value;
                    }
                    break;
            }
        }
        sink = local;
    });

    reportPerf("linked_list", n, ops, [&] {
        mylab::LinkedList<int> a;
        for (int x : seed) {
            a.pushBack(x);
        }
        long long local = 0;
        for (const PerfOp &op : sequence) {
            switch (op.kind) {
                case PerfOp::Kind::PushBack:
                    a.pushBack(op.value);
                    break;
                case PerfOp::Kind::Insert:
                    if (op.index <= a.size()) {
                        a.insertAt(op.index, op.value);
                    }
                    break;
                case PerfOp::Kind::Erase:
                    if (op.index < a.size()) {
                        a.eraseAt(op.index);
                    }
                    break;
                case PerfOp::Kind::Get: {
                    int v = 0;
                    if (a.access(op.index, v)) {
                        local += v;
                    }
                    break;
                }
                case PerfOp::Kind::Set:
                    if (op.index < a.size()) {
                        a.set(op.index, op.value);
                    }
                    break;
            }
        }
        sink = local;
    });

    (void)sink;
}

}  // namespace

int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    enum class Mode { Auto, Int, String };
    Mode mode = Mode::Auto;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--int") {
            mode = Mode::Int;
        } else if (arg == "--string") {
            mode = Mode::String;
        } else if (arg == "--help" || arg == "-h") {
            usage(argv[0]);
            return 0;
        } else {
            std::cerr << "未知参数：" << arg << "（可用 --help 查看用法）\n";
            return 2;
        }
    }

    const std::vector<std::string> tokens = mylab::io::readAllTokens();
    if (tokens.size() < 3) {
        std::cerr << "输入错误：至少需要 capacity、n 与操作次数 m\n";
        return 1;
    }

    if (mode == Mode::Auto) {
        // capacity 与 n 之后紧跟着 n 个初始元素：全为整数时用 int。
        std::size_t n = 0;
        try {
            std::size_t used = 0;
            n = std::stoull(tokens[1], &used);
            if (used != tokens[1].size()) {
                n = 0;
            }
        } catch (const std::exception &) {
            n = 0;
        }
        bool numeric = true;
        for (std::size_t k = 0; k < n && 2 + k < tokens.size(); ++k) {
            if (!mylab::io::looksLikeInteger(tokens[2 + k])) {
                numeric = false;
                break;
            }
        }
        mode = numeric ? Mode::Int : Mode::String;
    }

    const int result = mode == Mode::Int ? run<int>(tokens, std::cout)
                                         : run<std::string>(tokens, std::cout);
    if (result != 0) {
        return result;
    }
    benchmark();  // 功能输出全部结束后，追加性能统计（README 任务 2 要求）
    return 0;
}