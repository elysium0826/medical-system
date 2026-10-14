/* schedule.h  按周排班、号源查询与占放号接口（M09，owner: A）
 * 作者：A   创建：2026-10-14   骨架：D */
#ifndef MED_SCHEDULE_H
#define MED_SCHEDULE_H

#include "common.h"

/* 按周生成排班（FR-M09-01）。monday 必须是周一且不早于今天，quota 取 0~200，
   0 表示该时段不出诊不生成记录。从 monday 起 7 天 × 2 时段最多 14 条，
   已存在的 (doctor_id, work_date, time_slot) 跳过且不计入 *out_created。
   单事务，全部成功才提交。事务发起函数。 */
int schedule_add_week(const char *doctor_id, const char *monday, int quota_am, int quota_pm,
                      int *out_created);

/* 排班查询（FR-M09-02）。doctor_id 为 NULL 不限医生；date_from / date_to 均可为
   NULL，两者都为 NULL 时默认今天起 SCHEDULE_DAYS 天；only_available 为 1 时只返回
   status=1 且 quota_used < quota_total 且 work_date 不早于今天的记录。
   JOIN t_doctor 带出姓名与科室，排序 work_date ASC, FIELD(time_slot,'上午','下午')。
   患者角色强制 only_available=1。无记录时 *out_n=0、*out=NULL 并返回 0。只读。 */
int schedule_query_by_doctor(const char *doctor_id, const char *date_from, const char *date_to,
                             int only_available, schedule_t **out, int *out_n);

int schedule_query_by_id(int schedule_id, schedule_t *out);

/* 调整号源总数（FR-M09-03）。仅 admin；work_date 早于今天返回 ERR_BAD_STATE；
   new_total 取 0~200 且不得小于已用号源，条件 UPDATE + 影响行数判断。
   不开事务。事务发起函数（SRS 4.4 白名单）。 */
int schedule_set_quota(int schedule_id, int new_total);

/* 停诊 / 复诊（FR-M09-03）。new_status 只允许 0 或 1。停诊前统计该时段
   status=0 的预约条数，大于 0 时返回 ERR_BAD_STATE 且 *out_blocked 为条数。
   停诊走事务，复诊无条件。事务发起函数（SRS 4.4 白名单）。 */
int schedule_set_status(int schedule_id, int new_status, int *out_blocked);

/* 占用一个号源（FR-M09-04）。quota_used 增加的唯一途径。
   条件 UPDATE，影响 0 行时再查一次区分 ERR_BAD_STATE / ERR_NOT_FOUND / ERR_NO_QUOTA。
   内部禁止事务控制，由 appointment_add / appointment_reschedule 的事务包裹。 */
int schedule_take_quota(int schedule_id);

/* 释放一个号源（FR-M09-04）。quota_used 减少的唯一途径。条件 UPDATE 限定
   quota_used > 0，影响 0 行时返回 0 并写一条 stderr 警告，不视为失败。
   内部禁止事务控制。 */
int schedule_release_quota(int schedule_id);

void schedule_list_free(schedule_t *list);

#endif
