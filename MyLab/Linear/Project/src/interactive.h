// interactive.h
// 交互模式的入口（真人游玩）。批处理模式在 main.cpp 里，两者互不影响。

#ifndef ZUMA_INTERACTIVE_H
#define ZUMA_INTERACTIVE_H

namespace zuma {

// 运行交互模式，返回进程退出码。
int runInteractive();

}  // namespace zuma

#endif  // ZUMA_INTERACTIVE_H