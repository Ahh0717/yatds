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

#include <iostream>
#include <string>

#include "dynamic_array.h"
#include "input_util.h"

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

    return mode == Mode::Int ? run<int>(tokens, std::cout)
                             : run<std::string>(tokens, std::cout);
}