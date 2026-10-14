/* errcode.h  全局错误码定义（SRS 4.1）
 * 作者：D   创建：2026-10-14 */
#ifndef MED_ERRCODE_H
#define MED_ERRCODE_H

#define ERR_DB_CONN (-1)   /* 数据库连接失败 */
#define ERR_DB_EXEC (-2)   /* SQL 执行失败 */
#define ERR_PARAM (-3)     /* 参数非法 */
#define ERR_NOT_FOUND (-4) /* 记录不存在或已注销 */
#define ERR_DUPLICATE (-5) /* 唯一约束冲突：身份证、账号、同日同医生挂号 */
#define ERR_BAD_STATE (-6) /* 状态不允许该操作：非法状态迁移 */
#define ERR_PERM (-7)      /* 越权 */
#define ERR_NO_QUOTA (-8)  /* 无余号 */
#define ERR_STOCK_LOW (-9) /* 库存不足 */
#define ERR_LOCKED (-10)   /* 账号锁定，或病历超过可修改时限 */
#define ERR_TIMEOUT (-11)  /* 会话超时 */
#define ERR_TX (-12)       /* 事务开启或提交失败 */
#define ERR_NOMEM (-13)    /* 内存分配失败 */

const char *err_str(int code); /* 返回中文提示语，未知码返回"未知错误" */

#endif
