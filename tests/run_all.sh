#!/usr/bin/env bash
# 运行实际存在的测试，桩结构测试不等于业务验收（owner: D）。
set -euo pipefail
cd "$(dirname "$0")/.."
count=0
for source in tests/test_*.c; do
    [[ -f "$source" ]] || continue
    test="obj/$(basename "${source%.c}")"
    if [[ ! -f "$test" || ! -x "$test" ]]; then
        echo "缺少对应可执行文件：$test；请先 make test" >&2
        exit 1
    fi
    echo "运行 $test"
    "$test"
    count=$((count + 1))
done
if [[ "$count" == 0 ]]; then
    echo '没有编译出的测试可执行文件' >&2
    exit 1
fi
echo "完成 $count 个测试程序；业务桩的结构自检不代表FR/52条TC通过。"
