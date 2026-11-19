// deflate.cpp
// 简化 Deflate 的实现。
//
// 当前进度：
//   1. tokenize：窗口 64、最长 18、最短 3，距离小者优先       —— 已实现
//   2. collect_frequencies：字面量 / 长度符号计数               —— 已实现
//   3. serialize_header：小端文件头                            —— 已实现
//      parse_header                                            —— 待实现
//   4. compress：分词 -> 频次 -> 建树 -> 文件头 + 比特流       —— 已实现
//      decompress                                              —— 待实现
// BitWriter / BitReader 已在 deflate.h 内联实现。

#include "deflate.h"

#include "huffman.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace dsaac {
namespace {

using FrequencyTable = std::array<std::uint32_t, kSymbolCount>;

// token 对应的 273 项频次表（EOS 已含在 tokens 里）。
FrequencyTable frequency_table_of(const std::vector<Token> &tokens) {
    FrequencyTable freq{};
    for (const Token &token : tokens) {
        switch (token.kind) {
        case TokenKind::Literal:
            ++freq[token.literal];
            break;
        case TokenKind::Match:
            // 长度符号 = 256 + (len - 3)。
            ++freq[static_cast<std::size_t>(kLengthSymbolBase) +
                   static_cast<std::size_t>(token.length) - kMinMatchLength];
            break;
        case TokenKind::Eos:
            ++freq[kEosSymbol];
            break;
        }
    }
    return freq;
}

std::uint16_t read_u16(const std::vector<std::uint8_t> &buf, std::size_t offset) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(buf[offset]) |
        static_cast<std::uint16_t>(buf[offset + 1] << 8));
}

std::uint32_t read_u32(const std::vector<std::uint8_t> &buf, std::size_t offset) {
    return static_cast<std::uint32_t>(buf[offset]) |
           (static_cast<std::uint32_t>(buf[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(buf[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(buf[offset + 3]) << 24);
}

void append_u16(std::vector<std::uint8_t> &out, std::uint16_t value) {
    out.push_back(static_cast<std::uint8_t>(value & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFFu));
}

void append_u32(std::vector<std::uint8_t> &out, std::uint32_t value) {
    out.push_back(static_cast<std::uint8_t>(value & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFFu));
}

// 按字符串顺序写一条 '0'/'1' 编码。
void put_code(BitWriter &writer, const std::string &code) {
    for (const char bit : code) {
        writer.put(bit == '1' ? 1u : 0u, 1);
    }
}

} // namespace

std::vector<Token> tokenize(const std::string &input) {
    const std::size_t size = input.size();
    std::vector<Token> tokens;
    tokens.reserve(size / 2 + 1);

    std::size_t i = 0;
    while (i < size) {
        // 窗口是 [max(0, i-64), i-1]；最多能比到输入末尾。
        const std::size_t max_distance = std::min(kMaxDistance, i);
        const std::size_t max_length = std::min(kMaxMatchLength, size - i);

        std::size_t best_length = 0;
        std::size_t best_distance = 0;

        // 距离从小到大扫：只有严格更长才替换，长度相同自然保留小距离，
        // 结果与扫描顺序一起确定下来。
        for (std::size_t distance = 1; distance <= max_distance; ++distance) {
            const std::size_t source = i - distance;
            std::size_t length = 0;
            // 允许重叠：比较过程中 source + length 可以追上并越过 i，
            // 因为复制时这些字节已经由本次匹配自己产出。
            while (length < max_length &&
                   input[source + length] == input[i + length]) {
                ++length;
            }
            if (length > best_length) {
                best_length = length;
                best_distance = distance;
                if (best_length == max_length) {
                    break; // 已达本轮上限，不可能再长。
                }
            }
        }

        if (best_length >= kMinMatchLength) {
            Token token{};
            token.kind = TokenKind::Match;
            token.literal = 0;
            token.length = static_cast<std::uint8_t>(best_length);
            token.distance = static_cast<std::uint8_t>(best_distance);
            tokens.push_back(token);
            i += best_length;
        } else {
            Token token{};
            token.kind = TokenKind::Literal;
            token.literal = static_cast<std::uint8_t>(
                static_cast<unsigned char>(input[i]));
            token.length = 0;
            token.distance = 0;
            tokens.push_back(token);
            i += 1;
        }
    }

    Token eos{};
    eos.kind = TokenKind::Eos;
    eos.literal = 0;
    eos.length = 0;
    eos.distance = 0;
    tokens.push_back(eos);
    return tokens;
}

std::vector<SymbolFrequency>
collect_frequencies(const std::vector<Token> &tokens) {
    const FrequencyTable freq = frequency_table_of(tokens);

    std::vector<SymbolFrequency> result;
    result.reserve(freq.size());
    for (std::size_t symbol = 0; symbol < freq.size(); ++symbol) {
        if (freq[symbol] > 0) {
            result.push_back({static_cast<std::uint16_t>(symbol), freq[symbol]});
        }
    }
    return result;
}

std::vector<std::uint8_t> serialize_header(const FileHeader &header) {
    std::vector<SymbolFrequency> sorted = header.frequencies;
    std::sort(sorted.begin(), sorted.end(),
              [](const SymbolFrequency &lhs, const SymbolFrequency &rhs) {
                  return lhs.symbol < rhs.symbol;
              });

    if (sorted.empty() || sorted.size() > 273) {
        throw std::invalid_argument("serialize_header: k 必须在 1..273");
    }

    std::vector<std::uint8_t> out;
    out.reserve(kHeaderFixedSize + sorted.size() * kFrequencyEntrySize);
    append_u32(out, kFileMagic);
    append_u32(out, header.original_length);
    append_u16(out, static_cast<std::uint16_t>(sorted.size()));
    for (const SymbolFrequency &entry : sorted) {
        append_u16(out, entry.symbol);
        append_u32(out, entry.frequency);
    }
    return out;
}

FileHeader parse_header(const std::vector<std::uint8_t> &file,
                        std::size_t &body_offset) {
    if (file.size() < kHeaderFixedSize) {
        throw std::runtime_error("truncated header");
    }

    const std::uint32_t magic = read_u32(file, 0);
    if (magic != kFileMagic) {
        throw std::runtime_error("bad magic");
    }

    FileHeader header{};
    header.original_length = read_u32(file, 4);

    const std::uint16_t count = read_u16(file, 8);
    if (count < 1 || count > 273) {
        throw std::runtime_error("bad symbol count");
    }

    const std::size_t header_size =
        kHeaderFixedSize + static_cast<std::size_t>(count) * kFrequencyEntrySize;
    if (file.size() < header_size) {
        throw std::runtime_error("truncated header");
    }

    header.frequencies.reserve(count);
    std::uint16_t previous = 0;
    for (std::uint16_t i = 0; i < count; ++i) {
        const std::size_t offset =
            kHeaderFixedSize + static_cast<std::size_t>(i) * kFrequencyEntrySize;
        const std::uint16_t symbol = read_u16(file, offset);
        const std::uint32_t frequency = read_u32(file, offset + 2);

        if (symbol >= 273 || frequency == 0) {
            throw std::runtime_error("bad frequency table");
        }
        if (i > 0 && symbol <= previous) {
            throw std::runtime_error("bad frequency table");
        }
        previous = symbol;
        header.frequencies.push_back({symbol, frequency});
    }

    body_offset = header_size;
    return header;
}

std::vector<std::uint8_t> compress(const std::string &input) {
    if (input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::invalid_argument("compress: 原文长度超过 u32 上限");
    }

    // 1. 分词（末尾已含 EOS）。
    const std::vector<Token> tokens = tokenize(input);

    // 2. 统计频次；build_codes 用同一张表建树，两端因此一致。
    const FrequencyTable freq = frequency_table_of(tokens);

    // 3. 编码表：273 个符号，未出现的符号为空串。
    const std::array<std::string, 273> codes = build_codes(freq);

    // 4. 文件头。
    FileHeader header{};
    header.original_length = static_cast<std::uint32_t>(input.size());
    header.frequencies = collect_frequencies(tokens);

    std::vector<std::uint8_t> file = serialize_header(header);

    // 5. 比特流。
    BitWriter writer;
    for (const Token &token : tokens) {
        switch (token.kind) {
        case TokenKind::Literal:
            put_code(writer, codes[token.literal]);
            break;
        case TokenKind::Match: {
            const std::size_t symbol = static_cast<std::size_t>(kLengthSymbolBase) +
                                       static_cast<std::size_t>(token.length) -
                                       kMinMatchLength;
            put_code(writer, codes[symbol]);
            // 距离减 1，固定 6 位。
            writer.put(static_cast<std::uint32_t>(token.distance - 1u),
                       static_cast<int>(kDistanceBits));
            break;
        }
        case TokenKind::Eos:
            put_code(writer, codes[kEosSymbol]);
            break;
        }
    }
    writer.flush();

    const std::vector<std::uint8_t> &body = writer.data();
    file.insert(file.end(), body.begin(), body.end());
    return file;
}

std::string decompress(const std::vector<std::uint8_t> &file) {
    // 1. 文件头：魔数、长度、k、频次表。
    std::size_t body_offset = 0;
    const FileHeader header = parse_header(file, body_offset);

    // 2. 用同一张频次表重建编码表——与压缩端完全相同的确定性规则。
    std::array<std::uint32_t, 273> freq{};
    for (const SymbolFrequency &entry : header.frequencies) {
        freq[entry.symbol] = entry.frequency;
    }
    if (freq[kEosSymbol] == 0) {
        throw std::runtime_error("missing EOS in frequency table");
    }
    const std::array<std::string, 273> codes = build_codes(freq);

    // 3. 把编码表建成二叉解码树：每读一位往对应孩子走，走到带符号的节点即命中。
    //    前缀码保证不会走到死路，也保证不存在「一个码字是另一个的前缀」。
    struct DecodeNode {
        int child[2] = {-1, -1};
        int symbol = -1;
    };
    std::vector<DecodeNode> trie(1);
    for (std::uint16_t symbol = 0; symbol < 273; ++symbol) {
        const std::string &code = codes[symbol];
        if (code.empty()) {
            continue;
        }
        std::size_t node = 0;
        for (const char c : code) {
            const int bit = (c == '1') ? 1 : 0;
            if (trie[node].child[bit] < 0) {
                trie.push_back(DecodeNode{});
                trie[node].child[bit] = static_cast<int>(trie.size() - 1);
            }
            node = static_cast<std::size_t>(trie[node].child[bit]);
        }
        if (trie[node].symbol >= 0) {
            throw std::runtime_error("bad frequency table: duplicate code");
        }
        trie[node].symbol = symbol;
    }

    // 4. 逐个符号解码。比特流从文件头之后开始。
    const std::vector<std::uint8_t> body(
        file.begin() + static_cast<std::ptrdiff_t>(body_offset), file.end());
    BitReader reader(body);

    std::string output;
    output.reserve(header.original_length);

    while (true) {
        // 沿解码树走，直到命中一个符号。
        std::size_t node = 0;
        while (trie[node].symbol < 0) {
            std::uint32_t bit = 0;
            if (!reader.get(1, bit)) {
                throw std::runtime_error("unexpected end of bitstream");
            }
            const int next = trie[node].child[bit];
            if (next < 0) {
                throw std::runtime_error("bad bitstream: no matching code");
            }
            node = static_cast<std::size_t>(next);
        }

        const int symbol = trie[node].symbol;
        if (symbol < 256) {
            output.push_back(static_cast<char>(symbol));
        } else if (symbol < 272) {
            std::uint32_t raw = 0;
            if (!reader.get(static_cast<int>(kDistanceBits), raw)) {
                throw std::runtime_error("unexpected end of bitstream");
            }
            const std::size_t distance = static_cast<std::size_t>(raw) + 1;
            if (distance > output.size()) {
                throw std::runtime_error("distance exceeds output");
            }
            const std::size_t match_length =
                static_cast<std::size_t>(symbol) - 256 + kMinMatchLength;
            // 允许重叠：边复制边读已产出的内容。
            for (std::size_t i = 0; i < match_length; ++i) {
                output.push_back(output[output.size() - distance]);
            }
        } else {
            break; // EOS
        }

        if (output.size() > header.original_length) {
            throw std::runtime_error("length mismatch");
        }
    }

    if (output.size() != header.original_length) {
        throw std::runtime_error("length mismatch");
    }
    return output;
}

} // namespace dsaac