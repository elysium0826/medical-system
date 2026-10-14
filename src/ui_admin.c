/*
 * ui_admin.c  管理员端菜单与待办入口（C）
 * 作者：C   创建：2026-10-14
 */
#include "ui.h"

#include "doctor.h"
#include "export.h"
#include "medicine.h"
#include "todo.h"

#include <stdio.h>
static void admin_doctor_menu(void)
{
    doctor_t *doctors = NULL;
    int count = 0;
    int total = 0;
    int rc = doctor_query_all(NULL, 1, &doctors, &count, &total);

    printf("医生档案未实现：%s\n", err_str(rc));
    if (rc == 0)
        doctor_list_free(doctors);
}

static void admin_medicine_menu(void)
{
    medicine_t *medicines = NULL;
    int count = 0;
    int total = 0;
    int rc = medicine_query_all(NULL, 1, &medicines, &count, &total);

    printf("药品库存未实现：%s\n", err_str(rc));
    if (rc == 0)
        medicine_list_free(medicines);
}

static void admin_export_menu(void)
{
    const char *fields[1] = {NULL};
    int rc = export_field_list("patient", fields, 1);

    printf("列表导出未实现：%s\n", err_str(rc));
}

void ui_admin_main(void)
{
    int choice;
    int rc = session_require_role(ROLE_ADMIN);

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
        print_title("管理员端");
        printf("  1. 今日待办\n  2. 医生档案\n  3. 药品库存\n  4. 列表导出\n  0. 退出\n");
        choice = read_choice("请输入选项[0-4]：", 0, 4);
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
            todo_print(ROLE_ADMIN);
            break;
        case 2:
            admin_doctor_menu();
            break;
        case 3:
            admin_medicine_menu();
            break;
        case 4:
            admin_export_menu();
            break;
        }
    }
}
