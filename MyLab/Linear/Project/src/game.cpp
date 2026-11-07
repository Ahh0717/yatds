// game.cpp
// Game 的实现：轨道 + 待发射队列 + 撤销栈。
//
// 状态改变类操作统一走「先拍快照、再改状态」的顺序，保证任意一步都能被
// UNDO 精确回退；非法操作在校验阶段就返回 false，既不压栈也不改状态。

#include "game.h"

#include <algorithm>

namespace zuma {

namespace {

std::string joinColors(const std::vector<char>& colors) {
    if (colors.empty()) {
        return "EMPTY";
    }
    std::string out;
    out.reserve(colors.size() * 2);
    for (std::size_t i = 0; i < colors.size(); ++i) {
        if (i != 0) {
            out.push_back(' ');
        }
        out.push_back(colors[i]);
    }
    return out;
}

bool isUpperLetter(char c) {
    return c >= 'A' && c <= 'Z';
}

}  // namespace

void Game::init(const std::vector<char>& track, const std::vector<char>& queue) {
    track_.clear();
    for (char c : track) {
        track_.insertAt(track_.size(), c);
    }

    queue_cap_ = queue.size();
    queue_ = BallQueue(queue_cap_);
    for (char c : queue) {
        queue_.push(c);
    }

    undo_.clear();
    won_ = false;
    lost_ = false;
    shots_ = 0;
    last_eliminations_.clear();
}

GameState Game::snapshot() const {
    GameState s;
    s.track = track_.toVector();
    s.queue = queue_.toVector();
    s.won = won_;
    s.lost = lost_;
    s.shots = shots_;
    return s;
}

void Game::restore(const GameState& s) {
    track_.clear();
    for (char c : s.track) {
        track_.insertAt(track_.size(), c);
    }

    // 重建队列而不是 clear + push：BallQueue 的容量固定，重建才能按快照
    // 需要的长度分配，并至少保留初始容量。
    queue_ = BallQueue(std::max(queue_cap_, s.queue.size()));
    for (char c : s.queue) {
        queue_.push(c);
    }

    won_ = s.won;
    lost_ = s.lost;
    shots_ = s.shots;
    // 消除明细只用于界面提示，撤销后不该继续显示上一次操作的结果。
    last_eliminations_.clear();
}

bool Game::fire(std::size_t pos, std::string* out_track, bool* won, bool* lost) {
    if (queue_.empty() || pos > track_.size()) {
        return false;
    }

    undo_.push(snapshot());

    const char color = queue_.pop();
    Node* inserted = track_.insertAt(pos, color);
    ++shots_;
    last_eliminations_.clear();
    eliminateFrom(inserted);

    won_ = track_.empty();
    lost_ = !won_ && queue_.empty();

    if (out_track != nullptr) {
        *out_track = trackStr();
    }
    if (won != nullptr) {
        *won = won_;
    }
    if (lost != nullptr) {
        *lost = lost_;
    }
    return true;
}

bool Game::bomb(std::size_t pos, std::string* out_track, bool* won) {
    if (pos >= track_.size()) {
        return false;
    }

    undo_.push(snapshot());

    Node* target = track_.nodeAt(pos);
    Node* left = isBall(target->prev) ? target->prev : nullptr;
    Node* right = isBall(target->next) ? target->next : nullptr;

    last_eliminations_.clear();
    track_.erase(target);

    // 只有左右相邻同色时才可能新形成 >= 3 的同色段，其余位置原本已稳定。
    if (left != nullptr && right != nullptr && left->color == right->color) {
        eliminateFrom(left);
    }

    won_ = track_.empty();
    // 炸弹不消耗待发射队列，因此不判定失败；只有真的清空轨道时才清除
    // 既有的失败标志（胜利与失败互斥）。否则保留原标志，避免一次不相干
    // 的炸弹悄悄改写先前 FIRE 得出的失败状态。
    if (won_) {
        lost_ = false;
    }

    if (out_track != nullptr) {
        *out_track = trackStr();
    }
    if (won != nullptr) {
        *won = won_;
    }
    return true;
}

bool Game::undo(std::string* out_track) {
    GameState s;
    if (!undo_.pop(s)) {
        return false;
    }

    restore(s);

    if (out_track != nullptr) {
        *out_track = trackStr();
    }
    return true;
}

bool Game::paint(char c, std::string* out_queue) {
    if (queue_.empty() || !isUpperLetter(c)) {
        return false;
    }

    undo_.push(snapshot());
    queue_.setFront(c);

    if (out_queue != nullptr) {
        *out_queue = queueStr();
    }
    return true;
}

char Game::colorAt(std::size_t index) const {
    return track_.nodeAt(index)->color;
}

std::string Game::trackStr() const {
    return joinColors(track_.toVector());
}

std::string Game::queueStr() const {
    return joinColors(queue_.toVector());
}

bool Game::eliminateFrom(Node* p) {
    if (!isBall(p)) {
        return false;
    }

    bool eliminated = false;
    Node* probe = p;

    while (isBall(probe)) {
        const char color = probe->color;

        // 1) 向左右扩展，找到包含 probe 的极大同色段 [first, last]。
        Node* first = probe;
        while (isBall(first->prev) && first->prev->color == color) {
            first = first->prev;
        }
        Node* last = probe;
        while (isBall(last->next) && last->next->color == color) {
            last = last->next;
        }

        std::size_t length = 1;
        for (Node* q = first; q != last; q = q->next) {
            ++length;
        }

        // 2) 不足 3 个：本轮不消除，直接结束。
        if (length < 3) {
            break;
        }

        // 3) 删除整段前，先记下删除后将成为左右邻居的节点（哨兵记为 nullptr）。
        Node* left_ball = isBall(first->prev) ? first->prev : nullptr;
        Node* right_ball = isBall(last->next) ? last->next : nullptr;

        // 从 first 删到 last；先存 next 并判断是否已到段尾，
        // 因为 erase 之后当前节点已失效、不能再拿来比较。
        Node* q = first;
        while (true) {
            Node* next = q->next;
            const bool at_last = (q == last);
            track_.erase(q);
            if (at_last) {
                break;
            }
            q = next;
        }
        eliminated = true;
        last_eliminations_.push_back(Elimination{color, length});

        // 4) 左右两段此时相邻；同色则以它们为新的受影响节点继续判断，
        //    这就是连锁消除。不同色（或已到端点）说明不会再产生新段。
        if (left_ball != nullptr && right_ball != nullptr &&
            left_ball->color == right_ball->color) {
            probe = left_ball;
        } else {
            probe = nullptr;
        }
    }

    return eliminated;
}

}  // namespace zuma