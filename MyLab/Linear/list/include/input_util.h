// input_util.h
// 两个驱动程序共用的输入解析工具：按空白切分后逐个 token 读取，
// 越界与格式错误都通过返回值报告，不抛异常给调用方。
//
// 这里的 std::vector<std::string> 只用来缓存输入 token，与实验要求中
// “不得用 STL 容器存数据元素”的限制无关——链表/动态数组内部仍然只用自己的存储。

#ifndef MYLAB_INPUT_UTIL_H
#define MYLAB_INPUT_UTIL_H

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace mylab {
namespace io {

// 判断一个 token 是否为十进制整数（允许前导 '+' / '-'）。
inline bool looksLikeInteger(const std::string &s) {
    if (s.empty()) {
        return false;
    }
    const std::size_t start = (s[0] == '+' || s[0] == '-') ? 1 : 0;
    if (start == s.size()) {
        return false;
    }
    for (std::size_t i = start; i < s.size(); ++i) {
        if (s[i] < '0' || s[i] > '9') {
            return false;
        }
    }
    return true;
}

// 把整个标准输入按空白切分成 token 序列。
inline std::vector<std::string> readAllTokens() {
    std::vector<std::string> tokens;
    std::string token;
    while (std::cin >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// 顺序读取 token 的游标；越界时置 error 标志而不是抛异常。
class Cursor {
public:
    explicit Cursor(const std::vector<std::string> &tokens)
        : tokens_(tokens), pos_(0) {}

    const std::string &next() {
        static const std::string kEmpty;
        if (pos_ >= tokens_.size()) {
            error_ = true;
            return kEmpty;
        }
        return tokens_[pos_++];
    }

    bool error() const { return error_; }

private:
    const std::vector<std::string> &tokens_;
    std::size_t pos_;
    bool error_ = false;
};

// 读取一个非负十进制下标；格式非法或缺失时向 stderr 报告并返回 false。
inline bool readIndex(Cursor &cur, std::size_t &out, const char *what,
                      std::size_t step = 0) {
    const std::string &token = cur.next();
    if (cur.error()) {
        std::cerr << "输入错误：缺少" << what << '\n';
        return false;
    }
    std::size_t used = 0;
    try {
        out = std::stoull(token, &used);
    } catch (const std::exception &) {
        used = 0;
    }
    if (used != token.size()) {
        std::cerr << "输入错误：";
        if (step != 0) {
            std::cerr << "第 " << step << " 条操作里";
        }
        std::cerr << what << " \"" << token << "\" 不是非负整数\n";
        return false;
    }
    return true;
}

// 把 token 解析成元素类型 T；int 与 std::string 各有一份特化。
template <typename T>
struct TokenValue;

template <>
struct TokenValue<int> {
    static int parse(const std::string &token) {
        std::size_t used = 0;
        const int value = std::stoi(token, &used);
        // 只接受完整解析的 token，杜绝 "12abc" 这类静默截断。
        if (used != token.size()) {
            throw std::invalid_argument("非整数：" + token);
        }
        return value;
    }
};

template <>
struct TokenValue<std::string> {
    static std::string parse(const std::string &token) { return token; }
};

// 读取一个元素值；失败时报告“第几步的哪个操作”后返回 false。
template <typename T>
bool readValue(Cursor &cur, const char *op, std::size_t step, T &out) {
    const std::string &token = cur.next();
    if (cur.error()) {
        std::cerr << "输入错误：第 " << step << " 条操作 " << op << " 缺少参数\n";
        return false;
    }
    try {
        out = TokenValue<T>::parse(token);
        return true;
    } catch (const std::exception &) {
        std::cerr << "输入错误：第 " << step << " 条操作 " << op << " 的参数 \""
                  << token << "\" 不是合法的元素值\n";
        return false;
    }
}

}  // namespace io
}  // namespace mylab

#endif  // MYLAB_INPUT_UTIL_H