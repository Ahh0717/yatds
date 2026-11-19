// deflate.h
// 简化 Deflate：分词、比特流、文件头与编解码。
//
// 本文件只放数据结构与接口声明，不含算法实现；实现位于 deflate.cpp。
//
// 固定参数（题面示例即本项目参数）：
//   滑动窗口 W = 64，最长匹配 18，最短匹配 3；
//   距离范围 1 ~ 64，写入比特流时占 6 位，值为 dist - 1。
//
// 文件格式（所有多字节字段均为小端）：
//   offset 0    4 字节   魔数 'S' 'D' 'F' '1'
//   offset 4    4 字节   原始文本长度 u32
//   offset 8    2 字节   非零频次的符号个数 k（1 ~ 273）
//   offset 10   k 组     每组 u16 符号 + u32 频次，按符号升序
//   之后                 压缩主体的比特流
//
// 压缩主体：
//   literal(c)      -> symbol c 的哈夫曼码字
//   match(len,dist) -> symbol (256 + len - 3) 的哈夫曼码字，再写 6 位 (dist - 1)
//   最后             -> EOS 的哈夫曼码字
//
// 比特流约定：
//   - 一律高位先写：字节内从 bit7 填到 bit0；一个值内从最高位写到最低位；
//     put(0b101, 3) 依次写入 1、0、1；
//   - 哈夫曼码字按字符串顺序写（第一个字符先写）；
//   - 6 位距离从 bit5 写到 bit0；
//   - 末尾不足一字节补 0。
//
// 单符号退化树（空原文只有 EOS）的编码是 1 位 0，解码时先读入这 1 位
// 再返回该符号，保证两端对“已消耗比特数”的理解一致。

#ifndef MYLAB_TREE_PROJECT_DEFLATE_H
#define MYLAB_TREE_PROJECT_DEFLATE_H

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace dsaac {

// 文件头中的一条频次记录。huffman 模块使用 std::array<uint32_t, 273>，
// 这里保留「符号 + 频次」的成对形式，便于按 k 组逐条读写。
struct SymbolFrequency {
    std::uint16_t symbol;
    std::uint32_t frequency;
};

inline constexpr std::size_t kWindowSize = 64;
inline constexpr std::size_t kMaxDistance = 64;
inline constexpr std::size_t kDistanceBits = 6;
inline constexpr std::size_t kMinMatchLength = 3;
inline constexpr std::size_t kMaxMatchLength = 18;

// 273 符号空间的分界：0..255 字面量，256..271 长度 3..18，272 EOS。
inline constexpr std::size_t kSymbolCount = 273;
inline constexpr std::uint16_t kLengthSymbolBase = 256;
inline constexpr std::uint16_t kEosSymbol = 272;

// 'S' 'D' 'F' '1' 按小端写出的 u32。
inline constexpr std::uint32_t kFileMagic = 0x31464453u;

inline constexpr std::size_t kHeaderFixedSize = 10;
inline constexpr std::size_t kFrequencyEntrySize = 6;

// 三种 token：字面量、回溯、结束符。
enum class TokenKind { Literal, Match, Eos };

// 分词结果。三个字段按 kind 取用：
//   Literal -> literal（原始字节），length/distance 为 0；
//   Match   -> length（3~18）、distance（1~64），literal 为 0；
//   Eos     -> 三个字段都为 0。
struct Token {
    TokenKind kind;
    std::uint8_t literal;
    std::uint8_t length;
    std::uint8_t distance;
};

struct FileHeader {
    std::uint32_t original_length;
    std::vector<SymbolFrequency> frequencies; // 非零频次，按符号升序
};

// 把一段字节扫描成 token 序列，末尾补一个 EOS。
//
// 规则：从头扫，位置 i 从 0 到 n-1。
//   - 窗口是 [max(0, i-64), i-1]，窗口内每个距离 d 从 1 到 min(64, i)，
//     从 i-d 开始与 i 逐字节比较，最多比 min(18, n-i) 个字节；
//   - 取最长的一次匹配，长度相同取距离小的（d 从小到大扫，严格更长方替换，
//     因此天然保留小距离）；
//   - 最长匹配长度 >= 3 输出 Match(len, dist)，i 前进 len；
//     否则输出 Literal(input[i])，i 前进 1；
//   - 扫完追加 Eos。
//
// 允许重叠：比较时源位置可以追上并越过当前位置（如 100 个 'a' 用 dist = 1
// 连续复制），这样重复串才能被压缩。
//
// 例："abababab" -> Literal('a'), Literal('b'), Match(6, 2), Eos。
// 空串 -> 只有 Eos。
std::vector<Token> tokenize(const std::string &input);

// 统计 token 对应的符号频次。输入的 tokens 应是 tokenize 的输出（EOS 已含在内），
// 这里不再额外补 EOS。返回按符号升序排列、只含非零频次的条目。
std::vector<SymbolFrequency> collect_frequencies(const std::vector<Token> &tokens);

// 序列化 / 解析文件头。
// serialize_header 会先把 frequencies 按符号升序排序再写出，
// 不修改传入的 FileHeader。
// parse_header 校验魔数、k 的范围与文件长度；失败抛 std::invalid_argument。
// body_offset 输出压缩主体第一个字节的下标。
std::vector<std::uint8_t> serialize_header(const FileHeader &header);
FileHeader parse_header(const std::vector<std::uint8_t> &file,
                        std::size_t &body_offset);

// 比特写入器：按「高位先写」的顺序把 value 的低 nbits 位写进比特流。
// 例：put(0b101, 3) 依次写入 1、0、1。
// 字节内同样从高位往低位填，写满 8 位换下一个字节。
// flush() 把最后一个未写满的字节的低位补 0；未 flush 的部分不在 data() 里。
class BitWriter {
public:
    // 写入 value 的低 nbits 位（1 <= nbits <= 32），从 bit (nbits-1) 写到 bit 0。
    // nbits 越界抛 std::invalid_argument。
    void put(std::uint32_t value, int nbits) {
        if (nbits < 1 || nbits > 32) {
            throw std::invalid_argument("BitWriter::put: nbits 必须在 1..32");
        }
        for (int i = nbits - 1; i >= 0; --i) {
            current_ = static_cast<std::uint8_t>(
                (current_ << 1) | static_cast<std::uint8_t>((value >> i) & 1u));
            ++filled_;
            if (filled_ == 8) {
                bytes_.push_back(current_);
                current_ = 0;
                filled_ = 0;
            }
        }
    }

    // 把当前字节的低位补 0 后收进 bytes_；重复调用无副作用。
    void flush() {
        if (filled_ != 0) {
            current_ = static_cast<std::uint8_t>(current_ << (8 - filled_));
            bytes_.push_back(current_);
            current_ = 0;
            filled_ = 0;
        }
    }

    const std::vector<std::uint8_t> &data() const { return bytes_; }

private:
    std::vector<std::uint8_t> bytes_;
    std::uint8_t current_ = 0;
    int filled_ = 0; // current_ 中已写入的比特数（0..7）
};

// 比特读取器：按 BitWriter 相同的顺序（字节内高位先读、值内高位先读）取比特。
// buf 必须比 reader 活得久（内部只持有引用，不拷贝）。
class BitReader {
public:
    explicit BitReader(const std::vector<std::uint8_t> &buf)
        : buf_(buf), bit_pos_(0) {}

    // 读 nbits 位（1 <= nbits <= 32）到 value。
    // 剩余比特不足 nbits 时返回 false，value 与读位置都不变。
    // nbits 越界抛 std::invalid_argument。
    bool get(int nbits, std::uint32_t &value) {
        if (nbits < 1 || nbits > 32) {
            throw std::invalid_argument("BitReader::get: nbits 必须在 1..32");
        }
        const std::size_t total = buf_.size() * 8;
        if (bit_pos_ > total || total - bit_pos_ < static_cast<std::size_t>(nbits)) {
            return false;
        }
        std::uint32_t result = 0;
        for (int i = 0; i < nbits; ++i) {
            const std::size_t index = bit_pos_ >> 3;
            const int shift = 7 - static_cast<int>(bit_pos_ & 7u);
            const std::uint32_t bit = (buf_[index] >> shift) & 1u;
            result = (result << 1) | bit;
            ++bit_pos_;
        }
        value = result;
        return true;
    }

private:
    const std::vector<std::uint8_t> &buf_;
    std::size_t bit_pos_; // 已读比特数，即位下标
};

// 压缩：原文 -> 完整文件字节。
// 原文长度超过 u32 上限时抛 std::invalid_argument。
std::vector<std::uint8_t> compress(const std::string &input);

// 解压：完整文件字节 -> 原文。
// 魔数 / 文件头 / 回溯距离 / 解码长度任一不合法时抛 std::invalid_argument。
std::string decompress(const std::vector<std::uint8_t> &file);

} // namespace dsaac

#endif // MYLAB_TREE_PROJECT_DEFLATE_H