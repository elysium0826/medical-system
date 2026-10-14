/*
 * medical_record.c  病历书写与打印（M07）
 * 作者：B   创建：2026-10-14
 */

#include "medical_record.h"

#include <stdlib.h>
#include <string.h>

/* 业务接口均为未实现桩；仅清空出参并返回 ERR_PARAM，不模拟业务成功。 */

int medical_record_add(const medrec_t *m, int *out_record_id)
{
    (void)m;
    if (out_record_id != NULL)
        *out_record_id = 0;
    return ERR_PARAM;
}

int medical_record_update(const medrec_t *m)
{
    (void)m;
    return ERR_PARAM;
}

int medical_record_query_by_visit(int visit_id, medrec_t *out)
{
    (void)visit_id;
    if (out != NULL)
        memset(out, 0, sizeof(*out));
    return ERR_PARAM;
}

int medical_record_query_by_doctor(const char *doctor_id, int page, medrec_t **out, int *out_n,
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

int medical_record_print(int visit_id, char *out_path, int outlen)
{
    (void)visit_id;
    if (out_path != NULL && outlen > 0)
        out_path[0] = '\0';
    return ERR_PARAM;
}
