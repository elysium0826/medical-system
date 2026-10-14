#!/usr/bin/env bash
# 正式源码机器检查；业务与事务正确性仍须人工审查（owner: D）。
set -euo pipefail
cd "$(dirname "$0")/.."
python3 scripts/lint.py
