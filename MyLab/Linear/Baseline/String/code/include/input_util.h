// input_util.h
// 字符串实验三个任务共用的输入解析工具。
//
// 设计要点：
//   - 本实验的输入没有“空行”语义，所有字段都是空白分隔的非空 token
//     （第 1 行的初始串用 EMPTY 表示空串），因此一次性 token 化后按序读取即可；
//   - 解析失败不抛异常，只置错误标志并向 stderr 报告，由 main 决定退出方式；
//   - 这里的 std::vector<std::string> 仅用于缓存输入 token，不参与任何
//     数据结构内部的元素存储。
//
// 三个任务的 main_*.cpp 都只 include 本头文件，不依赖其他任务目录。

#ifndef MYLAB_STRING_INPUT_UTIL_H
#define MYLAB_STRING_INPUT_UTIL_H

#include <cstddef>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace mylab {
namespace io {

// 把整个标准输入按空白切分成 token 序列。
inline std::vector<std::string> readAllTokens() {
    std::vector<std::string> tokens;
    std::string token;
    while (std::cin >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// 顺序读取 token 的游标。越界时置 error 标志而不抛异常。
class Cursor {
public:
    explicit Cursor(const std::vector<std::string> &tokens)
        : tokens_(tokens) {}

    // 取下一个 token；已到末尾时返回空串并置 error。
    const std::string &next() {
        static const std::string kEmpty;
        if (pos_ >= tokens_.size()) {
            error_ = true;
            return kEmpty;
        }
        return tokens_[pos_++];
    }

    // 是否发生过越界读取。
    bool error() const { return error_; }

    // 是否已经读完所有 token。
    bool atEnd() const { return pos_ >= tokens_.size(); }

    // 已经成功读取的 token 个数。
    std::size_t position() const { return pos_; }

private:
    const std::vector<std::string> &tokens_;
    std::size_t pos_ = 0;
    bool error_ = false;
};

// 判断 token 是否为纯十进制数字串（不含符号，只有 [0-9]）。
// size_t 不接受负号：调用方若需要识别负数，请先取原始 token 自行判断。
inline bool looksLikeSize(const std::string &token) {
    if (token.empty()) {
        return false;
    }
    for (std::size_t i = 0; i < token.size(); ++i) {
        if (token[i] < '0' || token[i] > '9') {
            return false;
        }
    }
    return true;
}

// 打印统一的错误前缀：“输入错误：[第 N 步的]what”。
inline void reportError(const char *what, std::size_t step) {
    std::cerr << "输入错误：";
    if (step != 0) {
        std::cerr << "第 " << step << " 步的";
    }
    std::cerr << what;
}

// 读取一个字符串 token（原样返回，不做 EMPTY 转换）。
// 失败时向 stderr 报告缺失字段，返回 false，且不修改 out。
inline bool readString(Cursor &cur, std::string &out, const char *what,
                       std::size_t step = 0) {
    const std::string token = cur.next();
    if (cur.error()) {
        reportError(what, step);
        std::cerr << "缺失\n";
        return false;
    }
    out = token;
    return true;
}

// 读取一个非负整数 token（用于长度、下标、操作次数、查询边界等）。
// 只接受纯数字，拒绝空串、正负号与 "12abc" 这类静默截断。
// 失败时向 stderr 报告，返回 false，且不修改 out。
inline bool readSize(Cursor &cur, std::size_t &out, const char *what,
                     std::size_t step = 0) {
    const std::string token = cur.next();
    if (cur.error()) {
        reportError(what, step);
        std::cerr << "缺失\n";
        return false;
    }
    if (!looksLikeSize(token)) {
        reportError(what, step);
        std::cerr << " \"" << token << "\" 不是非负整数\n";
        return false;
    }
    std::size_t used = 0;
    std::size_t value = 0;
    try {
        value = std::stoull(token, &used);
    } catch (const std::exception &) {
        used = 0;
    }
    if (used != token.size()) {
        reportError(what, step);
        std::cerr << " \"" << token << "\" 超出可表示范围\n";
        return false;
    }
    out = value;
    return true;
}

// 读取操作名（PRINT / LENGTH / APPEND ...），纯字符串 token。
inline bool readOp(Cursor &cur, std::string &out) {
    return readString(cur, out, "操作名");
}

// 把表示空串的哨兵 token（约定为 EMPTY）转换为 ""，其余原样返回。
inline std::string decodeEmptyToken(const std::string &token,
                                    const char *sentinel = "EMPTY") {
    return token == sentinel ? std::string() : token;
}

}  // namespace io
}  // namespace mylab

#endif  // MYLAB_STRING_INPUT_UTIL_H