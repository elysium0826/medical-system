/*
 * admin_1.c  管理员端菜单与待办入口
 * 作者：shangshuo  创建：2026-10-14
 */
#include "ui.h"
#include "ui_account.h"

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

static void admin_medicine_print(const medicine_t *medicines, int count)
{
    int i;

    if (count == 0) {
        printf("暂无药品记录\n");
        return;
    }

    print_table_header("编号 | 药品 | 规格 | 单价 | 库存 | 状态");
    for (i = 0; i < count; i++) {
        const char *status = "停用";
        const char *stock_note = "";

        if (medicines[i].status == MED_ON_SALE)
            status = "在售";

        /* 库存低于预警值时标注提示 */
        if (medicines[i].stock < LOW_STOCK)
            stock_note = " [库存偏低]";

        print_table_row("%s | %s | %s | %.2f | %d | %s%s", medicines[i].med_id,
                        medicines[i].med_name, medicines[i].spec, medicines[i].price,
                        medicines[i].stock, status, stock_note);
    }
    print_table_end();
}

static void admin_medicine_menu(void)
{
    char keyword[NAME_LEN];
    int page = 1;
    int choice;
    int rc;

    if (read_line("药名或厂家关键字（回车查全部）：", keyword, sizeof(keyword)) != 0)
        return;

    for (;;) {
        medicine_t *medicines = NULL;
        int count = 0;
        int total = 0;

        rc = session_check_timeout();
        if (rc != 0) {
            printf("%s\n", err_str(rc));
            return;
        }
        session_touch();

        /* 按药名或厂家关键字查询当前页 */
        rc = medicine_query_all(keyword, page, &medicines, &count, &total);
        if (rc != 0) {
            printf("药品查询失败：%s\n", err_str(rc));
            pause_wait();
            return;
        }

        /* 显示库存列表和页码 */
        print_title("药品库存");
        admin_medicine_print(medicines, count);
        print_page_info(page, total);

        /* 释放当前页数据，下次翻页时重新查询 */
        medicine_list_free(medicines);

        printf("  1. 下一页\n  2. 上一页\n  0. 返回\n");
        choice = read_choice("请输入选项[0-2]：", 0, 2);
        if (choice <= 0)
            return;

        if (choice == 1 && page < page_count(total))
            page++;
        else if (choice == 2 && page > 1)
            page--;
        else if (choice == 1)
            printf("已经是最后一页\n");
        else
            printf("已经是第一页\n");
    }
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
        printf("  1. 今日待办\n  2. 医生档案\n  3. 药品库存\n  4. 列表导出\n");
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
        case 5:
            ui_change_password();
            break;
        }
    }
}
