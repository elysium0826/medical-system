/* user.h  登录、锁定、改密与患者注册入口（M08，owner: D）
 * 作者：D   创建：2026-10-14 */
#ifndef MED_USER_H
#define MED_USER_H

#include "common.h"

/* 登录。role 取 ROLE_ADMIN / ROLE_DOCTOR / ROLE_PATIENT，username 为对应表的
   admin_id / doctor_id / patient_id。成功时填充 g_session 并返回 0。
   失败计数与锁定全部落库（FR-CMN-02），不使用进程内静态变量：
   锁定中返回 ERR_LOCKED，密码错误返回 ERR_PARAM，账号不存在或已注销返回
   ERR_NOT_FOUND。 */
int user_login(int role, const char *username, const char *password);

/* 查询账号锁定剩余分钟数，未锁定或账号不存在时 *out_minutes = 0 并返回 0。
   供 UI 输出"账号已锁定，请 N 分钟后重试"。 */
int user_lock_remain(int role, const char *username, int *out_minutes);

/* 修改当前登录账号的密码，old_pass 不匹配返回 ERR_PARAM。
   t_admin 的唯一写入口（SRS 4.6）。 */
int user_change_password(const char *old_pass, const char *new_pass);

/* 患者自助注册：生成编号与写库由 A 的 patient_register 完成（SRS 4.6），
   本函数只做入参校验与角色无关的公共处理。 */
int user_register_patient(const patient_t *p, char *out_patient_id);

/* 密码哈希空桩：当前原样拷贝，启用哈希时只改这一个函数（方案报告 5.4） */
void hash_password(const char *raw, char *out, int outlen);

#endif
