/* db.h  数据库访问层接口（SRS 7.1，owner: D）
 * 作者：D   创建：2026-10-14 */
#ifndef MED_DB_H
#define MED_DB_H

#include <stddef.h>

typedef struct {
    int rows;     /* 结果行数，0 表示空结果集 */
    int cols;     /* 列数 */
    char ***data; /* data[row][col]，数据库 NULL 存为 C 的 NULL 指针 */
} db_result_t;

/* 连接与关闭。参数为 NULL 时取环境变量 DB_HOST/DB_USER/DB_PASS/DB_NAME，
   默认 localhost / root / 空密码 / medical_db。失败返回 ERR_DB_CONN。 */
int db_connect(const char *host, const char *user, const char *pass, const char *dbname,
               unsigned int port);
void db_close(void);

/* 执行不返回结果集的语句（INSERT/UPDATE/DELETE）。
   affected 可为 NULL。SQL 失败返回 ERR_DB_EXEC，连接断开返回 ERR_DB_CONN。
   取自增值用 db_query("SELECT LAST_INSERT_ID()")，同一连接内有效，
   编号生成必须与插入处于同一事务。 */
int db_exec(const char *sql, unsigned long long *affected);

/* 执行查询。成功返回 0，*out 指向结果集（空结果集时 rows=0，data 非 NULL）。
   失败时 *out 置 NULL 并返回错误码。结果集必须用 db_result_free 释放。 */
int db_query(const char *sql, db_result_t **out);
void db_result_free(db_result_t *rs);

/* 按行列取值的便捷访问器：越界或该位置为数据库 NULL 时返回空串 ""，
   调用方因此不需要判 NULL。返回的指针在 db_result_free 后失效。 */
const char *db_get(const db_result_t *rs, int row, int col);

/* 事务原语。嵌套调用 db_tx_begin 返回 ERR_TX。
   只有第 4.4 节白名单中的函数可以调用这三个接口。 */
int db_tx_begin(void);
int db_tx_commit(void);
int db_tx_rollback(void);

/* SQL 字符串转义，内部是 mysql_real_escape_string。
   outlen 必须大于 strlen(in)*2+1，不足时 out 写成空串并在 stderr 记一条警告。
   连接未建立时退化为手工转义（只处理 ' " \ \n \r 和 Ctrl-Z），
   同样在 stderr 写一条警告。 */
void db_escape(const char *in, char *out, size_t outlen);

#endif
