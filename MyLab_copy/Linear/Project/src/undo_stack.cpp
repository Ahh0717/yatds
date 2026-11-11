// undo_stack.cpp
// UndoStack 的实现：容忍重复压栈、后进先出；底层 std::vector 只做栈尾操作。

#include "undo_stack.h"

#include <stdexcept>
#include <utility>

namespace zuma {

void UndoStack::push(const GameState& s) {
    states_.push_back(s);
}

const GameState& UndoStack::top() const {
    if (states_.empty()) {
        throw std::out_of_range("UndoStack::top: stack is empty");
    }
    return states_.back();
}

bool UndoStack::pop(GameState& out) {
    if (states_.empty()) {
        return false;
    }
    // GameState 只含两个 vector，移动出来避免整份拷贝；随后弹出栈顶。
    out = std::move(states_.back());
    states_.pop_back();
    return true;
}

void UndoStack::clear() {
    states_.clear();
}

}  // namespace zuma