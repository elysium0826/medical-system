/* prescription.h  处方开立、作废、明细查询与处方模板接口（M05，owner: B）
 * 作者：B   创建：2026-10-14   骨架：D */
#ifndef MED_PRESCRIPTION_H
#define MED_PRESCRIPTION_H

#include "common.h"

/* 开方（SRS 4.4 白名单）。items 为待提交明细（med_id / quantity / dosage），
   unit_price 由本函数按当前 t_medicine.price 快照写入，subtotal 与 total_price
   由本函数计算。事务内先插主表取自增 presc_id，再逐条插明细，每条明细插入之后
   调 medicine_change_stock(mid, -qty, 3, presc_id, operator) 扣库存；
   任一子步骤失败整体回滚。out_presc_id 可为 NULL。事务发起函数。 */
int prescription_add(int visit_id, const presc_item_t *items, int n, int *out_presc_id);

/* 作废处方（FR-M05-06，SRS 4.4 白名单）。status 置 0，库存不回滚；事务内
   每条明细调 medicine_change_stock(mid, 0, 6, presc_id, operator) 留痕。
   仅 status=1 可作废，重复作废返回 ERR_BAD_STATE；仅开方医生本人或管理员，
   否则 ERR_PERM。事务发起函数。 */
int prescription_void(int presc_id);

int prescription_query_by_id(int presc_id, presc_t *out);

int prescription_query_by_visit(int visit_id, presc_t **out, int *out_n);

/* 分页模糊查询（FR-CMN-01）：keyword 匹配患者姓名（JOIN t_patient）。 */
int prescription_query_by_patient(const char *patient_id, int page, presc_t **out, int *out_n,
                                  int *out_total);

int prescription_query_items(int presc_id, presc_item_t **out, int *out_n);

void prescription_list_free(presc_t *list);

void presc_item_list_free(presc_item_t *list);

/* 保存处方模板（FR-M05-05，SRS 4.4 白名单）。归属当前登录医生，不接受外部
   doctor_id；同医生下模板名唯一，重复返回 ERR_DUPLICATE；n 取 1~20，
   med_id 必须存在且 status=1；同模板内 med_id 不得重复。
   不检查也不扣减库存。事务发起函数。 */
int presc_template_add(const char *tpl_name, const char *remark, const presc_item_t *items, int n);

/* 套用模板（FR-M05-05）。只读，把模板明细复制为待提交列表，unit_price 取当前
   t_medicine.price；停用药跳过并计入 *out_skipped，UI 输出
   "其中 N 种药品已停用，未加入处方"。模板或就诊记录不属于当前医生返回
   ERR_PERM。医生确认后由 UI 调 prescription_add 正式开方。
   注意 dosage 数据库列为 VARCHAR(50)，presc_item_t.dosage 缓冲区为 50 字节，
   输入长度须控制在 49 字符以内。 */
int presc_template_apply(int tpl_id, int visit_id, const char *diagnosis, presc_item_t **out_items,
                         int *out_n, int *out_skipped);

int presc_template_delete(int tpl_id);

/* 当前登录医生的模板列表，排序 tpl_id DESC。 */
int presc_template_query_all(presc_tpl_t **out, int *out_n);

void presc_tpl_list_free(presc_tpl_t *list);

#endif
