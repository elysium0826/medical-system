/*
 * test_stock.c  库存变更与流水测试（FR-M10-01）
 * 作者：shangshuo  创建：2026-10-16
 */
#include "medicine.h"

#include "db.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_db_open(void);

static void login_role(int role, const char *username)
{
    session_clear();
    g_session.role = role;
    g_session.last_action = time(NULL);
    snprintf(g_session.username, sizeof(g_session.username), "%s", username);
}

static int query_number(const char *sql)
{
    db_result_t *rs = NULL;
    int value;

    ASSERT_RC(db_query(sql, &rs));
    ASSERT_EQ(rs->rows, 1);
    value = atoi(db_get(rs, 0, 0));
    db_result_free(rs);
    return value;
}

static void check_log(int type, int quantity, int before, int after, const char *ref_id)
{
    const char *remarks[] = {"", "入库", "出库", "处方扣减", "盘盈", "盘亏", "作废留痕"};
    db_result_t *rs = NULL;

    ASSERT_RC(db_query("SELECT med_id,change_type,quantity,before_stock,after_stock,"
                       "ref_id,operator,remark,create_time FROM t_stock_log "
                       "ORDER BY log_id DESC LIMIT 1",
                       &rs));
    ASSERT_EQ(rs->rows, 1);
    ASSERT_STR_EQ(db_get(rs, 0, 0), "UTSTOCK");
    ASSERT_EQ(atoi(db_get(rs, 0, 1)), type);
    ASSERT_EQ(atoi(db_get(rs, 0, 2)), quantity);
    ASSERT_EQ(atoi(db_get(rs, 0, 3)), before);
    ASSERT_EQ(atoi(db_get(rs, 0, 4)), after);
    ASSERT_STR_EQ(db_get(rs, 0, 5), ref_id);
    ASSERT_STR_EQ(db_get(rs, 0, 6), g_session.username);
    ASSERT_STR_EQ(db_get(rs, 0, 7), remarks[type]);
    ASSERT_EQ(strlen(db_get(rs, 0, 8)), 19);
    db_result_free(rs);
}

static void test_request_errors(void)
{
    ASSERT_EQ(medicine_change_stock(NULL, 1, STOCK_IN, NULL, "admin"), ERR_PARAM);
    ASSERT_EQ(medicine_change_stock("UTSTOCK", 1, 0, "", "admin"), ERR_PARAM);
    ASSERT_EQ(medicine_change_stock("UTSTOCK", 0, STOCK_IN, "", "admin"), ERR_PARAM);
    ASSERT_EQ(medicine_change_stock("UTSTOCK", 1, STOCK_VOID, "", "admin"), ERR_PARAM);
    ASSERT_EQ(medicine_stock_in("UTSTOCK", 0, "admin"), ERR_PARAM);
    ASSERT_EQ(medicine_stock_out("UTSTOCK", INT_MIN, "admin"), ERR_PARAM);
    session_clear();
    ASSERT_EQ(medicine_stock_in("UTSTOCK", 1, "admin"), ERR_PERM);
    login_role(ROLE_PATIENT, "patient");
    ASSERT_EQ(medicine_stock_in("UTSTOCK", 1, "patient"), ERR_PERM);
    login_role(ROLE_DOCTOR, "doctor");
    ASSERT_EQ(medicine_stock_out("UTSTOCK", 1, "doctor"), ERR_PERM);
    ASSERT_EQ(medicine_change_stock("UTSTOCK", -1, STOCK_PRESC, "1", "admin"), ERR_PERM);
    login_role(ROLE_ADMIN, "admin");
    ASSERT_EQ(medicine_stock_in("UTSTOCK", 1, "other"), ERR_PERM);
    g_session.last_action -= SESSION_TIMEOUT_MIN * 60 + 1;
    ASSERT_EQ(medicine_stock_in("UTSTOCK", 1, "admin"), ERR_TIMEOUT);
    login_role(ROLE_ADMIN, "admin");
    ASSERT_EQ(medicine_stock_in("UTSTOCK", 1, "admin"), ERR_DB_CONN);
    session_clear();
}

static void prepare_tables(void)
{
    ASSERT_RC(test_db_open());
    /* 临时表只对本连接可见，字段类型沿用项目 DDL，连接关闭后自动消失。 */
    ASSERT_RC(db_exec("CREATE TEMPORARY TABLE t_medicine ("
                      "med_id VARCHAR(20) PRIMARY KEY, med_name VARCHAR(63) NOT NULL, "
                      "stock INT NOT NULL DEFAULT 0, status TINYINT NOT NULL DEFAULT 1, "
                      "update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP "
                      "ON UPDATE CURRENT_TIMESTAMP) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4",
                      NULL));
    ASSERT_RC(db_exec("CREATE TEMPORARY TABLE t_stock_log ("
                      "log_id BIGINT NOT NULL AUTO_INCREMENT PRIMARY KEY, "
                      "med_id VARCHAR(20) NOT NULL, change_type TINYINT NOT NULL, "
                      "quantity INT NOT NULL, before_stock INT NOT NULL, after_stock INT NOT NULL, "
                      "ref_id VARCHAR(20) NOT NULL DEFAULT '', operator VARCHAR(20) NOT NULL, "
                      "remark VARCHAR(100) NOT NULL DEFAULT '', "
                      "create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP) "
                      "ENGINE=InnoDB DEFAULT CHARSET=utf8mb4",
                      NULL));
    ASSERT_RC(db_exec("INSERT INTO t_medicine(med_id,med_name,stock,status) VALUES"
                      "('UTSTOCK','库存测试药',20,1),('UTOFF','停用药',5,0),"
                      "('UTBIG','库存边界药',2147483647,1)",
                      NULL));
    login_role(ROLE_ADMIN, "admin");
}

static void test_in_and_out(void)
{
    int count;

    ASSERT_RC(db_tx_begin());
    ASSERT_RC(medicine_stock_in("UTSTOCK", 100, "admin"));
    ASSERT_RC(db_tx_commit());
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTSTOCK'"), 120);
    ASSERT_EQ(query_number("SELECT COUNT(*) FROM t_stock_log"), 1);
    check_log(STOCK_IN, 100, 20, 120, "");

    count = query_number("SELECT COUNT(*) FROM t_stock_log");
    ASSERT_RC(db_tx_begin());
    ASSERT_EQ(medicine_stock_out("UTSTOCK", 121, "admin"), ERR_STOCK_LOW);
    ASSERT_RC(db_tx_rollback());
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTSTOCK'"), 120);
    ASSERT_EQ(query_number("SELECT COUNT(*) FROM t_stock_log"), count);

    ASSERT_RC(db_tx_begin());
    ASSERT_RC(medicine_stock_out("UTSTOCK", 3, "admin"));
    ASSERT_RC(db_tx_commit());
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTSTOCK'"), 117);
    check_log(STOCK_OUT, -3, 120, 117, "");
}

static void test_prescription_and_void(void)
{
    int i;

    login_role(ROLE_DOCTOR, "doctor");
    ASSERT_RC(db_tx_begin());
    ASSERT_RC(medicine_change_stock("UTSTOCK", -7, STOCK_PRESC, "1001", "doctor"));
    ASSERT_RC(db_tx_commit());
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTSTOCK'"), 110);
    check_log(STOCK_PRESC, -7, 117, 110, "1001");

    /* 同秒留痕两次，不能因库存和更新时间没变就误判失败。 */
    for (i = 0; i < 2; i++) {
        ASSERT_RC(db_tx_begin());
        ASSERT_RC(medicine_change_stock("UTSTOCK", 0, STOCK_VOID, "1001", "doctor"));
        ASSERT_RC(db_tx_commit());
        ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTSTOCK'"), 110);
        check_log(STOCK_VOID, 0, 110, 110, "1001");
    }
    login_role(ROLE_ADMIN, "admin");
}

static void test_boundaries(void)
{
    int count = query_number("SELECT COUNT(*) FROM t_stock_log");

    ASSERT_RC(db_tx_begin());
    ASSERT_EQ(medicine_stock_in("MISSING", 1, "admin"), ERR_NOT_FOUND);
    ASSERT_EQ(medicine_stock_in("UTBIG", 1, "admin"), ERR_PARAM);
    ASSERT_EQ(medicine_change_stock("UTSTOCK", INT_MIN, STOCK_OUT, "", "admin"), ERR_STOCK_LOW);
    ASSERT_RC(db_tx_rollback());
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTSTOCK'"), 110);
    ASSERT_EQ(query_number("SELECT COUNT(*) FROM t_stock_log"), count);
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTBIG'"), INT_MAX);

    ASSERT_RC(db_tx_begin());
    ASSERT_RC(medicine_change_stock("UTOFF", 0, STOCK_VOID, "old", "admin"));
    ASSERT_RC(db_tx_commit());
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTOFF'"), 5);

    ASSERT_RC(db_tx_begin());
    ASSERT_RC(medicine_stock_out("UTSTOCK", 110, "admin"));
    ASSERT_RC(db_tx_commit());
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTSTOCK'"), 0);
    check_log(STOCK_OUT, -110, 110, 0, "");
}

static void test_outer_rollback(void)
{
    int count = query_number("SELECT COUNT(*) FROM t_stock_log");

    /* 第二种药不足时，第一种药及其流水也由外层事务一起撤回。 */
    ASSERT_RC(db_tx_begin());
    ASSERT_RC(medicine_stock_in("UTSTOCK", 50, "admin"));
    ASSERT_EQ(medicine_stock_out("UTOFF", 6, "admin"), ERR_STOCK_LOW);
    ASSERT_RC(db_tx_rollback());
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTSTOCK'"), 0);
    ASSERT_EQ(query_number("SELECT COUNT(*) FROM t_stock_log"), count);
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTOFF'"), 5);

    /* 故意让临时流水表拒绝指定单据，验证流水失败时库存可回滚。 */
    ASSERT_RC(db_exec("ALTER TABLE t_stock_log ADD CONSTRAINT reject_test_log "
                      "CHECK (ref_id <> 'UTFAIL')",
                      NULL));
    ASSERT_RC(db_tx_begin());
    ASSERT_EQ(medicine_change_stock("UTSTOCK", 10, STOCK_IN, "UTFAIL", "admin"), ERR_DB_EXEC);
    ASSERT_RC(db_tx_rollback());
    ASSERT_EQ(query_number("SELECT stock FROM t_medicine WHERE med_id='UTSTOCK'"), 0);
    ASSERT_EQ(query_number("SELECT COUNT(*) FROM t_stock_log"), count);
}

static void test_escaped_fields(void)
{
    db_result_t *rs = NULL;
    int count = query_number("SELECT COUNT(*) FROM t_stock_log");

    login_role(ROLE_ADMIN, "a'b\\c");
    ASSERT_RC(db_tx_begin());
    ASSERT_RC(medicine_change_stock("UTSTOCK", 4, STOCK_GAIN, "单'据\\1", "a'b\\c"));
    ASSERT_RC(db_tx_commit());
    check_log(STOCK_GAIN, 4, 0, 4, "单'据\\1");
    ASSERT_EQ(query_number("SELECT COUNT(*) FROM t_stock_log"), count + 1);

    ASSERT_RC(db_exec("INSERT INTO t_medicine(med_id,med_name,stock) "
                      "VALUES('UT''MED\\\\1','特殊编号药',1)",
                      NULL));
    ASSERT_RC(db_tx_begin());
    ASSERT_RC(medicine_stock_out("UT'MED\\1", 1, "a'b\\c"));
    ASSERT_RC(db_tx_commit());
    ASSERT_RC(db_query("SELECT med_id,before_stock,after_stock FROM t_stock_log "
                       "ORDER BY log_id DESC LIMIT 1",
                       &rs));
    ASSERT_STR_EQ(db_get(rs, 0, 0), "UT'MED\\1");
    ASSERT_EQ(atoi(db_get(rs, 0, 1)), 1);
    ASSERT_EQ(atoi(db_get(rs, 0, 2)), 0);
    db_result_free(rs);
}

int main(void)
{
    const char *enabled = getenv("RUN_DB_TESTS");

    test_request_errors();
    if (enabled != NULL && strcmp(enabled, "1") == 0) {
        prepare_tables();
        test_in_and_out();
        test_prescription_and_void();
        test_boundaries();
        test_outer_rollback();
        test_escaped_fields();
        db_close();
        session_clear();
        puts("PASS: 真实MySQL库存变化、流水、库存不足、留痕、转义和外层回滚");
    } else {
        puts("PASS: 库存参数/权限/超时/断连；SKIP: 真实MySQL库存测试（需RUN_DB_TESTS=1）");
    }
    return 0;
}
