/*
 * visit.c  接诊与就诊记录（M04）
 * 作者：B   创建：2026-10-14
 */

#include "visit.h"

#include <stdlib.h>
#include <string.h>

/* 业务接口均为未实现桩；仅清空出参并返回 ERR_PARAM，不模拟业务成功。 */

int visit_take(int appoint_id, const visit_t *v, int *out_visit_id)
{
    (void)appoint_id;
    (void)v;
    if (out_visit_id != NULL)
        *out_visit_id = 0;
    return ERR_PARAM;
}

int visit_add(const visit_t *v, int *out_visit_id)
{
    (void)v;
    if (out_visit_id != NULL)
        *out_visit_id = 0;
    return ERR_PARAM;
}

int visit_query_by_id(int visit_id, visit_t *out)
{
    (void)visit_id;
    if (out != NULL)
        memset(out, 0, sizeof(*out));
    return ERR_PARAM;
}

int visit_query_by_appoint(int appoint_id, visit_t *out)
{
    (void)appoint_id;
    if (out != NULL)
        memset(out, 0, sizeof(*out));
    return ERR_PARAM;
}

int visit_query_by_patient(const char *patient_id, int page, visit_t **out, int *out_n,
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

int visit_query_by_doctor(const char *doctor_id, int page, visit_t **out, int *out_n,
                          int *out_total)
{
    (void)doctor_id;
    (void)page;
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    if (out_total != NULL)
        *out_total = 0;
    return ERR_PARAM;
}

void visit_list_free(visit_t *list)
{
    free(list);
}
