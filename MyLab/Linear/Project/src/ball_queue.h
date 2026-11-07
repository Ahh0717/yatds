// ball_queue.h
// 待发射彩球队列：手写循环队列（数组环形缓冲区）。
//
// 选型理由：
//   - 游戏只需要“队首出一颗、队尾进一颗”，循环队列让 front/pop/push
//     都是 O(1)，且不需要像普通数组队列那样整体搬移；
//   - 用「额外一个空槽」区分空/满：底层分配 cap_ + 1 个槽，
//     front_ == back_ 为空，back_ + 1 == front_ 为满，不必另存标记位。
//
// 注意：本类不使用 std::queue；缓冲区用 new char[] 手动管理。

#ifndef ZUMA_BALL_QUEUE_H
#define ZUMA_BALL_QUEUE_H

#include <cstddef>
#include <vector>

namespace zuma {

class BallQueue {
public:
    // capacity 为可存彩球个数（>= 0）。容量 0 时队列恒空，push 必失败。
    explicit BallQueue(std::size_t capacity = 0);
    ~BallQueue();

    BallQueue(const BallQueue& other);
    BallQueue& operator=(const BallQueue& other);

    // 在队尾入队；满返回 false，队列不变。
    bool push(char c);

    // 队首元素；空队抛 std::out_of_range。
    char front() const;

    // 出队并返回队首元素；空队抛 std::out_of_range。
    char pop();

    // 修改队首颜色（PAINT 使用）；空队抛 std::out_of_range。
    void setFront(char c);

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // 队首 → 队尾；空队返回空 vector。
    std::vector<char> toVector() const;

    // 清空元素，保留容量与缓冲区（清空后可继续使用）。
    void clear();

private:
    char* buf_;        // 指向 cap_ + 1 个槽的环形缓冲区
    std::size_t front_; // 队首下标
    std::size_t back_;  // 下一个可写入位置的下标
    std::size_t size_;  // 当前元素个数
    std::size_t cap_;   // 可存元素个数

    // 下标回绕：走到缓冲区末尾后回到 0。
    std::size_t nextIndex(std::size_t i) const {
        return (i + 1 == cap_ + 1) ? 0 : i + 1;
    }

    void swapWith(BallQueue& other) noexcept;
};

}  // namespace zuma

#endif  // ZUMA_BALL_QUEUE_H