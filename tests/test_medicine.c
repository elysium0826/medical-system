/*
 * test_medicine.c  药品库存查询测试
 * 作者：shangshuo  创建：2026-10-15
 */
#include "medicine.h"

#include "db.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_db_open(void);

static void login_role(int role)
{
    session_clear();
    g_session.role = role;
    g_session.last_action = time(NULL);
}

static void check_list(const char *keyword, int page, int count, int total, const char *first)
{
    medicine_t *entries = NULL;
    int out_n = -1, out_total = -1;

    ASSERT_RC(medicine_query_all(keyword, page, &entries, &out_n, &out_total));
    ASSERT_EQ(out_n, count);
    ASSERT_EQ(out_total, total);
    if (count == 0)
        ASSERT_TRUE(entries == NULL);
    else
        ASSERT_STR_EQ(entries[0].med_id, first);
    medicine_list_free(entries);
}

static void test_query_errors(void)
{
    medicine_t *entries = NULL;
    int count = 7, total = 7;
    char keyword[NAME_LEN + 1];

    ASSERT_EQ(medicine_query_all(NULL, 1, NULL, &count, &total), ERR_PARAM);
    session_clear();
    ASSERT_EQ(medicine_query_all(NULL, 1, &entries, &count, &total), ERR_PERM);
    login_role(ROLE_ADMIN);
    ASSERT_EQ(medicine_query_all(NULL, 1, &entries, &count, &total), ERR_DB_CONN);
    ASSERT_TRUE(entries == NULL);
    ASSERT_EQ(count, 0);
    ASSERT_EQ(total, 0);
    memset(keyword, 'x', sizeof(keyword) - 1);
    keyword[sizeof(keyword) - 1] = '\0';
    ASSERT_EQ(medicine_query_all(keyword, 1, &entries, &count, &total), ERR_PARAM);
    g_session.last_action -= SESSION_TIMEOUT_MIN * 60 + 1;
    ASSERT_EQ(medicine_query_all(NULL, 1, &entries, &count, &total), ERR_TIMEOUT);
}

static void add_examples(void)
{
    char sql[SQL_BUF];
    int i, written;

    ASSERT_RC(db_exec("INSERT INTO t_medicine "
                      "(med_id,med_name,spec,manufacturer,price,stock,status) VALUES "
                      "('UTMED001','含%药品','10片/盒','测试厂家',12.50,49,1),"
                      "('UTMED002','含_药品','20片/盒','测试厂家',8.00,0,0),"
                      "('UTMED003','O''Reilly','10片/盒','测试厂家',15.00,50,1),"
                      "('UTMED004','路径\\\\药品','5片/盒','特殊厂家',6.20,120,1)",
                      NULL));
    for (i = 5; i <= 25; i++) {
        written = snprintf(sql, sizeof(sql),
                           "INSERT INTO t_medicine(med_id,med_name,manufacturer,stock) "
                           "VALUES('UTMED%03d','普通药品%02d','普通厂家',100)",
                           i, i);
        ASSERT_TRUE(written > 0 && written < (int)sizeof(sql));
        ASSERT_RC(db_exec(sql, NULL));
    }
}

static void test_database_query(void)
{
    medicine_t *entries = NULL;
    int count, total;

    ASSERT_RC(test_db_open());
    /* 临时表只在本连接内遮蔽原表，不清空现有测试数据。 */
    ASSERT_RC(db_exec("CREATE TEMPORARY TABLE t_medicine ("
                      "med_id VARCHAR(20) PRIMARY KEY, med_name VARCHAR(63) NOT NULL, "
                      "spec VARCHAR(63) NOT NULL DEFAULT '', "
                      "manufacturer VARCHAR(63) NOT NULL DEFAULT '', "
                      "price DECIMAL(10,2) NOT NULL DEFAULT 0, stock INT NOT NULL DEFAULT 0, "
                      "status TINYINT NOT NULL DEFAULT 1, "
                      "create_time DATETIME DEFAULT CURRENT_TIMESTAMP, "
                      "update_time DATETIME DEFAULT CURRENT_TIMESTAMP)",
                      NULL));
    login_role(ROLE_ADMIN);
    check_list(NULL, 1, 0, 0, NULL);
    add_examples();
    check_list(NULL, 1, PAGE_SIZE, 25, "UTMED001");
    check_list("", 2, 5, 25, "UTMED021");
    check_list(NULL, 0, PAGE_SIZE, 25, "UTMED001");
    check_list(NULL, INT_MAX, 0, 25, NULL);
    check_list("不存在的药", 1, 0, 0, NULL);
    check_list("特殊厂家", 1, 1, 1, "UTMED004");
    check_list("%", 1, 1, 1, "UTMED001");
    check_list("_", 1, 1, 1, "UTMED002");
    check_list("O'Reilly", 1, 1, 1, "UTMED003");
    check_list("\\", 1, 1, 1, "UTMED004");
    check_list("%' OR 1=1 --", 1, 0, 0, NULL);
    ASSERT_RC(medicine_query_all(NULL, 1, &entries, &count, NULL));
    ASSERT_STR_EQ(entries[0].med_name, "含%药品");
    ASSERT_STR_EQ(entries[0].spec, "10片/盒");
    ASSERT_STR_EQ(entries[0].manufacturer, "测试厂家");
    ASSERT_TRUE(entries[0].price > 12.49 && entries[0].price < 12.51);
    ASSERT_EQ(entries[0].stock, 49);
    ASSERT_EQ(entries[1].status, MED_OFF_SALE);
    ASSERT_EQ(strlen(entries[0].create_time), 19);
    medicine_list_free(entries);
    login_role(ROLE_DOCTOR);
    check_list(NULL, 2, 4, 24, "UTMED022");
    check_list("_", 1, 0, 0, NULL);
    login_role(ROLE_PATIENT);
    check_list(NULL, 1, PAGE_SIZE, 24, "UTMED001");
    login_role(ROLE_ADMIN);
    ASSERT_RC(db_exec("ALTER TABLE t_medicine DROP COLUMN manufacturer", NULL));
    ASSERT_EQ(medicine_query_all(NULL, 1, &entries, &count, &total), ERR_DB_EXEC);
    ASSERT_TRUE(entries == NULL);
    ASSERT_EQ(count, 0);
    ASSERT_EQ(total, 0);
    db_close();
    session_clear();
}

int main(void)
{
    const char *enabled = getenv("RUN_DB_TESTS");

    test_query_errors();
    if (enabled != NULL && strcmp(enabled, "1") == 0) {
        test_database_query();
        puts("PASS: 真实MySQL药品列表、分页、字面搜索、权限、空结果及查询失败");
    } else {
        puts("PASS: 查询参数/会话/断连；SKIP: 真实MySQL查询（需RUN_DB_TESTS=1）");
    }
    return 0;
}
