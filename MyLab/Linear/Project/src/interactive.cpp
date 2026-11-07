// interactive.cpp
// 交互模式：真人游玩用的命令行界面。
//
// 与批处理模式的边界：
//   - 这里的所有提示、菜单、消除播报都只属于交互模式；
//   - Game 层的输出约定不受影响，--script 的逐行结果保持不变。
//
// 界面结构：
//   开局 → 选预设关卡或自选轨道/队列 → 循环（显示局面 → 读命令 → 执行）
//   → 判定 WIN/LOSE → 询问是否再来一局。

#include "interactive.h"

#include <cstddef>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "game.h"

namespace zuma {

namespace {

// 预设关卡。轨道与队列都用 R/G/B/Y 四种常见颜色。
struct Preset {
    const char* name;
    std::vector<char> track;
    std::vector<char> queue;
};

const std::vector<Preset>& presets() {
    static const std::vector<Preset> kPresets = {
        {"入门：一发三连",
         {'R', 'R', 'B', 'B', 'R', 'R'},
         {'B'}},
        {"进阶：中间开花",
         {'R', 'R', 'G', 'G', 'R', 'R'},
         {'B', 'B'}},
        {"连锁：连续塌缩",
         {'R', 'R', 'B', 'B', 'B', 'R', 'R', 'G'},
         {'Y', 'G', 'R'}},
    };
    return kPresets;
}

void printHelp() {
    std::cout << "可用命令：\n"
              << "  fire <pos>   从队首取一球，插入到轨道下标 pos 处\n"
              << "  bomb <pos>   道具：炸掉轨道下标 pos 处的一颗球\n"
              << "  paint <c>    道具：把队首球改成颜色 c（A–Z，不消耗球）\n"
              << "  undo         撤销上一步 fire / bomb / paint\n"
              << "  show         重新显示当前局面\n"
              << "  help         显示本帮助\n"
              << "  quit         退出游戏\n";
}

// 一行颜色 → vector；非法字符（非 A–Z）直接拒绝。
bool parseColors(const std::string& line, std::vector<char>& out) {
    out.clear();
    std::istringstream in(line);
    char c = '\0';
    while (in >> c) {
        if (c < 'A' || c > 'Z') {
            return false;
        }
        out.push_back(c);
    }
    return true;
}

// 解析非负整数；允许前后空白，拒绝负号与多余字符。
bool parseIndex(const std::string& token, std::size_t& value) {
    if (token.empty()) {
        return false;
    }
    unsigned long long result = 0;
    for (const char ch : token) {
        if (ch < '0' || ch > '9') {
            return false;
        }
        result = result * 10 + static_cast<unsigned long long>(ch - '0');
        if (result > std::numeric_limits<std::size_t>::max()) {
            return false;
        }
    }
    value = static_cast<std::size_t>(result);
    return true;
}

void printTrack(const Game& game) {
    if (game.trackSize() == 0) {
        std::cout << "  轨道：(空)\n";
        return;
    }
    // 带上位置编号，方便玩家输入 fire/bomb 的 pos。
    std::cout << "  轨道：" << game.trackStr() << '\n';
    std::cout << "  编号：";
    for (std::size_t i = 0; i < game.trackSize(); ++i) {
        if (i != 0) {
            std::cout << ' ';
        }
        std::cout << i << ':' << game.colorAt(i);
    }
    std::cout << '\n';
}

void printStatus(const Game& game) {
    printTrack(game);
    if (game.queueSize() == 0) {
        std::cout << "  队首：-（队列已空）\n";
    } else {
        std::cout << "  队首：" << game.frontColor() << '\n';
    }
    std::cout << "  队列剩余：" << game.queueSize() << " 颗"
              << "    已发射：" << game.shots() << " 颗"
              << "    可撤销：" << game.undoSize() << " 步\n";
}

// 播报一次操作的消除明细，让连锁消除可见。
void printEliminations(const Game& game) {
    for (const Game::Elimination& e : game.lastEliminations()) {
        std::cout << "  消除了 " << e.count << " 个 " << e.color << " 球\n";
    }
}

bool readNonEmptyLine(std::string& line) {
    if (!std::getline(std::cin, line)) {
        return false;
    }
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return true;
}

// 读一行颜色；空行表示空序列。语法错误时重新询问。
bool askColors(const std::string& prompt, std::vector<char>& out) {
    std::string line;
    for (;;) {
        std::cout << prompt;
        std::cout.flush();
        if (!readNonEmptyLine(line)) {
            return false;  // EOF：退出
        }
        if (line.find_first_not_of(" \t") == std::string::npos) {
            out.clear();
            return true;
        }
        if (parseColors(line, out)) {
            return true;
        }
        std::cout << "颜色只能是 A–Z 的大写字母，空格分隔。请重新输入。\n";
    }
}

// 开局设置：选预设关卡或自己输入。
bool setupGame(Game& game) {
    const std::vector<Preset>& list = presets();
    std::cout << "\n请选择开局方式：\n"
              << "  0  自定义轨道与队列\n";
    for (std::size_t i = 0; i < list.size(); ++i) {
        std::cout << "  " << i + 1 << "  " << list[i].name << "\n";
    }

    std::string line;
    for (;;) {
        std::cout << "请输入编号：";
        std::cout.flush();
        if (!readNonEmptyLine(line)) {
            return false;
        }
        std::size_t choice = 0;
        if (!parseIndex(line, choice)) {
            std::cout << "请输入 0–" << list.size() << " 之间的数字。\n";
            continue;
        }
        if (choice == 0) {
            std::vector<char> track;
            std::vector<char> queue;
            std::cout << "初始轨道（一行颜色，如 R R B B R R；空行表示空轨道）：\n";
            if (!askColors("  > ", track)) {
                return false;
            }
            std::cout << "待发射队列（一行颜色，队首在前；空行表示没有球）：\n";
            if (!askColors("  > ", queue)) {
                return false;
            }
            game.init(track, queue);
            return true;
        }
        if (choice <= list.size()) {
            game.init(list[choice - 1].track, list[choice - 1].queue);
            return true;
        }
        std::cout << "请输入 0–" << list.size() << " 之间的数字。\n";
    }
}

// 一局结束后的处理：打印结果，询问是否再来一局。
// 返回 true 表示再来一局，false 表示退出。
bool askReplay() {
    std::string line;
    for (;;) {
        std::cout << "是否再来一局？(y/n)：";
        std::cout.flush();
        if (!readNonEmptyLine(line)) {
            return false;
        }
        if (line == "y" || line == "Y" || line == "yes") {
            return true;
        }
        if (line == "n" || line == "N" || line == "no" || line.empty()) {
            return false;
        }
        std::cout << "请输入 y 或 n。\n";
    }
}

}  // namespace

int runInteractive() {
    std::cout << "========== 祖玛 ==========\n"
              << "目标：把轨道上的球清空。插入或炸掉球后，连续 3 个及以上同色球会自动消除，\n"
              << "消除后左右两段合拢还可能继续连锁消除。\n";

    Game game;
    bool playing = true;
    while (playing) {
        if (!setupGame(game)) {
            std::cout << "\n再见。\n";
            return 0;
        }

        printHelp();
        if (game.queueSize() == 0 && game.trackSize() > 0) {
            std::cout << "\n注意：待发射队列为空，只能用 bomb 清球，或用 quit 退出。\n";
        }
        printStatus(game);

        std::string line;
        while (!game.won() && !game.lost()) {
            std::cout << "\n> ";
            std::cout.flush();
            if (!readNonEmptyLine(line)) {
                std::cout << "\n再见。\n";
                return 0;
            }

            std::istringstream in(line);
            std::string op;
            in >> op;
            if (op.empty()) {
                continue;
            }

            if (op == "quit" || op == "exit" || op == "q") {
                std::cout << "再见。\n";
                return 0;
            }
            if (op == "help") {
                printHelp();
                continue;
            }
            if (op == "show") {
                printStatus(game);
                continue;
            }
            if (op == "fire" || op == "bomb") {
                std::string arg;
                std::size_t pos = 0;
                if (!(in >> arg) || !parseIndex(arg, pos)) {
                    std::cout << "用法：" << op << " <pos>，pos 是非负整数。\n";
                    continue;
                }
                std::string track;
                bool won = false;
                bool lost = false;
                const bool ok = (op == "fire")
                                    ? game.fire(pos, &track, &won, &lost)
                                    : game.bomb(pos, &track, &won);
                if (!ok) {
                    if (op == "fire" && game.queueSize() == 0) {
                        std::cout << "待发射队列已空，没有球可发了。\n";
                    } else if (op == "fire") {
                        std::cout << "位置不合法：可插入范围为 0–" << game.trackSize() << "。\n";
                    } else {
                        std::cout << "位置不合法：可炸范围为 0–"
                                  << (game.trackSize() == 0 ? 0 : game.trackSize() - 1)
                                  << "。\n";
                    }
                    continue;
                }
                printEliminations(game);
                printStatus(game);
                if (won || lost) {
                    break;
                }
                continue;
            }
            if (op == "paint") {
                std::string color;
                if (!(in >> color) || color.size() != 1 ||
                    color[0] < 'A' || color[0] > 'Z') {
                    std::cout << "用法：paint <c>，c 是 A–Z 的单个大写字母。\n";
                    continue;
                }
                std::string queue;
                if (!game.paint(color[0], &queue)) {
                    std::cout << "待发射队列为空，没有球可以改色。\n";
                    continue;
                }
                std::cout << "队首已改为 " << color[0] << "。\n";
                printStatus(game);
                continue;
            }
            if (op == "undo") {
                std::string track;
                if (!game.undo(&track)) {
                    std::cout << "没有可撤销的操作。\n";
                    continue;
                }
                std::cout << "已撤销上一步。\n";
                printStatus(game);
                continue;
            }
            std::cout << "未知命令，输入 help 查看可用命令。\n";
        }

        if (game.won()) {
            std::cout << "\n*** WIN！轨道已清空，共发射 " << game.shots() << " 颗球。***\n";
        } else {
            std::cout << "\n*** LOSE。球用完了，轨道上还剩 " << game.trackSize()
                      << " 颗球。***\n";
        }
        playing = askReplay();
    }

    std::cout << "再见。\n";
    return 0;
}

}  // namespace zuma