// main_task3.cpp
// Baseline 任务 3 的驱动程序：栈与队列的相互实现，以及二者之间的转换。
//
// 输入格式：
//   第 1 行   T                      场景个数
//   之后每个场景以类型行开头：
//     STACK_BY_QUEUE  n / 栈底到栈顶 n 个元素 / m / m 行栈操作
//     QUEUE_BY_STACK  n / 队首到队尾 n 个元素 / m / m 行队列操作
//     CONVERT         n / n 个元素 / STACK_TO_QUEUE 或 QUEUE_TO_STACK
//
// 操作：栈场景 PUSH/POP/TOP/PRINT；队列场景 PUSH/POP/FRONT/PRINT。
// 输出：每场景先输出 CASE t；空结构访问输出 ERROR；空结构打印输出 EMPTY。
//
// 转换语义（两者都等于“把输入序列反转”）：
//   QUEUE_TO_STACK  输入按队首 -> 队尾理解，输出按栈顶 -> 栈底。
//   STACK_TO_QUEUE  输入按栈底 -> 栈顶理解，输出按队首 -> 队尾。
//
// 功能输出结束后追加四行性能统计（PERF）。outputs/ 中不保存这些行，
// test 比对时忽略以 "PERF " 开头的行。
//
// 用法：
//   ./build/task3                 # 元素类型按输入自动判断
//   ./build/task3 --int|--string  # 显式指定元素类型

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <queue>
#include <random>
#include <stack>
#include <string>
#include <vector>

#include "input_util.h"
#include "queue_by_stack.h"
#include "stack_by_queue.h"
#include "vector_queue.h"
#include "vector_stack.h"

namespace {

using Clock = std::chrono::steady_clock;

volatile long long g_benchSink = 0;

double msSince(Clock::time_point start) {
    const auto ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start)
            .count();
    return static_cast<double>(ns) / 1e6;
}

void usage(const char *prog) {
    std::cout << "用法：" << prog << " [--int | --string]\n"
              << "  从标准输入读取多个场景并输出结果。\n"
              << "  默认根据全部元素自动选择 int / string；可显式指定。\n";
}

// ---------------------------------------------------------------------
// 场景处理
// ---------------------------------------------------------------------

// 栈语义的统一操作处理：Stack 需提供 empty/size/push/pop/top/
// printTopToBottom，且 top() 返回 T。
template <typename T, typename Stack>
bool runStackOps(mylab::io::Cursor &cur, std::size_t m, Stack &stack,
                 std::ostream &out) {
    for (std::size_t k = 0; k < m; ++k) {
        const std::size_t step = k + 1;
        const std::string &op = cur.next();
        if (cur.error()) {
            std::cerr << "输入错误：在读取第 " << step << " 条栈操作时意外结束\n";
            return false;
        }
        if (op == "PUSH") {
            T x;
            if (!mylab::io::readValue(cur, op.c_str(), step, x)) {
                return false;
            }
            stack.push(x);
        } else if (op == "POP") {
            try {
                T value = stack.top();
                stack.pop();
                out << value << '\n';
            } catch (const std::out_of_range &) {
                out << "ERROR\n";
            }
        } else if (op == "TOP") {
            try {
                out << stack.top() << '\n';
            } catch (const std::out_of_range &) {
                out << "ERROR\n";
            }
        } else if (op == "PRINT") {
            stack.printTopToBottom(out);
            out << '\n';
        } else {
            std::cerr << "警告：忽略第 " << step << " 条未知栈操作 " << op << '\n';
        }
    }
    return true;
}

// 队列语义的统一操作处理：Queue 需提供 empty/size/push/pop/front/
// printFrontToBack。
template <typename T, typename Queue>
bool runQueueOps(mylab::io::Cursor &cur, std::size_t m, Queue &queue,
                 std::ostream &out) {
    for (std::size_t k = 0; k < m; ++k) {
        const std::size_t step = k + 1;
        const std::string &op = cur.next();
        if (cur.error()) {
            std::cerr << "输入错误：在读取第 " << step << " 条队列操作时意外结束\n";
            return false;
        }
        if (op == "PUSH") {
            T x;
            if (!mylab::io::readValue(cur, op.c_str(), step, x)) {
                return false;
            }
            queue.push(x);
        } else if (op == "POP") {
            try {
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
        } else if (op == "PRINT") {
            queue.printFrontToBack(out);
            out << '\n';
        } else {
            std::cerr << "警告：忽略第 " << step << " 条未知队列操作 " << op << '\n';
        }
    }
    return true;
}

template <typename T>
bool readElements(mylab::io::Cursor &cur, std::size_t n, std::vector<T> &out) {
    out.clear();
    out.reserve(n);
    for (std::size_t k = 0; k < n; ++k) {
        T x;
        if (!mylab::io::readValue(cur, "元素", 0, x)) {
            return false;
        }
        out.push_back(x);
    }
    return true;
}

template <typename T, typename Stack>
bool runStackByQueue(mylab::io::Cursor &cur, std::ostream &out) {
    using mylab::io::readIndex;
    std::size_t n = 0;
    if (!readIndex(cur, n, "STACK_BY_QUEUE 的元素个数 n")) {
        return false;
    }
    std::vector<T> elements;
    if (!readElements(cur, n, elements)) {
        return false;
    }
    Stack stack;
    for (const T &x : elements) {  // 栈底 -> 栈顶：按顺序 push 即可
        stack.push(x);
    }
    std::size_t m = 0;
    if (!readIndex(cur, m, "STACK_BY_QUEUE 的操作次数 m")) {
        return false;
    }
    return runStackOps<T>(cur, m, stack, out);
}

template <typename T, typename Queue>
bool runQueueByStack(mylab::io::Cursor &cur, std::ostream &out) {
    using mylab::io::readIndex;
    std::size_t n = 0;
    if (!readIndex(cur, n, "QUEUE_BY_STACK 的元素个数 n")) {
        return false;
    }
    std::vector<T> elements;
    if (!readElements(cur, n, elements)) {
        return false;
    }
    Queue queue;
    for (const T &x : elements) {  // 队首 -> 队尾：按顺序 push 即可
        queue.push(x);
    }
    std::size_t m = 0;
    if (!readIndex(cur, m, "QUEUE_BY_STACK 的操作次数 m")) {
        return false;
    }
    return runQueueOps<T>(cur, m, queue, out);
}

// CONVERT 场景：两种方向的结果都是把输入序列反转输出。
template <typename T>
bool runConvert(mylab::io::Cursor &cur, std::ostream &out) {
    using mylab::io::readIndex;
    std::size_t n = 0;
    if (!readIndex(cur, n, "CONVERT 的元素个数 n")) {
        return false;
    }
    std::vector<T> elements;
    if (!readElements(cur, n, elements)) {
        return false;
    }
    const std::string &direction = cur.next();
    if (cur.error()) {
        std::cerr << "输入错误：CONVERT 缺少转换方向\n";
        return false;
    }

    dsaac::VectorStack<T> stack;  // 题目要求：用 <vector> 实现的两个类
    dsaac::VectorQueue<T> queue;

    if (direction == "QUEUE_TO_STACK") {
        // 队列按队首 -> 队尾存放
        for (const T &x : elements) {
            queue.push(x);
        }
        // 转成栈，按栈顶 -> 栈底输出
        const dsaac::VectorStack<T> converted = queue.toStack();
        if (converted.empty()) {
            out << "EMPTY\n";
        } else {
            converted.printTopToBottom(out);
            out << '\n';
        }
    } else if (direction == "STACK_TO_QUEUE") {
        // 栈按栈底 -> 栈顶存放
        for (const T &x : elements) {
            stack.push(x);
        }
        // 转成队列，按队首 -> 队尾输出
        const dsaac::VectorQueue<T> converted = stack.toQueue();
        if (converted.empty()) {
            out << "EMPTY\n";
        } else {
            converted.printFrontToBack(out);
            out << '\n';
        }
    } else {
        std::cerr << "输入错误：未知的转换方向 " << direction << '\n';
        return false;
    }
    return true;
}

template <typename T>
int run(const std::vector<std::string> &tokens, std::ostream &out) {
    using mylab::io::Cursor;
    using mylab::io::readIndex;

    Cursor cur(tokens);
    std::size_t cases = 0;
    if (!readIndex(cur, cases, "场景个数 T")) {
        return 1;
    }

    for (std::size_t t = 1; t <= cases; ++t) {
        const std::string &kind = cur.next();
        if (cur.error()) {
            std::cerr << "输入错误：第 " << t << " 个场景缺少类型\n";
            return 1;
        }
        out << "CASE " << t << '\n';

        bool ok = false;
        if (kind == "STACK_BY_QUEUE") {
            ok = runStackByQueue<T, dsaac::StackByQueue<T>>(cur, out);
        } else if (kind == "QUEUE_BY_STACK") {
            ok = runQueueByStack<T, dsaac::QueueByStack<T>>(cur, out);
        } else if (kind == "CONVERT") {
            ok = runConvert<T>(cur, out);
        } else {
            std::cerr << "输入错误：第 " << t << " 个场景类型未知：" << kind << '\n';
            return 1;
        }
        if (!ok) {
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------
// 性能统计
// ---------------------------------------------------------------------

struct BenchOp {
    enum class Kind { Push, Pop, Top } kind;
    int value;
};

std::vector<BenchOp> makeBenchOps(std::size_t count, unsigned seed) {
    // StackByQueue 的 pop/top 是 O(n)，全量跑 n=100000 会天文数字般慢，
    // 因此这里的规模按题目给的 ops 做“工作负载等价”的缩小：
    // 三种实现面对完全相同的序列，只比较常数因子。
    std::mt19937 rng(seed);
    std::vector<BenchOp> ops;
    ops.reserve(count);
    for (std::size_t k = 0; k < count; ++k) {
        const int pick = static_cast<int>(rng() % 10);
        if (pick < 5 || k == 0) {
            ops.push_back({BenchOp::Kind::Push, static_cast<int>(rng() % 1000)});
        } else if (pick < 8) {
            ops.push_back({BenchOp::Kind::Pop, 0});
        } else {
            ops.push_back({BenchOp::Kind::Top, 0});
        }
    }
    return ops;
}

template <typename Stack>
double runBench(Stack &stack, const std::vector<int> &initial,
                const std::vector<BenchOp> &ops) {
    const auto start = Clock::now();
    for (const int value : initial) {
        stack.push(value);
    }
    long long sink = 0;
    for (const BenchOp &op : ops) {
        switch (op.kind) {
            case BenchOp::Kind::Push:
                stack.push(op.value);
                break;
            case BenchOp::Kind::Pop:
                sink += stack.top();
                stack.pop();
                break;
            case BenchOp::Kind::Top:
                sink += stack.top();
                break;
        }
    }
    const double elapsed = msSince(start);
    g_benchSink += sink;
    return elapsed;
}

// 同一个模板跑队列语义（front 即队首）。std::queue 与 VectorQueue 共用，
// 只有 QueueByStack 的 front() 返回 const T&，其余返回 T / const T&，都能用。
template <typename Queue>
double runQueueBench(Queue &queue, const std::vector<int> &initial,
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
                sink += static_cast<long long>(queue.front());
                queue.pop();
                break;
            case BenchOp::Kind::Top:
                sink += static_cast<long long>(queue.front());
                break;
        }
    }
    const double elapsed = msSince(start);
    g_benchSink += sink;
    return elapsed;
}

void printPerf(const char *name, std::size_t n, std::size_t ops, double ms) {
    std::cout << "PERF name=" << name << " n=" << n << " ops=" << ops
              << " time_ms=" << std::fixed << std::setprecision(2) << ms << '\n';
}

void benchmark() {
    // 题目要求的规模是 n = 100000 / ops = 200000，但 StackByQueue 的
    // pop/top 是 O(n)，那就是约 10^10 次队列操作，跑不完。
    // 这里让三种实现跑同一序列、同一规模，并把规模降到能在一秒内完成的
    // 量级；PERF 行的 n/ops 如实打印当前实际规模。
    const std::size_t n = 2000;
    const std::size_t ops = 2000;

    std::mt19937 rng(20261031u);
    std::vector<int> initial(n);
    for (int &value : initial) {
        value = static_cast<int>(rng() % 1000);
    }
    const std::vector<BenchOp> sequence = makeBenchOps(ops, 20261031u);

    {
        dsaac::StackByQueue<int> stack;
        printPerf("stack_by_queue", n, ops, runBench(stack, initial, sequence));
    }
    {
        dsaac::QueueByStack<int> queue;
        printPerf("queue_by_stack", n, ops, runQueueBench(queue, initial, sequence));
    }
    {
        dsaac::VectorStack<int> stack;
        printPerf("vector_based", n, ops, runBench(stack, initial, sequence));
    }
    {
        // 题目还要求对比“栈实现队列”与“<vector> 实现队列”的差异，
        // 所以队列一侧也补一行 vector 实现作对照。
        dsaac::VectorQueue<int> queue;
        printPerf("vector_queue", n, ops, runQueueBench(queue, initial, sequence));
    }
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
    if (tokens.empty()) {
        std::cerr << "输入错误：至少需要场景个数 T\n";
        return 1;
    }

    if (mode == Mode::Auto) {
        // 扫描全部 token：跳过 T 与各场景的类型关键字/方向关键字/
        // 操作关键字，剩下的数字类 token 全是整数就用 int。
        static const std::vector<std::string> keywords = {
            "STACK_BY_QUEUE", "QUEUE_BY_STACK", "CONVERT",
            "QUEUE_TO_STACK", "STACK_TO_QUEUE",
            "PUSH", "POP", "TOP", "FRONT", "PRINT"};
        bool numeric = true;
        bool sawValue = false;
        for (const std::string &token : tokens) {
            bool isKeyword = false;
            for (const std::string &keyword : keywords) {
                if (token == keyword) {
                    isKeyword = true;
                    break;
                }
            }
            if (isKeyword) {
                continue;
            }
            // 结构里的计数值（T / n / m）也是整数；元素若出现非整数就转 string。
            if (!mylab::io::looksLikeInteger(token)) {
                numeric = false;
                break;
            }
            sawValue = true;
        }
        mode = (numeric || !sawValue) ? Mode::Int : Mode::String;
    }

    int rc = 0;
    if (mode == Mode::Int) {
        rc = run<int>(tokens, std::cout);
    } else {
        rc = run<std::string>(tokens, std::cout);
    }
    if (rc != 0) {
        return rc;
    }

    std::cout.flush();
    benchmark();
    return 0;
}