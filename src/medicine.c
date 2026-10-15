/*
 * medicine.c  药品与库存流水（M06 / M10）
 * 作者：shangshuo   创建：2026-10-14
 */

#include "medicine.h"

#include "db.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 库存写入、盘点和流水接口仍为未实现桩。 */

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

static int medicine_where(const char *keyword, char *where, int size)
{
    char escaped[NAME_LEN * 4 + 1];
    char pattern[NAME_LEN * 6 + 1];
    const char *visible = g_session.role == ROLE_ADMIN ? "1=1" : "status = 1";
    int written;

    if (str_empty(keyword)) {
        written = snprintf(where, size, "%s", visible);
    } else {
        if (strlen(keyword) >= NAME_LEN)
            return ERR_PARAM;

        /* 与 patient.c / appointment.c 统一转义链路：db_escape 防注入，like_escape 再补一层
           LIKE 转义（% _ 各自转义，成对的 \ 翻倍）。不要在这里手工加倍反斜杠，
           否则会和 like_escape 叠加成两层转义，含反斜杠的药名就搜不到了。 */
        db_escape(keyword, escaped, sizeof(escaped));
        like_escape(escaped, pattern, sizeof(pattern));
        if (pattern[0] == '\0')
            return ERR_PARAM;

        written =
            snprintf(where, size, "%s AND (med_name LIKE '%%%s%%' OR manufacturer LIKE '%%%s%%')",
                     visible, pattern, pattern);
    }

    return written < 0 || written >= size ? ERR_PARAM : 0;
}

static int medicine_read_row(const db_result_t *rs, int row, medicine_t *entry)
{
    /* 检查字段长度，避免药品信息超出结构体容量 */
    if (strlen(db_get(rs, row, 0)) >= sizeof(entry->med_id) ||
        strlen(db_get(rs, row, 1)) >= sizeof(entry->med_name) ||
        strlen(db_get(rs, row, 2)) >= sizeof(entry->spec) ||
        strlen(db_get(rs, row, 3)) >= sizeof(entry->manufacturer) ||
        strlen(db_get(rs, row, 7)) >= sizeof(entry->create_time) ||
        strlen(db_get(rs, row, 8)) >= sizeof(entry->update_time))
        return ERR_DB_EXEC;

    /* 按 SELECT 的字段顺序读取药品信息 */
    snprintf(entry->med_id, sizeof(entry->med_id), "%s", db_get(rs, row, 0));
    snprintf(entry->med_name, sizeof(entry->med_name), "%s", db_get(rs, row, 1));
    snprintf(entry->spec, sizeof(entry->spec), "%s", db_get(rs, row, 2));
    snprintf(entry->manufacturer, sizeof(entry->manufacturer), "%s", db_get(rs, row, 3));

    entry->price = strtod(db_get(rs, row, 4), NULL);
    entry->stock = atoi(db_get(rs, row, 5));
    entry->status = atoi(db_get(rs, row, 6));

    snprintf(entry->create_time, sizeof(entry->create_time), "%s", db_get(rs, row, 7));
    snprintf(entry->update_time, sizeof(entry->update_time), "%s", db_get(rs, row, 8));

    return 0;
}

int medicine_query_all(const char *keyword, int page, medicine_t **out, int *out_n, int *out_total)
{
    db_result_t *rs = NULL;
    medicine_t *entries = NULL;
    char where[SQL_BUF / 2], sql[SQL_BUF];
    long total;
    int rc, written, i;

    if (out == NULL || out_n == NULL)
        return ERR_PARAM;

    *out = NULL;
    *out_n = 0;
    if (out_total != NULL)
        *out_total = 0;

    /* 查询前检查登录状态和会话是否超时 */
    if (!session_is_login())
        return ERR_PERM;
    rc = session_check_timeout();
    if (rc != 0)
        return rc;

    rc = medicine_where(keyword, where, sizeof(where));
    if (rc != 0)
        return rc;

    /* 统计符合条件的药品总数，用于分页 */
    written = snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM t_medicine WHERE %s", where);
    if (written < 0 || written >= (int)sizeof(sql))
        return ERR_PARAM;

    rc = db_query(sql, &rs);
    if (rc != 0)
        return rc;

    total = strtol(db_get(rs, 0, 0), NULL, 10);
    db_result_free(rs);
    rs = NULL;
    if (total < 0 || total > INT_MAX)
        return ERR_DB_EXEC;

    /* 按药品编号排序，查询当前页 */
    written = snprintf(sql, sizeof(sql),
                       "SELECT med_id, med_name, spec, manufacturer, price, stock, status, "
                       "create_time, update_time FROM t_medicine WHERE %s "
                       "ORDER BY med_id LIMIT %d OFFSET %d",
                       where, PAGE_SIZE, page_offset(page));
    if (written < 0 || written >= (int)sizeof(sql))
        return ERR_PARAM;

    rc = db_query(sql, &rs);
    if (rc != 0)
        return rc;

    if (rs->rows > 0) {
        entries = calloc((size_t)rs->rows, sizeof(*entries));
        if (entries == NULL) {
            rc = ERR_NOMEM;
            goto cleanup;
        }

        for (i = 0; i < rs->rows; i++) {
            rc = medicine_read_row(rs, i, &entries[i]);
            if (rc != 0)
                goto cleanup;
        }
    }

    *out = entries;
    *out_n = rs->rows;
    if (out_total != NULL)
        *out_total = (int)total;

cleanup:
    if (rc != 0)
        free(entries);
    db_result_free(rs);
    return rc;
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
