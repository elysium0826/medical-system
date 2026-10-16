/*
 * test_ui.c  账号菜单交互回归测试（M08）
 * 作者：D   创建：2026-10-16
 */
#include "common.h"
#include "db.h"
#include "ui.h"
#include "user.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 重定向真实终端读写；不替换菜单、输入函数或账号业务函数。 */
static void run_ui(int role, const char *account, const char *input, char *output, size_t capacity)
{
    FILE *in = tmpfile();
    FILE *out = tmpfile();
    int saved_in = dup(STDIN_FILENO);
    int saved_out = dup(STDOUT_FILENO);
    size_t count;

    ASSERT_TRUE(in != NULL && out != NULL && saved_in >= 0 && saved_out >= 0);
    ASSERT_TRUE(fputs(input, in) >= 0);
    rewind(in);
    ASSERT_EQ(fflush(stdout), 0);
    ASSERT_TRUE(dup2(fileno(in), STDIN_FILENO) >= 0);
    ASSERT_TRUE(dup2(fileno(out), STDOUT_FILENO) >= 0);
    clearerr(stdin);
    session_clear();
    g_session.role = role;
    if (account != NULL) {
        ASSERT_TRUE(snprintf(g_session.user_id, sizeof(g_session.user_id), "%s", account) <
                    (int)sizeof(g_session.user_id));
    }
    session_touch();
    switch (role) {
    case ROLE_ADMIN:
        ui_admin_main();
        break;
    case ROLE_DOCTOR:
        ui_doctor_main();
        break;
    case ROLE_PATIENT:
        ui_patient_main();
        break;
    default:
        ASSERT_EQ(ui_login(), ROLE_NONE);
        ASSERT_TRUE(!session_is_login());
        break;
    }
    ASSERT_EQ(fflush(stdout), 0);
    rewind(out);
    count = fread(output, 1, capacity - 1, out);
    ASSERT_TRUE(!ferror(out) && count < capacity - 1);
    output[count] = '\0';
    ASSERT_TRUE(dup2(saved_in, STDIN_FILENO) >= 0);
    ASSERT_TRUE(dup2(saved_out, STDOUT_FILENO) >= 0);
    clearerr(stdin);
    ASSERT_EQ(close(saved_in), 0);
    ASSERT_EQ(close(saved_out), 0);
    ASSERT_EQ(fclose(in), 0);
    ASSERT_EQ(fclose(out), 0);
    session_clear();
}

static void test_password_confirmation(int role)
{
    char output[8192];

    run_ui(role, NULL, "5\nold-secret\nnew-secret\nother-secret\n\n0\n", output, sizeof(output));
    ASSERT_TRUE(strstr(output, "两次输入的密码不一致") != NULL);
    ASSERT_TRUE(strstr(output, "数据库连接失败") == NULL);
    ASSERT_TRUE(strstr(output, "密码修改成功") == NULL);
    ASSERT_TRUE(strstr(output, "old-secret") == NULL);
    ASSERT_TRUE(strstr(output, "new-secret") == NULL);
    ASSERT_TRUE(strstr(output, "other-secret") == NULL);
}

static void test_password_interruption(void)
{
    char output[8192];

    run_ui(ROLE_PATIENT, NULL, "5\nold-secret\n", output, sizeof(output));
    ASSERT_TRUE(strstr(output, "新密码") != NULL);
    ASSERT_TRUE(strstr(output, "数据库连接失败") == NULL);
    ASSERT_TRUE(strstr(output, "密码修改成功") == NULL);
    run_ui(ROLE_DOCTOR, NULL, "5\n\n\n0\n", output, sizeof(output));
    ASSERT_TRUE(strstr(output, "原密码不能为空") != NULL);
    ASSERT_TRUE(strstr(output, "数据库连接失败") == NULL);
}

static void test_register_confirmation(void)
{
    char output[8192];
    const char *input = "4\n测试患者\n1\n2000-01-01\nUT-UI-REGISTER\n13800000000\n"
                        "测试住址\n联系人\n13900000000\nnew-secret\nother-secret\n\n0\n";

    run_ui(ROLE_NONE, NULL, input, output, sizeof(output));
    ASSERT_TRUE(strstr(output, "姓名") != NULL);
    ASSERT_TRUE(strstr(output, "两次输入的密码不一致") != NULL);
    ASSERT_TRUE(strstr(output, "数据库连接失败") == NULL);
    ASSERT_TRUE(strstr(output, "注册成功") == NULL);
    ASSERT_TRUE(strstr(output, "new-secret") == NULL);
}

static void test_register_interruption(void)
{
    char output[8192];
    char input[256];

    run_ui(ROLE_NONE, NULL, "4\n测试患者\n1\n", output, sizeof(output));
    ASSERT_TRUE(strstr(output, "出生日期") != NULL);
    ASSERT_TRUE(strstr(output, "数据库连接失败") == NULL);
    ASSERT_TRUE(strstr(output, "注册成功") == NULL);
    memset(input, 'x', sizeof(input));
    input[0] = '4';
    input[1] = '\n';
    input[sizeof(input) - 1] = '\0';
    run_ui(ROLE_NONE, NULL, input, output, sizeof(output));
    ASSERT_TRUE(strstr(output, "姓名") != NULL);
    ASSERT_TRUE(strstr(output, "数据库连接失败") == NULL);
    ASSERT_TRUE(strstr(output, "注册成功") == NULL);
}

static void test_password_db(int role, const char *account)
{
    char output[8192];

    run_ui(role, account, "5\nwrong-secret\nnext-secret\nnext-secret\n\n0\n", output,
           sizeof(output));
    ASSERT_TRUE(strstr(output, "参数非法") != NULL);
    ASSERT_TRUE(strstr(output, "密码修改成功") == NULL);
    ASSERT_RC(user_login(role, account, "new-secret"));
    run_ui(role, account, "5\nnew-secret\nnext-secret\nnext-secret\n\n0\n", output, sizeof(output));
    ASSERT_TRUE(strstr(output, "密码修改成功") != NULL);
    ASSERT_TRUE(strstr(output, "next-secret") == NULL);
    ASSERT_EQ(user_login(role, account, "new-secret"), ERR_PARAM);
    ASSERT_RC(user_login(role, account, "next-secret"));
    session_clear();
}

static void test_register_db(void)
{
    char output[8192], patient_id[PID_LEN];
    db_result_t *rs = NULL;
    const char *input = "4\n测试患者\n2\n2000-01-01\nUT-UI-REGISTER\n13800000000\n"
                        "测试住址\n联系人\n13900000000\nnew-secret\nnew-secret\n\n0\n";

    run_ui(ROLE_NONE, NULL, input, output, sizeof(output));
    ASSERT_TRUE(strstr(output, "注册成功") != NULL);
    ASSERT_RC(db_query("SELECT patient_id,name,gender,phone,contact_name,contact_phone "
                       "FROM t_patient WHERE id_card='UT-UI-REGISTER'",
                       &rs));
    ASSERT_EQ(rs->rows, 1);
    ASSERT_TRUE(snprintf(patient_id, sizeof(patient_id), "%s", db_get(rs, 0, 0)) <
                (int)sizeof(patient_id));
    ASSERT_TRUE(strstr(output, patient_id) != NULL);
    ASSERT_STR_EQ(db_get(rs, 0, 1), "测试患者");
    ASSERT_STR_EQ(db_get(rs, 0, 2), "女");
    ASSERT_STR_EQ(db_get(rs, 0, 3), "13800000000");
    ASSERT_STR_EQ(db_get(rs, 0, 4), "联系人");
    ASSERT_STR_EQ(db_get(rs, 0, 5), "13900000000");
    db_result_free(rs);
    ASSERT_RC(user_login(ROLE_PATIENT, patient_id, "new-secret"));
    run_ui(ROLE_NONE, NULL, input, output, sizeof(output));
    ASSERT_TRUE(strstr(output, "记录重复") != NULL);
    ASSERT_TRUE(strstr(output, "注册成功") == NULL);
    test_password_db(ROLE_PATIENT, patient_id);
    ASSERT_RC(db_exec("DELETE FROM t_patient WHERE id_card='UT-UI-REGISTER'", NULL));
}

static void test_accounts_db(void)
{
    ASSERT_RC(db_exec("INSERT INTO t_admin(admin_id,password,name) "
                      "VALUES('utuiadm','new-secret','交互测试管理员')",
                      NULL));
    ASSERT_RC(db_exec("INSERT INTO t_doctor(doctor_id,password,name,dept) "
                      "VALUES('UTUI01','new-secret','交互测试医生','内科')",
                      NULL));
    test_register_db();
    test_password_db(ROLE_ADMIN, "utuiadm");
    test_password_db(ROLE_DOCTOR, "UTUI01");
    ASSERT_RC(db_exec("DELETE FROM t_admin WHERE admin_id='utuiadm'", NULL));
    ASSERT_RC(db_exec("DELETE FROM t_doctor WHERE doctor_id='UTUI01'", NULL));
}

int test_db_open(void);

int main(void)
{
    const char *enabled = getenv("RUN_DB_TESTS");

    test_password_confirmation(ROLE_ADMIN);
    test_password_confirmation(ROLE_DOCTOR);
    test_password_confirmation(ROLE_PATIENT);
    test_password_interruption();
    test_register_confirmation();
    test_register_interruption();
    if (enabled != NULL && strcmp(enabled, "1") == 0) {
        ASSERT_RC(test_db_open());
        test_accounts_db();
        db_close();
        puts("PASS: 注册成功/重复注册/账号回显/不自动登录，三端旧密码拒绝与新密码登录");
    } else {
        puts("PASS: 三端改密确认/输入中断与注册表单；SKIP: 成功路径需RUN_DB_TESTS=1");
    }
    return 0;
}
