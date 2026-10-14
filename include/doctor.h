/* doctor.h  医生档案维护接口（M02，owner: C）
 * 作者：C   创建：2026-10-14   骨架：D */
#ifndef MED_DOCTOR_H
#define MED_DOCTOR_H

#include "common.h"

/* 新增医生（SRS 4.4 白名单）。编号按 SRS 4.3 生成（D + 3 位流水），
   生成与插入在同一事务内。账号重复返回 ERR_DUPLICATE。
   out_doctor_id 可为 NULL。事务发起函数。 */
int doctor_add(const doctor_t *d, char *out_doctor_id);

int doctor_update(const doctor_t *d);

/* 状态：1 在职，0 离职。内部禁止事务控制（SRS 4.4）。 */
int doctor_set_status(const char *doctor_id, int new_status);

int doctor_query_by_id(const char *doctor_id, doctor_t *out);

/* 分页模糊查询（FR-CMN-01）：keyword 匹配 name / dept / title，
   排序固定按 doctor_id。 */
int doctor_query_all(const char *keyword, int page, doctor_t **out, int *out_n, int *out_total);

/* 按科室列出在职医生，供患者端挂号选医生使用。 */
int doctor_query_by_dept(const char *dept, doctor_t **out, int *out_n);

void doctor_list_free(doctor_t *list);

#endif
