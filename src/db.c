/* db.c  数据库访问层实现，业务模块统一走本文件的接口（SRS 7.1，owner: D）
 * 作者：D   创建：2026-10-14 */
#include "db.h"

#include "db_mysql.h"
#include "errcode.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql/mysql.h>

#define DB_FALLBACK_HOST "localhost"
#define DB_FALLBACK_USER "root"
#define DB_FALLBACK_PASS ""
#define DB_FALLBACK_NAME "medical_db"
#define DB_FALLBACK_PORT 3306

#define TX_SQL_BEGIN "START TRANSACTION"
#define TX_SQL_COMMIT "COMMIT"
#define TX_SQL_ROLLBACK "ROLLBACK"

static DB_HANDLE *g_dbh;
static int g_tx_depth;

static const char *db_env(const char *name, const char *fallback)
{
    const char *value = getenv(name);

    if (value == NULL || value[0] == '\0') {
        return fallback;
    }
    return value;
}

static int db_resolve_port(unsigned int *port)
{
    const char *value;
    char *end;
    unsigned long parsed;

    if (*port != 0) {
        return *port <= 65535 ? 0 : ERR_PARAM;
    }
    value = db_env("DB_PORT", "3306");
    for (const char *p = value; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return ERR_PARAM;
        }
    }
    errno = 0;
    parsed = strtoul(value, &end, 10);
    if (errno == ERANGE || *end != '\0' || parsed < 1 || parsed > 65535) {
        return ERR_PARAM;
    }
    *port = (unsigned int)parsed;
    return 0;
}

static int db_conn_rc(void)
{
    if (g_dbh == NULL || g_dbh->mysql == NULL || g_dbh->is_connected == 0) {
        return ERR_DB_CONN;
    }
    return 0;
}

/* 事务语句不能进预处理协议（MySQL 不接受 START TRANSACTION 走 COM_STMT_PREPARE），
   所以这三条直接在连接上按文本协议执行。 */
static int db_run_raw(const char *sql)
{
    if (g_dbh == NULL || g_dbh->mysql == NULL) {
        return ERR_DB_CONN;
    }
    if (mysql_real_query(g_dbh->mysql, sql, strlen(sql)) != 0) {
        fprintf(stderr, "SQL 执行失败: %s\n", mysql_error(g_dbh->mysql));
        return ERR_DB_EXEC;
    }
    return 0;
}

int db_connect(const char *host, const char *user, const char *pass, const char *dbname,
               unsigned int port)
{
    int rc = db_resolve_port(&port);

    if (rc != 0) {
        return rc;
    }
    if (g_dbh != NULL) {
        return db_conn_rc();
    }
    if (host == NULL) {
        host = db_env("DB_HOST", DB_FALLBACK_HOST);
    }
    if (user == NULL) {
        user = db_env("DB_USER", DB_FALLBACK_USER);
    }
    if (pass == NULL) {
        pass = db_env("DB_PASS", DB_FALLBACK_PASS);
    }
    if (dbname == NULL) {
        dbname = db_env("DB_NAME", DB_FALLBACK_NAME);
    }
    g_dbh = dbm_create();
    if (g_dbh == NULL) {
        return ERR_NOMEM;
    }
    if (dbm_connect(g_dbh, host, user, pass, dbname, (int)port) != 0) {
        dbm_close(g_dbh);
        g_dbh = NULL;
        return ERR_DB_CONN;
    }
    g_tx_depth = 0;
    return 0;
}

void db_close(void)
{
    if (g_dbh == NULL) {
        return;
    }
    if (g_tx_depth > 0) {
        fprintf(stderr, "退出时仍有未结束的事务，已回滚\n");
        mysql_real_query(g_dbh->mysql, TX_SQL_ROLLBACK, strlen(TX_SQL_ROLLBACK));
        g_tx_depth = 0;
    }
    dbm_close(g_dbh);
    g_dbh = NULL;
}

int db_exec(const char *sql, unsigned long long *affected)
{
    int ret;
    int rc = db_conn_rc();

    if (rc != 0) {
        return rc;
    }
    if (sql == NULL) {
        return ERR_PARAM;
    }

    ret = dbm_execute(g_dbh, sql, NULL, 0);
    if (ret < 0) {
        return ERR_DB_EXEC;
    }
    if (affected != NULL) {
        *affected = (unsigned long long)ret;
    }
    return 0;
}

int db_query(const char *sql, db_result_t **out)
{
    DB_RES *res;
    db_result_t *rs;
    int rc = db_conn_rc();

    if (out != NULL) {
        *out = NULL;
    }
    if (rc != 0) {
        return rc;
    }
    if (sql == NULL || out == NULL) {
        return ERR_PARAM;
    }

    res = dbm_query(g_dbh, sql, NULL, 0);
    if (res == NULL) {
        return ERR_DB_EXEC;
    }

    rs = (db_result_t *)malloc(sizeof(db_result_t));
    if (rs == NULL) {
        dbm_res_free(res);
        return ERR_NOMEM;
    }
    rs->rows = res->row_cnt;
    rs->cols = res->col_cnt;
    if (res->row_cnt > 0 && res->data != NULL) {
        /* 收养底层结果集的行数组：先把所有权取走再释放外壳，避免整表深拷贝 */
        rs->data = res->data;
        res->data = NULL;
        res->row_cnt = 0;
    } else {
        rs->rows = 0;
        rs->data = (char ***)calloc(1, sizeof(char **));
        if (rs->data == NULL) {
            free(rs);
            dbm_res_free(res);
            return ERR_NOMEM;
        }
    }
    dbm_res_free(res);

    *out = rs;
    return 0;
}

void db_result_free(db_result_t *rs)
{
    int i;
    int j;

    if (rs == NULL) {
        return;
    }
    for (i = 0; i < rs->rows; i++) {
        if (rs->data[i] == NULL) {
            continue;
        }
        for (j = 0; j < rs->cols; j++) {
            free(rs->data[i][j]);
        }
        free(rs->data[i]);
    }
    free(rs->data);
    free(rs);
}

const char *db_get(const db_result_t *rs, int row, int col)
{
    if (rs == NULL || rs->data == NULL) {
        return "";
    }
    if (row < 0 || row >= rs->rows || col < 0 || col >= rs->cols) {
        return "";
    }
    if (rs->data[row] == NULL || rs->data[row][col] == NULL) {
        return "";
    }
    return rs->data[row][col];
}

int db_tx_begin(void)
{
    int rc = db_conn_rc();

    if (rc != 0) {
        return rc;
    }
    if (g_tx_depth > 0) {
        return ERR_TX;
    }
    rc = db_run_raw(TX_SQL_BEGIN);
    if (rc != 0) {
        return rc;
    }
    g_tx_depth = 1;
    return 0;
}

int db_tx_commit(void)
{
    int rc = db_conn_rc();

    if (rc != 0) {
        return rc;
    }
    if (g_tx_depth <= 0) {
        return ERR_TX;
    }
    if (db_run_raw(TX_SQL_COMMIT) != 0) {
        fprintf(stderr, "事务提交失败，保留事务状态供调用方回滚\n");
        return ERR_DB_EXEC;
    }
    g_tx_depth = 0;
    return 0;
}

int db_tx_rollback(void)
{
    int rc = db_conn_rc();

    if (rc != 0) {
        return rc;
    }
    if (g_tx_depth <= 0) {
        return ERR_TX;
    }
    if (db_run_raw(TX_SQL_ROLLBACK) != 0) {
        return ERR_DB_EXEC;
    }
    g_tx_depth = 0;
    return 0;
}

void db_escape(const char *in, char *out, size_t outlen)
{
    size_t len;
    size_t i;
    size_t o = 0;
    unsigned long written;

    if (out == NULL || outlen == 0) {
        return;
    }
    out[0] = '\0';
    if (in == NULL) {
        return;
    }

    len = strlen(in);
    if (len > ULONG_MAX || len > (outlen - 1) / 2) {
        fprintf(stderr, "db_escape: 输出缓冲区不足，已拒绝转义\n");
        return;
    }
    if (db_conn_rc() == 0) {
        written = mysql_real_escape_string(g_dbh->mysql, out, in, (unsigned long)len);
        if (written != (unsigned long)-1) {
            return;
        }
        out[0] = '\0';
        fprintf(stderr, "db_escape: 连接层转义失败，已拒绝降级，请检查 SQL mode\n");
        return;
    }
    /* 离线课程测试无法得知连接字符集或 SQL mode，不供实际 SQL 执行。 */
    fprintf(stderr, "db_escape: 未连接，手工转义仅供课程离线测试，有 SQL mode 风险\n");

    for (i = 0; i < len; i++) {
        char c = in[i];

        if (c == '\'' || c == '"' || c == '\\' || c == '\n' || c == '\r' || c == '\x1a') {
            out[o++] = '\\';
            if (c == '\n') {
                c = 'n';
            } else if (c == '\r') {
                c = 'r';
            } else if (c == '\x1a') {
                c = 'Z';
            }
        }
        out[o++] = c;
    }
    out[o] = '\0';
}
