/*
 * appointment.c  预约挂号与状态迁移（M03）
 * 作者：A   创建：2026-10-14
 */

#include "appointment.h"

#include "db.h"
#include "schedule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 列表与详情共用同一段 SELECT，列顺序和 appoint_read_row 的序号一一对应。 */
#define APPOINT_FIELDS                                                                             \
    "a.appoint_id, a.patient_id, p.name, a.doctor_id, d.name, d.dept, "                            \
    "a.schedule_id, a.appoint_date, a.time_slot, a.symptom, a.status, "                            \
    "a.create_time, a.update_time, a.change_time, a.change_count"

#define APPOINT_FROM                                                                               \
    " FROM t_appointment a "                                                                       \
    "LEFT JOIN t_patient p ON p.patient_id = a.patient_id "                                        \
    "LEFT JOIN t_doctor d ON d.doctor_id = a.doctor_id"

#define APPOINT_CHANGE_MAX 3

static void appoint_read_row(const db_result_t *rs, int row, appoint_t *out)
{
    memset(out, 0, sizeof(*out));
    out->appoint_id = atoi(db_get(rs, row, 0));
    snprintf(out->patient_id, sizeof(out->patient_id), "%s", db_get(rs, row, 1));
    snprintf(out->patient_name, sizeof(out->patient_name), "%s", db_get(rs, row, 2));
    snprintf(out->doctor_id, sizeof(out->doctor_id), "%s", db_get(rs, row, 3));
    snprintf(out->doctor_name, sizeof(out->doctor_name), "%s", db_get(rs, row, 4));
    snprintf(out->dept, sizeof(out->dept), "%s", db_get(rs, row, 5));
    out->schedule_id = atoi(db_get(rs, row, 6));
    snprintf(out->appoint_date, sizeof(out->appoint_date), "%s", db_get(rs, row, 7));
    snprintf(out->time_slot, sizeof(out->time_slot), "%s", db_get(rs, row, 8));
    snprintf(out->symptom, sizeof(out->symptom), "%s", db_get(rs, row, 9));
    out->status = atoi(db_get(rs, row, 10));
    snprintf(out->create_time, sizeof(out->create_time), "%s", db_get(rs, row, 11));
    snprintf(out->update_time, sizeof(out->update_time), "%s", db_get(rs, row, 12));
    snprintf(out->change_time, sizeof(out->change_time), "%s", db_get(rs, row, 13));
    out->change_count = atoi(db_get(rs, row, 14));
}

/* 按主键取一条预约（带患者姓名、医生姓名、科室），不存在返回 ERR_NOT_FOUND。 */
static int appoint_load(int appoint_id, appoint_t *out)
{
    char sql[SQL_BUF];
    db_result_t *rs = NULL;
    int rc;

    snprintf(sql, sizeof(sql), "SELECT " APPOINT_FIELDS APPOINT_FROM " WHERE a.appoint_id = %d",
             appoint_id);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    if (rs->rows == 0) {
        db_result_free(rs);
        return ERR_NOT_FOUND;
    }
    appoint_read_row(rs, 0, out);
    db_result_free(rs);
    return 0;
}

/* 分页列表公共实现：filter 是已转义且以 " WHERE ..." 开头的条件串（可为空串）。 */
static int appoint_paged(const char *filter, int page, appoint_t **out, int *out_n, int *out_total)
{
    char sql[SQL_BUF];
    db_result_t *rs = NULL;
    appoint_t *list = NULL;
    int rc, i, n;

    snprintf(sql, sizeof(sql), "SELECT COUNT(*)%s%s", APPOINT_FROM, filter);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    if (out_total != NULL) {
        *out_total = atoi(db_get(rs, 0, 0));
    }
    db_result_free(rs);

    snprintf(sql, sizeof(sql),
             "SELECT " APPOINT_FIELDS APPOINT_FROM "%s "
             "ORDER BY a.appoint_date DESC, a.appoint_id DESC LIMIT %d OFFSET %d",
             filter, PAGE_SIZE, page_offset(page));
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
            appoint_read_row(rs, i, &list[i]);
        }
    }
    db_result_free(rs);
    *out = list;
    *out_n = n;
    return 0;
}

/* 列表入口共用的过滤入参：字段为 NULL 表示该维度不筛，能不能筛交给 appoint_list_scoped 裁决。 */
typedef struct {
    const char *pid;
    const char *did;
    const char *kw;
    const char *date;
    int page;
} appoint_filter_t;

/* 在过滤串尾部追加一个条件：首条用 WHERE，其后用 AND。 */
static void appoint_append(char *filter, size_t n, const char *clause)
{
    snprintf(filter + strlen(filter), n - strlen(filter), "%s%s",
             filter[0] == '\0' ? " WHERE " : " AND ", clause);
}

/* 列表查询的公共实现（P1 整改）：先按会话锁定可见范围，再叠加入口条件。
   三个入口共用这一条路径，换个入口也绕不过权限：患者只见本人，医生只见本人接诊，管理员不限。 */
static int appoint_list_scoped(const appoint_filter_t *f, appoint_t **out, int *out_n,
                               int *out_total)
{
    char filter[SQL_BUF], clause[NAME_LEN * 8 + 32], esc[NAME_LEN * 8 + 1], like[NAME_LEN * 8 + 1];
    const char *pid = f->pid, *did = f->did;

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
    if ((pid != NULL && str_empty(pid)) || (did != NULL && str_empty(did)) ||
        (f->date != NULL && !date_valid(f->date))) {
        return ERR_PARAM;
    }
    /* 会话范围优先：越权的目标编号在这里就断掉，不是等到拼 SQL 才过滤 */
    if (g_session.role == ROLE_PATIENT) {
        if ((pid != NULL && strcmp(pid, g_session.user_id) != 0) || did != NULL) {
            return ERR_PERM;
        }
        pid = g_session.user_id;
    } else if (g_session.role == ROLE_DOCTOR) {
        if (did != NULL && strcmp(did, g_session.user_id) != 0) {
            return ERR_PERM;
        }
        did = g_session.user_id;
    } else if (g_session.role != ROLE_ADMIN) {
        return ERR_PERM;
    }
    filter[0] = '\0';
    if (pid != NULL) {
        db_escape(pid, esc, sizeof(esc));
        snprintf(clause, sizeof(clause), "a.patient_id = '%s'", esc);
        appoint_append(filter, sizeof(filter), clause);
    }
    if (did != NULL) {
        db_escape(did, esc, sizeof(esc));
        snprintf(clause, sizeof(clause), "a.doctor_id = '%s'", esc);
        appoint_append(filter, sizeof(filter), clause);
    }
    if (!str_empty(f->kw)) {
        db_escape(f->kw, esc, sizeof(esc));
        like_escape(esc, like, sizeof(like));
        snprintf(clause, sizeof(clause), "p.name LIKE '%%%s%%'", like);
        appoint_append(filter, sizeof(filter), clause);
    }
    if (f->date != NULL) {
        snprintf(clause, sizeof(clause), "a.appoint_date = '%s'", f->date);
        appoint_append(filter, sizeof(filter), clause);
    }
    return appoint_paged(filter, f->page, out, out_n, out_total);
}

/* 探针查询只关心命中行数：返回行数，出错原样返回负错误码。 */
static int appoint_probe_rows(const char *sql)
{
    db_result_t *rs = NULL;
    int rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    rc = rs->rows;
    db_result_free(rs);
    return rc;
}

/* 挂号前的四项校验：患者存在且在册、医生在职、无重复待就诊、排班在诊。 */
static int appoint_add_precheck(const char *esc_pid, const char *esc_did, int schedule_id,
                                const char *date, const char *slot)
{
    char sql[SQL_BUF];
    int rc;

    snprintf(sql, sizeof(sql), "SELECT 1 FROM t_patient WHERE patient_id = '%s' AND status = 1",
             esc_pid);
    rc = appoint_probe_rows(sql);
    if (rc <= 0) {
        return rc < 0 ? rc : ERR_NOT_FOUND;
    }
    /* 停职/离职医生不能再被挂上号，按"无此医生"处理 */
    snprintf(sql, sizeof(sql), "SELECT 1 FROM t_doctor WHERE doctor_id = '%s' AND status = 1",
             esc_did);
    rc = appoint_probe_rows(sql);
    if (rc <= 0) {
        return rc < 0 ? rc : ERR_NOT_FOUND;
    }
    snprintf(sql, sizeof(sql),
             "SELECT 1 FROM t_appointment WHERE patient_id = '%s' AND doctor_id = '%s' "
             "AND appoint_date = '%s' AND status = 0",
             esc_pid, esc_did, date);
    rc = appoint_probe_rows(sql);
    if (rc < 0) {
        return rc;
    }
    if (rc > 0) {
        return ERR_DUPLICATE;
    }
    snprintf(sql, sizeof(sql),
             "SELECT 1 FROM t_schedule WHERE schedule_id = %d AND doctor_id = '%s' "
             "AND work_date = '%s' AND time_slot = '%s' AND status = 1",
             schedule_id, esc_did, date, slot);
    rc = appoint_probe_rows(sql);
    if (rc <= 0) {
        return rc < 0 ? rc : ERR_NOT_FOUND;
    }
    return 0;
}

int appointment_add(const char *patient_id, const char *doctor_id, int schedule_id,
                    const char *appoint_date, const char *time_slot, const char *symptom)
{
    char sql[SQL_BUF];
    char esc_pid[PID_LEN * 2 + 1], esc_did[DID_LEN * 2 + 1], esc_sym[512];
    char today[11];
    int rc;

    if (str_empty(patient_id) || str_empty(doctor_id) || schedule_id <= 0 ||
        !date_valid(appoint_date) || str_empty(time_slot) || symptom == NULL ||
        (strcmp(time_slot, SLOT_AM) != 0 && strcmp(time_slot, SLOT_PM) != 0)) {
        return ERR_PARAM;
    }
    /* 只能约今天起 APPOINT_AHEAD_DAYS 天内，越界直接拒，不查库 */
    today_str(today, sizeof(today));
    if (date_diff_days(appoint_date, today) < 0 ||
        date_diff_days(appoint_date, today) > APPOINT_AHEAD_DAYS) {
        return ERR_PARAM;
    }
    /* 挂号权限：患者只能给自己挂，管理员可代挂；未登录与其他角色一律拒，且在开事务前拦截 */
    if (!session_is_login() ||
        (g_session.role == ROLE_PATIENT && strcmp(patient_id, g_session.user_id) != 0) ||
        (g_session.role != ROLE_PATIENT && g_session.role != ROLE_ADMIN)) {
        return ERR_PERM;
    }
    db_escape(patient_id, esc_pid, sizeof(esc_pid));
    db_escape(doctor_id, esc_did, sizeof(esc_did));
    db_escape(symptom, esc_sym, sizeof(esc_sym));
    rc = db_tx_begin();
    if (rc != 0) {
        return rc;
    }
    rc = appoint_add_precheck(esc_pid, esc_did, schedule_id, appoint_date, time_slot);
    if (rc != 0) {
        db_tx_rollback();
        return rc;
    }
    /* 校验过关才占号，占号失败整体回滚，不会留下占号却没挂上的脏数据 */
    rc = schedule_take_quota(schedule_id);
    if (rc != 0) {
        db_tx_rollback();
        return rc;
    }
    snprintf(sql, sizeof(sql),
             "INSERT INTO t_appointment "
             "(patient_id, doctor_id, schedule_id, appoint_date, time_slot, symptom, status) "
             "VALUES ('%s', '%s', %d, '%s', '%s', '%s', 0)",
             esc_pid, esc_did, schedule_id, appoint_date, time_slot, esc_sym);
    rc = db_exec(sql, NULL);
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

/* 状态迁移的角色、归属与日期规则（P1 整改）：写入口自己判死，不依赖菜单隐藏或外层包装。
   取消=患者本人或管理员；已就诊/爽约=接诊医生本人或管理员，且不能标在未来。 */
static int appoint_status_allowed(const appoint_t *cur, int new_status)
{
    char today[11];
    if (g_session.role == ROLE_ADMIN) {
        return 0;
    }
    if (new_status == APPT_CANCELED) {
        return (g_session.role == ROLE_PATIENT && strcmp(cur->patient_id, g_session.user_id) == 0)
                   ? 0
                   : ERR_PERM;
    }
    /* 已就诊与爽约都是就诊侧动作，只有接诊医生本人能做 */
    if (g_session.role != ROLE_DOCTOR || strcmp(cur->doctor_id, g_session.user_id) != 0) {
        return ERR_PERM;
    }
    today_str(today, sizeof(today));
    if (new_status == APPT_NOSHOW) {
        return date_diff_days(cur->appoint_date, today) < 0 ? 0 : ERR_BAD_STATE;
    }
    return date_diff_days(cur->appoint_date, today) <= 0 ? 0 : ERR_BAD_STATE;
}

int appointment_set_status(int appoint_id, int new_status, const char *reason)
{
    char sql[SQL_BUF];
    appoint_t cur;
    unsigned long long affected = 0;
    int rc;

    (void)reason; /* reason 只给界面回显用，不落库 */
    if (appoint_id <= 0 || new_status < APPT_WAITING || new_status > APPT_NOSHOW) {
        return ERR_PARAM;
    }
    /* 状态迁移是写操作，未登录一律拒 */
    if (!session_is_login()) {
        return ERR_PERM;
    }
    rc = appoint_load(appoint_id, &cur);
    if (rc != 0) {
        return rc;
    }
    /* 合法迁移表（SRS 附录 D）：只有待就诊可迁出，1/2/3 都是终态 */
    if (cur.status != APPT_WAITING || new_status == APPT_WAITING) {
        return ERR_BAD_STATE;
    }
    /* 角色、归属、日期三合一校验：直接调这个函数也绕不过去 */
    rc = appoint_status_allowed(&cur, new_status);
    if (rc != 0) {
        return rc;
    }
    /* UPDATE 带上旧状态，两步之间若被别处改掉则不覆盖，影响 0 行按并发兜底处理 */
    snprintf(sql, sizeof(sql),
             "UPDATE t_appointment SET status = %d, update_time = NOW() "
             "WHERE appoint_id = %d AND status = %d",
             new_status, appoint_id, cur.status);
    rc = db_exec(sql, &affected);
    if (rc != 0) {
        return rc;
    }
    if (affected != 1) {
        return ERR_BAD_STATE;
    }
    /* 0→2 是本模块唯一的放号点，患者与管理员两条取消路径都经过这里，只放一次 */
    if (new_status == APPT_CANCELED && cur.schedule_id > 0) {
        rc = schedule_release_quota(cur.schedule_id);
        if (rc != 0) {
            return rc;
        }
    }
    return 0;
}

/* 取消挂号：未登录先拒，不白开事务；角色与归属由 set_status 判死。事务发起函数。 */
int appointment_cancel(int appoint_id)
{
    int rc;

    if (appoint_id <= 0 || !session_is_login()) {
        return appoint_id <= 0 ? ERR_PARAM : ERR_PERM;
    }
    rc = db_tx_begin();
    if (rc != 0) {
        return rc;
    }
    rc = appointment_set_status(appoint_id, APPT_CANCELED, "取消挂号");
    if (rc != 0) {
        db_tx_rollback();
        return rc;
    }
    if (db_tx_commit() == 0) {
        return 0;
    }
    db_tx_rollback();
    return ERR_TX;
}

/* 改期落库：校验目标排班在诊且本人当天无重复待就诊，换号时先放旧号再占新号。 */
static int appoint_apply_reschedule(int appoint_id, const appoint_t *cur, const char *esc_doctor,
                                    const char *esc_patient, const char *new_date,
                                    const char *new_slot)
{
    char sql[SQL_BUF];
    db_result_t *rs = NULL;
    int rc, new_schedule;

    /* 目标排班必须在诊，且医生本人仍在职：停用医生的号源不能再被改期占用（P2 整改） */
    snprintf(sql, sizeof(sql),
             "SELECT s.schedule_id FROM t_schedule s "
             "JOIN t_doctor d ON d.doctor_id = s.doctor_id "
             "WHERE s.doctor_id = '%s' AND s.work_date = '%s' AND s.time_slot = '%s' "
             "AND s.status = 1 AND d.status = 1",
             esc_doctor, new_date, new_slot);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    new_schedule = rs->rows == 0 ? ERR_NOT_FOUND : atoi(db_get(rs, 0, 0));
    db_result_free(rs);
    if (new_schedule < 0) {
        return new_schedule;
    }

    snprintf(sql, sizeof(sql),
             "SELECT 1 FROM t_appointment WHERE patient_id = '%s' AND doctor_id = '%s' "
             "AND appoint_date = '%s' AND status = 0 AND appoint_id <> %d",
             esc_patient, esc_doctor, new_date, appoint_id);
    rc = appoint_probe_rows(sql);
    if (rc < 0) {
        return rc;
    }
    if (rc > 0) {
        return ERR_DUPLICATE;
    }
    /* 原地不动只改日期时段，不动号源；换号才走放号+占号 */
    if (new_schedule != cur->schedule_id) {
        if (cur->schedule_id > 0) {
            rc = schedule_release_quota(cur->schedule_id);
            if (rc != 0) {
                return rc;
            }
        }
        rc = schedule_take_quota(new_schedule);
        if (rc != 0) {
            return rc;
        }
    }
    snprintf(sql, sizeof(sql),
             "UPDATE t_appointment SET appoint_date = '%s', time_slot = '%s', schedule_id = %d, "
             "change_time = NOW(), change_count = change_count + 1, update_time = NOW() "
             "WHERE appoint_id = %d AND status = 0",
             new_date, new_slot, new_schedule, appoint_id);
    return db_exec(sql, NULL);
}

int appointment_reschedule(int appoint_id, const char *new_date, const char *new_slot,
                           int operator_role, appoint_t *out_new_appoint)
{
    char esc_did[DID_LEN * 2 + 1], esc_pid[PID_LEN * 2 + 1], today[11];
    appoint_t cur;
    int rc;

    if (out_new_appoint != NULL) {
        memset(out_new_appoint, 0, sizeof(*out_new_appoint));
    }
    if (appoint_id <= 0 || !date_valid(new_date) || str_empty(new_slot) ||
        (strcmp(new_slot, SLOT_AM) != 0 && strcmp(new_slot, SLOT_PM) != 0)) {
        return ERR_PARAM;
    }
    today_str(today, sizeof(today));
    if (date_diff_days(new_date, today) < 0 ||
        date_diff_days(new_date, today) > APPOINT_AHEAD_DAYS) {
        return ERR_PARAM;
    }
    /* operator_role 只做一致性校验，权限以会话为准：改期只给患者本人与管理员 */
    if (operator_role != g_session.role || !session_is_login() ||
        (g_session.role != ROLE_PATIENT && g_session.role != ROLE_ADMIN)) {
        return ERR_PERM;
    }
    rc = db_tx_begin();
    if (rc != 0) {
        return rc;
    }
    rc = appoint_load(appoint_id, &cur);
    if (rc != 0) {
        db_tx_rollback();
        return rc;
    }
    /* 必须待就诊且没超改期次数；患者只能改本人的号 */
    if (cur.status != APPT_WAITING || cur.change_count >= APPOINT_CHANGE_MAX) {
        db_tx_rollback();
        return ERR_BAD_STATE;
    }
    if (g_session.role == ROLE_PATIENT && strcmp(cur.patient_id, g_session.user_id) != 0) {
        db_tx_rollback();
        return ERR_PERM;
    }
    db_escape(cur.doctor_id, esc_did, sizeof(esc_did));
    db_escape(cur.patient_id, esc_pid, sizeof(esc_pid));
    rc = appoint_apply_reschedule(appoint_id, &cur, esc_did, esc_pid, new_date, new_slot);
    if (rc != 0) {
        db_tx_rollback();
        return rc;
    }
    if (out_new_appoint != NULL && appoint_load(appoint_id, out_new_appoint) != 0) {
        out_new_appoint = NULL;
    }
    if (db_tx_commit() == 0) {
        return 0;
    }
    db_tx_rollback();
    return ERR_TX;
}

/* 标记爽约（FR-M03-06）：未登录先拒；日期与角色规则由 set_status 判死。事务发起函数。 */
int appointment_mark_noshow(int appoint_id)
{
    int rc;

    if (appoint_id <= 0 || !session_is_login()) {
        return appoint_id <= 0 ? ERR_PARAM : ERR_PERM;
    }
    rc = db_tx_begin();
    if (rc != 0) {
        return rc;
    }
    rc = appointment_set_status(appoint_id, APPT_NOSHOW, "爽约");
    if (rc != 0) {
        db_tx_rollback();
        return rc;
    }
    if (db_tx_commit() == 0) {
        return 0;
    }
    db_tx_rollback();
    return ERR_TX;
}

int appointment_query_by_id(int appoint_id, appoint_t *out)
{
    appoint_t cur;
    int rc;

    if (out != NULL) {
        memset(out, 0, sizeof(*out));
    }
    if (out == NULL || appoint_id <= 0) {
        return ERR_PARAM;
    }
    /* 未登录一律拒；患者只能看本人预约，医生只能看自己接诊的，管理员不限 */
    if (!session_is_login()) {
        return ERR_PERM;
    }
    rc = appoint_load(appoint_id, &cur);
    if (rc != 0) {
        return rc;
    }
    if ((g_session.role == ROLE_PATIENT && strcmp(cur.patient_id, g_session.user_id) != 0) ||
        (g_session.role == ROLE_DOCTOR && strcmp(cur.doctor_id, g_session.user_id) != 0)) {
        return ERR_PERM;
    }
    *out = cur;
    return 0;
}

/* 分页模糊查询（FR-CMN-01）：keyword 匹配患者姓名；可见范围由 appoint_list_scoped 按会话锁定。 */
int appointment_query_all(const char *keyword, int page, appoint_t **out, int *out_n,
                          int *out_total)
{
    appoint_filter_t f = {NULL, NULL, keyword, NULL, page};
    return appoint_list_scoped(&f, out, out_n, out_total);
}

/* 按患者编号查预约：患者只能传本人编号，医生仍被限制在"本人接诊"范围内。 */
int appointment_query_by_patient(const char *patient_id, int page, appoint_t **out, int *out_n,
                                 int *out_total)
{
    appoint_filter_t f = {patient_id, NULL, NULL, NULL, page};
    return appoint_list_scoped(&f, out, out_n, out_total);
}

/* 按医生编号查预约：date 为 NULL 查全部，否则只查该日；患者无权调此入口。 */
int appointment_query_by_doctor(const char *doctor_id, const char *date, int page, appoint_t **out,
                                int *out_n, int *out_total)
{
    appoint_filter_t f = {NULL, doctor_id, NULL, date, page};
    return appoint_list_scoped(&f, out, out_n, out_total);
}

void appointment_list_free(appoint_t *list)
{
    free(list);
}
