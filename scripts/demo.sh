#!/usr/bin/env bash
# D: reproducible database/package demonstration, NOT an end-to-end business test.
set -euo pipefail
ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd -- "$ROOT_DIR"
export DB_HOST="${DB_HOST:-localhost}"
export DB_PORT="${DB_PORT:-3306}"
export DB_USER="${DB_USER:-root}"
export DB_PASS="${DB_PASS-}"
export DB_NAME="${DB_NAME:-medical_db}"

cat <<'NOTICE'
=== 社区医疗系统开发包演示（业务尚桩，非端到端验收） ===
明文课程账号：admin / 123456；D001 / 123456；P20260001 / 123456。
种子全部合成，无真实姓名/身份证/医疗数据；不得用于生产。
本脚本只运行只读数据一致性检查，不自动初始化、不灌种、不reset/删除数据。
首次准备（空medical_db）：
  bash scripts/db.sh init
  bash scripts/db.sh migrate
  bash scripts/db.sh seed-demo
Docker连接需显式 export DB_PASS=medical123；本地默认密码为空。
独立测试库：DB_NAME=medical_test bash scripts/db.sh init
            DB_NAME=medical_test bash scripts/db.sh seed-test
下面4条是待业务实现后由A/B/C/D补齐的演示验收清单，不是假执行日志：
  1. 管理员排班 → 患者预约；核对quota_used。
  2. 患者改期 → 医生待办；核对旧放号/新占号/次数并验证回滚。
  3. 医生开方 → 库存流水；核对明细金额/扣库存/失败全回滚。
  4. 创建与套用模板 → 再开方/打印；核对停用药跳过与权限。
未调用业务桩并伪称成功；SQL检查通过也不证明上述业务流程、52条TC或ASan通过。
NOTICE
bash scripts/db.sh check
printf '\n数据库一致性检查通过；业务端到端演示尚未实现，不能据此验收FR/TC。\n'
