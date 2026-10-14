/*
 * schedule.c  排班与号源（M09）
 * 作者：A   创建：2026-10-14
 */

#include "schedule.h"

#include <stdlib.h>
#include <string.h>

/* 业务接口均为未实现桩；仅清空出参并返回 ERR_PARAM，不模拟业务成功。 */

int schedule_add_week(const char *doctor_id, const char *monday, int quota_am, int quota_pm,
                      int *out_created)
{
    (void)doctor_id;
    (void)monday;
    (void)quota_am;
    (void)quota_pm;
    if (out_created != NULL)
        *out_created = 0;
    return ERR_PARAM;
}

int schedule_query_by_doctor(const char *doctor_id, const char *date_from, const char *date_to,
                             int only_available, schedule_t **out, int *out_n)
{
    (void)doctor_id;
    (void)date_from;
    (void)date_to;
    (void)only_available;
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    return ERR_PARAM;
}

int schedule_query_by_id(int schedule_id, schedule_t *out)
{
    (void)schedule_id;
    if (out != NULL)
        memset(out, 0, sizeof(*out));
    return ERR_PARAM;
}

int schedule_set_quota(int schedule_id, int new_total)
{
    (void)schedule_id;
    (void)new_total;
    return ERR_PARAM;
}

int schedule_set_status(int schedule_id, int new_status, int *out_blocked)
{
    (void)schedule_id;
    (void)new_status;
    if (out_blocked != NULL)
        *out_blocked = 0;
    return ERR_PARAM;
}

int schedule_take_quota(int schedule_id)
{
    (void)schedule_id;
    return ERR_PARAM;
}

int schedule_release_quota(int schedule_id)
{
    (void)schedule_id;
    return ERR_PARAM;
}

void schedule_list_free(schedule_t *list)
{
    free(list);
}
