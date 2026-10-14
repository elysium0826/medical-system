/* medical_record.h  病历书写与修改接口（M07，owner: B）
 * 作者：B   创建：2026-10-14   骨架：D */
#ifndef MED_MEDICAL_RECORD_H
#define MED_MEDICAL_RECORD_H

#include "common.h"

/* 新增病历。一次就诊只允许一份病历，visit_id 重复返回 ERR_DUPLICATE；
   就诊记录必须属于当前医生，否则 ERR_PERM。 */
int medical_record_add(const medrec_t *m, int *out_record_id);

/* 修改病历。仅本人书写且 update_time 在 MEDREC_EDIT_HOURS 小时之内，
   超时返回 ERR_LOCKED，非本人返回 ERR_PERM。更新语句显式写 NOW()
   （SRS 4.5 第 6 条双保险）。 */
int medical_record_update(const medrec_t *m);

int medical_record_query_by_visit(int visit_id, medrec_t *out);

/* 分页模糊查询（FR-CMN-01）：keyword 匹配患者姓名（JOIN t_patient）。 */
int medical_record_query_by_doctor(const char *doctor_id, int page, medrec_t **out, int *out_n,
                                   int *out_total);

/* 病历打印视图（FR-CMN-04）。按固定版式输出到终端，同时写
   data/record_<visit_id>.txt。本人医生、管理员、本人患者可见，否则 ERR_PERM。
   out_path 可为 NULL。 */
int medical_record_print(int visit_id, char *out_path, int outlen);

#endif
