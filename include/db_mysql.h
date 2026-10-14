/* db_mysql.h  MySQL C API 薄封装：连接、执行、结果集与预处理语句
 * 作者：D   创建：2026-10-14 */
#ifndef MED_DB_MYSQL_H
#define MED_DB_MYSQL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <mysql/mysql.h>

/* 参数类型枚举 */
typedef enum { DB_PARAM_INT, DB_PARAM_STR } DB_PARAM_TYPE;

/* 预编译绑定参数结构体（兼容原有接口） */
typedef struct {
    DB_PARAM_TYPE type;
    int int_val;
    const char *str_val;
    unsigned long str_len;
} DB_PARAM;

/* 查询单行记录 */
typedef struct {
    char **fields;
    int col_cnt;
} DB_ROW;

/* 查询结果集（纯内存存储，与MySQL原生API完全解耦） */
typedef struct {
    char ***data;       /* 所有行数据：data[行号][列号]，NULL表示SQL NULL */
    int row_cnt;        /* 总行数 */
    int col_cnt;        /* 总列数 */
    int cur_row;        /* 当前读取游标 */
    DB_ROW current_row; /* 当前行缓存（每个结果集独立，无全局静态变量问题） */
} DB_RES;

/* 数据库句柄封装 */
typedef struct {
    MYSQL *mysql;
    int is_connected;
} DB_HANDLE;

/* 预处理语句句柄（Java风格） */
typedef struct {
    MYSQL_STMT *stmt;   /* MySQL原生预处理句柄 */
    MYSQL_BIND *bind;   /* 参数绑定数组 */
    int *int_buf;       /* int参数独立缓冲区 */
    _Bool *is_null_buf; /* NULL标志缓冲区（C99 _Bool，与API类型一致） */
    int param_cnt;      /* 参数总数 */
} DB_STMT;

/* 基础接口 */
DB_HANDLE *dbm_create(void);
int dbm_connect(DB_HANDLE *h, const char *host, const char *user, const char *passwd,
                const char *dbname, int port);
void dbm_close(DB_HANDLE *h);
int dbm_execute(DB_HANDLE *h, const char *sql, DB_PARAM *params, int param_cnt);
DB_RES *dbm_query(DB_HANDLE *h, const char *sql, DB_PARAM *params, int param_cnt);
DB_ROW *dbm_fetch_row(DB_RES *res);
void dbm_res_free(DB_RES *res);

/* Java风格PreparedStatement接口 */
DB_STMT *dbm_stmt_prepare(DB_HANDLE *h, const char *sql);
int dbm_stmt_set_int(DB_STMT *stmt, int index, int value);
int dbm_stmt_set_string(DB_STMT *stmt, int index, const char *str);
int dbm_stmt_execute_update(DB_STMT *stmt);
DB_RES *dbm_stmt_execute_query(DB_STMT *stmt);
void dbm_stmt_close(DB_STMT *stmt);

/* 一键可变参数接口：%d 绑定 int，%s 绑定字符串，%% 输出字面 % */
DB_RES *dbm_query_fmt(DB_HANDLE *h, const char *sql_fmt, ...);
int dbm_exec_fmt(DB_HANDLE *h, const char *sql_fmt, ...);

#endif
