/*
 * ui_doctor.c  医生端菜单与待办入口（B）
 * 作者：B   创建：2026-10-14
 */
#include "ui.h"
#include "ui_account.h"

#include "medical_record.h"
#include "prescription.h"
#include "todo.h"
#include "visit.h"

#include <stdio.h>
static void doctor_visit_menu(void)
{
    visit_t *visits = NULL;
    int count = 0;
    int total = 0;
    int rc = visit_query_by_doctor(g_session.user_id, 1, &visits, &count, &total);

    printf("接诊记录未实现：%s\n", err_str(rc));
    if (rc == 0)
        visit_list_free(visits);
}

static void doctor_prescription_menu(void)
{
    presc_tpl_t *templates = NULL;
    int count = 0;
    int rc = presc_template_query_all(&templates, &count);

    printf("处方与模板未实现：%s\n", err_str(rc));
    if (rc == 0)
        presc_tpl_list_free(templates);
}

static void doctor_record_menu(void)
{
    char path[TEXT_LEN] = {0};
    int rc = medical_record_print(0, path, sizeof(path));

    printf("病历书写与打印未实现：%s\n", err_str(rc));
}

void ui_doctor_main(void)
{
    int choice;
    int rc = session_require_role(ROLE_DOCTOR);

    if (rc != 0) {
        printf("%s\n", err_str(rc));
        return;
    }
    for (;;) {
        rc = session_check_timeout();
        if (rc != 0) {
            printf("%s\n", err_str(rc));
            return;
        }
        session_touch();
        print_title("医生端");
        printf("  1. 今日待办\n  2. 接诊记录\n  3. 处方与模板\n  4. 病历书写与打印\n");
        printf("  5. 修改密码\n  0. 退出\n");
        choice = read_choice("请输入选项[0-5]：", 0, 5);
        if (choice < 0)
            return;
        rc = session_check_timeout();
        if (rc != 0) {
            printf("%s\n", err_str(rc));
            return;
        }
        session_touch();
        switch (choice) {
        case 0:
            return;
        case 1:
            todo_print(ROLE_DOCTOR);
            break;
        case 2:
            doctor_visit_menu();
            break;
        case 3:
            doctor_prescription_menu();
            break;
        case 4:
            doctor_record_menu();
            break;
        case 5:
            ui_change_password();
            break;
        }
    }
}
