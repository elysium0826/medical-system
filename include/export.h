/* export.h  列表导出 CSV 接口（FR-CMN-03，owner: C）
 * 作者：C   创建：2026-10-14   骨架：D */
#ifndef MED_EXPORT_H
#define MED_EXPORT_H

#include "common.h"

/* 可导出字段清单。module 取 "patient" 或 "doctor"，把字段名（数据库列名）
   依次写入 out_fields，最多 max_n 个，返回实际字段数；module 非法返回
   ERR_PARAM。密码与登录锁定字段永不出现在清单中。 */
int export_field_list(const char *module, const char **out_fields, int max_n);

/* 导出 CSV。fields 为勾选的字段名，field_n 至少 1，否则 ERR_PARAM；
   keyword 与对应模块的列表查询同义，可为 NULL。文件写入
   data/<module>_export_YYYYMMDD_HHMMSS.csv，data 目录不存在时创建，
   完整路径写入 out_path。首行为中文表头，值含逗号或双引号时按 CSV 规则
   加引号并转义。权限：患者导出仅 admin，医生导出限本人相关，
   患者角色一律 ERR_PERM。 */
int export_table(const char *module, const char *keyword, const char *const *fields, int field_n,
                 char *out_path, int outlen);

#endif
