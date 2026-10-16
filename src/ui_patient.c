/*
 * ui_patient.c  患者端菜单与待办入口（A）
 * 作者：A   创建：2026-10-14
 */
#include "ui.h"
#include "ui_account.h"

#include "appointment.h"
#include "patient.h"
#include "schedule.h"
#include "todo.h"

#include <stdio.h>

/* 读一行，非空才覆盖 dst；用于"回车保持不变"的可选修改字段。 */
static void read_or_keep(const char *prompt, char *dst, int dstlen)
{
    char line[128];

    if (read_line(prompt, line, sizeof(line)) == 0 && line[0] != '\0') {
        snprintf(dst, (size_t)dstlen, "%s", line);
    }
}

static const char *appt_status_text(int status)
{
    switch (status) {
    case APPT_WAITING:
        return "待就诊";
    case APPT_VISITED:
        return "已就诊";
    case APPT_CANCELED:
        return "已取消";
    case APPT_NOSHOW:
        return "爽约";
    default:
        return "未知";
    }
}

/* 档案：先展示本人完整档案，再按需改电话、住址与紧急联系人。 */
static void patient_profile_menu(void)
{
    patient_t p;
    int rc = patient_query_by_id(g_session.user_id, &p);

    if (rc != 0) {
        printf("%s\n", err_str(rc));
        pause_wait();
        return;
    }
    print_title("我的档案");
    printf("编号：%s\n姓名：%s\n性别：%s\n出生日期：%s\n身份证：%s\n", p.patient_id, p.name,
           p.gender, p.birth_date, p.id_card);
    printf("电话：%s\n住址：%s\n紧急联系人：%s（%s）\n状态：%s\n", p.phone, p.address,
           p.contact_name, p.contact_phone, p.status == 1 ? "正常" : "注销");
    if (read_choice("是否修改联系方式？[1 是 / 0 否]：", 0, 1) != 1) {
        return;
    }
    read_or_keep("新电话（回车不改）：", p.phone, sizeof(p.phone));
    read_or_keep("新住址（回车不改）：", p.address, sizeof(p.address));
    read_or_keep("新紧急联系人（回车不改）：", p.contact_name, sizeof(p.contact_name));
    read_or_keep("新联系人电话（回车不改）：", p.contact_phone, sizeof(p.contact_phone));
    rc = patient_update_self(&p);
    printf("%s\n", rc == 0 ? "资料已更新。" : err_str(rc));
    pause_wait();
}

/* 打印本人预约列表（分页取第一页，够演示用）。 */
static void patient_show_appointments(void)
{
    appoint_t *list = NULL;
    int n = 0, total = 0, i;
    int rc = appointment_query_by_patient(g_session.user_id, 1, &list, &n, &total);

    if (rc != 0) {
        printf("查询预约失败：%s\n", err_str(rc));
        return;
    }
    print_table_header("%-6s %-10s %-12s %-12s %-6s %-6s", "预约号", "医生", "科室", "日期", "时段",
                       "状态");
    for (i = 0; i < n; i++) {
        print_table_row("%-6d %-10s %-12s %-12s %-6s %-6s", list[i].appoint_id, list[i].doctor_name,
                        list[i].dept, list[i].appoint_date, list[i].time_slot,
                        appt_status_text(list[i].status));
    }
    print_table_end();
    print_page_info(1, total);
    appointment_list_free(list);
}

static void patient_cancel_appointment(void)
{
    int id, rc;

    if (read_int("请输入要取消的预约编号（0 返回）：", &id) != 0 || id <= 0) {
        return;
    }
    rc = appointment_cancel(id);
    printf("%s\n", rc == 0 ? "已取消，号源已释放。" : err_str(rc));
    pause_wait();
}

static void patient_reschedule_appointment(void)
{
    appoint_t out;
    char date[11];
    int id, choice, rc;

    if (read_int("请输入要改期的预约编号（0 返回）：", &id) != 0 || id <= 0) {
        return;
    }
    if (read_date("请输入新就诊日期(YYYY-MM-DD)：", date) != 0) {
        return;
    }
    choice = read_choice("时段[1 上午 / 2 下午]：", 1, 2);
    if (choice < 0) {
        return;
    }
    rc = appointment_reschedule(id, date, choice == 1 ? SLOT_AM : SLOT_PM, g_session.role, &out);
    printf("%s\n", rc == 0 ? "改期成功。" : err_str(rc));
    pause_wait();
}

/* 用选中的号源给本人挂号。 */
static void patient_book(const schedule_t *s)
{
    char symptom[256] = {0};
    int rc;

    /* 读取失败（EOF 或输入异常）不能当成"空症状"继续挂号，直接取消这次操作 */
    if (read_line("请输入症状（可空，回车跳过）：", symptom, sizeof(symptom)) != 0) {
        printf("输入已中断，已取消挂号。\n");
        pause_wait();
        return;
    }
    rc = appointment_add(g_session.user_id, s->doctor_id, s->schedule_id, s->work_date,
                         s->time_slot, symptom);
    printf("%s\n", rc == 0 ? "挂号成功。" : err_str(rc));
    pause_wait();
}

/* 排班号源：列出今天起一周内在诊且未约满的号源，选一个即挂号。 */
static void patient_schedule_menu(void)
{
    schedule_t *list = NULL;
    schedule_t picked;
    int n = 0, i, id, rc;

    rc = schedule_query_by_doctor(NULL, NULL, NULL, 1, &list, &n);
    if (rc != 0) {
        printf("查询排班失败：%s\n", err_str(rc));
        return;
    }
    if (n == 0) {
        printf("今天起一周内暂无可预约号源。\n");
        schedule_list_free(list);
        pause_wait();
        return;
    }
    print_table_header("%-6s %-10s %-12s %-12s %-6s %-8s", "号源", "医生", "科室", "日期", "时段",
                       "余号");
    for (i = 0; i < n; i++) {
        char quota[16];

        snprintf(quota, sizeof(quota), "%d/%d", list[i].quota_total - list[i].quota_used,
                 list[i].quota_total);
        print_table_row("%-6d %-10s %-12s %-12s %-6s %-8s", list[i].schedule_id,
                        list[i].doctor_name, list[i].dept, list[i].work_date, list[i].time_slot,
                        quota);
    }
    print_table_end();
    schedule_list_free(list);
    if (read_int("请输入要挂号的号源编号（0 返回）：", &id) != 0 || id <= 0) {
        return;
    }
    rc = schedule_query_by_id(id, &picked);
    if (rc != 0) {
        printf("%s\n", err_str(rc));
        pause_wait();
        return;
    }
    patient_book(&picked);
}

static void patient_appointment_menu(void)
{
    int choice;

    for (;;) {
        patient_show_appointments();
        printf("\n  1. 挂号\n  2. 取消预约\n  3. 改期\n  0. 返回\n");
        choice = read_choice("请输入选项[0-3]：", 0, 3);
        if (choice < 0 || choice == 0) {
            return;
        }
        if (session_check_timeout() != 0) {
            printf("%s\n", err_str(ERR_TIMEOUT));
            return;
        }
        session_touch();
        switch (choice) {
        case 1:
            patient_schedule_menu();
            break;
        case 2:
            patient_cancel_appointment();
            break;
        case 3:
            patient_reschedule_appointment();
            break;
        }
    }
}

void ui_patient_main(void)
{
    int choice;
    int rc = session_require_role(ROLE_PATIENT);

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
        print_title("患者端");
        printf("  1. 今日待办\n  2. 患者档案\n  3. 预约挂号\n  4. 排班号源\n");
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
            todo_print(ROLE_PATIENT);
            break;
        case 2:
            patient_profile_menu();
            break;
        case 3:
            patient_appointment_menu();
            break;
        case 4:
            patient_schedule_menu();
            break;
        case 5:
            ui_change_password();
            break;
        }
    }
}
