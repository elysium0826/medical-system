/* appointment.h  预约挂号、状态迁移、改期与爽约接口（M03，owner: A）
 * 作者：A   创建：2026-10-14   骨架：D */
#ifndef MED_APPOINTMENT_H
#define MED_APPOINTMENT_H

#include "common.h"

/* 预约状态（SRS 7.4），合法迁移见 SRS 附录 D */
typedef enum {
    APPT_WAITING = 0,
    APPT_VISITED = 1,
    APPT_CANCELED = 2,
    APPT_NOSHOW = 3
} appt_status_t;

/* 挂号。schedule_id 由 UI 从排班列表选定后传入（FR-M03-07）。
   事务内先 schedule_take_quota，占号失败则整个挂号失败且不写 t_appointment。
   同患者同日同医生重复挂号返回 ERR_DUPLICATE，检查在占号之前。事务发起函数。 */
int appointment_add(const char *patient_id, const char *doctor_id, int schedule_id,
                    const char *appoint_date, const char *time_slot, const char *symptom);

/* 预约状态迁移的唯一入口（SRS 7.2、附录 D），函数内校验当前会话及角色、归属。
   接诊限归属医生；取消限本人患者或管理员；爽约限管理员或归属医生且日期早于今天。
   仅 0 -> 2 在更新成功后释放一次号源，调用方不得重复释放；历史 schedule_id=0 跳过。
   内部禁止事务控制，由调用方包裹事务。非法迁移返回 ERR_BAD_STATE，越权返回 ERR_PERM。
   reason 可为 NULL，仅用于 UI 回显，不落库。 */
int appointment_set_status(int appoint_id, int new_status, const char *reason);

/* 取消挂号。仅本人患者或管理员可操作。事务内调 appointment_set_status 置 2，
   由该入口释放一次号源，本函数不重复释放；任一步失败回滚。事务发起函数。 */
int appointment_cancel(int appoint_id);

/* 改期（FR-M03-06）。operator_role 须与当前会话一致；患者限本人，管理员任意，医生无权。
   change_count 超过 3 返回 ERR_BAD_STATE。事务内顺序：释放原号源 → 占用新号源
   → 更新字段。out_new_appoint 可为 NULL。事务发起函数。 */
int appointment_reschedule(int appoint_id, const char *new_date, const char *new_slot,
                           int operator_role, appoint_t *out_new_appoint);

/* 标记爽约（FR-M03-06）。仅 status=0 且 appoint_date 早于今天，否则 ERR_BAD_STATE；
   仅管理员或该预约的归属医生可操作，否则 ERR_PERM。号源不释放。事务发起函数。 */
int appointment_mark_noshow(int appoint_id);

int appointment_query_by_id(int appoint_id, appoint_t *out);

/* 分页模糊查询（FR-CMN-01）：keyword 匹配患者姓名（JOIN t_patient）。
   排序固定 appoint_date DESC, appoint_id DESC。 */
int appointment_query_all(const char *keyword, int page, appoint_t **out, int *out_n,
                          int *out_total);

int appointment_query_by_patient(const char *patient_id, int page, appoint_t **out, int *out_n,
                                 int *out_total);

/* date 为 NULL 时查全部，否则只查该日；供医生端今日待接诊列表使用。 */
int appointment_query_by_doctor(const char *doctor_id, const char *date, int page, appoint_t **out,
                                int *out_n, int *out_total);

void appointment_list_free(appoint_t *list);

#endif
