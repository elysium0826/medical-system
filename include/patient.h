/* patient.h  患者档案与注册接口（M01，owner: A）
 * 作者：A   创建：2026-10-14   骨架：D */
#ifndef MED_PATIENT_H
#define MED_PATIENT_H

#include "common.h"

/* 注册。编号按 SRS 4.3 生成（P + 4 位年 + 4 位流水），生成与插入在同一事务内。
   身份证重复返回 ERR_DUPLICATE。out_patient_id 可为 NULL。事务发起函数。 */
int patient_register(const patient_t *p, char *out_patient_id);

/* 患者修改本人资料（姓名、电话、住址、紧急联系人）。仅本人，否则 ERR_PERM。 */
int patient_update_self(const patient_t *p);

/* 状态：1 正常，0 注销。跨模块唯一写入口（C 的患者注销 UI 调它），
   内部禁止事务控制，由调用方事务包裹。注销后不可登录、不可挂号。 */
int patient_set_status(const char *patient_id, int new_status);

/* 删除患者档案。事务发起函数。 */
int patient_delete(const char *patient_id);

int patient_query_by_id(const char *patient_id, patient_t *out);

/* 分页模糊查询（FR-CMN-01）：keyword 匹配 name / phone / patient_id，
   NULL 或空串表示不过滤；page 从 1 起；out_total 可为 NULL。
   排序固定按 patient_id。仅 admin 与本人可见全部字段。 */
int patient_query_all(const char *keyword, int page, patient_t **out, int *out_n, int *out_total);

void patient_list_free(patient_t *list);

#endif
