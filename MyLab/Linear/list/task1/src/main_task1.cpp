// main_task1.cpp
// Baseline 任务 1 的驱动程序：从 stdin 读入初始链表与 m 条操作，按格式输出结果。
//
// 输入格式：
//   第 1 行            n            初始链表长度
//   第 2 行            a1 a2 ... an 初始元素（n = 0 时为空行）
//   第 3 行            m            操作次数
//   接下来 m 行        op args...   每行一个操作
//
// 模板实例的选择：默认根据初始元素自动判断——全部是整数时用 LinkedList<int>，
// 否则用 LinkedList<std::string>。也可以用 --int / --string 显式指定
// （例如初始为空、但后续要插入字符串时）。

#include <iostream>
#include <string>

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
    mylab::LinkedList<T> list;

    std::size_t n = 0;
    if (!readIndex(cur, n, "初始长度 n")) {
        return 1;
    }
    for (std::size_t k = 0; k < n; ++k) {
        T x;
        if (!mylab::io::readValue(cur, "初始元素", 0, x)) {
            return 1;
        }
        list.pushBack(x);
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

        if (op == "PUSH_FRONT" || op == "PUSH_BACK") {
            T x;
            if (!mylab::io::readValue(cur, op.c_str(), step, x)) {
                return 1;
            }
            if (op == "PUSH_FRONT") {
                list.pushFront(x);
            } else {
                list.pushBack(x);
            }
        } else if (op == "INSERT") {
            std::size_t i = 0;
            if (!readIndex(cur, i, "INSERT 的下标", step)) {
                return 1;
            }
            T x;
            if (!mylab::io::readValue(cur, op.c_str(), step, x)) {
                return 1;
            }
            // 越界插入不输出任何内容，也不修改链表。
            list.insertAt(i, x);
        } else if (op == "ERASE" || op == "ACCESS") {
            std::size_t i = 0;
            if (!readIndex(cur, i, op.c_str(), step)) {
                return 1;
            }
            if (op == "ERASE") {
                list.eraseAt(i);
            } else {
                T value;
                if (list.access(i, value)) {
                    out << value << '\n';
                } else {
                    out << "ERROR\n";
                }
            }
        } else if (op == "FIND" || op == "CONTAINS") {
            T x;
            if (!mylab::io::readValue(cur, op.c_str(), step, x)) {
                return 1;
            }
            if (op == "FIND") {
                out << list.find(x) << '\n';
            } else {
                out << (list.contains(x) ? "true" : "false") << '\n';
            }
        } else if (op == "SIZE") {
            out << list.size() << '\n';
        } else if (op == "PRINT") {
            list.print(out);
            out << '\n';
        } else if (op == "CLEAR") {
            list.clear();
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
    if (tokens.size() < 2) {
        std::cerr << "输入错误：至少需要初始长度 n 与操作次数 m 两行\n";
        return 1;
    }

    if (mode == Mode::Auto) {
        // 只看初始元素：n 之后的 n 个 token 全部是整数时用 int。
        std::size_t n = 0;
        try {
            std::size_t used = 0;
            n = std::stoull(tokens[0], &used);
            if (used != tokens[0].size()) {
                n = 0;
            }
        } catch (const std::exception &) {
            n = 0;
        }
        bool numeric = true;
        for (std::size_t k = 0; k < n && 1 + k < tokens.size(); ++k) {
            if (!mylab::io::looksLikeInteger(tokens[1 + k])) {
                numeric = false;
                break;
            }
        }
        mode = numeric ? Mode::Int : Mode::String;
    }

    return mode == Mode::Int ? run<int>(tokens, std::cout)
                             : run<std::string>(tokens, std::cout);
}