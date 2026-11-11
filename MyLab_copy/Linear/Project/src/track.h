// track.h
// 祖玛轨道：手写双向链表 + 首尾哨兵。
//
// 选型理由：
//   - 消除逻辑需要从某个彩球出发，沿左右两个方向扫描同色段，并在删除后
//     把左右两段重新接上，双向链表对这两种操作都是 O(1)（已有节点指针时）；
//   - 首尾各放一个哨兵节点，插入/删除时不必对空链表、头部、尾部做特判，
//     链表永远非空（至少有两个哨兵），代码分支少、不容易出错。
//
// 注意：本类不使用 std::list，节点全部手动 new/delete。
//
// 与 Game 层的约定：消除逻辑要读节点的 color/prev/next，所以 Node 结构
// 公开；但 head_/tail_ 两个哨兵是私有的，外部只能通过 nodeAt() 拿到真实
// 彩球节点，不会误把哨兵当成彩球处理。

#ifndef ZUMA_TRACK_H
#define ZUMA_TRACK_H

#include <cstddef>
#include <vector>

namespace zuma {

class Track {
public:
    // 轨道节点。prev/next 永远非空（哨兵使链表首尾闭合），color 为 A–Z。
    // 两个哨兵的 color 恒为 '\0'（不是合法彩球色），Game 层可以据此判断
    // 某个 prev/next 是否已经走到轨道端点——这是 Track 与 Game 之间的约定。
    struct Node {
        char color;
        Node* prev;
        Node* next;
    };

    Track();
    ~Track();

    // 深拷贝：逐个节点复制。游戏里做状态快照时可能用到；
    // 不提供移动语义，以免外部持有 Node* 时被悄悄搬走。
    Track(const Track& other);
    Track& operator=(const Track& other);

    // 在下标 pos 处插入一颗彩球，插入后新球位于下标 pos。
    // 0 <= pos <= size()；pos == size() 表示追加到尾端（O(1)）。
    // 越界抛 std::out_of_range，此时链表不变。返回新节点指针。
    Node* insertAt(std::size_t pos, char color);

    // 从链表摘下 p 并 delete。调用方若之后还要用到 p 的前驱/后继，
    // 必须在调用前先保存好（erase 之后 p 已失效）。
    // p 为空或指向哨兵时抛 std::invalid_argument。
    void erase(Node* p);

    // 返回下标 pos 处的节点，0 <= pos < size()；越界抛 std::out_of_range。
    Node* nodeAt(std::size_t pos);
    // 只读版本，供 const 成员函数（如打印、查询颜色）使用。
    const Node* nodeAt(std::size_t pos) const;

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // 按轨道头 → 尾的顺序导出所有颜色，供打印、快照与比对。
    std::vector<char> toVector() const;

    // 释放全部彩球节点，只保留两个哨兵；清空后可继续复用。
    void clear();

private:
    Node* head_;       // 哨兵：head_->next 是第 0 颗彩球
    Node* tail_;       // 哨兵：tail_->prev 是最后一颗彩球
    std::size_t size_; // 彩球个数（不含哨兵）

    void swapWith(Track& other) noexcept;
};

}  // namespace zuma

#endif  // ZUMA_TRACK_H