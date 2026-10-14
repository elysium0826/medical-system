/* visit.h  接诊与就诊记录接口（M04，owner: B）
 * 作者：B   创建：2026-10-14   骨架：D */
#ifndef MED_VISIT_H
#define MED_VISIT_H

#include "common.h"

/* 接诊（SRS 4.4 白名单）。v 携带主诉、现病史、体征、诊断与 patient_id /
   doctor_id；事务内先写 t_visit_record，再调 appointment_set_status(appoint_id, 1,
   "接诊")；接诊不释放号源。预约不是待就诊状态返回 ERR_BAD_STATE，
   不是本人预约返回 ERR_PERM。out_visit_id 可为 NULL。事务发起函数。 */
int visit_take(int appoint_id, const visit_t *v, int *out_visit_id);

/* t_visit_record 的唯一写入口（SRS 4.6）。内部禁止事务控制，
   由 visit_take 的事务包裹。out_visit_id 可为 NULL。 */
int visit_add(const visit_t *v, int *out_visit_id);

int visit_query_by_id(int visit_id, visit_t *out);

int visit_query_by_appoint(int appoint_id, visit_t *out);

/* 分页模糊查询（FR-CMN-01）：keyword 匹配患者姓名（JOIN t_patient），
   排序按主键 DESC。患者只能查本人记录，医生只能查本人接诊记录。 */
int visit_query_by_patient(const char *patient_id, int page, visit_t **out, int *out_n,
                           int *out_total);

int visit_query_by_doctor(const char *doctor_id, int page, visit_t **out, int *out_n,
                          int *out_total);

void visit_list_free(visit_t *list);

#endif
