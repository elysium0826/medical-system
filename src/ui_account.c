/*
 * ui_account.c  登录、患者注册与三端共享改密交互（M08）
 * 作者：D   创建：2026-10-16
 */
#include "ui_account.h"

#include "common.h"
#include "ui.h"
#include "user.h"

#include <stdio.h>
#include <string.h>

static int account_read_password(const char *prompt, char *password, int capacity)
{
    char confirm[NAME_LEN] = {0};
    int rc = ERR_PARAM;

    read_password(prompt, password, capacity);
    if (str_empty(password)) {
        printf("密码不能为空或输入已中断\n");
        return rc;
    }
    read_password("确认密码：", confirm, sizeof(confirm));
    if (str_empty(confirm)) {
        printf("确认密码不能为空或输入已中断\n");
    } else if (strcmp(password, confirm) != 0) {
        printf("两次输入的密码不一致\n");
    } else {
        rc = 0;
    }
    memset(confirm, 0, sizeof(confirm));
    return rc;
}

void ui_change_password(void)
{
    char old_pass[NAME_LEN] = {0};
    char new_pass[NAME_LEN] = {0};
    int rc = session_check_timeout();

    if (rc != 0) {
        printf("%s\n", err_str(rc));
        return;
    }
    print_title("修改密码");
    read_password("原密码：", old_pass, sizeof(old_pass));
    if (str_empty(old_pass)) {
        printf("原密码不能为空或输入已中断\n");
        goto cleanup;
    }
    if (account_read_password("新密码：", new_pass, sizeof(new_pass)) != 0) {
        goto cleanup;
    }
    /* 填表期间也可能超时，提交前重新检查，不能只依赖主菜单的检查。 */
    rc = session_check_timeout();
    if (rc == 0) {
        session_touch();
        rc = user_change_password(old_pass, new_pass);
    }
    printf("%s\n", rc == 0 ? "密码修改成功" : err_str(rc));

cleanup:
    memset(old_pass, 0, sizeof(old_pass));
    memset(new_pass, 0, sizeof(new_pass));
    if (!feof(stdin)) {
        pause_wait();
    }
}

static int account_read_patient(patient_t *patient)
{
    int gender;

    if (read_line("姓名：", patient->name, sizeof(patient->name)) != 0) {
        return ERR_PARAM;
    }
    gender = read_choice("性别[1 男 / 2 女 / 0 返回]：", 0, 2);
    if (gender <= 0) {
        return ERR_PARAM;
    }
    memcpy(patient->gender, gender == 1 ? "男" : "女", sizeof("男"));
    if (read_date("出生日期(YYYY-MM-DD)：", patient->birth_date) != 0 ||
        read_line("身份证：", patient->id_card, sizeof(patient->id_card)) != 0 ||
        read_line("电话（可空）：", patient->phone, sizeof(patient->phone)) != 0 ||
        read_line("住址（可空）：", patient->address, sizeof(patient->address)) != 0 ||
        read_line("紧急联系人（可空）：", patient->contact_name, sizeof(patient->contact_name)) !=
            0 ||
        read_line("联系人电话（可空）：", patient->contact_phone, sizeof(patient->contact_phone)) !=
            0) {
        return ERR_PARAM;
    }
    return account_read_password("登录密码：", patient->password, sizeof(patient->password));
}

void ui_register_patient(void)
{
    patient_t patient = {0};
    char patient_id[PID_LEN] = {0};
    int rc;

    print_title("患者注册");
    rc = account_read_patient(&patient);
    if (rc != 0) {
        printf("已取消注册\n");
    } else {
        rc = user_register_patient(&patient, patient_id);
        if (rc == 0) {
            printf("注册成功，患者账号：%s，请返回登录\n", patient_id);
        } else {
            printf("注册失败：%s\n", err_str(rc));
        }
    }
    memset(&patient, 0, sizeof(patient));
    if (!feof(stdin)) {
        pause_wait();
    }
}
