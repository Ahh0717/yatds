// main_task2.cpp —— 任务 2 驱动：朴素匹配 / KMP / nextval
//
// 输入（按行序）：
//   第 1 行  text     主串
//   第 2 行  pattern  模式串
//   第 3 行  m        操作次数
//   之后 m 行         每行一个操作：
//                     NEXT / NEXTVAL / NAIVE / KMP / KMP_NEXTVAL / ALL
//
// 输出：仅操作对应的结果写 stdout，顺序与输入一致；全部功能输出之后
// 追加三行 PERF 性能统计（test 比对时忽略 PERF 行）。
//
// 说明：本阶段输入没有“空串用 EMPTY 表示”的约定，text / pattern 按原样
// token 读取；空串只能通过“缺省行”表达，实际评测数据不会出现。

#include "input_util.h"
#include "match.hpp"

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
using mylab::io::Cursor;

// 打印一个整数数组，单个空格分隔；空数组输出空行。
void printArray(const std::vector<int>& arr) {
    for (std::size_t i = 0; i < arr.size(); ++i) {
        if (i != 0) std::cout << ' ';
        std::cout << arr[i];
    }
    std::cout << '\n';
}

// 打印匹配下标；-1 表示不存在。
void printIndex(int index) {
    std::cout << index << '\n';
}

// 对一次匹配调用计时（微秒级精度足够，统一换回毫秒）。
template <typename Fn>
double timeMs(Fn fn) {
    const auto begin = Clock::now();
    fn();
    const auto end = Clock::now();
    return std::chrono::duration<double, std::milli>(end - begin).count();
}

// ---------------------------------------------------------------------
// PERF 性能统计
//
// 题目要求“对三种匹配各计时一次”，因此每种算法只执行一次、不做重复取最小。
// 结果累加到 volatile 汇总量，防止整个调用被编译器优化掉。
// ---------------------------------------------------------------------

volatile int g_benchSink = 0;

void emitPerf(const char* name, std::size_t n, std::size_t m, double ms) {
    std::cout << "PERF " << name << " n=" << n << " m=" << m
              << " time_ms=" << std::fixed << std::setprecision(3) << ms << '\n';
}

void runPerf(const std::string& text, const std::string& pattern) {
    const double tNaive = timeMs([&] { g_benchSink += dsaac::naiveMatch(text, pattern); });
    const double tKmp = timeMs([&] { g_benchSink += dsaac::kmpMatch(text, pattern); });
    const double tNv = timeMs([&] { g_benchSink += dsaac::kmpMatchNextVal(text, pattern); });

    emitPerf("naive", text.size(), pattern.size(), tNaive);
    emitPerf("kmp", text.size(), pattern.size(), tKmp);
    emitPerf("kmp_nextval", text.size(), pattern.size(), tNv);
}

// ---------------------------------------------------------------------
// 操作分发
// ---------------------------------------------------------------------

// 执行 m 条操作；输入格式不正确时返回 false，由 main 以非零码退出。
bool runOperations(Cursor& cur, const std::string& text,
                   const std::string& pattern, std::size_t opCount) {
    std::string op;

    // next / nextval 只取决于模式串，且同一次运行内模式不变，预计算一次。
    const std::vector<int> next = dsaac::getNext(pattern);
    const std::vector<int> nextval = dsaac::getNextVal(pattern);

    for (std::size_t step = 1; step <= opCount; ++step) {
        if (!mylab::io::readOp(cur, op)) {
            return false;
        }

        if (op == "NEXT") {
            printArray(next);
        } else if (op == "NEXTVAL") {
            printArray(nextval);
        } else if (op == "NAIVE") {
            printIndex(dsaac::naiveMatch(text, pattern));
        } else if (op == "KMP") {
            printIndex(dsaac::kmpMatch(text, pattern));
        } else if (op == "KMP_NEXTVAL") {
            printIndex(dsaac::kmpMatchNextVal(text, pattern));
        } else if (op == "ALL") {
            printIndex(dsaac::naiveMatch(text, pattern));
            printIndex(dsaac::kmpMatch(text, pattern));
            printIndex(dsaac::kmpMatchNextVal(text, pattern));
        } else {
            std::cerr << "输入错误：第 " << step << " 步的操作 \"" << op
                      << "\" 不认识\n";
            return false;
        }
    }
    return true;
}

}  // namespace

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    const std::vector<std::string> tokens = mylab::io::readAllTokens();
    Cursor cur(tokens);

    std::string text;
    std::string pattern;
    std::size_t opCount = 0;
    if (!mylab::io::readString(cur, text, "主串") ||
        !mylab::io::readString(cur, pattern, "模式串") ||
        !mylab::io::readSize(cur, opCount, "操作次数")) {
        return 1;
    }

    if (!runOperations(cur, text, pattern, opCount)) {
        return 1;
    }

    runPerf(text, pattern);
    return 0;
}