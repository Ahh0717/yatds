// main_task1.cpp —— 任务 1 驱动：顺序字符串 MyString
//
// 输入（token 化后按序读取）：
//   第 1 行  s    初始字符串，空串用 EMPTY 表示
//   第 2 行  m    操作次数
//   之后 m 行     每行一个操作：PRINT / LENGTH / APPEND t / SUBSTR pos len /
//                 FIND t / REPLACE old new / COMPARE t / CLEAR
//
// 输出：仅“产生输出的操作”写 stdout，顺序与输入一致；全部功能输出之后
// 追加三行 PERF 性能统计（test 比对时忽略 PERF 行）。
//
// 说明：EMPTY 只在第 1 行有“空串”语义；操作参数都是原样 token。

#include "input_util.h"
#include "mystring.hpp"

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
using mylab::io::Cursor;

// 打印当前串：空串按约定输出 EMPTY。
void printString(const MyString& s) {
    if (s.empty()) {
        std::cout << "EMPTY\n";
    } else {
        std::cout << s.c_str() << '\n';
    }
}

// ---------------------------------------------------------------------
// PERF 性能统计
//
// 三种对象执行完全相同的工作：从空串开始，连续 append 同一个 16 字符
// 片段 kBenchOps 次，累计 n = kBenchOps * 16 = 3,200,000 个字符。
//   my_string         : MyString，只靠倍增扩容
//   std_string        : std::string，靠标准库自身扩容
//   my_string_reserve : MyString，先 reserve(n) 再 append
//
// 计时方法（重要）：三类对象轮转着跑，第 1 轮不计时（预热堆与分配器），
// 之后每轮取最小值。原因是首次触达几 MB 新内存要付缺页中断的代价，
// 谁先跑谁吃亏；固定顺序会让领先者把差距归因到算法本身，结论就错了。
// 累加到 volatile 变量，防止整段循环被编译器优化掉。
// ---------------------------------------------------------------------

const std::size_t kBenchOps = 200000;
const char* const kBenchChunk = "0123456789ABCDEF";  // 16 个字符
const std::size_t kBenchFragLen = 16;
const std::size_t kBenchTotal = kBenchOps * kBenchFragLen;
const int kBenchRounds = 7;  // 第 1 轮热身，其余取最小

enum BenchKind { kMyGrow = 0, kStdGrow = 1, kMyReserved = 2, kBenchKinds = 3 };

volatile std::size_t g_benchSink = 0;

// 跑一遍指定对象的工作负载，返回 append 循环的耗时（毫秒）。
// 计时只覆盖循环本身，不含构造与 reserve 的开销。
double benchOnce(BenchKind kind) {
    const auto begin = Clock::now();
    if (kind == kStdGrow) {
        std::string s;
        const std::string piece(kBenchChunk);
        for (std::size_t i = 0; i < kBenchOps; ++i) {
            s += piece;
        }
        g_benchSink += s.size();
    } else {
        MyString s;
        if (kind == kMyReserved) {
            s.reserve(kBenchTotal);
        }
        const MyString piece(kBenchChunk);
        for (std::size_t i = 0; i < kBenchOps; ++i) {
            s.append(piece);
        }
        g_benchSink += s.length();
    }
    const auto end = Clock::now();
    return std::chrono::duration<double, std::milli>(end - begin).count();
}

void emitPerf(const char* name, double ms) {
    std::cout << "PERF " << name << " n=" << kBenchTotal << " ops=" << kBenchOps
              << " time_ms=" << std::fixed << std::setprecision(2) << ms << '\n';
}

void runPerf() {
    // 初值取一个必然被首次测量覆盖的大数。
    double best[kBenchKinds] = {1e18, 1e18, 1e18};
    for (int round = 0; round < kBenchRounds; ++round) {
        for (int i = 0; i < kBenchKinds; ++i) {
            const BenchKind kind = static_cast<BenchKind>((i + round) % kBenchKinds);
            const double ms = benchOnce(kind);
            if (round == 0) {
                continue;  // 热身轮不记成绩
            }
            const int slot = static_cast<int>(kind);
            if (ms < best[slot]) {
                best[slot] = ms;
            }
        }
    }
    emitPerf("my_string", best[kMyGrow]);
    emitPerf("std_string", best[kStdGrow]);
    emitPerf("my_string_reserve", best[kMyReserved]);
}

// ---------------------------------------------------------------------
// 操作分发
// ---------------------------------------------------------------------

// 读取一个字符串操作参数，直接构造 MyString（省掉一次临时对象拷贝）。
bool readMyString(Cursor& cur, MyString& out, const char* what,
                  std::size_t step) {
    std::string token;
    if (!mylab::io::readString(cur, token, what, step)) {
        return false;
    }
    out = MyString(token.c_str());
    return true;
}

// 执行 m 条操作；输入格式不正确时返回 false，由 main 以非零码退出。
bool runOperations(Cursor& cur, MyString& s, std::size_t opCount) {
    std::string op;
    MyString arg1;
    MyString arg2;

    for (std::size_t step = 1; step <= opCount; ++step) {
        if (!mylab::io::readOp(cur, op)) {
            return false;
        }

        if (op == "PRINT") {
            printString(s);
        } else if (op == "LENGTH") {
            std::cout << s.length() << '\n';
        } else if (op == "APPEND") {
            if (!readMyString(cur, arg1, "APPEND 的参数", step)) {
                return false;
            }
            s.append(arg1);
        } else if (op == "SUBSTR") {
            std::size_t pos = 0;
            std::size_t len = 0;
            if (!mylab::io::readSize(cur, pos, "SUBSTR 的 pos", step) ||
                !mylab::io::readSize(cur, len, "SUBSTR 的 len", step)) {
                return false;
            }
            if (pos >= s.length()) {
                std::cout << "ERROR\n";
            } else {
                printString(s.substr(pos, len));
            }
        } else if (op == "FIND") {
            if (!readMyString(cur, arg1, "FIND 的参数", step)) {
                return false;
            }
            const std::size_t pos = s.find(arg1);
            if (pos == MyString::npos) {
                std::cout << "-1\n";
            } else {
                std::cout << pos << '\n';
            }
        } else if (op == "REPLACE") {
            if (!readMyString(cur, arg1, "REPLACE 的 old", step) ||
                !readMyString(cur, arg2, "REPLACE 的 new", step)) {
                return false;
            }
            s.replace(arg1, arg2);
        } else if (op == "COMPARE") {
            if (!readMyString(cur, arg1, "COMPARE 的参数", step)) {
                return false;
            }
            std::cout << s.compare(arg1) << '\n';
        } else if (op == "CLEAR") {
            s.clear();
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
    // 按实验约定统一以 utf-8 输出，Windows 控制台也不会因编码问题报错。
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    const std::vector<std::string> tokens = mylab::io::readAllTokens();
    Cursor cur(tokens);

    std::string head;
    std::size_t opCount = 0;
    if (!mylab::io::readString(cur, head, "初始字符串") ||
        !mylab::io::readSize(cur, opCount, "操作次数")) {
        return 1;
    }

    MyString s(mylab::io::decodeEmptyToken(head).c_str());
    if (!runOperations(cur, s, opCount)) {
        return 1;
    }

    runPerf();
    return 0;
}