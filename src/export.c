/*
 * export.c  列表导出（FR-CMN-03）
 * 作者：C   创建：2026-10-14
 */

#include "export.h"

#include <stdlib.h>
#include <string.h>

/* 业务接口均为未实现桩；仅清空出参并返回 ERR_PARAM，不模拟业务成功。 */

int export_field_list(const char *module, const char **out_fields, int max_n)
{
    (void)module;
    if (out_fields != NULL && max_n > 0) {
        for (int i = 0; i < max_n; i++)
            out_fields[i] = NULL;
    }
    return ERR_PARAM;
}

int export_table(const char *module, const char *keyword, const char *const *fields, int field_n,
                 char *out_path, int outlen)
{
    (void)module;
    (void)keyword;
    (void)fields;
    (void)field_n;
    if (out_path != NULL && outlen > 0)
        out_path[0] = '\0';
    return ERR_PARAM;
}
