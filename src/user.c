/* user.c  登录失败落库与密码修改（FR-CMN-02，owner: D）
 * 作者：D   创建：2026-10-14 */
#include "user.h"
#include "db.h"
#include "patient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int account_meta(int role, const char **table, const char **key)
{
    switch (role) {
    case ROLE_ADMIN:
        *table = "t_admin";
        *key = "admin_id";
        return 0;
    case ROLE_DOCTOR:
        *table = "t_doctor";
        *key = "doctor_id";
        return 0;
    case ROLE_PATIENT:
        *table = "t_patient";
        *key = "patient_id";
        return 0;
    default:
        return ERR_PARAM;
    }
}

static int credentials_valid(const char *name, const char *password)
{
    return !str_empty(name) && !str_empty(password) && strlen(name) < sizeof(g_session.user_id) &&
           strlen(password) < NAME_LEN;
}

void hash_password(const char *raw, char *out, int outlen)
{
    if (out == NULL || outlen <= 0) {
        return;
    }
    out[0] = '\0';
    if (raw != NULL && strlen(raw) < (size_t)outlen) {
        snprintf(out, (size_t)outlen, "%s", raw);
    }
}

/* 表名和主键仅取角色白名单；用户输入只作为已转义 SQL 字面量。 */
static int account_query(int role, const char *name, const char *password, db_result_t **out)
{
    const char *table, *key;
    char sql[SQL_BUF], user[NAME_LEN * 2 + 1], pass[NAME_LEN * 2 + 1];
    const char *active;

    if (account_meta(role, &table, &key) != 0) {
        return ERR_PARAM;
    }
    db_escape(name, user, sizeof(user));
    db_escape(password, pass, sizeof(pass));
    active = role == ROLE_ADMIN ? "1" : "status=1";
    snprintf(sql, sizeof(sql),
             "SELECT name,(BINARY password=BINARY '%s'),"
             "IF(lock_until>NOW(),TIMESTAMPDIFF(MINUTE,NOW(),lock_until)+1,0) "
             "FROM %s WHERE %s='%s' AND %s",
             pass, table, key, user, active);
    return db_query(sql, out);
}

int user_lock_remain(int role, const char *username, int *out_minutes)
{
    db_result_t *rs = NULL;
    int rc;

    if (out_minutes == NULL || str_empty(username) || strlen(username) >= NAME_LEN) {
        return ERR_PARAM;
    }
    *out_minutes = 0;
    rc = account_query(role, username, "", &rs);
    if (rc == 0 && rs->rows > 0) {
        *out_minutes = atoi(db_get(rs, 0, 2));
    }
    db_result_free(rs);
    return rc;
}

static int account_login_update(int role, const char *name, const char *password, int correct)
{
    const char *table, *key, *active;
    char sql[SQL_BUF], user[NAME_LEN * 2 + 1], pass[NAME_LEN * 2 + 1];

    if (account_meta(role, &table, &key) != 0) {
        return ERR_PARAM;
    }
    active = role == ROLE_ADMIN ? "1" : "status=1";
    db_escape(name, user, sizeof(user));
    db_escape(password, pass, sizeof(pass));
    if (correct) {
        snprintf(sql, sizeof(sql),
                 "UPDATE %s SET fail_count=0,lock_until=NULL WHERE %s='%s' "
                 "AND %s AND BINARY password=BINARY '%s' "
                 "AND (lock_until IS NULL OR lock_until<=NOW())",
                 table, key, user, active, pass);
    } else {
        /* MySQL 单表 UPDATE 从左到右求值；先基于旧计数设置锁，再更新计数。 */
        snprintf(sql, sizeof(sql),
                 "UPDATE %s SET lock_until=IF(fail_count+1>=%d,"
                 "NOW()+INTERVAL %d MINUTE,NULL),"
                 "fail_count=IF(fail_count+1>=%d,0,fail_count+1) "
                 "WHERE %s='%s' AND %s AND BINARY password<>BINARY '%s' "
                 "AND (lock_until IS NULL OR lock_until<=NOW())",
                 table, LOCK_FAIL_MAX, LOCK_MINUTES, LOCK_FAIL_MAX, key, user, active, pass);
    }
    return db_exec(sql, NULL);
}

int user_login(int role, const char *username, const char *password)
{
    db_result_t *rs = NULL;
    int rc, correct, remain;
    char display[NAME_LEN];

    session_clear();
    if (!credentials_valid(username, password)) {
        return ERR_PARAM;
    }
    rc = account_query(role, username, password, &rs);
    if (rc != 0) {
        return rc;
    }
    if (rs->rows == 0) {
        db_result_free(rs);
        return ERR_NOT_FOUND;
    }
    correct = atoi(db_get(rs, 0, 1));
    remain = atoi(db_get(rs, 0, 2));
    db_result_free(rs);
    if (remain > 0) {
        return ERR_LOCKED;
    }
    rc = account_login_update(role, username, password, correct);
    if (rc != 0) {
        return rc;
    }
    /* 重新读取，锁定计数由数据库决定；第五次错误当次即返回 ERR_LOCKED。 */
    rc = account_query(role, username, password, &rs);
    if (rc != 0) {
        return rc;
    }
    if (rs->rows == 0) {
        db_result_free(rs);
        return ERR_NOT_FOUND;
    }
    remain = atoi(db_get(rs, 0, 2));
    correct = correct && atoi(db_get(rs, 0, 1));
    snprintf(display, sizeof(display), "%s", db_get(rs, 0, 0));
    db_result_free(rs);
    if (remain > 0) {
        return ERR_LOCKED;
    }
    if (!correct) {
        return ERR_PARAM;
    }
    g_session.role = role;
    snprintf(g_session.username, sizeof(g_session.username), "%s", username);
    snprintf(g_session.user_id, sizeof(g_session.user_id), "%s", username);
    snprintf(g_session.user_name, sizeof(g_session.user_name), "%s", display);
    g_session.login_at = time(NULL);
    g_session.last_action = g_session.login_at;
    return 0;
}

int user_change_password(const char *old_pass, const char *new_pass)
{
    const char *table, *key, *active;
    char sql[SQL_BUF], old[NAME_LEN * 2 + 1], pass[NAME_LEN * 2 + 1];
    char user[NAME_LEN * 2 + 1];
    db_result_t *rs = NULL;
    int rc;

    if (!session_is_login()) {
        return ERR_PERM;
    }
    if (!credentials_valid(g_session.user_id, old_pass) || str_empty(new_pass) ||
        strlen(new_pass) >= NAME_LEN) {
        return ERR_PARAM;
    }
    rc = account_query(g_session.role, g_session.user_id, old_pass, &rs);
    if (rc != 0) {
        return rc;
    }
    if (rs->rows == 0) {
        db_result_free(rs);
        return ERR_NOT_FOUND;
    }
    rc = atoi(db_get(rs, 0, 2)) > 0 ? ERR_LOCKED : (atoi(db_get(rs, 0, 1)) ? 0 : ERR_PARAM);
    db_result_free(rs);
    if (rc != 0) {
        return rc;
    }
    if (account_meta(g_session.role, &table, &key) != 0) {
        return ERR_PERM;
    }
    active = g_session.role == ROLE_ADMIN ? "1" : "status=1";
    db_escape(old_pass, old, sizeof(old));
    db_escape(new_pass, pass, sizeof(pass));
    db_escape(g_session.user_id, user, sizeof(user));
    snprintf(sql, sizeof(sql),
             "UPDATE %s SET password='%s' WHERE %s='%s' AND %s "
             "AND BINARY password=BINARY '%s' AND (lock_until IS NULL OR lock_until<=NOW())",
             table, pass, key, user, active, old);
    return db_exec(sql, NULL);
}

int user_register_patient(const patient_t *p, char *out_patient_id)
{
    if (p == NULL || out_patient_id == NULL) {
        return ERR_PARAM;
    }
    out_patient_id[0] = '\0';
    return patient_register(p, out_patient_id);
}
