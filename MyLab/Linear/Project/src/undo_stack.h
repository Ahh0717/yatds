// undo_stack.h
// 撤销历史：栈（后进先出）。
//
// 这里用栈管理撤销历史——每次状态改变前把整份 GameState 压栈，
// UNDO 时弹出最近的一份快照恢复，天然满足“撤销上一步”的语义。
// 底层存储用 std::vector<GameState>，但对外只暴露栈操作
// （push / pop / top / empty / size / clear），不允许按下标随机访问，
// 以免破坏“只能操作栈顶”的约束；并不使用 std::stack。

#ifndef ZUMA_UNDO_STACK_H
#define ZUMA_UNDO_STACK_H

#include <cstddef>
#include <vector>

namespace zuma {

// 撤销单元：一次状态改变前的完整可恢复信息。
// 阶段 1 只有轨道与队列；阶段 2 补上胜负标志，避免撤销后胜负状态残留。
struct GameState {
    std::vector<char> track;  // 轨道头 → 尾
    std::vector<char> queue;  // 队列队首 → 队尾
    bool won = false;         // 该快照时刻是否已判定胜利
    bool lost = false;        // 该快照时刻是否已判定失败
    std::size_t shots = 0;    // 该快照时刻的累计发射数
};

class UndoStack {
public:
    UndoStack() = default;

    // 压入一份快照（栈顶）。
    void push(const GameState& s);

    // 栈顶快照的只读引用；空栈抛 std::out_of_range。
    const GameState& top() const;

    // 弹出栈顶快照写入 out；空栈返回 false 且不改 out。
    bool pop(GameState& out);

    bool empty() const { return states_.empty(); }
    std::size_t size() const { return states_.size(); }

    // 销毁全部撤销历史。
    void clear();

private:
    std::vector<GameState> states_;
};

}  // namespace zuma

#endif  // ZUMA_UNDO_STACK_H