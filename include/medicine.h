/* medicine.h  药品档案、库存变化、流水查询与盘点接口（M06 / M10，owner: C）
 * 作者：C   创建：2026-10-14   骨架：D */
#ifndef MED_MEDICINE_H
#define MED_MEDICINE_H

#include "common.h"

/* 库存变化类型（SRS 7.4） */
typedef enum {
    STOCK_IN = 1,
    STOCK_OUT = 2,
    STOCK_PRESC = 3,
    STOCK_GAIN = 4,
    STOCK_LOSS = 5,
    STOCK_VOID = 6
} stock_change_t;

/* 药品状态（SRS 7.4）：停用药不出现在开方选药列表 */
typedef enum { MED_OFF_SALE = 0, MED_ON_SALE = 1 } med_status_t;

/* 新增药品（SRS 4.4 白名单）。编号沿用 v1.0 规则（MED001 起），
   生成与插入在同一事务内。out_med_id 可为 NULL。事务发起函数。 */
int medicine_add(const medicine_t *m, char *out_med_id);

int medicine_update_price(const char *med_id, double new_price);

/* 库存变化的唯一入口，同时写 t_stock_log（SRS 7.2）。delta 为带符号变化量，
   0 表示只留痕（此时 change_type 必须为 STOCK_VOID）。条件 UPDATE 限定
   stock + delta >= 0，影响 0 行且 delta < 0 时返回 ERR_STOCK_LOW。
   before_stock 与 after_stock 都以数据库实际值为准。
   内部禁止事务控制，由 prescription_add / medicine_check_stock 的事务包裹。 */
int medicine_change_stock(const char *med_id, int delta, int change_type, const char *ref_id,
                          const char *operator);

/* 药品状态：1 在售，0 停用。内部禁止事务控制（SRS 7.2）。 */
int medicine_set_status(const char *med_id, int new_status);

/* v1.0 兼容入口，内部改为调 medicine_change_stock（类型 1 / 2），
   对外签名不变（FR-M10-01 第 7 条）。 */
int medicine_stock_in(const char *med_id, int quantity, const char *operator);
int medicine_stock_out(const char *med_id, int quantity, const char *operator);

/* 盘点（FR-M10-04，SRS 4.4 白名单）。book 从 t_medicine 读出，diff = actual - book：
   0 时提示账实相符、不写流水、不开事务；大于 0 调 change_stock(4)、
   小于 0 调 change_stock(5)，delta 带符号不取绝对值。batch 为盘点批次号
   （YYYYMMDDHHMMSS），由 UI 层生成，一次盘点共用。事务发起函数。 */
int medicine_check_stock(const char *med_id, int actual, const char *batch, const char *operator);

int medicine_query_by_id(const char *med_id, medicine_t *out);

/* 分页模糊查询（FR-CMN-01）：keyword 匹配 med_name / manufacturer，
   排序固定按 med_id。列表需标注库存偏低与停用状态。 */
int medicine_query_all(const char *keyword, int page, medicine_t **out, int *out_n, int *out_total);

/* 低库存预警（FR-M10-03）：stock < LOW_STOCK 且 status=1，
   排序 stock ASC 同库存按 med_id，不分页全返回。仅 admin，否则 ERR_PERM。 */
int medicine_query_low_stock(medicine_t **out, int *out_n);

/* 库存流水查询（FR-M10-02）。四个筛选条件均可为空或 0 表示不限，change_type
   非 0 时须在 1~6 之间；JOIN t_medicine 带出 med_name 与 spec；
   排序 create_time DESC, log_id DESC。admin 可查全部；doctor 强制追加
   change_type=3 且限本人处方，外部传入其他类型直接 ERR_PERM；patient 一律
   ERR_PERM。签名与参数个数为 SRS 6.5 原文规定（8 个），
   是 CR-10 参数不超过 6 个的唯一例外，scripts/lint.sh 已按名单放行。 */
int medicine_query_stock_log(const char *med_id, int change_type, const char *date_from,
                             const char *date_to, int page, stock_log_t **out, int *out_n,
                             int *out_total);

void medicine_list_free(medicine_t *list);

void stock_log_list_free(stock_log_t *list);

#endif
