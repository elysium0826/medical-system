/*
 * db_mysql.c  数据库底层封装（dbm_命名空间）
 * 作者：D   创建：2026-10-14
 * 来源：用户提供的根目录 db_mysql.c，保留类型与调用约定。
 */
#include "db_mysql.h"

#include <limits.h>
#include <stdint.h>

static int array_fits(my_ulonglong count, size_t item_size)
{
    return count <= SIZE_MAX / item_size;
}

static int stmt_error(MYSQL_STMT *stmt, const char *operation)
{
    fprintf(stderr, "dbm: %s failed: %s\n", operation, mysql_stmt_error(stmt));
    return -1;
}

static int parse_sql_fmt(const char *fmt, char **out_sql, DB_PARAM_TYPE **out_types, int *out_cnt)
{
    size_t len = strlen(fmt), pos = 0;
    int count = 0;
    char *sql = NULL;
    DB_PARAM_TYPE *types = NULL;

    if (len > INT_MAX || len > SIZE_MAX / sizeof(*types))
        return -1;
    sql = malloc(len + 1);
    if (len > 0)
        types = calloc(len, sizeof(*types));
    if (!sql || (len > 0 && !types)) {
        free(sql);
        free(types);
        return -1;
    }
    for (size_t i = 0; i < len; i++) {
        if (fmt[i] != '%') {
            sql[pos++] = fmt[i];
            continue;
        }
        if (++i == len) {
            free(sql);
            free(types);
            return -1;
        }
        if (fmt[i] == '%') {
            sql[pos++] = '%';
        } else if (fmt[i] == 'd' || fmt[i] == 's') {
            types[count++] = fmt[i] == 'd' ? DB_PARAM_INT : DB_PARAM_STR;
            sql[pos++] = '?';
        } else {
            sql[pos++] = '%';
            sql[pos++] = fmt[i];
        }
    }
    sql[pos] = '\0';
    *out_sql = sql;
    *out_types = types;
    *out_cnt = count;
    return 0;
}

DB_HANDLE *dbm_create(void)
{
    return calloc(1, sizeof(DB_HANDLE));
}

int dbm_connect(DB_HANDLE *h, const char *host, const char *user, const char *passwd,
                const char *dbname, int port)
{
    if (!h || port < 1 || port > 65535)
        return -1;
    if (h->is_connected)
        return 0;
    if (h->mysql)
        mysql_close(h->mysql);
    h->mysql = mysql_init(NULL);
    h->is_connected = 0;
    if (!h->mysql) {
        fprintf(stderr, "dbm: mysql_init failed\n");
        return -1;
    }
    if (!mysql_real_connect(h->mysql, host, user, passwd, dbname, (unsigned int)port, NULL, 0) ||
        mysql_set_character_set(h->mysql, "utf8mb4")) {
        fprintf(stderr, "dbm: connect/charset failed: %s\n", mysql_error(h->mysql));
        mysql_close(h->mysql);
        h->mysql = NULL;
        return -1;
    }
    h->is_connected = 1;
    return 0;
}

void dbm_close(DB_HANDLE *h)
{
    if (!h)
        return;
    if (h->mysql)
        mysql_close(h->mysql);
    h->mysql = NULL;
    h->is_connected = 0;
    free(h);
}

void dbm_stmt_close(DB_STMT *stmt)
{
    if (!stmt)
        return;
    if (stmt->stmt)
        mysql_stmt_close(stmt->stmt);
    free(stmt->bind);
    free(stmt->int_buf);
    free(stmt->is_null_buf);
    free(stmt);
}

DB_STMT *dbm_stmt_prepare(DB_HANDLE *h, const char *sql)
{
    DB_STMT *stmt;
    unsigned long count;
    size_t len;

    if (!h || !h->is_connected || !h->mysql || !sql)
        return NULL;
    len = strlen(sql);
    if (len > ULONG_MAX)
        return NULL;
    stmt = calloc(1, sizeof(*stmt));
    if (!stmt)
        return NULL;
    stmt->stmt = mysql_stmt_init(h->mysql);
    if (!stmt->stmt) {
        fprintf(stderr, "dbm: stmt init failed: %s\n", mysql_error(h->mysql));
        dbm_stmt_close(stmt);
        return NULL;
    }
    if (mysql_stmt_prepare(stmt->stmt, sql, (unsigned long)len)) {
        stmt_error(stmt->stmt, "prepare");
        dbm_stmt_close(stmt);
        return NULL;
    }
    count = mysql_stmt_param_count(stmt->stmt);
    if (count > INT_MAX || !array_fits(count, sizeof(*stmt->bind))) {
        dbm_stmt_close(stmt);
        return NULL;
    }
    stmt->param_cnt = (int)count;
    if (count > 0) {
        stmt->bind = calloc(count, sizeof(*stmt->bind));
        stmt->int_buf = calloc(count, sizeof(*stmt->int_buf));
        stmt->is_null_buf = calloc(count, sizeof(*stmt->is_null_buf));
        if (!stmt->bind || !stmt->int_buf || !stmt->is_null_buf) {
            dbm_stmt_close(stmt);
            return NULL;
        }
    }
    return stmt;
}

static int valid_index(DB_STMT *stmt, int index)
{
    return stmt && stmt->stmt && stmt->bind && stmt->int_buf && stmt->is_null_buf && index >= 1 &&
           index <= stmt->param_cnt;
}

int dbm_stmt_set_int(DB_STMT *stmt, int index, int value)
{
    MYSQL_BIND *bind;
    int idx;

    if (!valid_index(stmt, index))
        return -1;
    idx = index - 1;
    bind = &stmt->bind[idx];
    memset(bind, 0, sizeof(*bind));
    stmt->int_buf[idx] = value;
    stmt->is_null_buf[idx] = 0;
    bind->buffer_type = MYSQL_TYPE_LONG;
    bind->buffer = &stmt->int_buf[idx];
    bind->buffer_length = sizeof(int);
    /* MySQL 的 bool* 与 MariaDB 的 char* 均使用 0/1 标志。 */
    bind->is_null = (void *)&stmt->is_null_buf[idx];
    return 0;
}

static int set_string(DB_STMT *stmt, int index, const char *str, unsigned long len)
{
    MYSQL_BIND *bind;
    int idx;

    if (!valid_index(stmt, index) || (!str && len != 0))
        return -1;
    idx = index - 1;
    bind = &stmt->bind[idx];
    memset(bind, 0, sizeof(*bind));
    stmt->is_null_buf[idx] = str == NULL;
    bind->buffer_type = str ? MYSQL_TYPE_STRING : MYSQL_TYPE_NULL;
    bind->is_null = (void *)&stmt->is_null_buf[idx];
    bind->buffer = (void *)str;
    bind->buffer_length = len;
    bind->length = &bind->buffer_length;
    return 0;
}

int dbm_stmt_set_string(DB_STMT *stmt, int index, const char *str)
{
    size_t len = str ? strlen(str) : 0;

    if (len > ULONG_MAX)
        return -1;
    return set_string(stmt, index, str, (unsigned long)len);
}

static int bind_params(DB_STMT *stmt, DB_PARAM *params, int count)
{
    if (!stmt || count < 0 || count != stmt->param_cnt || (count > 0 && !params))
        return -1;
    for (int i = 0; i < count; i++) {
        int rc;
        if (params[i].type == DB_PARAM_INT)
            rc = dbm_stmt_set_int(stmt, i + 1, params[i].int_val);
        else if (params[i].type == DB_PARAM_STR)
            rc = set_string(stmt, i + 1, params[i].str_val, params[i].str_len);
        else
            return -1;
        if (rc != 0)
            return -1;
    }
    return 0;
}

static int run_stmt(DB_STMT *stmt)
{
    if (!stmt || !stmt->stmt || stmt->param_cnt < 0)
        return -1;
    if (stmt->param_cnt > 0 && !stmt->bind)
        return -1;
    for (int i = 0; i < stmt->param_cnt; i++) {
        MYSQL_BIND *b = &stmt->bind[i];
        if (b->buffer_type != MYSQL_TYPE_LONG && b->buffer_type != MYSQL_TYPE_STRING &&
            b->buffer_type != MYSQL_TYPE_NULL)
            return -1;
        if ((b->buffer_type == MYSQL_TYPE_LONG || b->buffer_type == MYSQL_TYPE_STRING) &&
            !b->buffer)
            return -1;
    }
    if (mysql_stmt_free_result(stmt->stmt))
        return stmt_error(stmt->stmt, "free previous result");
    if (stmt->param_cnt > 0 && mysql_stmt_bind_param(stmt->stmt, stmt->bind))
        return stmt_error(stmt->stmt, "bind parameters");
    if (mysql_stmt_execute(stmt->stmt))
        return stmt_error(stmt->stmt, "execute");
    return 0;
}

int dbm_stmt_execute_update(DB_STMT *stmt)
{
    my_ulonglong rows;

    if (run_stmt(stmt) != 0)
        return -1;
    rows = mysql_stmt_affected_rows(stmt->stmt);
    if (rows > INT_MAX) {
        fprintf(stderr, "dbm: affected rows exceeds int range\n");
        return -1;
    }
    return (int)rows;
}

DB_ROW *dbm_fetch_row(DB_RES *res)
{
    if (!res || !res->data || res->cur_row < 0 || res->cur_row >= res->row_cnt)
        return NULL;
    res->current_row.fields = res->data[res->cur_row++];
    res->current_row.col_cnt = res->col_cnt;
    return &res->current_row;
}

void dbm_res_free(DB_RES *res)
{
    if (!res)
        return;
    for (int i = 0; res->data && i < res->row_cnt; i++) {
        if (!res->data[i])
            continue;
        for (int j = 0; j < res->col_cnt; j++)
            free(res->data[i][j]);
        free(res->data[i]);
    }
    free(res->data);
    free(res);
}

static DB_RES *allocate_result(MYSQL_STMT *stmt, unsigned int cols)
{
    DB_RES *res;
    my_ulonglong rows = mysql_stmt_num_rows(stmt);

    if (rows > INT_MAX || cols == 0 || cols > INT_MAX || !array_fits(rows, sizeof(char **)) ||
        !array_fits(cols, sizeof(char *)))
        return NULL;
    res = calloc(1, sizeof(*res));
    if (!res)
        return NULL;
    res->row_cnt = (int)rows;
    res->col_cnt = (int)cols;
    if (rows > 0) {
        res->data = calloc((size_t)rows, sizeof(*res->data));
        if (!res->data) {
            dbm_res_free(res);
            return NULL;
        }
    }
    return res;
}

static int fetch_cells(MYSQL_STMT *stmt, DB_RES *res, int row, MYSQL_BIND *bind,
                       unsigned long *lengths, _Bool *nulls)
{
    res->data[row] = calloc((size_t)res->col_cnt, sizeof(*res->data[row]));
    if (!res->data[row])
        return -1;
    for (int col = 0; col < res->col_cnt; col++) {
        MYSQL_BIND cell = bind[col];
        my_ulonglong bytes = lengths[col];
        size_t len;
        if (nulls[col])
            continue;
        if (bytes >= SIZE_MAX)
            return -1;
        len = (size_t)bytes;
        res->data[row][col] = calloc(len + 1, 1);
        if (!res->data[row][col])
            return -1;
        cell.buffer = res->data[row][col];
        cell.buffer_length = lengths[col];
        /* 按真实长度取完整字段，禁止从截断缓冲区复制。 */
        if (mysql_stmt_fetch_column(stmt, &cell, (unsigned int)col, 0))
            return stmt_error(stmt, "fetch column");
        if (nulls[col] || lengths[col] != len)
            return -1;
    }
    return 0;
}

static int fetch_result(MYSQL_STMT *stmt, DB_RES *res)
{
    MYSQL_BIND *bind = calloc((size_t)res->col_cnt, sizeof(*bind));
    unsigned long *lengths = calloc((size_t)res->col_cnt, sizeof(*lengths));
    _Bool *nulls = calloc((size_t)res->col_cnt, sizeof(*nulls));
    int rc = -1;

    if (!bind || !lengths || !nulls)
        goto cleanup;
    for (int col = 0; col < res->col_cnt; col++) {
        bind[col].buffer_type = MYSQL_TYPE_STRING;
        bind[col].length = &lengths[col];
        bind[col].is_null = (void *)&nulls[col];
    }
    /* 零长度绑定只探测真实长度；长 TEXT 再由 fetch_column 完整读取。 */
    if (mysql_stmt_bind_result(stmt, bind)) {
        stmt_error(stmt, "bind result");
        goto cleanup;
    }
    for (int row = 0; row < res->row_cnt; row++) {
        int status = mysql_stmt_fetch(stmt);
        if (status != 0 && status != MYSQL_DATA_TRUNCATED) {
            stmt_error(stmt, "fetch row");
            goto cleanup;
        }
        if (fetch_cells(stmt, res, row, bind, lengths, nulls) != 0)
            goto cleanup;
    }
    if (mysql_stmt_fetch(stmt) != MYSQL_NO_DATA) {
        stmt_error(stmt, "fetch end");
        goto cleanup;
    }
    rc = 0;
cleanup:
    free(bind);
    free(lengths);
    free(nulls);
    return rc;
}

DB_RES *dbm_stmt_execute_query(DB_STMT *stmt)
{
    MYSQL_RES *meta = NULL;
    DB_RES *res = NULL;

    if (run_stmt(stmt) != 0)
        return NULL;
    meta = mysql_stmt_result_metadata(stmt->stmt);
    if (!meta) {
        stmt_error(stmt->stmt, "result metadata");
        goto cleanup;
    }
    if (mysql_stmt_store_result(stmt->stmt)) {
        stmt_error(stmt->stmt, "store result");
        goto cleanup;
    }
    res = allocate_result(stmt->stmt, mysql_num_fields(meta));
    if (res && fetch_result(stmt->stmt, res) != 0) {
        dbm_res_free(res);
        res = NULL;
    }
cleanup:
    if (meta)
        mysql_free_result(meta);
    if (mysql_stmt_free_result(stmt->stmt)) {
        stmt_error(stmt->stmt, "free result");
        dbm_res_free(res);
        res = NULL;
    }
    return res;
}

int dbm_execute(DB_HANDLE *h, const char *sql, DB_PARAM *params, int param_cnt)
{
    DB_STMT *stmt = dbm_stmt_prepare(h, sql);
    int rc = -1;

    if (bind_params(stmt, params, param_cnt) == 0)
        rc = dbm_stmt_execute_update(stmt);
    dbm_stmt_close(stmt);
    return rc;
}

DB_RES *dbm_query(DB_HANDLE *h, const char *sql, DB_PARAM *params, int param_cnt)
{
    DB_STMT *stmt = dbm_stmt_prepare(h, sql);
    DB_RES *res = NULL;

    if (bind_params(stmt, params, param_cnt) == 0)
        res = dbm_stmt_execute_query(stmt);
    dbm_stmt_close(stmt);
    return res;
}

static DB_STMT *prepare_fmt(DB_HANDLE *h, const char *fmt, va_list args)
{
    char *sql = NULL;
    DB_PARAM_TYPE *types = NULL;
    int count = 0;
    DB_STMT *stmt = NULL;

    if (!h || !fmt || parse_sql_fmt(fmt, &sql, &types, &count) != 0)
        return NULL;
    stmt = dbm_stmt_prepare(h, sql);
    free(sql);
    if (!stmt || count != stmt->param_cnt) {
        dbm_stmt_close(stmt);
        free(types);
        return NULL;
    }
    for (int i = 0; i < count; i++) {
        int rc;
        if (types[i] == DB_PARAM_INT)
            rc = dbm_stmt_set_int(stmt, i + 1, va_arg(args, int));
        else
            rc = dbm_stmt_set_string(stmt, i + 1, va_arg(args, const char *));
        if (rc != 0) {
            dbm_stmt_close(stmt);
            stmt = NULL;
            break;
        }
    }
    free(types);
    return stmt;
}

DB_RES *dbm_query_fmt(DB_HANDLE *h, const char *sql_fmt, ...)
{
    va_list args;
    DB_STMT *stmt;
    DB_RES *res;

    va_start(args, sql_fmt);
    stmt = prepare_fmt(h, sql_fmt, args);
    va_end(args);
    res = dbm_stmt_execute_query(stmt);
    dbm_stmt_close(stmt);
    return res;
}

int dbm_exec_fmt(DB_HANDLE *h, const char *sql_fmt, ...)
{
    va_list args;
    DB_STMT *stmt;
    int rc;

    va_start(args, sql_fmt);
    stmt = prepare_fmt(h, sql_fmt, args);
    va_end(args);
    rc = dbm_stmt_execute_update(stmt);
    dbm_stmt_close(stmt);
    return rc;
}
