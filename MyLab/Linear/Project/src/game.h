// game.h
// 祖玛游戏逻辑：把轨道（双向链表）、待发射队列（循环队列）与撤销栈
// 组合成一个可快照、可回退的游戏状态。
//
// 状态改变类操作（FIRE / BOMB / PAINT）在改动之前先把当前 GameState
// 压入 undo_，因此任何一步都可以被 UNDO 精确撤销；非法的操作直接返回
// false，不压栈、不改状态。
//
// 胜负判定：
//   - 轨道被彻底清空 → 胜利；
//   - FIRE 之后队列为空且轨道非空 → 失败（没有球可发了）。

#ifndef ZUMA_GAME_H
#define ZUMA_GAME_H

#include <cstddef>
#include <string>
#include <vector>

#include "ball_queue.h"
#include "track.h"
#include "undo_stack.h"

namespace zuma {

class Game {
public:
    using Node = Track::Node;

    // 一次「消除一段」的记录：颜色与个数（个数恒 >= 3）。
    // 一场连锁消除会留下多条记录，按发生顺序排列。
    struct Elimination {
        char color;
        std::size_t count;
    };

    Game() = default;

    // 按给定颜色序列重置整局游戏：重建轨道与队列，清空撤销历史，
    // 并把胜负状态归零。轨道序列按头 → 尾，队列序列按队首 → 队尾。
    void init(const std::vector<char>& track, const std::vector<char>& queue);

    // 从队列队首取一颗球插入 pos（0 <= pos <= 轨道长度），随后自动消除
    // 与连锁消除，再判定胜负。成功返回 true 并通过输出参数带回轨道行、
    // 胜利/失败标志；队列为空或 pos 越界返回 false（状态不变）。
    bool fire(std::size_t pos, std::string* out_track, bool* won, bool* lost);

    // 炸弹小道具：删除轨道 pos 处的球，再处理由此产生的消除与连锁。
    // 轨道为空或 pos 越界返回 false。轨道清空时通过 won 返回 true。
    bool bomb(std::size_t pos, std::string* out_track, bool* won);

    // 撤销最近一次状态改变（FIRE / BOMB / PAINT）。撤销栈为空返回 false。
    bool undo(std::string* out_track);

    // 把队列队首球改成 c（A–Z）。队列为空或 c 非法返回 false。
    bool paint(char c, std::string* out_queue);

    // 轨道上第 index 颗球的颜色（0 <= index < trackSize）。越界抛 std::out_of_range。
    char colorAt(std::size_t index) const;
    // 队首球颜色；空队列抛 std::out_of_range。
    char frontColor() const { return queue_.front(); }

    // 轨道行：头 → 尾，空格分隔；空轨道为 "EMPTY"。
    std::string trackStr() const;
    // 队列行：队首 → 队尾，空格分隔；空队列为 "EMPTY"。
    std::string queueStr() const;

    bool won() const { return won_; }
    bool lost() const { return lost_; }
    std::size_t trackSize() const { return track_.size(); }
    std::size_t queueSize() const { return queue_.size(); }

    // 已发射数（FIRE 成功次数），供交互界面显示。UNDO 会一并回退。
    std::size_t shots() const { return shots_; }

    // 撤销栈中可撤销的步数，供交互界面显示。
    std::size_t undoSize() const { return undo_.size(); }

    // 最近一次 FIRE / BOMB 产生的消除明细（按发生顺序，连锁消除有多条）。
    // 每次 FIRE / BOMB 开始时清空；供交互模式打印「消除了 N 个 X 球」。
    const std::vector<Elimination>& lastEliminations() const {
        return last_eliminations_;
    }

private:
    Track track_;
    BallQueue queue_;
    UndoStack undo_;
    bool won_ = false;
    bool lost_ = false;
    std::size_t shots_ = 0;
    std::vector<Elimination> last_eliminations_;

    // 初始队列长度；撤销时按它重建缓冲区，保证容量足够。
    std::size_t queue_cap_ = 0;

    // 从 p 出发做「极大同色段」消除：段长 >= 3 就删掉，删除后左右邻接
    // 同色则继续合并判断（连锁）。返回本轮是否发生过消除。
    bool eliminateFrom(Node* p);

    // 当前状态的可撤销快照。
    GameState snapshot() const;
    // 用快照覆盖当前状态（含胜负标志）。
    void restore(const GameState& s);

    // 节点是否是真实彩球（哨兵的颜色为 '\0'，见 track.h 的约定）。
    static bool isBall(const Node* p) {
        return p != nullptr && p->color != '\0';
    }
};

}  // namespace zuma

#endif  // ZUMA_GAME_H