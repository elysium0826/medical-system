/*
 * prescription.c  处方与处方模板（M05）
 * 作者：B   创建：2026-10-14
 */

#include "prescription.h"

#include <stdlib.h>
#include <string.h>

/* 业务接口均为未实现桩；仅清空出参并返回 ERR_PARAM，不模拟业务成功。 */

int prescription_add(int visit_id, const presc_item_t *items, int n, int *out_presc_id)
{
    (void)visit_id;
    (void)items;
    (void)n;
    if (out_presc_id != NULL)
        *out_presc_id = 0;
    return ERR_PARAM;
}

int prescription_void(int presc_id)
{
    (void)presc_id;
    return ERR_PARAM;
}

int prescription_query_by_id(int presc_id, presc_t *out)
{
    (void)presc_id;
    if (out != NULL)
        memset(out, 0, sizeof(*out));
    return ERR_PARAM;
}

int prescription_query_by_visit(int visit_id, presc_t **out, int *out_n)
{
    (void)visit_id;
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    return ERR_PARAM;
}

int prescription_query_by_patient(const char *patient_id, int page, presc_t **out, int *out_n,
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

int prescription_query_items(int presc_id, presc_item_t **out, int *out_n)
{
    (void)presc_id;
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    return ERR_PARAM;
}

void prescription_list_free(presc_t *list)
{
    free(list);
}

void presc_item_list_free(presc_item_t *list)
{
    free(list);
}

int presc_template_add(const char *tpl_name, const char *remark, const presc_item_t *items, int n)
{
    (void)tpl_name;
    (void)remark;
    (void)items;
    (void)n;
    return ERR_PARAM;
}

int presc_template_apply(int tpl_id, int visit_id, const char *diagnosis, presc_item_t **out_items,
                         int *out_n, int *out_skipped)
{
    (void)tpl_id;
    (void)visit_id;
    (void)diagnosis;
    if (out_items != NULL)
        *out_items = NULL;
    if (out_n != NULL)
        *out_n = 0;
    if (out_skipped != NULL)
        *out_skipped = 0;
    return ERR_PARAM;
}

int presc_template_delete(int tpl_id)
{
    (void)tpl_id;
    return ERR_PARAM;
}

int presc_template_query_all(presc_tpl_t **out, int *out_n)
{
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    return ERR_PARAM;
}

void presc_tpl_list_free(presc_tpl_t *list)
{
    free(list);
}
