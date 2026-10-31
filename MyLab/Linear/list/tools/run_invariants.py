#!/usr/bin/env python3
"""运行算法级不变量检查（build/invariants），把结果并进 make test 的汇总。

退出码：全部通过为 0，否则为 1。
"""

import os
import subprocess
import sys


def resolve_exe(path):
    """补上 Windows 下缺失的 .exe 后缀，并转成绝对路径。"""
    if os.name == "nt" and not path.lower().endswith(".exe"):
        path += ".exe"
    return os.path.abspath(path)


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    exe = resolve_exe(os.path.join(root, "build", "invariants"))
    if not os.path.isfile(exe):
        print("找不到 build/invariants，请先运行 make")
        return 1

    proc = subprocess.run([exe], capture_output=True, text=True, encoding="utf-8")
    # 只转发失败行与汇总行，避免把上百条 [ok] 刷满终端。
    for line in proc.stdout.splitlines():
        stripped = line.strip()
        if stripped.startswith("[FAIL]") or stripped.startswith("通过"):
            print(line)
    if proc.returncode != 0:
        print("不变量检查未通过")
        if proc.stderr.strip():
            print(proc.stderr.strip())
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())