/*
 * patient.c  患者档案与注册（M01）
 * 作者：A   创建：2026-10-14
 */

#include "patient.h"

#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 查询按下面顺序取列，解析也按同一序号读，改列顺序必须同步改 patient_read_row。
   刻意不查 password：密码只供登录校验用，不能经业务查询下发给调用方。 */
#define PATIENT_FIELDS                                                                             \
    "patient_id, name, gender, birth_date, id_card, phone, address, "                              \
    "status, create_time, update_time, contact_name, contact_phone"

static void patient_read_row(const db_result_t *rs, int row, patient_t *out)
{
    memset(out, 0, sizeof(*out));
    snprintf(out->patient_id, sizeof(out->patient_id), "%s", db_get(rs, row, 0));
    snprintf(out->name, sizeof(out->name), "%s", db_get(rs, row, 1));
    snprintf(out->gender, sizeof(out->gender), "%s", db_get(rs, row, 2));
    snprintf(out->birth_date, sizeof(out->birth_date), "%s", db_get(rs, row, 3));
    snprintf(out->id_card, sizeof(out->id_card), "%s", db_get(rs, row, 4));
    snprintf(out->phone, sizeof(out->phone), "%s", db_get(rs, row, 5));
    snprintf(out->address, sizeof(out->address), "%s", db_get(rs, row, 6));
    out->status = atoi(db_get(rs, row, 7));
    snprintf(out->create_time, sizeof(out->create_time), "%s", db_get(rs, row, 8));
    snprintf(out->update_time, sizeof(out->update_time), "%s", db_get(rs, row, 9));
    snprintf(out->contact_name, sizeof(out->contact_name), "%s", db_get(rs, row, 10));
    snprintf(out->contact_phone, sizeof(out->contact_phone), "%s", db_get(rs, row, 11));
}

/* 生成当年编号 P + 4 位年 + 4 位流水，取当年最大流水 + 1，无记录从 0001 起。
   必须在调用方的事务里执行，否则并发注册可能拿到同一个号。 */
static int patient_next_id(char *out, int outlen)
{
    char sql[SQL_BUF];
    char today[11], prefix[PID_LEN];
    db_result_t *rs = NULL;
    int rc, seq;

    today_str(today, sizeof(today));
    snprintf(prefix, sizeof(prefix), "P%.4s", today);
    snprintf(sql, sizeof(sql),
             "SELECT IFNULL(MAX(CAST(SUBSTRING(patient_id, 6) AS UNSIGNED)), 0) "
             "FROM t_patient WHERE patient_id LIKE '%s%%'",
             prefix);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    seq = rs->rows > 0 ? atoi(db_get(rs, 0, 0)) : 0;
    db_result_free(rs);
    snprintf(out, outlen, "P%.4s%04d", today, seq + 1);
    return 0;
}

/* 拼 INSERT 并写入一行，各字段先转义；调用方需已在事务内。 */
static int patient_insert_row(const patient_t *p, const char *pid, const char *esc_id)
{
    char sql[SQL_BUF];
    char esc_name[NAME_LEN * 2 + 1], esc_pass[NAME_LEN * 2 + 1];
    char esc_gender[32], esc_phone[64], esc_addr[288];
    char esc_contact[64], esc_contact_phone[64];

    db_escape(p->name, esc_name, sizeof(esc_name));
    db_escape(p->password, esc_pass, sizeof(esc_pass));
    db_escape(p->gender, esc_gender, sizeof(esc_gender));
    db_escape(p->phone, esc_phone, sizeof(esc_phone));
    db_escape(p->address, esc_addr, sizeof(esc_addr));
    db_escape(p->contact_name, esc_contact, sizeof(esc_contact));
    db_escape(p->contact_phone, esc_contact_phone, sizeof(esc_contact_phone));
    snprintf(sql, sizeof(sql),
             "INSERT INTO t_patient "
             "(patient_id, password, name, gender, birth_date, id_card, phone, address, "
             " status, contact_name, contact_phone) "
             "VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, '%s', '%s')",
             pid, esc_pass, esc_name, esc_gender, p->birth_date, esc_id, esc_phone, esc_addr,
             esc_contact, esc_contact_phone);
    return db_exec(sql, NULL);
}

int patient_register(const patient_t *p, char *out_patient_id)
{
    char sql[SQL_BUF];
    char esc_id[64], pid[PID_LEN];
    db_result_t *rs = NULL;
    int rc;

    if (out_patient_id != NULL) {
        out_patient_id[0] = '\0';
    }
    if (p == NULL || str_empty(p->name) || str_empty(p->password) || str_empty(p->gender) ||
        str_empty(p->id_card) || !date_valid(p->birth_date)) {
        return ERR_PARAM;
    }

    rc = db_tx_begin();
    if (rc != 0) {
        return rc;
    }
    rc = ERR_DB_EXEC;

    /* 身份证唯一，先在事务内查一次，命中直接给 ERR_DUPLICATE，不靠报错文本猜原因 */
    db_escape(p->id_card, esc_id, sizeof(esc_id));
    snprintf(sql, sizeof(sql), "SELECT patient_id FROM t_patient WHERE id_card = '%s'", esc_id);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        goto cleanup;
    }
    if (rs->rows > 0) {
        rc = ERR_DUPLICATE;
        goto cleanup;
    }
    db_result_free(rs);
    rs = NULL;

    rc = patient_next_id(pid, sizeof(pid));
    if (rc != 0) {
        goto cleanup;
    }

    rc = patient_insert_row(p, pid, esc_id);
    if (rc != 0) {
        goto cleanup;
    }
    if (out_patient_id != NULL) {
        snprintf(out_patient_id, PID_LEN, "%s", pid);
    }
    rc = 0;

cleanup:
    if (rs != NULL) {
        db_result_free(rs);
    }
    if (rc != 0) {
        db_tx_rollback();
        return rc;
    }
    /* 提交失败时事务状态仍在，必须补一次回滚，否则后续操作全报 ERR_TX */
    if (db_tx_commit() == 0) {
        return 0;
    }
    db_tx_rollback();
    return ERR_TX;
}

int patient_update_self(const patient_t *p)
{
    char sql[SQL_BUF];
    char esc_name[NAME_LEN * 2 + 1], esc_phone[64], esc_addr[288];
    char esc_contact[64], esc_contact_phone[64], esc_pid[PID_LEN * 2 + 1];
    unsigned long long affected = 0;
    int rc;

    /* 只允许本人改，且只改姓名、电话、住址、紧急联系人；编号与身份证不动 */
    if (p == NULL || !session_is_login() || g_session.role != ROLE_PATIENT ||
        strcmp(p->patient_id, g_session.user_id) != 0) {
        return ERR_PERM;
    }
    if (str_empty(p->name)) {
        return ERR_PARAM;
    }
    db_escape(p->name, esc_name, sizeof(esc_name));
    db_escape(p->phone, esc_phone, sizeof(esc_phone));
    db_escape(p->address, esc_addr, sizeof(esc_addr));
    db_escape(p->contact_name, esc_contact, sizeof(esc_contact));
    db_escape(p->contact_phone, esc_contact_phone, sizeof(esc_contact_phone));
    db_escape(p->patient_id, esc_pid, sizeof(esc_pid));
    snprintf(sql, sizeof(sql),
             "UPDATE t_patient SET name = '%s', phone = '%s', address = '%s', "
             "contact_name = '%s', contact_phone = '%s', update_time = NOW() "
             "WHERE patient_id = '%s' AND status = 1",
             esc_name, esc_phone, esc_addr, esc_contact, esc_contact_phone, esc_pid);
    rc = db_exec(sql, &affected);
    if (rc != 0) {
        return rc;
    }
    return affected == 1 ? 0 : ERR_NOT_FOUND;
}

int patient_set_status(const char *patient_id, int new_status)
{
    char sql[SQL_BUF], esc_pid[PID_LEN * 2 + 1];
    unsigned long long affected = 0;
    int rc;

    /* 跨模块唯一写入口，内部不得开事务，由调用方（如 C 的注销）包裹 */
    if (str_empty(patient_id) || (new_status != 0 && new_status != 1)) {
        return ERR_PARAM;
    }
    /* 注销与恢复只有管理员能做，必须在函数内校验，不能只把菜单藏起来 */
    if (!session_is_login() || g_session.role != ROLE_ADMIN) {
        return ERR_PERM;
    }
    db_escape(patient_id, esc_pid, sizeof(esc_pid));
    snprintf(sql, sizeof(sql),
             "UPDATE t_patient SET status = %d, update_time = NOW() WHERE patient_id = '%s'",
             new_status, esc_pid);
    rc = db_exec(sql, &affected);
    if (rc != 0) {
        return rc;
    }
    return affected == 1 ? 0 : ERR_NOT_FOUND;
}

int patient_delete(const char *patient_id)
{
    char sql[SQL_BUF], esc_pid[PID_LEN * 2 + 1];
    db_result_t *rs = NULL;
    unsigned long long affected = 0;
    int rc;

    if (str_empty(patient_id)) {
        return ERR_PARAM;
    }
    /* 硬删除只开放给管理员，且必须已登录 */
    if (!session_is_login() || g_session.role != ROLE_ADMIN) {
        return ERR_PERM;
    }
    rc = db_tx_begin();
    if (rc != 0) {
        return rc;
    }
    rc = ERR_DB_EXEC;

    db_escape(patient_id, esc_pid, sizeof(esc_pid));
    /* 有预约、就诊、处方或病历引用时禁止硬删：删了会让这些历史记录指向不存在的患者，
       预约占用的号源也不会回退。要保留历史请改用 patient_set_status 软注销。 */
    snprintf(sql, sizeof(sql),
             "SELECT (SELECT COUNT(*) FROM t_appointment WHERE patient_id = '%s')"
             " + (SELECT COUNT(*) FROM t_visit_record WHERE patient_id = '%s')"
             " + (SELECT COUNT(*) FROM t_prescription WHERE patient_id = '%s')"
             " + (SELECT COUNT(*) FROM t_medical_record WHERE patient_id = '%s')",
             esc_pid, esc_pid, esc_pid, esc_pid);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        goto cleanup;
    }
    if (atoi(db_get(rs, 0, 0)) > 0) {
        rc = ERR_BAD_STATE;
        goto cleanup;
    }
    db_result_free(rs);
    rs = NULL;

    snprintf(sql, sizeof(sql), "DELETE FROM t_patient WHERE patient_id = '%s'", esc_pid);
    rc = db_exec(sql, &affected);
    if (rc != 0) {
        goto cleanup;
    }
    rc = affected == 1 ? 0 : ERR_NOT_FOUND;

cleanup:
    if (rs != NULL) {
        db_result_free(rs);
    }
    if (rc != 0) {
        db_tx_rollback();
        return rc;
    }
    /* 提交失败时事务状态仍在，必须补一次回滚，否则后续操作全报 ERR_TX */
    if (db_tx_commit() == 0) {
        return 0;
    }
    db_tx_rollback();
    return ERR_TX;
}

int patient_query_by_id(const char *patient_id, patient_t *out)
{
    char sql[SQL_BUF], esc_pid[PID_LEN * 2 + 1];
    db_result_t *rs = NULL;
    int rc;

    if (out == NULL) {
        return ERR_PARAM;
    }
    memset(out, 0, sizeof(*out));
    if (str_empty(patient_id)) {
        return ERR_PARAM;
    }
    /* 未登录一律拒；患者只能查本人，管理员与医生可按需查阅（管理、接诊需要） */
    if (!session_is_login()) {
        return ERR_PERM;
    }
    if (g_session.role == ROLE_PATIENT && strcmp(patient_id, g_session.user_id) != 0) {
        return ERR_PERM;
    }
    db_escape(patient_id, esc_pid, sizeof(esc_pid));
    snprintf(sql, sizeof(sql), "SELECT %s FROM t_patient WHERE patient_id = '%s'", PATIENT_FIELDS,
             esc_pid);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    if (rs->rows == 0) {
        db_result_free(rs);
        return ERR_NOT_FOUND;
    }
    patient_read_row(rs, 0, out);
    db_result_free(rs);
    return 0;
}

/* 按 filter 分页取一页患者，成功时 *out 指向新分配的数组，由调用方 patient_list_free 释放。 */
static int patient_paged(const char *filter, int page, patient_t **out, int *out_n)
{
    char sql[SQL_BUF];
    db_result_t *rs = NULL;
    patient_t *list = NULL;
    int rc, i, n;

    snprintf(sql, sizeof(sql), "SELECT %s FROM t_patient%s ORDER BY patient_id LIMIT %d OFFSET %d",
             PATIENT_FIELDS, filter, PAGE_SIZE, page_offset(page));
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    n = rs->rows;
    if (n > 0) {
        list = malloc((size_t)n * sizeof(*list));
        if (list == NULL) {
            db_result_free(rs);
            return ERR_NOMEM;
        }
        for (i = 0; i < n; i++) {
            patient_read_row(rs, i, &list[i]);
        }
    }
    db_result_free(rs);
    *out = list;
    *out_n = n;
    return 0;
}

int patient_query_all(const char *keyword, int page, patient_t **out, int *out_n, int *out_total)
{
    char sql[SQL_BUF];
    char esc_kw[NAME_LEN * 2 + 1], like_kw[NAME_LEN * 8 + 1], filter[SQL_BUF / 2];
    db_result_t *rs = NULL;
    int rc, total = 0;

    if (out == NULL || out_n == NULL) {
        return ERR_PARAM;
    }
    *out = NULL;
    *out_n = 0;
    if (out_total != NULL) {
        *out_total = 0;
    }
    if (!session_is_login()) {
        return ERR_PERM;
    }

    filter[0] = '\0';
    /* 患者只能看本人档案；管理员与医生沿用同一查询，关键词匹配姓名、电话、编号 */
    if (g_session.role == ROLE_PATIENT) {
        char esc_pid[PID_LEN * 2 + 1];

        db_escape(g_session.user_id, esc_pid, sizeof(esc_pid));
        snprintf(filter, sizeof(filter), " WHERE patient_id = '%s'", esc_pid);
    } else if (!str_empty(keyword)) {
        db_escape(keyword, esc_kw, sizeof(esc_kw));
        like_escape(esc_kw, like_kw, sizeof(like_kw));
        snprintf(filter, sizeof(filter),
                 " WHERE (name LIKE '%%%s%%' OR phone LIKE '%%%s%%' OR patient_id LIKE '%%%s%%')",
                 like_kw, like_kw, like_kw);
    }

    snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM t_patient%s", filter);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    total = atoi(db_get(rs, 0, 0));
    db_result_free(rs);

    rc = patient_paged(filter, page, out, out_n);
    if (rc != 0) {
        return rc;
    }
    if (out_total != NULL) {
        *out_total = total;
    }
    return 0;
}

void patient_list_free(patient_t *list)
{
    free(list);
}
