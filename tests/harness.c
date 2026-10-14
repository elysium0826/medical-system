/* harness.c  测试数据库安全边界（owner: D）
 * 作者：D   创建：2026-10-14 */
#include "common.h"
#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 测试代码只允许显式独立库，不能对演示库或真实业务库执行写用例。 */
int test_db_open(void)
{
    const char *name = getenv("DB_NAME");

    if (name == NULL || strcmp(name, "medical_test") != 0) {
        fprintf(stderr, "DB 测试拒绝运行：必须显式 DB_NAME=medical_test\n");
        return ERR_PARAM;
    }
    return db_connect(NULL, NULL, NULL, name, 0);
}
