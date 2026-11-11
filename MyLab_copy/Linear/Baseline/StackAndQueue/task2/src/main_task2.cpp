// main_task2.cpp
// Baseline 任务 2 的驱动程序：循环队列 / 平移式队列 / 链式队列。
//
// 输入格式：
//   第 1 行            capacity n   队列容量 + 初始元素个数（链式队列忽略 capacity）
//   第 2 行            a1 ... an    从队首到队尾的初始元素（n = 0 时为空行）
//   第 3 行            m            操作次数
//   接下来 m 行        op args...   每行一个操作
//
// 操作（题目要求）：
//   PUSH x  POP  FRONT  SIZE  EMPTY  PRINT
// 扩展操作（不影响题目样例的输出）：
//   BACK       输出队尾元素
//   CLEAR      清空队列
//
// 输出：POP / FRONT / SIZE / EMPTY / PRINT 各一行；空队列 POP/FRONT/BACK
// 输出 ERROR；EMPTY 输出 true/false；PRINT 按队首到队尾、空队列输出 EMPTY；
// PUSH 不输出。满队列 PUSH 在数组实现上被静默忽略（不输出）；
// 链式队列不会满。
//
// 功能输出结束后追加三行性能统计（PERF）。outputs/ 中不保存这三行，
// test 比对时忽略以 "PERF " 开头的行。
//
// 用法：
//   ./build/task2            # 循环队列（默认），元素类型按初始元素自动判断
//   ./build/task2 --shift    # 平移式数组队列
//   ./build/task2 --linked   # 链式队列
//   ./build/task2 --int|--string  # 显式指定元素类型（初始为空时用）

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include "input_util.h"
#include "linked_queue.h"
#include "queue.hpp"
#include "shift_queue.h"

namespace {

using Clock = std::chrono::steady_clock;

// benchmark 的防优化累加器。
volatile long long g_benchSink = 0;

double msSince(Clock::time_point start) {
    const auto ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start)
            .count();
    return static_cast<double>(ns) / 1e6;
}

void usage(const char *prog) {
    std::cout << "用法：" << prog
              << " [--circular | --shift | --linked] [--int | --string]\n"
              << "  --circular  循环队列（默认）\n"
              << "  --shift     平移式数组队列（出队后整体左移）\n"
              << "  --linked    链式队列（忽略 capacity）\n"
              << "  默认按初始元素自动选择 int / string；--int/--string 可显式指定。\n";
}

// ---------------------------------------------------------------------
// 功能部分：三种队列共用同一套操作处理。队列类型只需提供
// empty/push/pop/front/back/size/clear/printFrontToBack。
// ---------------------------------------------------------------------

template <typename T, typename Queue>
bool readInitial(mylab::io::Cursor &cur, std::size_t n, Queue &queue) {
    for (std::size_t k = 0; k < n; ++k) {
        T x;
        if (!mylab::io::readValue(cur, "初始元素", 0, x)) {
            return false;
        }
        queue.push(x);
    }
    return true;
}

template <typename T, typename Queue>
bool processOps(mylab::io::Cursor &cur, std::size_t m, Queue &queue,
                std::ostream &out) {
    using mylab::io::readValue;

    for (std::size_t k = 0; k < m; ++k) {
        const std::size_t step = k + 1;
        const std::string &op = cur.next();
        if (cur.error()) {
            std::cerr << "输入错误：在读取第 " << step << " 条操作时意外结束\n";
            return false;
        }

        if (op == "PUSH") {
            T x;
            if (!readValue(cur, op.c_str(), step, x)) {
                return false;
            }
            try {
                queue.push(x);
            } catch (const std::overflow_error &) {
                // 数组队列已满：满队列 PUSH 不输出任何内容。
            }
        } else if (op == "POP") {
            try {
                // pop() 不返回值，先取队首再出队才能输出被删元素。
                T value = queue.front();
                queue.pop();
                out << value << '\n';
            } catch (const std::out_of_range &) {
                out << "ERROR\n";
            }
        } else if (op == "FRONT") {
            try {
                out << queue.front() << '\n';
            } catch (const std::out_of_range &) {
                out << "ERROR\n";
            }
        } else if (op == "BACK") {
            try {
                out << queue.back() << '\n';
            } catch (const std::out_of_range &) {
                out << "ERROR\n";
            }
        } else if (op == "SIZE") {
            out << queue.size() << '\n';
        } else if (op == "EMPTY") {
            out << (queue.empty() ? "true" : "false") << '\n';
        } else if (op == "PRINT") {
            queue.printFrontToBack(out);
            out << '\n';
        } else if (op == "CLEAR") {
            queue.clear();
        } else {
            // 未知操作：只报告到 stderr，不影响已产生的标准输出。
            std::cerr << "警告：忽略第 " << step << " 条未知操作 " << op << '\n';
        }
    }
    return true;
}

// 通用流程：读 capacity / n / 初始元素 / m。
// hasCapacityLimit 为 true 时检查 n <= capacity（数组实现装不下就报错）。
template <typename T, typename MakeQueue>
int run(const std::vector<std::string> &tokens, bool hasCapacityLimit,
        MakeQueue makeQueue, std::ostream &out) {
    using mylab::io::Cursor;
    using mylab::io::readIndex;

    Cursor cur(tokens);

    std::size_t capacity = 0;
    if (!readIndex(cur, capacity, "容量 capacity")) {
        return 1;
    }
    std::size_t n = 0;
    if (!readIndex(cur, n, "初始元素个数 n")) {
        return 1;
    }
    if (hasCapacityLimit && n > capacity) {
        std::cerr << "输入错误：初始元素个数 " << n << " 超过数组队列容量 "
                  << capacity << '\n';
        return 1;
    }

    auto queue = makeQueue(capacity);
    if (!readInitial<T>(cur, n, queue)) {
        return 1;
    }

    std::size_t m = 0;
    if (!readIndex(cur, m, "操作次数 m")) {
        return 1;
    }
    if (!processOps<T>(cur, m, queue, out)) {
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------
// 性能统计：相同规模、相同操作序列，分别测三种队列。
// 计时从“压入初始元素”开始，不含对象构造与数据生成。
// ---------------------------------------------------------------------

struct BenchOp {
    enum class Kind { Push, Pop, Front } kind;
    int value;
};

std::vector<BenchOp> makeBenchOps(std::size_t initialSize, std::size_t count,
                                  unsigned seed) {
    std::mt19937 rng(seed);
    std::vector<BenchOp> ops;
    ops.reserve(count);
    std::size_t size = initialSize;
    for (std::size_t k = 0; k < count; ++k) {
        const int pick = static_cast<int>(rng() % 10);
        if (size == 0 || pick < 5) {
            ops.push_back({BenchOp::Kind::Push, static_cast<int>(rng() % 1000)});
            ++size;
        } else if (pick < 8) {
            ops.push_back({BenchOp::Kind::Pop, 0});
            --size;
        } else {
            ops.push_back({BenchOp::Kind::Front, 0});
        }
    }
    return ops;
}

template <typename Queue>
double runBench(Queue &queue, const std::vector<int> &initial,
                const std::vector<BenchOp> &ops) {
    const auto start = Clock::now();
    for (const int value : initial) {
        queue.push(value);
    }
    long long sink = 0;
    for (const BenchOp &op : ops) {
        switch (op.kind) {
            case BenchOp::Kind::Push:
                queue.push(op.value);
                break;
            case BenchOp::Kind::Pop:
                sink += queue.front();
                queue.pop();
                break;
            case BenchOp::Kind::Front:
                sink += queue.front();
                break;
        }
    }
    const double elapsed = msSince(start);
    g_benchSink += sink;
    return elapsed;
}

void printPerf(const char *name, std::size_t n, std::size_t ops, double ms) {
    std::cout << "PERF " << name << " n=" << n << " ops=" << ops
              << " time_ms=" << std::fixed << std::setprecision(2) << ms << '\n';
}

void benchmark() {
    // 与题目要求一致：n = 100000 个初始元素，ops = 200000 条操作。
    const std::size_t n = 100000;
    const std::size_t ops = 200000;

    std::mt19937 rng(20261031u);
    std::vector<int> initial(n);
    for (int &value : initial) {
        value = static_cast<int>(rng() % 1000);
    }
    const std::vector<BenchOp> sequence = makeBenchOps(n, ops, 20261031u);

    // 循环队列 / 平移队列的容量：初始 n 个，之后最多再入队 ops 个。
    const std::size_t capacity = n + ops;

    {
        dsaac::CircularQueue<int> queue(capacity);
        printPerf("circular_queue", n, ops, runBench(queue, initial, sequence));
    }
    {
        dsaac::LinkedQueue<int> queue;
        printPerf("linked_queue", n, ops, runBench(queue, initial, sequence));
    }
    {
        std::queue<int> queue;
        printPerf("std_queue", n, ops, runBench(queue, initial, sequence));
    }
}

}  // namespace

int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    enum class Mode { Auto, Int, String };
    enum class Impl { Circular, Shift, Linked };
    Mode mode = Mode::Auto;
    Impl impl = Impl::Circular;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--int") {
            mode = Mode::Int;
        } else if (arg == "--string") {
            mode = Mode::String;
        } else if (arg == "--circular") {
            impl = Impl::Circular;
        } else if (arg == "--shift") {
            impl = Impl::Shift;
        } else if (arg == "--linked") {
            impl = Impl::Linked;
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
        std::cerr << "输入错误：至少需要 capacity、n、m 三项\n";
        return 1;
    }

    if (mode == Mode::Auto) {
        // 用 capacity/n 之后的 n 个 token 判断：全是整数就用 int。
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

    // 链式队列没有容量上限，不检查 n <= capacity。
    const bool hasLimit = impl != Impl::Linked;

    int rc = 0;
    if (mode == Mode::Int) {
        if (impl == Impl::Circular) {
            rc = run<int>(tokens, hasLimit,
                          [](std::size_t cap) { return dsaac::CircularQueue<int>(cap); },
                          std::cout);
        } else if (impl == Impl::Shift) {
            rc = run<int>(tokens, hasLimit,
                          [](std::size_t cap) { return dsaac::ShiftQueue<int>(cap); },
                          std::cout);
        } else {
            rc = run<int>(tokens, hasLimit,
                          [](std::size_t) { return dsaac::LinkedQueue<int>(); },
                          std::cout);
        }
    } else {
        if (impl == Impl::Circular) {
            rc = run<std::string>(
                tokens, hasLimit,
                [](std::size_t cap) { return dsaac::CircularQueue<std::string>(cap); },
                std::cout);
        } else if (impl == Impl::Shift) {
            rc = run<std::string>(
                tokens, hasLimit,
                [](std::size_t cap) { return dsaac::ShiftQueue<std::string>(cap); },
                std::cout);
        } else {
            rc = run<std::string>(
                tokens, hasLimit,
                [](std::size_t) { return dsaac::LinkedQueue<std::string>(); },
                std::cout);
        }
    }
    if (rc != 0) {
        return rc;
    }

    // 功能输出全部结束后，追加性能统计。
    std::cout.flush();
    benchmark();
    return 0;
}