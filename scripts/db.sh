#!/usr/bin/env bash
# D: MySQL 8 development-package helper. No implicit business-data deletion.
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd -- "$ROOT_DIR"
export DB_HOST="${DB_HOST:-localhost}"
export DB_PORT="${DB_PORT:-3306}"
export DB_USER="${DB_USER:-root}"
export DB_PASS="${DB_PASS-}"
export DB_NAME="${DB_NAME:-medical_db}"
MYSQL_BIN="${MYSQL_BIN:-mysql}"

fail() { printf 'db.sh: %s\n' "$*" >&2; exit 2; }
usage() {
    cat <<'USAGE'
Usage: bash scripts/db.sh init|migrate|seed-demo|seed-test|check|reset
  init       create DB if absent; apply 13 DDL files (no seed, no delete)
  migrate    require 9 existing baseline tables; add 4 new tables and 13 items
  seed-demo  synthetic fixtures in EMPTY medical_db only (alias: seed)
  seed-test  independent fixtures in EMPTY medical_test only
  check      print checks and TOTAL_FAILURES; return 1 if failures > 0
  reset      DESTRUCTIVE: only medical_db/medical_test, CONFIRM_RESET=yes required
Environment: DB_HOST=localhost DB_PORT=3306 DB_USER=root DB_PASS='' DB_NAME=medical_db
Docker example: export DB_PASS=medical123   # compose test password, not the empty default
MYSQL_BIN may name a mysql-client executable; passwords are never passed as -p arguments.
Backup and Day-0 owner review of the v1.0 baseline are required before migration.
Business modules remain stubs; these commands do not claim FR/TC or end-to-end success.
USAGE
}

[[ $# -eq 1 ]] || { usage; exit 2; }
action="$1"
case "$action" in
    -h|--help|help) usage; exit 0 ;;
    init|migrate|seed|seed-demo|seed-test|check|reset) ;;
    *) usage; fail "unknown action: $action" ;;
esac
# Validate identifiers before interpolating the only dynamic SQL identifier.
[[ "$DB_NAME" =~ ^[A-Za-z][A-Za-z0-9_]{0,63}$ ]] || fail 'invalid DB_NAME identifier'
case "${DB_NAME,,}" in mysql|information_schema|performance_schema|sys) fail 'system database is forbidden' ;; esac
[[ "$DB_PORT" =~ ^[0-9]{1,5}$ ]] || fail 'invalid DB_PORT'
(( 10#$DB_PORT >= 1 && 10#$DB_PORT <= 65535 )) || fail 'DB_PORT must be 1..65535'
if [[ "$action" == reset ]]; then
    [[ "$DB_NAME" == medical_db || "$DB_NAME" == medical_test ]] || fail 'reset target must be exactly medical_db or medical_test'
    [[ "${CONFIRM_RESET-}" == yes ]] || fail 'reset refused: set CONFIRM_RESET=yes explicitly after backup'
fi
if [[ "$action" == seed-test && "$DB_NAME" != medical_test ]]; then
    fail 'seed-test requires DB_NAME=medical_test; never clear a business database to run tests'
fi
if [[ "$action" == seed || "$action" == seed-demo ]]; then
    [[ "$DB_NAME" == medical_db ]] || fail 'seed-demo requires DB_NAME=medical_db'
fi
command -v "$MYSQL_BIN" >/dev/null 2>&1 || fail "mysql client not found: $MYSQL_BIN"
client=("$MYSQL_BIN" --protocol=TCP "--host=$DB_HOST" "--port=$DB_PORT" "--user=$DB_USER"
        --connect-timeout=5 --default-character-set=utf8mb4 --batch --raw --skip-column-names)
server_sql() { MYSQL_PWD="$DB_PASS" "${client[@]}" "$@"; }
db_sql() { server_sql "--database=$DB_NAME" "$@"; }
run_file() { db_sql < "$1"; }
seed_file() {
    local result
    result="$(run_file "$1")" || return "$?"
    printf '%s\n' "$result"
    [[ "$result" == SEED_OK ]] || fail 'seed refused: wrong database or nonempty data'
}

case "$action" in
    init)
        server_sql --execute="CREATE DATABASE IF NOT EXISTS \`$DB_NAME\` CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci;"
        # MySQL SOURCE may continue after nested errors. Read the SOURCE-only manifest
        # and run each file directly so any DDL failure stops this command immediately.
        count=0
        while IFS= read -r line || [[ -n "$line" ]]; do
            [[ "$line" =~ ^SOURCE\ (scripts/ddl/[0-9][0-9]_[a-z_]+\.sql)\;$ ]] || fail 'invalid SOURCE-only init manifest'
            run_file "${BASH_REMATCH[1]}"
            ((count+=1))
        done < scripts/init_db.sql
        [[ "$count" -eq 13 ]] || fail 'expected 13 DDL files'
        printf 'Initialized %s (13 tables; existing data untouched).\n' "$DB_NAME"
        ;;
    migrate)
        baseline="$(db_sql --execute="SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME IN ('t_patient','t_doctor','t_admin','t_medicine','t_appointment','t_visit_record','t_prescription','t_presc_detail','t_medical_record');")"
        [[ "$baseline" == 9 ]] || fail 'missing v1.0 baseline tables: stop and obtain owner-approved schema; do not invent existing production schema'
        printf 'Migration: verify backup and Day-0 owner approval of existing v1.0 schema.\n' >&2
        for ddl in scripts/ddl/{05_stock_log,07_schedule,11_presc_template,12_presc_template_item}.sql; do
            run_file "$ddl"
        done
        run_file scripts/migrate_v1.1.sql
        ;;
    seed|seed-demo) seed_file scripts/seed_demo.sql ;;
    seed-test) seed_file scripts/seed_test.sql ;;
    check)
        result="$(run_file scripts/check_data.sql)"
        printf '%s\n' "$result"
        total="$(printf '%s\n' "$result" | awk -F '\t' '$1=="TOTAL_FAILURES" {n++; v=$2} END {if(n==1 && v ~ /^[0-9]+$/) print v; else exit 2}')"
        [[ "$total" == 0 ]] || exit 1
        ;;
    reset)
        printf 'WARNING: explicitly resetting %s on %s:%s; all its data will be lost.\n' "$DB_NAME" "$DB_HOST" "$DB_PORT" >&2
        server_sql --execute="DROP DATABASE IF EXISTS \`$DB_NAME\`; CREATE DATABASE \`$DB_NAME\` CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci;"
        printf 'Reset empty database only. Run init, migrate and the appropriate seed explicitly.\n'
        ;;
esac
