/*
 * schedule.c  排班与号源（M09）
 * 作者：A   创建：2026-10-14
 */

#include "schedule.h"

#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 排班列表与详情共用同一段 SELECT，列顺序和 schedule_read_row 的序号一一对应。 */
#define SCHEDULE_FIELDS                                                                            \
    "s.schedule_id, s.doctor_id, d.name, d.dept, s.work_date, s.time_slot, "                       \
    "s.quota_total, s.quota_used, s.status"

#define SCHEDULE_FROM                                                                              \
    " FROM t_schedule s "                                                                          \
    "LEFT JOIN t_doctor d ON d.doctor_id = s.doctor_id"

/* 只看还能约的时段：在诊、未约满、不早于今天，且医生本人是在职状态（离职医生不再放出号源） */
#define SCHEDULE_AVAILABLE                                                                         \
    " AND s.status = 1 AND s.quota_used < s.quota_total AND s.work_date >= CURDATE() "             \
    "AND d.status = 1"

static void schedule_read_row(const db_result_t *rs, int row, schedule_t *out)
{
    memset(out, 0, sizeof(*out));
    out->schedule_id = atoi(db_get(rs, row, 0));
    snprintf(out->doctor_id, sizeof(out->doctor_id), "%s", db_get(rs, row, 1));
    snprintf(out->doctor_name, sizeof(out->doctor_name), "%s", db_get(rs, row, 2));
    snprintf(out->dept, sizeof(out->dept), "%s", db_get(rs, row, 3));
    snprintf(out->work_date, sizeof(out->work_date), "%s", db_get(rs, row, 4));
    snprintf(out->time_slot, sizeof(out->time_slot), "%s", db_get(rs, row, 5));
    out->quota_total = atoi(db_get(rs, row, 6));
    out->quota_used = atoi(db_get(rs, row, 7));
    out->status = atoi(db_get(rs, row, 8));
}

/* 号源占放与查询已实现；按周生成、调号源、停复诊仍为未实现桩，
   保持"清空出参并返回 ERR_PARAM"，不模拟业务成功。 */

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

/* 拼查询 SQL：医生、日期区间、两端都空时默认今天起一周，患者角色强制只看有余号的。 */
static void schedule_build_sql(const char *doctor_id, const char *date_from, const char *date_to,
                               int only_available, char *sql, size_t sql_size)
{
    char esc_did[DID_LEN * 2 + 1];
    char doc_clause[96] = "", from_clause[96] = "", to_clause[96] = "";

    if (!str_empty(doctor_id)) {
        db_escape(doctor_id, esc_did, sizeof(esc_did));
        snprintf(doc_clause, sizeof(doc_clause), " AND s.doctor_id = '%s'", esc_did);
    }
    if (date_from != NULL) {
        snprintf(from_clause, sizeof(from_clause), " AND s.work_date >= '%s'", date_from);
    }
    if (date_to != NULL) {
        snprintf(to_clause, sizeof(to_clause), " AND s.work_date <= '%s'", date_to);
    }
    /* 两端都不填时默认今天起 SCHEDULE_DAYS 天（含今天）：只卡下界会把 30 天后的排班
       也当成"一周内"返回，所以要同时补上界 */
    if (date_from == NULL && date_to == NULL) {
        snprintf(from_clause, sizeof(from_clause), " AND s.work_date >= CURDATE()");
        snprintf(to_clause, sizeof(to_clause),
                 " AND s.work_date <= DATE_ADD(CURDATE(), INTERVAL %d DAY)", SCHEDULE_DAYS - 1);
    }
    if (g_session.role == ROLE_PATIENT) {
        only_available = 1;
    }
    snprintf(sql, sql_size,
             "SELECT " SCHEDULE_FIELDS SCHEDULE_FROM " WHERE 1 = 1%s%s%s%s "
             "ORDER BY s.work_date ASC, FIELD(s.time_slot, '%s', '%s'), s.schedule_id",
             doc_clause, from_clause, to_clause, only_available ? SCHEDULE_AVAILABLE : "", SLOT_AM,
             SLOT_PM);
}

/* 把查询结果搬到堆上，交给调用方 free；出错时出参保持调用方置好的空值。 */
static int schedule_fetch_list(const db_result_t *rs, schedule_t **out, int *out_n)
{
    schedule_t *list = NULL;
    int i, n = rs->rows;

    if (n > 0) {
        list = malloc((size_t)n * sizeof(*list));
        if (list == NULL) {
            return ERR_NOMEM;
        }
        for (i = 0; i < n; i++) {
            schedule_read_row(rs, i, &list[i]);
        }
    }
    *out = list;
    *out_n = n;
    return 0;
}

int schedule_query_by_doctor(const char *doctor_id, const char *date_from, const char *date_to,
                             int only_available, schedule_t **out, int *out_n)
{
    char sql[SQL_BUF];
    db_result_t *rs = NULL;
    int rc;

    if (out == NULL || out_n == NULL) {
        return ERR_PARAM;
    }
    *out = NULL;
    *out_n = 0;
    if (!session_is_login()) {
        return ERR_PERM;
    }
    if ((date_from != NULL && !date_valid(date_from)) ||
        (date_to != NULL && !date_valid(date_to))) {
        return ERR_PARAM;
    }
    schedule_build_sql(doctor_id, date_from, date_to, only_available, sql, sizeof(sql));
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    rc = schedule_fetch_list(rs, out, out_n);
    db_result_free(rs);
    return rc;
}

int schedule_query_by_id(int schedule_id, schedule_t *out)
{
    char sql[SQL_BUF];
    db_result_t *rs = NULL;
    int rc;

    if (out == NULL) {
        return ERR_PARAM;
    }
    memset(out, 0, sizeof(*out));
    if (schedule_id <= 0) {
        return ERR_PARAM;
    }
    snprintf(sql, sizeof(sql), "SELECT " SCHEDULE_FIELDS SCHEDULE_FROM " WHERE s.schedule_id = %d",
             schedule_id);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    if (rs->rows == 0) {
        db_result_free(rs);
        return ERR_NOT_FOUND;
    }
    schedule_read_row(rs, 0, out);
    db_result_free(rs);
    return 0;
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
    char sql[SQL_BUF];
    db_result_t *rs = NULL;
    unsigned long long affected = 0;
    int rc;

    if (schedule_id <= 0) {
        return ERR_PARAM;
    }
    /* 条件 UPDATE 一步占号：只有在诊且没约满才命中，不会先查后改留下竞态窗口 */
    snprintf(sql, sizeof(sql),
             "UPDATE t_schedule SET quota_used = quota_used + 1 "
             "WHERE schedule_id = %d AND status = 1 AND quota_used < quota_total",
             schedule_id);
    rc = db_exec(sql, &affected);
    if (rc != 0) {
        return rc;
    }
    if (affected == 1) {
        return 0;
    }
    /* 没占上再查一次，把"排班不存在 / 已停诊 / 已约满"三种情况分开报 */
    snprintf(sql, sizeof(sql),
             "SELECT status, quota_total, quota_used FROM t_schedule WHERE schedule_id = %d",
             schedule_id);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    if (rs->rows == 0) {
        db_result_free(rs);
        return ERR_NOT_FOUND;
    }
    rc = atoi(db_get(rs, 0, 0)) != 1 ? ERR_BAD_STATE : ERR_NO_QUOTA;
    db_result_free(rs);
    return rc;
}

int schedule_release_quota(int schedule_id)
{
    char sql[SQL_BUF];
    unsigned long long affected = 0;
    int rc;

    if (schedule_id <= 0) {
        return ERR_PARAM;
    }
    /* 限定 quota_used > 0，无论怎么调都不会把号源数减成负数 */
    snprintf(sql, sizeof(sql),
             "UPDATE t_schedule SET quota_used = quota_used - 1 "
             "WHERE schedule_id = %d AND quota_used > 0",
             schedule_id);
    rc = db_exec(sql, &affected);
    if (rc != 0) {
        return rc;
    }
    /* 影响 0 行说明这条排班已无可放号源，按幂等处理，只留一条警告不报错 */
    if (affected == 0) {
        fprintf(stderr, "schedule_release_quota: 排班 %d 无号可放\n", schedule_id);
    }
    return 0;
}

void schedule_list_free(schedule_t *list)
{
    free(list);
}
