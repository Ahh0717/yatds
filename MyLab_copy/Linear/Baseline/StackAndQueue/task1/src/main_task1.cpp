// main_task1.cpp
// Baseline 任务 1 的驱动程序：数组栈 ArrayStack<T> 与链式栈 LinkedStack<T>。
//
// 输入格式：
//   第 1 行            capacity n   数组栈容量 + 初始元素个数（链式栈忽略 capacity）
//   第 2 行            a1 ... an    从栈底到栈顶的初始元素（n = 0 时为空行）
//   第 3 行            m            操作次数
//   接下来 m 行        op args...   每行一个操作
//
// 操作（题目要求）：
//   PUSH x  POP  TOP  SIZE  EMPTY  PRINT
// 扩展操作（不改变题目样例的输出）：
//   CLEAR      清空栈
//   SETTOP x   把栈顶元素改为 x（空栈时静默忽略，与 PUSH/DELETE 的约定一致）
//
// 输出：POP / TOP / SIZE / EMPTY / PRINT 各一行；空栈 POP/TOP 输出 ERROR；
// EMPTY 输出 true/false；PRINT 按栈顶到栈底、空栈输出 EMPTY；PUSH 不输出。
// 满栈 PUSH 在数组栈上被静默忽略（不输出）；链式栈不会满。
//
// 功能输出结束后追加三行性能统计（PERF）。outputs/ 中不保存这三行，
// test 比对时忽略以 "PERF " 开头的行。
//
// 用法：
//   ./build/task1                 # 数组栈（默认），元素类型按初始元素自动判断
//   ./build/task1 --linked        # 链式栈
//   ./build/task1 --int|--string  # 显式指定元素类型（初始为空时用）

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <stack>
#include <string>
#include <vector>

#include "input_util.h"
#include "stackar.hpp"
#include "stackli.hpp"

namespace {

using Clock = std::chrono::steady_clock;

// benchmark 的防优化累加器：把 POP/TOP 的结果混进来，避免编译器把
// “看起来没有副作用”的一整段操作优化掉。
volatile long long g_benchSink = 0;

double msSince(Clock::time_point start) {
    const auto ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start)
            .count();
    return static_cast<double>(ns) / 1e6;
}

void usage(const char *prog) {
    std::cout << "用法：" << prog << " [--array | --linked] [--int | --string]\n"
              << "  从标准输入读取数据，把结果写到标准输出。\n"
              << "  --array   使用固定容量的数组栈（默认）\n"
              << "  --linked  使用链式栈（忽略 capacity）\n"
              << "  默认按初始元素自动选择 int / string；--int/--string 可显式指定。\n";
}

// ---------------------------------------------------------------------
// 功能部分：两种栈共用同一套操作处理。栈类型只需提供
// empty/push/pop/top/size/clear/setTop/printTopToBottom。
// ---------------------------------------------------------------------

// 读取 n 个初始元素，按“栈底 -> 栈顶”的顺序压栈。
template <typename T, typename Stack>
bool readInitial(mylab::io::Cursor &cur, std::size_t n, Stack &stack) {
    for (std::size_t k = 0; k < n; ++k) {
        T x;
        if (!mylab::io::readValue(cur, "初始元素", 0, x)) {
            return false;
        }
        stack.push(x);
    }
    return true;
}

// 执行 m 条操作；返回 false 表示输入格式错误。
template <typename T, typename Stack>
bool processOps(mylab::io::Cursor &cur, std::size_t m, Stack &stack,
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
                stack.push(x);
            } catch (const std::overflow_error &) {
                // 数组栈已满：题目约定满栈 PUSH 不输出任何内容。
            }
        } else if (op == "POP") {
            try {
                // pop() 返回 void，先取栈顶再弹出，才能把被弹元素输出。
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
        } else if (op == "SIZE") {
            out << stack.size() << '\n';
        } else if (op == "EMPTY") {
            out << (stack.empty() ? "true" : "false") << '\n';
        } else if (op == "PRINT") {
            stack.printTopToBottom(out);
            out << '\n';
        } else if (op == "CLEAR") {
            stack.clear();
        } else if (op == "SETTOP") {
            T x;
            if (!readValue(cur, op.c_str(), step, x)) {
                return false;
            }
            try {
                stack.setTop(x);
            } catch (const std::out_of_range &) {
                // 空栈没有栈顶可改，静默忽略。
            }
        } else {
            // 未知操作：只报告到 stderr，不影响已产生的标准输出。
            std::cerr << "警告：忽略第 " << step << " 条未知操作 " << op << '\n';
        }
    }
    return true;
}

// 通用流程：读 n / 初始元素 / m，交给具体的栈实例处理。
// makeStack 负责按模式构造栈；linked 模式下容量检查被跳过。
template <typename T, typename MakeStack>
int run(const std::vector<std::string> &tokens, bool hasCapacityLimit,
        MakeStack makeStack, std::ostream &out) {
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
        std::cerr << "输入错误：初始元素个数 " << n << " 超过数组栈容量 "
                  << capacity << '\n';
        return 1;
    }

    auto stack = makeStack(capacity);
    if (!readInitial<T>(cur, n, stack)) {
        return 1;
    }

    std::size_t m = 0;
    if (!readIndex(cur, m, "操作次数 m")) {
        return 1;
    }
    if (!processOps<T>(cur, m, stack, out)) {
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------
// 性能统计：相同规模、相同操作序列，分别测三种栈。
// 计时从“压入初始元素”开始，不含对象构造与数据生成。
// ---------------------------------------------------------------------

struct BenchOp {
    enum class Kind { Push, Pop, Top } kind;
    int value;
};

// 生成保证合法（不会对空栈 pop/top）的操作序列，三种栈面对完全相同的输入。
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
    g_benchSink += sink;  // 防止整个循环被优化掉
    return elapsed;
}

void printPerf(const char *name, std::size_t n, std::size_t ops, double ms) {
    std::cout << "PERF " << name << " n=" << n << " ops=" << ops
              << " time_ms=" << std::fixed << std::setprecision(2) << ms << '\n';
}

void benchmark() {
    const std::size_t n = 100000;
    const std::size_t ops = 200000;

    std::mt19937 rng(20261031u);
    std::vector<int> initial(n);
    for (int &value : initial) {
        value = static_cast<int>(rng() % 1000);
    }
    const std::vector<BenchOp> sequence = makeBenchOps(n, ops, 20261031u);

    // 数组栈需要提前确定容量：初始 n 个元素，之后最多再压入 ops 个。
    {
        dsaac::ArrayStack<int> stack(n + ops);
        printPerf("array_stack", n, ops, runBench(stack, initial, sequence));
    }
    {
        dsaac::LinkedStack<int> stack;
        printPerf("linked_stack", n, ops, runBench(stack, initial, sequence));
    }
    {
        std::stack<int> stack;
        printPerf("std_stack", n, ops, runBench(stack, initial, sequence));
    }
}

}  // namespace

int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    enum class Mode { Auto, Int, String };
    enum class Impl { Array, Linked };
    Mode mode = Mode::Auto;
    Impl impl = Impl::Array;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--int") {
            mode = Mode::Int;
        } else if (arg == "--string") {
            mode = Mode::String;
        } else if (arg == "--array") {
            impl = Impl::Array;
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

    const bool linked = impl == Impl::Linked;
    int rc = 0;
    if (mode == Mode::Int) {
        rc = linked ? run<int>(tokens, false,
                               [](std::size_t) { return dsaac::LinkedStack<int>(); },
                               std::cout)
                    : run<int>(tokens, true,
                               [](std::size_t cap) { return dsaac::ArrayStack<int>(cap); },
                               std::cout);
    } else {
        rc = linked
                 ? run<std::string>(
                       tokens, false,
                       [](std::size_t) { return dsaac::LinkedStack<std::string>(); },
                       std::cout)
                 : run<std::string>(
                       tokens, true,
                       [](std::size_t cap) {
                           return dsaac::ArrayStack<std::string>(cap);
                       },
                       std::cout);
    }
    if (rc != 0) {
        return rc;
    }

    // 功能输出全部结束后，追加性能统计。
    std::cout.flush();
    benchmark();
    return 0;
}