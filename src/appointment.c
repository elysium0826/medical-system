/*
 * appointment.c  预约挂号与状态迁移（M03）
 * 作者：A   创建：2026-10-14
 */

#include "appointment.h"

#include <stdlib.h>
#include <string.h>

/* 业务接口均为未实现桩；仅清空出参并返回 ERR_PARAM，不模拟业务成功。 */

int appointment_add(const char *patient_id, const char *doctor_id, int schedule_id,
                    const char *appoint_date, const char *time_slot, const char *symptom)
{
    (void)patient_id;
    (void)doctor_id;
    (void)schedule_id;
    (void)appoint_date;
    (void)time_slot;
    (void)symptom;
    return ERR_PARAM;
}

int appointment_set_status(int appoint_id, int new_status, const char *reason)
{
    (void)appoint_id;
    (void)new_status;
    (void)reason;
    return ERR_PARAM;
}

int appointment_cancel(int appoint_id)
{
    (void)appoint_id;
    return ERR_PARAM;
}

int appointment_reschedule(int appoint_id, const char *new_date, const char *new_slot,
                           int operator_role, appoint_t *out_new_appoint)
{
    (void)appoint_id;
    (void)new_date;
    (void)new_slot;
    (void)operator_role;
    if (out_new_appoint != NULL)
        memset(out_new_appoint, 0, sizeof(*out_new_appoint));
    return ERR_PARAM;
}

int appointment_mark_noshow(int appoint_id)
{
    (void)appoint_id;
    return ERR_PARAM;
}

int appointment_query_by_id(int appoint_id, appoint_t *out)
{
    (void)appoint_id;
    if (out != NULL)
        memset(out, 0, sizeof(*out));
    return ERR_PARAM;
}

int appointment_query_all(const char *keyword, int page, appoint_t **out, int *out_n,
                          int *out_total)
{
    (void)keyword;
    (void)page;
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    if (out_total != NULL)
        *out_total = 0;
    return ERR_PARAM;
}

int appointment_query_by_patient(const char *patient_id, int page, appoint_t **out, int *out_n,
                                 int *out_total)
{
    (void)patient_id;
    (void)page;
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    if (out_total != NULL)
        *out_total = 0;
    return ERR_PARAM;
}

int appointment_query_by_doctor(const char *doctor_id, const char *date, int page, appoint_t **out,
                                int *out_n, int *out_total)
{
    (void)doctor_id;
    (void)date;
    (void)page;
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    if (out_total != NULL)
        *out_total = 0;
    return ERR_PARAM;
}

void appointment_list_free(appoint_t *list)
{
    free(list);
}
