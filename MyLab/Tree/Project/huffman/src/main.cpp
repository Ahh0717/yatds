// main.cpp
// 命令行入口：只负责解析参数、读写文件、转发错误，不含任何压缩逻辑。
//
// 用法：
//   huffman compress   <input> <output>
//   huffman decompress <input> <output>
//   huffman stats      <input>
//   huffman help
//
// stats 读原始文本，在内存里压缩一遍，输出固定五行：
//   original   原始字节数
//   compressed 压缩后总字节数（含文件头）
//   tokens     字面量 + 回溯的个数（不含 EOS）
//   literals   字面量 token 个数
//   matches    回溯 token 个数
//
// 输入输出均按二进制读写（压缩文件含 NUL 与高位字节，不能用文本模式）。
// 退出码：0 成功（含无参数 / help 打印用法）；2 参数错误或运行失败。

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "deflate.h"

// Windows 下把控制台切到 UTF-8，避免中文错误信息在 GBK 代码页下乱码。
#ifdef _WIN32
#include <windows.h>
#endif

namespace {

const char *const kUsage =
    "用法:\n"
    "  huffman compress   <input> <output>   压缩\n"
    "  huffman decompress <input> <output>   解压\n"
    "  huffman stats      <input>           输出压缩统计\n"
    "  huffman help                         显示本帮助\n"
    "示例:\n"
    "  huffman compress input.txt output.bin\n";

std::vector<std::uint8_t> read_binary_file(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("无法打开输入文件: " + path);
    }
    std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(in)),
                                   std::istreambuf_iterator<char>());
    if (in.bad()) {
        throw std::runtime_error("读取输入文件失败: " + path);
    }
    return data;
}

void write_binary_file(const std::string &path,
                       const std::vector<std::uint8_t> &data) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("无法打开输出文件: " + path);
    }
    out.write(reinterpret_cast<const char *>(data.data()),
              static_cast<std::streamsize>(data.size()));
    if (!out) {
        throw std::runtime_error("写入输出文件失败: " + path);
    }
}

// stats：读原文，在内存里压缩一遍，打印五行统计。
void print_stats(const std::string &text) {
    const std::vector<dsaac::Token> tokens = dsaac::tokenize(text);
    const std::vector<std::uint8_t> packed = dsaac::compress(text);

    std::size_t literals = 0;
    std::size_t matches = 0;
    for (const dsaac::Token &token : tokens) {
        if (token.kind == dsaac::TokenKind::Literal) {
            ++literals;
        } else if (token.kind == dsaac::TokenKind::Match) {
            ++matches;
        }
    }

    std::cout << "original " << text.size() << '\n';
    std::cout << "compressed " << packed.size() << '\n';
    std::cout << "tokens " << literals + matches << '\n';
    std::cout << "literals " << literals << '\n';
    std::cout << "matches " << matches << '\n';
}

int run(int argc, char **argv) {
    // 无参数、help、--help 都打印用法，退出码 0。
    if (argc == 1 ||
        (argc == 2 && (std::string(argv[1]) == "help" ||
                       std::string(argv[1]) == "--help"))) {
        std::cout << kUsage;
        return 0;
    }

    const std::string command = argv[1];
    const bool is_known = command == "compress" || command == "decompress" ||
                          command == "stats";
    if (!is_known) {
        std::cerr << "未知命令: " << command << "\n\n" << kUsage;
        return 2;
    }

    const int expected = (command == "stats") ? 3 : 4;
    if (argc != expected) {
        std::cerr << kUsage;
        return 2;
    }

    const std::string input_path = argv[2];

    if (command == "stats") {
        const std::vector<std::uint8_t> raw = read_binary_file(input_path);
        print_stats(std::string(raw.begin(), raw.end()));
        return 0;
    }

    const std::string output_path = argv[3];
    if (command == "compress") {
        const std::vector<std::uint8_t> raw = read_binary_file(input_path);
        const std::string text(raw.begin(), raw.end());
        write_binary_file(output_path, dsaac::compress(text));
    } else {
        const std::vector<std::uint8_t> file = read_binary_file(input_path);
        const std::string text = dsaac::decompress(file);
        write_binary_file(output_path,
                          std::vector<std::uint8_t>(text.begin(), text.end()));
    }
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    try {
        return run(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 2;
    }
}