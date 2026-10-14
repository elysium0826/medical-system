/*
 * medicine.c  药品与库存流水（M06 / M10）
 * 作者：C   创建：2026-10-14
 */

#include "medicine.h"

#include <stdlib.h>
#include <string.h>

/* 业务接口均为未实现桩；仅清空出参并返回 ERR_PARAM，不模拟业务成功。 */

int medicine_add(const medicine_t *m, char *out_med_id)
{
    (void)m;
    if (out_med_id != NULL)
        out_med_id[0] = '\0';
    return ERR_PARAM;
}

int medicine_update_price(const char *med_id, double new_price)
{
    (void)med_id;
    (void)new_price;
    return ERR_PARAM;
}

int medicine_change_stock(const char *med_id, int delta, int change_type, const char *ref_id,
                          const char *operator)
{
    (void)med_id;
    (void)delta;
    (void)change_type;
    (void)ref_id;
    (void)operator;
    return ERR_PARAM;
}

int medicine_set_status(const char *med_id, int new_status)
{
    (void)med_id;
    (void)new_status;
    return ERR_PARAM;
}

int medicine_stock_in(const char *med_id, int quantity, const char *operator)
{
    (void)med_id;
    (void)quantity;
    (void)operator;
    return ERR_PARAM;
}

int medicine_stock_out(const char *med_id, int quantity, const char *operator)
{
    (void)med_id;
    (void)quantity;
    (void)operator;
    return ERR_PARAM;
}

int medicine_check_stock(const char *med_id, int actual, const char *batch, const char *operator)
{
    (void)med_id;
    (void)actual;
    (void)batch;
    (void)operator;
    return ERR_PARAM;
}

int medicine_query_by_id(const char *med_id, medicine_t *out)
{
    (void)med_id;
    if (out != NULL)
        memset(out, 0, sizeof(*out));
    return ERR_PARAM;
}

int medicine_query_all(const char *keyword, int page, medicine_t **out, int *out_n, int *out_total)
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

int medicine_query_low_stock(medicine_t **out, int *out_n)
{
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    return ERR_PARAM;
}

int medicine_query_stock_log(const char *med_id, int change_type, const char *date_from,
                             const char *date_to, int page, stock_log_t **out, int *out_n,
                             int *out_total)
{
    (void)med_id;
    (void)change_type;
    (void)date_from;
    (void)date_to;
    (void)page;
    if (out != NULL)
        *out = NULL;
    if (out_n != NULL)
        *out_n = 0;
    if (out_total != NULL)
        *out_total = 0;
    return ERR_PARAM;
}

void medicine_list_free(medicine_t *list)
{
    free(list);
}

void stock_log_list_free(stock_log_t *list)
{
    free(list);
}
