/* test_user.c  公共工具/会话及隔离数据库登录与封装验证（owner: D）
 * 作者：D   创建：2026-10-14 */
#include "common.h"
#include "db.h"
#include "user.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_db_open(void);

static void test_utilities(void)
{
    char text[128];

    ASSERT_TRUE(date_valid("2024-02-29"));
    ASSERT_TRUE(!date_valid("2023-02-29"));
    ASSERT_TRUE(!date_valid("1900-02-29"));
    ASSERT_TRUE(date_valid("2000-02-29"));
    ASSERT_TRUE(!date_valid("2026-04-31"));
    ASSERT_TRUE(!date_valid("2026-1-01"));
    ASSERT_EQ(date_diff_days("2026-03-01", "2026-02-28"), 1);
    ASSERT_EQ(date_diff_days("2024-03-01", "2024-02-28"), 2);
    ASSERT_EQ(date_diff_days("2026-02-28", "2026-03-01"), -1);
    ASSERT_EQ(page_normalize(0), 1);
    ASSERT_EQ(page_offset(2), PAGE_SIZE);
    ASSERT_EQ(page_count(0), 1);
    ASSERT_EQ(page_count(21), 2);
    like_escape("a%b_c", text, sizeof(text));
    ASSERT_STR_EQ(text, "a\\\\%b\\\\_c");
    hash_password("123456", text, sizeof(text));
    ASSERT_STR_EQ(text, "123456");
    ASSERT_TRUE(str_empty(NULL));
}

static void test_session(void)
{
    session_clear();
    ASSERT_TRUE(!session_is_login());
    ASSERT_EQ(session_require_role(ROLE_ADMIN), ERR_PERM);
    g_session.role = ROLE_ADMIN;
    g_session.last_action = time(NULL);
    ASSERT_RC(session_check_timeout());
    ASSERT_RC(session_require_role(ROLE_ADMIN));
    ASSERT_EQ(session_require_role(ROLE_DOCTOR), ERR_PERM);
    g_session.last_action -= SESSION_TIMEOUT_MIN * 60 + 2;
    ASSERT_EQ(session_check_timeout(), ERR_TIMEOUT);
    ASSERT_TRUE(!session_is_login());
}

static void test_db_results(void)
{
    db_result_t *rs = NULL;
    char escaped[128], sql[SQL_BUF];
    const char *raw = "O'Reilly\\line\n\r\032";

    ASSERT_RC(db_query("SELECT REPEAT('x',16384),NULL,''", &rs));
    ASSERT_EQ(rs->rows, 1);
    ASSERT_EQ(strlen(db_get(rs, 0, 0)), 16384);
    ASSERT_STR_EQ(db_get(rs, 0, 1), "");
    ASSERT_STR_EQ(db_get(rs, 0, 2), "");
    ASSERT_STR_EQ(db_get(rs, 5, 0), "");
    db_result_free(rs);
    ASSERT_RC(db_query("SELECT 1 WHERE 0", &rs));
    ASSERT_EQ(rs->rows, 0);
    db_result_free(rs);
    db_escape(raw, escaped, sizeof(escaped));
    ASSERT_TRUE(snprintf(sql, sizeof(sql), "SELECT '%s'", escaped) < (int)sizeof(sql));
    ASSERT_RC(db_query(sql, &rs));
    ASSERT_STR_EQ(db_get(rs, 0, 0), raw);
    db_result_free(rs);
    ASSERT_RC(db_tx_begin());
    ASSERT_EQ(db_tx_begin(), ERR_TX);
    ASSERT_RC(db_tx_rollback());
    ASSERT_EQ(db_tx_commit(), ERR_TX);
    ASSERT_RC(db_tx_begin());
    ASSERT_RC(db_tx_commit());
}

static void test_login_lock(void)
{
    db_result_t *rs = NULL;
    int i, remain;

    ASSERT_RC(db_exec("INSERT INTO t_admin(admin_id,password,name) "
                      "VALUES('utadmin','123456','测试管理员') "
                      "ON DUPLICATE KEY UPDATE password='123456',fail_count=0,lock_until=NULL",
                      NULL));
    for (i = 0; i < LOCK_FAIL_MAX - 1; i++) {
        ASSERT_EQ(user_login(ROLE_ADMIN, "utadmin", "wrong"), ERR_PARAM);
    }
    ASSERT_EQ(user_login(ROLE_ADMIN, "utadmin", "wrong"), ERR_LOCKED);
    ASSERT_EQ(user_login(ROLE_ADMIN, "utadmin", "123456"), ERR_LOCKED);
    ASSERT_RC(user_lock_remain(ROLE_ADMIN, "utadmin", &remain));
    ASSERT_TRUE(remain > 0 && remain <= LOCK_MINUTES + 1);
    ASSERT_RC(db_query("SELECT fail_count FROM t_admin WHERE admin_id='utadmin'", &rs));
    ASSERT_STR_EQ(db_get(rs, 0, 0), "0");
    db_result_free(rs);
    ASSERT_RC(db_exec("UPDATE t_admin SET lock_until=NOW()-INTERVAL 1 MINUTE "
                      "WHERE admin_id='utadmin'",
                      NULL));
    ASSERT_RC(user_login(ROLE_ADMIN, "utadmin", "123456"));
    ASSERT_EQ(g_session.role, ROLE_ADMIN);
    ASSERT_STR_EQ(g_session.user_id, "utadmin");
    ASSERT_EQ(user_change_password("wrong", "abcdef"), ERR_PARAM);
    ASSERT_RC(user_change_password("123456", "abcdef"));
    ASSERT_RC(user_login(ROLE_ADMIN, "utadmin", "abcdef"));
    ASSERT_EQ(user_login(ROLE_ADMIN, "utadmin'", "abcdef"), ERR_NOT_FOUND);
    ASSERT_RC(db_exec("DELETE FROM t_admin WHERE admin_id='utadmin'", NULL));
    session_clear();
}

int main(void)
{
    const char *enabled = getenv("RUN_DB_TESTS");

    test_utilities();
    test_session();
    if (enabled != NULL && strcmp(enabled, "1") == 0) {
        ASSERT_RC(test_db_open());
        test_db_results();
        test_login_lock();
        db_close();
        puts("PASS: 公共工具/会话/真实数据库结果集、事务及管理员锁定测试");
    } else {
        puts("PASS: 公共工具/会话；SKIP: DB测试（RUN_DB_TESTS=1 DB_NAME=medical_test）");
    }
    return 0;
}
