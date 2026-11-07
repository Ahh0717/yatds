// main.cpp
// 祖玛驱动：
//   ./zuma --script < input.txt   批处理模式：按固定格式读脚本、输出确定结果，
//                                 供 test 自动比对；不输出任何提示语。
//   ./zuma                        交互模式（后续阶段实现）。
//
// 批量模式输入格式（按行组织）：
//   n            初始轨道球数
//   颜色...       n 个颜色（头 → 尾），n = 0 时该行为空行
//   q            待发射队列球数
//   颜色...       q 个颜色（队首 → 队尾），q = 0 时该行为空行
//   m            命令条数
//   命令...       每行一条：FIRE pos / BOMB pos / UNDO / PAINT c / SHOW
//
// 输出约定：
//   FIRE  成功 → 轨道行；轨道清空 → 追加 WIN；队列空且轨道非空 → 追加 LOSE
//   BOMB  成功 → 轨道行；轨道清空 → 追加 WIN
//   UNDO  成功 → 恢复后的轨道行
//   PAINT 成功 → 队列行
//   SHOW       → 轨道行 + 队列行
//   非法操作   → ERROR（不改变任何状态，也不产生撤销记录）
//
// 解析策略：整个输入按行切分，每条命令独立解析。这样一条命令格式错误
// 只影响它自己，不会把后续命令一起吞掉。

#include <cstddef>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "game.h"
#include "interactive.h"

namespace {

using namespace zuma;

// 去掉行尾的 '\r'，兼容 Windows 换行。
void stripCr(std::string& line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
}

// 读取下一行；已到文件末尾返回 false。
bool readLine(std::string& line) {
    if (!std::getline(std::cin, line)) {
        return false;
    }
    stripCr(line);
    return true;
}

// 解析非负整数：整串必须都是数字，否则失败（拒绝 "-1"、"1x"、空串）。
bool parseCount(const std::string& text, std::size_t& value) {
    if (text.empty()) {
        return false;
    }
    unsigned long long result = 0;
    for (const char ch : text) {
        if (ch < '0' || ch > '9') {
            return false;
        }
        result = result * 10 + static_cast<unsigned long long>(ch - '0');
        if (result > static_cast<unsigned long long>(
                          std::numeric_limits<std::size_t>::max())) {
            return false;
        }
    }
    value = static_cast<std::size_t>(result);
    return true;
}

bool isUpperLetter(char c) {
    return c >= 'A' && c <= 'Z';
}

// 从一行里按空白切分读取至多 count 个颜色。
// 样例保证颜色数量与 n / q 一致，因此缺读只可能来自样例自身格式错误。
std::vector<char> parseColors(const std::string& line, std::size_t count) {
    std::vector<char> colors;
    colors.reserve(count);
    std::istringstream in(line);
    for (std::size_t i = 0; i < count; ++i) {
        char c = '\0';
        if (!(in >> c)) {
            break;
        }
        colors.push_back(c);
    }
    return colors;
}

// 读取一行颜色。count == 0 时该行按格式是空行，但读掉它仍然必要，
// 否则后面的 q / m 会错位；行不存在时视作空行。
std::vector<char> readColorLine(std::size_t count) {
    std::string line;
    readLine(line);
    return parseColors(line, count);
}

// 一条命令拆成若干空白分隔的 token。
std::vector<std::string> splitTokens(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream in(line);
    std::string token;
    while (in >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int runScript() {
    std::string line;

    std::size_t n = 0;
    if (!readLine(line) || !parseCount(line, n)) {
        return 0;  // 空输入或格式错误：什么也不做
    }
    const std::vector<char> initial_track = readColorLine(n);

    std::size_t q = 0;
    if (!readLine(line) || !parseCount(line, q)) {
        return 0;
    }
    const std::vector<char> initial_queue = readColorLine(q);

    Game game;
    game.init(initial_track, initial_queue);

    std::size_t m = 0;
    if (!readLine(line) || !parseCount(line, m)) {
        return 0;
    }

    for (std::size_t i = 0; i < m; ++i) {
        if (!readLine(line)) {
            std::cout << "ERROR\n";
            continue;
        }

        const std::vector<std::string> tokens = splitTokens(line);
        if (tokens.empty()) {
            std::cout << "ERROR\n";
            continue;
        }
        const std::string& op = tokens[0];

        if (op == "FIRE") {
            std::size_t pos = 0;
            if (tokens.size() != 2 || !parseCount(tokens[1], pos)) {
                std::cout << "ERROR\n";
                continue;
            }
            std::string track;
            bool won = false;
            bool lost = false;
            if (!game.fire(pos, &track, &won, &lost)) {
                std::cout << "ERROR\n";
                continue;
            }
            std::cout << track << '\n';
            if (won) {
                std::cout << "WIN\n";
            } else if (lost) {
                std::cout << "LOSE\n";
            }
        } else if (op == "BOMB") {
            std::size_t pos = 0;
            if (tokens.size() != 2 || !parseCount(tokens[1], pos)) {
                std::cout << "ERROR\n";
                continue;
            }
            std::string track;
            bool won = false;
            if (!game.bomb(pos, &track, &won)) {
                std::cout << "ERROR\n";
                continue;
            }
            std::cout << track << '\n';
            if (won) {
                std::cout << "WIN\n";
            }
        } else if (op == "UNDO") {
            std::string track;
            if (tokens.size() != 1 || !game.undo(&track)) {
                std::cout << "ERROR\n";
                continue;
            }
            std::cout << track << '\n';
        } else if (op == "PAINT") {
            std::string queue;
            if (tokens.size() != 2 || tokens[1].size() != 1 ||
                !isUpperLetter(tokens[1][0]) || !game.paint(tokens[1][0], &queue)) {
                std::cout << "ERROR\n";
                continue;
            }
            std::cout << queue << '\n';
        } else if (op == "SHOW") {
            if (tokens.size() != 1) {
                std::cout << "ERROR\n";
                continue;
            }
            std::cout << game.trackStr() << '\n';
            std::cout << game.queueStr() << '\n';
        } else {
            std::cout << "ERROR\n";
        }
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    bool script = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--script" || arg == "-s") {
            script = true;
        }
    }

    if (script) {
        return runScript();
    }

    return zuma::runInteractive();
}