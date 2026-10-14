/*
 * patient.c  患者档案与注册（M01）
 * 作者：A   创建：2026-10-14
 */

#include "patient.h"

#include <stdlib.h>
#include <string.h>

/* 业务接口均为未实现桩；仅清空出参并返回 ERR_PARAM，不模拟业务成功。 */

int patient_register(const patient_t *p, char *out_patient_id)
{
    (void)p;
    if (out_patient_id != NULL)
        out_patient_id[0] = '\0';
    return ERR_PARAM;
}

int patient_update_self(const patient_t *p)
{
    (void)p;
    return ERR_PARAM;
}

int patient_set_status(const char *patient_id, int new_status)
{
    (void)patient_id;
    (void)new_status;
    return ERR_PARAM;
}

int patient_delete(const char *patient_id)
{
    (void)patient_id;
    return ERR_PARAM;
}

int patient_query_by_id(const char *patient_id, patient_t *out)
{
    (void)patient_id;
    if (out != NULL)
        memset(out, 0, sizeof(*out));
    return ERR_PARAM;
}

int patient_query_all(const char *keyword, int page, patient_t **out, int *out_n, int *out_total)
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

void patient_list_free(patient_t *list)
{
    free(list);
}
