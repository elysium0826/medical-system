/* todo.c  待办公共框架，按函数体共写（SRS 8.1，owner: A/B/C/D）
 * 作者：D   创建：2026-10-14 */
#include "todo.h"
#include "common.h"
#include "ui.h"

#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 可改期次数上限，与 appointment.c 的 APPOINT_CHANGE_MAX 一致，超过就不提示可改期 */
#define TODO_CHANGE_MAX 3

/* owner: B，仅替换本函数内部。业务待实现，不能将空待办作为FR验收通过。 */
int todo_doctor(char lines[][TODO_LINE_LEN], int *out_n)
{
    if (lines != NULL) {
        memset(lines, 0, TODO_MAX_LINES * TODO_LINE_LEN);
    }
    if (out_n != NULL) {
        *out_n = 0;
    }
    return ERR_PARAM;
}

/* 把 "YYYY-MM-DD" 显示成 "M月D日"，格式不对时原样返回，只做显示不做校验。 */
static void todo_date_text(const char *iso, char *out, int outlen)
{
    int month, day;

    if (strlen(iso) >= 10 && iso[4] == '-' && iso[7] == '-') {
        month = (iso[5] - '0') * 10 + (iso[6] - '0');
        day = (iso[8] - '0') * 10 + (iso[9] - '0');
        snprintf(out, (size_t)outlen, "%d月%d日", month, day);
        return;
    }
    snprintf(out, (size_t)outlen, "%s", iso);
}

/* DATEDIFF 天数转提示：0 天显示"今天"，其余显示还剩几天。 */
static void todo_days_text(int days, char *out, int outlen)
{
    if (days <= 0) {
        snprintf(out, (size_t)outlen, "今天");
    } else {
        snprintf(out, (size_t)outlen, "还有 %d 天", days);
    }
}

/* 规则 1、2：未来待就诊预约按日期升序取前 3 条，未达改期上限的追加可改期次数。 */
static int todo_add_future(const char *esc_pid, char lines[][TODO_LINE_LEN], int *n)
{
    char sql[SQL_BUF], date_text[32], days_text[32], change_text[48];
    db_result_t *rs = NULL;
    int rc, i, days, change;

    snprintf(sql, sizeof(sql),
             "SELECT a.appoint_date, a.time_slot, d.dept, d.name, a.change_count, "
             "DATEDIFF(a.appoint_date, CURDATE()) "
             "FROM t_appointment a LEFT JOIN t_doctor d ON d.doctor_id = a.doctor_id "
             "WHERE a.patient_id = '%s' AND a.status = 0 AND a.appoint_date >= CURDATE() "
             "ORDER BY a.appoint_date ASC, a.appoint_id ASC LIMIT 3",
             esc_pid);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    for (i = 0; i < rs->rows && *n < TODO_MAX_LINES; i++) {
        todo_date_text(db_get(rs, i, 0), date_text, sizeof(date_text));
        days = atoi(db_get(rs, i, 5));
        todo_days_text(days, days_text, sizeof(days_text));
        change = atoi(db_get(rs, i, 4));
        change_text[0] = '\0';
        if (change < TODO_CHANGE_MAX) {
            snprintf(change_text, sizeof(change_text), "（可改期 %d 次）",
                     TODO_CHANGE_MAX - change);
        }
        snprintf(lines[*n], TODO_LINE_LEN, "[预约] %s %s %s %s（%s）%s", date_text,
                 db_get(rs, i, 1), db_get(rs, i, 2), db_get(rs, i, 3), days_text, change_text);
        (*n)++;
    }
    db_result_free(rs);
    return 0;
}

/* 规则 3：最近一次已就诊预约，提示就诊记录与处方已生成可查看。 */
static int todo_add_visit(const char *esc_pid, char lines[][TODO_LINE_LEN], int *n)
{
    char sql[SQL_BUF], date_text[32];
    db_result_t *rs = NULL;
    int rc;

    if (*n >= TODO_MAX_LINES) {
        return 0;
    }
    snprintf(sql, sizeof(sql),
             "SELECT a.appoint_date FROM t_appointment a "
             "JOIN t_visit_record v ON v.appoint_id = a.appoint_id "
             "WHERE a.patient_id = '%s' AND a.status = 1 "
             "ORDER BY a.appoint_date DESC, a.appoint_id DESC LIMIT 1",
             esc_pid);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    if (rs->rows > 0) {
        todo_date_text(db_get(rs, 0, 0), date_text, sizeof(date_text));
        snprintf(lines[*n], TODO_LINE_LEN, "[就诊] %s 的就诊记录与处方已生成，可查看", date_text);
        (*n)++;
    }
    db_result_free(rs);
    return 0;
}

/* 规则 4：紧急联系人电话为空时提示补全资料。 */
static int todo_add_profile(const char *esc_pid, char lines[][TODO_LINE_LEN], int *n)
{
    char sql[SQL_BUF];
    db_result_t *rs = NULL;
    int rc;

    if (*n >= TODO_MAX_LINES) {
        return 0;
    }
    snprintf(sql, sizeof(sql), "SELECT contact_phone FROM t_patient WHERE patient_id = '%s'",
             esc_pid);
    rc = db_query(sql, &rs);
    if (rc != 0) {
        return rc;
    }
    if (rs->rows > 0 && str_empty(db_get(rs, 0, 0))) {
        snprintf(lines[*n], TODO_LINE_LEN, "[资料] 尚未填写紧急联系人，建议在个人信息中补充");
        (*n)++;
    }
    db_result_free(rs);
    return 0;
}

/* owner: A，仅替换本函数内部。规则见 SRS FR-M11-02，全部只读查询。 */
int todo_patient(char lines[][TODO_LINE_LEN], int *out_n)
{
    char esc_pid[PID_LEN * 2 + 1];
    int n = 0, rc;

    if (lines == NULL || out_n == NULL) {
        return ERR_PARAM;
    }
    memset(lines, 0, TODO_MAX_LINES * TODO_LINE_LEN);
    *out_n = 0;
    if (session_require_role(ROLE_PATIENT) != 0) {
        return ERR_PERM;
    }
    db_escape(g_session.user_id, esc_pid, sizeof(esc_pid));
    rc = todo_add_future(esc_pid, lines, &n);
    if (rc == 0) {
        rc = todo_add_visit(esc_pid, lines, &n);
    }
    if (rc == 0) {
        rc = todo_add_profile(esc_pid, lines, &n);
    }
    if (rc != 0) {
        return rc;
    }
    *out_n = n;
    return 0;
}

/* owner: C，仅替换本函数内部。 */
int todo_admin(char lines[][TODO_LINE_LEN], int *out_n)
{
    if (lines != NULL) {
        memset(lines, 0, TODO_MAX_LINES * TODO_LINE_LEN);
    }
    if (out_n != NULL) {
        *out_n = 0;
    }
    return ERR_PARAM;
}

/* owner: D，三端只调用此入口，不复制待办打印逻辑。 */
void todo_print(int role)
{
    char lines[TODO_MAX_LINES][TODO_LINE_LEN] = {{0}};
    int n = 0, i, rc;

    if (session_require_role(role) != 0) {
        return;
    }
    switch (role) {
    case ROLE_DOCTOR:
        rc = todo_doctor(lines, &n);
        break;
    case ROLE_PATIENT:
        rc = todo_patient(lines, &n);
        break;
    case ROLE_ADMIN:
        rc = todo_admin(lines, &n);
        break;
    default:
        return;
    }
    print_title("待办事项");
    if (rc != 0) {
        print_table_row("%s", "待办业务尚未实现，请由对应 owner 替换骨架桩");
    } else if (n == 0) {
        print_table_row("%s", "暂无待办事项");
    } else if (n < 0 || n > TODO_MAX_LINES) {
        print_table_row("%s", err_str(ERR_PARAM));
    } else {
        for (i = 0; i < n; i++) {
            lines[i][TODO_LINE_LEN - 1] = '\0';
            print_table_row("%s", lines[i]);
        }
    }
    pause_wait();
}
