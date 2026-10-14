/*
 * doctor.c  医生档案维护（M02）
 * 作者：C   创建：2026-10-14
 */

#include "doctor.h"

#include <stdlib.h>
#include <string.h>

/* 业务接口均为未实现桩；仅清空出参并返回 ERR_PARAM，不模拟业务成功。 */

int doctor_add(const doctor_t *d, char *out_doctor_id)
{
    (void)d;
    if (out_doctor_id != NULL)
        out_doctor_id[0] = '\0';
    return ERR_PARAM;
}

int doctor_update(const doctor_t *d)
{
    (void)d;
    return ERR_PARAM;
}

int doctor_set_status(const char *doctor_id, int new_status)
{
    (void)doctor_id;
    (void)new_status;
    return ERR_PARAM;
}

int doctor_query_by_id(const char *doctor_id, doctor_t *out)
{
    (void)doctor_id;
    if (out != NULL)
        memset(out, 0, sizeof(*out));
    return ERR_PARAM;
}

int doctor_query_all(const char *keyword, int page, doctor_t **out, int *out_n, int *out_total)
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

int doctor_query_by_dept(const char *dept, doctor_t **out, int *out_n)
{
    (void)dept;
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    return ERR_PARAM;
}

void doctor_list_free(doctor_t *list)
{
    free(list);
}
