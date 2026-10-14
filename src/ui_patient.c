/*
 * ui_patient.c  患者端菜单与待办入口（A）
 * 作者：A   创建：2026-10-14
 */
#include "ui.h"

#include "appointment.h"
#include "patient.h"
#include "schedule.h"
#include "todo.h"

#include <stdio.h>
static void patient_profile_menu(void)
{
    patient_t patient = {0};
    int rc = patient_query_by_id(g_session.user_id, &patient);

    printf("患者档案未实现：%s\n", err_str(rc));
}

static void patient_appointment_menu(void)
{
    appoint_t *appointments = NULL;
    int count = 0;
    int total = 0;
    int rc = appointment_query_by_patient(g_session.user_id, 1, &appointments, &count, &total);

    printf("预约挂号未实现：%s\n", err_str(rc));
    if (rc == 0)
        appointment_list_free(appointments);
}

static void patient_schedule_menu(void)
{
    schedule_t *schedules = NULL;
    int count = 0;
    int rc = schedule_query_by_doctor(NULL, NULL, NULL, 1, &schedules, &count);

    printf("排班号源未实现：%s\n", err_str(rc));
    if (rc == 0)
        schedule_list_free(schedules);
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
        printf("  1. 今日待办\n  2. 患者档案\n  3. 预约挂号\n  4. 排班号源\n  0. 退出\n");
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
        }
    }
}
