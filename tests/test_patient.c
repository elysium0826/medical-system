/*
 * test_patient.c  患者线 M01/M03 与号源真实用例（owner: A）
 * 作者：A   创建：2026-10-14
 */
#include "common.h"
#include "db.h"

#include "appointment.h"
#include "patient.h"
#include "schedule.h"
#include "todo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_db_open(void);

/* ---------------- 离线：参数与权限守卫（不依赖数据库） ---------------- */

static void test_param_guards(void)
{
    patient_t rp = {0};
    char pid[TEXT_LEN] = "sentinel";
    patient_t pt = {0};
    /* 缺姓名/姓名密码为空一律 ERR_PARAM */
    ASSERT_EQ(patient_register(&rp, pid), ERR_PARAM);
    ASSERT_STR_EQ(pid, "");
    ASSERT_EQ(patient_register(NULL, NULL), ERR_PARAM);
    ASSERT_EQ(patient_set_status("", 1), ERR_PARAM);
    ASSERT_EQ(patient_set_status("P00000001", 9), ERR_PARAM);
    ASSERT_EQ(patient_delete(""), ERR_PARAM);
    ASSERT_EQ(patient_query_by_id(NULL, &pt), ERR_PARAM);
    ASSERT_EQ(patient_query_by_id("P20260001", NULL), ERR_PARAM);
    ASSERT_EQ(patient_query_all(NULL, 1, NULL, NULL, NULL), ERR_PARAM);
    /* 挂号：空编号、非法时段、越界日期都在查库前被拦下 */
    ASSERT_EQ(appointment_add("", "D001", 1, "2026-10-15", SLOT_AM, "x"), ERR_PARAM);
    ASSERT_EQ(appointment_add("P20260001", "D001", 0, "2026-10-15", SLOT_AM, "x"), ERR_PARAM);
    ASSERT_EQ(appointment_add("P20260001", "D001", 1, "2026-13-40", SLOT_AM, "x"), ERR_PARAM);
    ASSERT_EQ(appointment_add("P20260001", "D001", 1, "2026-10-15", "夜里", "x"), ERR_PARAM);
    ASSERT_EQ(appointment_set_status(0, 1, "x"), ERR_PARAM);
    ASSERT_EQ(appointment_set_status(1, 9, "x"), ERR_PARAM);
    ASSERT_EQ(appointment_cancel(0), ERR_PARAM);
    ASSERT_EQ(appointment_reschedule(1, NULL, NULL, ROLE_PATIENT, NULL), ERR_PARAM);
    ASSERT_EQ(appointment_mark_noshow(0), ERR_PARAM);
    ASSERT_EQ(appointment_query_by_id(1, NULL), ERR_PARAM);
    ASSERT_EQ(appointment_query_all(NULL, 1, NULL, NULL, NULL), ERR_PARAM);
    ASSERT_EQ(appointment_query_by_patient(NULL, 1, NULL, NULL, NULL), ERR_PARAM);
    ASSERT_EQ(appointment_query_by_doctor(NULL, NULL, 1, NULL, NULL, NULL), ERR_PARAM);
    /* 号源：非法主键、空出参 */
    ASSERT_EQ(schedule_take_quota(0), ERR_PARAM);
    ASSERT_EQ(schedule_release_quota(0), ERR_PARAM);
    ASSERT_EQ(schedule_query_by_id(1, NULL), ERR_PARAM);
    ASSERT_EQ(schedule_query_by_doctor(NULL, NULL, NULL, 1, NULL, NULL), ERR_PARAM);
}

static void test_output_cleared(void)
{
    patient_t pt, zero_p = {0};
    schedule_t st, zero_s = {0};
    appoint_t at, zero_a = {0};
    memset(&pt, 0x5a, sizeof(pt));
    memset(&st, 0x5a, sizeof(st));
    memset(&at, 0x5a, sizeof(at));
    /* 出错时出参必须清零，避免调用方读到脏数据 */
    ASSERT_EQ(patient_query_by_id(NULL, &pt), ERR_PARAM);
    ASSERT_EQ(schedule_query_by_id(0, &st), ERR_PARAM);
    ASSERT_EQ(appointment_query_by_id(0, &at), ERR_PARAM);
    ASSERT_TRUE(memcmp(&pt, &zero_p, sizeof(pt)) == 0);
    ASSERT_TRUE(memcmp(&st, &zero_s, sizeof(st)) == 0);
    ASSERT_TRUE(memcmp(&at, &zero_a, sizeof(at)) == 0);
}

static void test_session_guards(void)
{
    patient_t pt = {0}, *plist = NULL;
    appoint_t one = {0}, *alist = NULL;
    schedule_t *slist = NULL;
    char today[11];
    int n = 0, total = 0;
    today_str(today, sizeof(today));
    session_clear();
    /* 未登录：修改资料与各类列表查询均持 ERR_PERM */
    ASSERT_EQ(patient_update_self(&pt), ERR_PERM);
    ASSERT_EQ(patient_query_all(NULL, 1, &plist, &n, &total), ERR_PERM);
    ASSERT_EQ(appointment_query_all(NULL, 1, &alist, &n, &total), ERR_PERM);
    ASSERT_EQ(appointment_query_by_patient("P20260001", 1, &alist, &n, &total), ERR_PERM);
    ASSERT_EQ(appointment_query_by_doctor("D001", NULL, 1, &alist, &n, &total), ERR_PERM);
    ASSERT_EQ(schedule_query_by_doctor(NULL, NULL, NULL, 1, &slist, &n), ERR_PERM);
    /* 未登录：单条查询与所有写操作也必须拒，且不得碰数据库 */
    ASSERT_EQ(patient_query_by_id("P20260001", &pt), ERR_PERM);
    ASSERT_EQ(patient_set_status("P20260001", 0), ERR_PERM);
    ASSERT_EQ(patient_delete("P20260001"), ERR_PERM);
    ASSERT_EQ(appointment_query_by_id(1, &one), ERR_PERM);
    ASSERT_EQ(appointment_set_status(1, APPT_CANCELED, "x"), ERR_PERM);
    ASSERT_EQ(appointment_add("P20260001", "D001", 1, today, SLOT_AM, ""), ERR_PERM);
    ASSERT_EQ(appointment_cancel(1), ERR_PERM);
    ASSERT_EQ(appointment_mark_noshow(1), ERR_PERM);
    ASSERT_EQ(appointment_reschedule(1, today, SLOT_AM, ROLE_NONE, NULL), ERR_PERM);
}

static void test_list_free_null(void)
{
    patient_list_free(NULL);
    appointment_list_free(NULL);
    schedule_list_free(NULL);
}

static void test_todo_guards(void)
{
    char lines[TODO_MAX_LINES][TODO_LINE_LEN];
    int n = 7;
    ASSERT_EQ(todo_patient(NULL, NULL), ERR_PARAM);
    session_clear();
    ASSERT_EQ(todo_patient(lines, &n), ERR_PERM);
    ASSERT_EQ(n, 0);
}

/* ---------------- 数据库用例（RUN_DB_TESTS=1 DB_NAME=medical_test） ---------------- */

static void fetch_scalar(const char *sql, char *out, int outlen)
{
    db_result_t *rs = NULL;
    ASSERT_RC(db_query(sql, &rs));
    snprintf(out, (size_t)outlen, "%s", db_get(rs, 0, 0));
    db_result_free(rs);
}

static int add_schedule_for(const char *did, const char *date, const char *slot, int total)
{
    char sql[SQL_BUF];
    db_result_t *rs = NULL;
    int id;

    snprintf(sql, sizeof(sql),
             "INSERT INTO t_schedule(doctor_id,work_date,time_slot,quota_total,quota_used,status) "
             "VALUES('%s','%s','%s',%d,0,1)",
             did, date, slot, total);
    ASSERT_RC(db_exec(sql, NULL));
    ASSERT_RC(db_query("SELECT LAST_INSERT_ID()", &rs));
    id = atoi(db_get(rs, 0, 0));
    db_result_free(rs);
    return id;
}

static int add_schedule(const char *date, const char *slot, int total)
{
    return add_schedule_for("UTD01", date, slot, total);
}

static int add_appointment_raw(const char *pid, int sid, const char *date, const char *slot,
                               int status)
{
    char sql[SQL_BUF];
    db_result_t *rs = NULL;
    int id;

    snprintf(sql, sizeof(sql),
             "INSERT INTO t_appointment(patient_id,doctor_id,schedule_id,appoint_date,time_slot,"
             "symptom,status) VALUES('%s','UTD01',%d,'%s','%s','用例',%d)",
             pid, sid, date, slot, status);
    ASSERT_RC(db_exec(sql, NULL));
    ASSERT_RC(db_query("SELECT LAST_INSERT_ID()", &rs));
    id = atoi(db_get(rs, 0, 0));
    db_result_free(rs);
    return id;
}

static void register_ut_patient(patient_t *p, const char *id_card, const char *name, char *pid)
{
    memset(p, 0, sizeof(*p));
    snprintf(p->name, sizeof(p->name), "%s", name);
    snprintf(p->password, sizeof(p->password), "%s", "123456");
    snprintf(p->gender, sizeof(p->gender), "%s", "男");
    snprintf(p->birth_date, sizeof(p->birth_date), "%s", "2000-01-01");
    snprintf(p->id_card, sizeof(p->id_card), "%s", id_card);
    snprintf(p->phone, sizeof(p->phone), "%s", "13800000000");
    ASSERT_RC(patient_register(p, pid));
}

/* 会话 user_id 只有 DID_LEN 长，snprintf 写入时会自动截断并补 '\0'。 */
static void set_session_id(const char *id)
{
    snprintf(g_session.user_id, sizeof(g_session.user_id), "%s", id);
}

/* 用例自净：清掉本线固定医生下的历史数据（中途失败时收尾清理不会跑）。 */
static void wipe_ut(const char *id_like)
{
    char sql[SQL_BUF];

    snprintf(sql, sizeof(sql), "DELETE FROM t_patient WHERE id_card LIKE '%s'", id_like);
    ASSERT_RC(db_exec(sql, NULL));
    ASSERT_RC(db_exec("DELETE FROM t_visit_record WHERE doctor_id='UTD01'", NULL));
    ASSERT_RC(db_exec("DELETE FROM t_appointment WHERE doctor_id='UTD01'", NULL));
    ASSERT_RC(db_exec("DELETE FROM t_schedule WHERE doctor_id='UTD01'", NULL));
}

static void test_patient_db_flow(void)
{
    patient_t p, p2, got;
    patient_t *list = NULL;
    char pid[PID_LEN], pid2[PID_LEN], pid_tmp[PID_LEN];
    int n = 0, total = 0, k;
    const char *kw3[3] = {"UTPCT%ZZZ", "UTUND_QQQ", "UTBSL\\WWW"};
    wipe_ut("UTID%");
    register_ut_patient(&p, "UTID000001", "归档患者UT", pid);
    register_ut_patient(&p2, "UTID000002", "旁证患者UT", pid2);
    ASSERT_TRUE(pid[0] == 'P');
    ASSERT_EQ(strlen(pid), 9);                            /* P + 4 位年 + 4 位流水 */
    ASSERT_EQ(patient_register(&p, NULL), ERR_DUPLICATE); /* 身份证唯一 */
    /* 未登录不得读档案；患者只能读本人，读别人一律 ERR_PERM */
    session_clear();
    ASSERT_EQ(patient_query_by_id(pid, &got), ERR_PERM);
    g_session.role = ROLE_PATIENT;
    set_session_id(pid);
    ASSERT_EQ(patient_query_by_id(pid2, &got), ERR_PERM);
    ASSERT_RC(patient_query_by_id(pid, &got));
    ASSERT_STR_EQ(got.name, "归档患者UT");
    ASSERT_EQ(got.status, 1);
    ASSERT_STR_EQ(got.password, ""); /* 密码不出现在查询出参里 */
    /* 改资料：未登录与被冒用都拒，本人可改电话 */
    session_clear();
    ASSERT_EQ(patient_update_self(&got), ERR_PERM);
    g_session.role = ROLE_PATIENT;
    set_session_id(pid);
    snprintf(got.phone, sizeof(got.phone), "%s", "13900000000");
    ASSERT_RC(patient_update_self(&got));
    /* 列表：管理员按关键词命中（含 P2-2 的字面量匹配），患者本人只看得到自己 */
    g_session.role = ROLE_ADMIN;
    ASSERT_RC(patient_query_all("归档患者UT", 1, &list, &n, &total));
    ASSERT_EQ(n, 1);
    ASSERT_EQ(total, 1);
    patient_list_free(list);
    /* P2-2：% _ \ 按字面量匹配（FR-CMN-01）。% 与 _ 各配一条"无特殊字符"的对照记录，
       转义失效时通配符会连对照一起命中；关键字带 UT 前缀，避开种子里已有的"%字面量"夹具。 */
    register_ut_patient(&p2, "UTID000003", "UTPCT%ZZZ", pid_tmp);
    register_ut_patient(&p2, "UTID000004", "UTPCTXXXZZZ", pid_tmp);
    register_ut_patient(&p2, "UTID000005", "UTUND_QQQ", pid_tmp);
    register_ut_patient(&p2, "UTID000006", "UTUNDYQQQ", pid_tmp);
    register_ut_patient(&p2, "UTID000007", "UTBSL\\WWW", pid_tmp);
    for (k = 0; k < 3; k++) {
        ASSERT_RC(patient_query_all(kw3[k], 1, &list, &n, &total));
        ASSERT_EQ(total, 1); /* 对照记录没被通配符带上 */
        patient_list_free(list);
    }
    g_session.role = ROLE_PATIENT;
    set_session_id(pid);
    ASSERT_RC(patient_query_all(NULL, 1, &list, &n, &total));
    ASSERT_EQ(n, 1);
    ASSERT_STR_EQ(list[0].patient_id, pid);
    patient_list_free(list);
    /* 注销与删除属于管理员权限，患者自己不能注销/删除 */
    ASSERT_EQ(patient_set_status(pid, 0), ERR_PERM);
    ASSERT_EQ(patient_delete(pid), ERR_PERM);
    session_clear();
    g_session.role = ROLE_ADMIN;
    /* 注销后状态 0，删除后查不到 */
    ASSERT_RC(patient_set_status(pid, 0));
    ASSERT_RC(patient_query_by_id(pid, &got));
    ASSERT_EQ(got.status, 0);
    ASSERT_RC(patient_delete(pid));
    ASSERT_EQ(patient_query_by_id(pid, &got), ERR_NOT_FOUND);
    session_clear();
}

static void test_schedule_db_flow(void)
{
    schedule_t sch;
    char sql[SQL_BUF], today[11];
    int sid;

    fetch_scalar("SELECT CURDATE()", today, sizeof(today));
    ASSERT_RC(db_exec("DELETE FROM t_schedule WHERE doctor_id='UTD01'", NULL));
    sid = add_schedule(today, SLOT_AM, 2);
    ASSERT_RC(schedule_query_by_id(sid, &sch));
    ASSERT_STR_EQ(sch.doctor_id, "UTD01");
    ASSERT_EQ(sch.quota_total, 2);
    ASSERT_EQ(sch.quota_used, 0);

    /* 占两个号，第三个 ERR_NO_QUOTA；放号幂等，放到 0 再放也不报错 */
    ASSERT_RC(schedule_take_quota(sid));
    ASSERT_RC(schedule_take_quota(sid));
    ASSERT_EQ(schedule_take_quota(sid), ERR_NO_QUOTA);
    ASSERT_RC(schedule_release_quota(sid));
    ASSERT_RC(schedule_release_quota(sid));
    ASSERT_RC(schedule_release_quota(sid));
    ASSERT_RC(schedule_query_by_id(sid, &sch));
    ASSERT_EQ(sch.quota_used, 0);

    /* 不存在的排班占号 ERR_NOT_FOUND；停诊后占号 ERR_BAD_STATE */
    ASSERT_EQ(schedule_take_quota(sid + 100000), ERR_NOT_FOUND);
    snprintf(sql, sizeof(sql), "UPDATE t_schedule SET status=0 WHERE schedule_id=%d", sid);
    ASSERT_RC(db_exec(sql, NULL));
    ASSERT_EQ(schedule_take_quota(sid), ERR_BAD_STATE);
}

static void test_appointment_db_flow(void)
{
    patient_t p;
    schedule_t sch;
    appoint_t ap, *list = NULL;
    char pid[PID_LEN], today[11], tomorrow[11];
    int n = 0, total = 0, aid, sid, sid2;

    /* 先清本用例的旧数据：上次中途失败时收尾清理不会执行，重跑必须自净 */
    wipe_ut("UTAP%");
    register_ut_patient(&p, "UTAP000001", "挂号患者UT", pid);
    fetch_scalar("SELECT CURDATE()", today, sizeof(today));
    fetch_scalar("SELECT DATE_ADD(CURDATE(), INTERVAL 1 DAY)", tomorrow, sizeof(tomorrow));
    sid = add_schedule(today, SLOT_AM, 2);
    sid2 = add_schedule(tomorrow, SLOT_PM, 2);

    session_clear();
    g_session.role = ROLE_PATIENT;
    set_session_id(pid);

    /* 挂号占号；同患者同日同医生重复 ERR_DUPLICATE；排班错 ERR_NOT_FOUND。
       排班校验在"重复校验"之后，故换到没约过的日期，让流程走到排班检查这一关。 */
    ASSERT_RC(appointment_add(pid, "UTD01", sid, today, SLOT_AM, "头痛"));
    ASSERT_EQ(appointment_add(pid, "UTD01", sid, today, SLOT_AM, "头痛"), ERR_DUPLICATE);
    ASSERT_EQ(appointment_add(pid, "UTD01", sid + 100000, tomorrow, SLOT_PM, "头痛"),
              ERR_NOT_FOUND);

    ASSERT_RC(appointment_query_by_patient(pid, 1, &list, &n, &total));
    ASSERT_EQ(n, 1);
    aid = list[0].appoint_id;
    ASSERT_EQ(list[0].status, APPT_WAITING);
    ASSERT_STR_EQ(list[0].doctor_name, "测试医生UT");
    appointment_list_free(list);
    /* 号源已被占 1 个 */
    ASSERT_RC(schedule_query_by_id(sid, &sch));
    ASSERT_EQ(sch.quota_used, 1);

    /* 改期换号：旧号释放、新号占用、次数累加 */
    ASSERT_RC(appointment_reschedule(aid, tomorrow, SLOT_PM, ROLE_PATIENT, &ap));
    ASSERT_EQ(ap.schedule_id, sid2);
    ASSERT_EQ(ap.change_count, 1);
    ASSERT_RC(schedule_query_by_id(sid, &sch));
    ASSERT_EQ(sch.quota_used, 0);
    ASSERT_RC(schedule_query_by_id(sid2, &sch));
    ASSERT_EQ(sch.quota_used, 1);
}

static void test_appointment_status_db_flow(void)
{
    patient_t p;
    schedule_t sch;
    appoint_t ap;
    char pid[PID_LEN], today[11], tomorrow[11];
    int aid, sid, past_id, past2, future_id;

    wipe_ut("UTNS%");
    register_ut_patient(&p, "UTNS000001", "爽约患者UT", pid);
    fetch_scalar("SELECT CURDATE()", today, sizeof(today));
    fetch_scalar("SELECT DATE_ADD(CURDATE(), INTERVAL 1 DAY)", tomorrow, sizeof(tomorrow));
    sid = add_schedule(today, SLOT_AM, 2);

    session_clear();
    g_session.role = ROLE_PATIENT;
    set_session_id(pid);
    ASSERT_RC(appointment_add(pid, "UTD01", sid, today, SLOT_AM, "复诊"));
    aid = add_appointment_raw(pid, sid, today, SLOT_AM, APPT_WAITING);
    future_id = add_appointment_raw(pid, sid, tomorrow, SLOT_PM, APPT_WAITING);
    past_id = add_appointment_raw(pid, sid, "2000-01-01", SLOT_AM, APPT_WAITING);
    past2 = add_appointment_raw(pid, sid, "2000-01-02", SLOT_AM, APPT_WAITING);
    /* 裸插的这条同样占一个号，才能验证"取消只回退自己那一份，另一条仍占号" */
    ASSERT_RC(schedule_take_quota(sid));
    /* 管理员不是通行证：当天/未来不能标爽约，也不能替接诊医生标已就诊（SRS 附录 D） */
    session_clear();
    g_session.role = ROLE_ADMIN;
    ASSERT_EQ(appointment_mark_noshow(aid), ERR_BAD_STATE);
    ASSERT_EQ(appointment_mark_noshow(future_id), ERR_BAD_STATE);
    ASSERT_EQ(appointment_set_status(aid, APPT_VISITED, "代诊"), ERR_PERM);
    ASSERT_RC(appointment_mark_noshow(past2)); /* 过去的预约：管理员可代标 */
    ASSERT_RC(appointment_query_by_id(past2, &ap));
    ASSERT_EQ(ap.status, APPT_NOSHOW);
    /* 取消：状态 2 且释放号源，终态不可再取消 */
    g_session.role = ROLE_PATIENT;
    set_session_id(pid);
    ASSERT_RC(appointment_cancel(aid));
    ASSERT_RC(appointment_query_by_id(aid, &ap));
    ASSERT_EQ(ap.status, APPT_CANCELED);
    ASSERT_RC(schedule_query_by_id(sid, &sch));
    ASSERT_EQ(sch.quota_used, 1); /* 只回退取消那一条，另一条仍在占号 */
    ASSERT_EQ(appointment_cancel(aid), ERR_BAD_STATE);
    /* 爽约：过期预约才可标；非本人医生拒，本人医生成功置 3 */
    g_session.role = ROLE_DOCTOR;
    set_session_id("UTD99");
    ASSERT_EQ(appointment_mark_noshow(past_id), ERR_PERM);
    set_session_id("UTD01");
    ASSERT_RC(appointment_mark_noshow(past_id));
    ASSERT_RC(appointment_query_by_id(past_id, &ap));
    ASSERT_EQ(ap.status, APPT_NOSHOW);
    session_clear();
}

static void test_appointment_auth_db_flow(void)
{
    patient_t p, p2;
    appoint_t ap, *list = NULL;
    char pid[PID_LEN], pid2[PID_LEN], today[11];
    int n = 0, total = 0, aid, aid2, sid;

    wipe_ut("UTAU%");
    register_ut_patient(&p, "UTAU000001", "授权患者UT", pid);
    register_ut_patient(&p2, "UTAU000002", "他人患者UT", pid2);
    fetch_scalar("SELECT CURDATE()", today, sizeof(today));
    sid = add_schedule(today, SLOT_AM, 4);

    session_clear();
    g_session.role = ROLE_PATIENT;
    set_session_id(pid);
    /* 患者不能替别人挂号，本人挂号放行 */
    ASSERT_EQ(appointment_add(pid2, "UTD01", sid, today, SLOT_AM, "代挂"), ERR_PERM);
    ASSERT_RC(appointment_add(pid, "UTD01", sid, today, SLOT_AM, "本人"));
    /* 再给另一个患者造一条预约，用来验证跨患者读取被拒 */
    aid2 = add_appointment_raw(pid2, sid, today, SLOT_PM, APPT_WAITING);
    ASSERT_RC(schedule_take_quota(sid));

    ASSERT_RC(appointment_query_by_patient(pid, 1, &list, &n, &total));
    ASSERT_EQ(n, 1);
    aid = list[0].appoint_id;
    appointment_list_free(list);

    /* 本人可见；读他人预约、查他人列表、按医生查列表一律 ERR_PERM */
    ASSERT_RC(appointment_query_by_id(aid, &ap));
    ASSERT_EQ(appointment_query_by_id(aid2, &ap), ERR_PERM);
    ASSERT_EQ(appointment_query_by_patient(pid2, 1, &list, &n, &total), ERR_PERM);
    ASSERT_EQ(appointment_query_by_doctor("UTD01", NULL, 1, &list, &n, &total), ERR_PERM);
    ASSERT_EQ(appointment_cancel(aid2), ERR_PERM); /* 取消别人的号也拒 */
    /* P1-1：绕开菜单直接调 set_status，对他人预约同样必须拒 */
    ASSERT_EQ(appointment_set_status(aid2, APPT_CANCELED, "越权"), ERR_PERM);
    ASSERT_EQ(appointment_set_status(aid2, APPT_NOSHOW, "越权"), ERR_PERM);
    session_clear();
    /* 无关医生：读、改期、按医生查、取消、改状态全部拒 */
    g_session.role = ROLE_DOCTOR;
    set_session_id("UTD99");
    ASSERT_EQ(appointment_query_by_id(aid, &ap), ERR_PERM);
    ASSERT_EQ(appointment_reschedule(aid, today, SLOT_PM, ROLE_DOCTOR, NULL), ERR_PERM);
    ASSERT_EQ(appointment_query_by_doctor("UTD01", NULL, 1, &list, &n, &total), ERR_PERM);
    ASSERT_EQ(appointment_cancel(aid), ERR_PERM);
    ASSERT_EQ(appointment_set_status(aid, APPT_CANCELED, "越权"), ERR_PERM);
    ASSERT_EQ(appointment_set_status(aid, APPT_VISITED, "越权"), ERR_PERM);
    ASSERT_EQ(appointment_set_status(aid, APPT_NOSHOW, "越权"), ERR_PERM);
    /* 本人医生查自己的接诊列表放行 */
    set_session_id("UTD01");
    ASSERT_RC(appointment_query_by_doctor("UTD01", NULL, 1, &list, &n, &total));
    ASSERT_EQ(total, 2);
    appointment_list_free(list);
    /* P1-2：换入口仍受限——按患者编号/关键词查询只返回本人接诊范围 */
    ASSERT_RC(appointment_query_by_patient(pid, 1, &list, &n, &total));
    ASSERT_EQ(total, 1);
    appointment_list_free(list);
    ASSERT_RC(appointment_query_all("授权患者UT", 1, &list, &n, &total));
    ASSERT_EQ(total, 1);
    appointment_list_free(list);
    session_clear();
}

static void test_delete_guard_db_flow(void)
{
    patient_t p, got;
    schedule_t sch;
    appoint_t *list = NULL;
    char pid[PID_LEN], today[11];
    int n = 0, total = 0, aid, sid, sid2;

    wipe_ut("UTDL%");
    register_ut_patient(&p, "UTDL000001", "删除守卫UT", pid);
    fetch_scalar("SELECT CURDATE()", today, sizeof(today));
    sid = add_schedule(today, SLOT_AM, 2);

    /* 停职医生不再放出号源：挂号按"无此医生"拒 */
    ASSERT_RC(db_exec("UPDATE t_doctor SET status=0 WHERE doctor_id='UTD01'", NULL));
    session_clear();
    g_session.role = ROLE_PATIENT;
    set_session_id(pid);
    ASSERT_EQ(appointment_add(pid, "UTD01", sid, today, SLOT_AM, "停诊"), ERR_NOT_FOUND);
    ASSERT_RC(db_exec("UPDATE t_doctor SET status=1 WHERE doctor_id='UTD01'", NULL));
    /* 正常挂号后有业务引用，硬删除必须被拦下 */
    ASSERT_RC(appointment_add(pid, "UTD01", sid, today, SLOT_AM, "复诊"));
    ASSERT_RC(appointment_query_by_patient(pid, 1, &list, &n, &total));
    aid = list[0].appoint_id;
    appointment_list_free(list);
    /* P2-1：医生停用后，既有预约改到他的其它号源也必须失败（复用挂号在职检查） */
    sid2 = add_schedule_for("UTD01", today, SLOT_PM, 2);
    ASSERT_RC(db_exec("UPDATE t_doctor SET status=0 WHERE doctor_id='UTD01'", NULL));
    ASSERT_EQ(appointment_reschedule(aid, today, SLOT_PM, ROLE_PATIENT, NULL), ERR_NOT_FOUND);
    ASSERT_RC(schedule_query_by_id(sid2, &sch));
    ASSERT_EQ(sch.quota_used, 0); /* 改期被拒，新号源没被占上 */
    ASSERT_RC(db_exec("UPDATE t_doctor SET status=1 WHERE doctor_id='UTD01'", NULL));
    session_clear();
    g_session.role = ROLE_ADMIN;
    ASSERT_EQ(patient_delete(pid), ERR_BAD_STATE);

    /* 软注销保留历史：档案可查、预约仍在、号源计数不变 */
    ASSERT_RC(patient_set_status(pid, 0));
    ASSERT_RC(patient_query_by_id(pid, &got));
    ASSERT_EQ(got.status, 0);
    ASSERT_RC(schedule_query_by_id(sid, &sch));
    ASSERT_EQ(sch.quota_used, 1);

    /* 取消预约号源立即回退；预约记录仍在，删除依旧被引用拦下 */
    ASSERT_RC(appointment_cancel(aid));
    ASSERT_RC(schedule_query_by_id(sid, &sch));
    ASSERT_EQ(sch.quota_used, 0);
    ASSERT_EQ(patient_delete(pid), ERR_BAD_STATE);
    session_clear();
}

static void test_todo_db_flow(void)
{
    patient_t p;
    char lines[TODO_MAX_LINES][TODO_LINE_LEN];
    char pid[PID_LEN], today[11], sql[SQL_BUF];
    int n = 0, sid, aid, i, has_appoint = 0, has_visit = 0, has_profile = 0;

    wipe_ut("UTTD%");
    register_ut_patient(&p, "UTTD000001", "待办患者UT", pid);
    fetch_scalar("SELECT CURDATE()", today, sizeof(today));
    sid = add_schedule(today, SLOT_AM, 2);

    session_clear();
    g_session.role = ROLE_PATIENT;
    set_session_id(pid);
    ASSERT_RC(appointment_add(pid, "UTD01", sid, today, SLOT_AM, "待办"));

    /* 再造一条已就诊且有就诊记录的预约，覆盖第 3 条待办 */
    aid = add_appointment_raw(pid, sid, today, SLOT_AM, APPT_VISITED);
    snprintf(sql, sizeof(sql),
             "INSERT INTO t_visit_record(appoint_id,patient_id,doctor_id,chief_complaint,"
             "diagnosis) VALUES(%d,'%s','UTD01','咳嗽','感冒')",
             aid, pid);
    ASSERT_RC(db_exec(sql, NULL));

    ASSERT_RC(todo_patient(lines, &n));
    for (i = 0; i < n; i++) {
        if (strncmp(lines[i], "[预约]", 8) == 0) {
            has_appoint = 1;
            ASSERT_TRUE(strstr(lines[i], "今天") != NULL); /* 当天显示"今天" */
        }
        if (strncmp(lines[i], "[就诊]", 8) == 0) {
            has_visit = 1;
        }
        if (strncmp(lines[i], "[资料]", 8) == 0) {
            has_profile = 1;
        }
    }
    ASSERT_TRUE(has_appoint);
    ASSERT_TRUE(has_visit);
    ASSERT_TRUE(has_profile);
    session_clear();
}

static void cleanup_fixtures(void)
{
    ASSERT_RC(db_exec("DELETE FROM t_visit_record WHERE doctor_id='UTD01'", NULL));
    ASSERT_RC(db_exec("DELETE FROM t_appointment WHERE doctor_id='UTD01'", NULL));
    ASSERT_RC(db_exec("DELETE FROM t_schedule WHERE doctor_id='UTD01'", NULL));
    ASSERT_RC(db_exec("DELETE FROM t_patient WHERE id_card LIKE 'UT%'", NULL));
    ASSERT_RC(db_exec("DELETE FROM t_doctor WHERE doctor_id='UTD01'", NULL));
}

static void test_fixtures_setup(void)
{
    ASSERT_RC(db_exec("INSERT INTO t_doctor(doctor_id,password,name,dept,title,status) "
                      "VALUES('UTD01','123456','测试医生UT','内科','医师',1) "
                      "ON DUPLICATE KEY UPDATE name='测试医生UT',dept='内科',status=1",
                      NULL));
}

int main(void)
{
    const char *enabled = getenv("RUN_DB_TESTS");

    test_param_guards();
    test_output_cleared();
    test_session_guards();
    test_list_free_null();
    test_todo_guards();
    if (enabled != NULL && strcmp(enabled, "1") == 0) {
        ASSERT_RC(test_db_open());
        test_fixtures_setup();
        test_patient_db_flow();
        test_schedule_db_flow();
        test_appointment_db_flow();
        test_appointment_status_db_flow();
        test_appointment_auth_db_flow();
        test_delete_guard_db_flow();
        test_todo_db_flow();
        cleanup_fixtures();
        db_close();
        puts("PASS: 患者线 M01/M03 与号源真实数据库用例");
    } else {
        puts("PASS: 患者线参数与权限守卫；SKIP: DB测试（RUN_DB_TESTS=1 DB_NAME=medical_test）");
    }
    return 0;
}
