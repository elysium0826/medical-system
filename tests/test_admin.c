/*
 * test_admin.c  管理员角色结构冒烟
 * 作者：shangshuo   创建：2026-10-14
 */

#include "doctor.h"
#include "export.h"
#include "medicine.h"
#include "todo.h"

#ifdef NDEBUG
#error "Structural smoke tests require active assertions"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* 检查未实现接口、列表查询权限和释放接口，不代表整模块验收。 */

static void test_doctor_add(void)
{
    doctor_t input = {0};
    char out_doctor_id[TEXT_LEN] = "sentinel";

    assert(doctor_add(&input, out_doctor_id) == ERR_PARAM);
    assert(out_doctor_id[0] == '\0');
}

static void test_doctor_update(void)
{
    doctor_t input = {0};

    assert(doctor_update(&input) == ERR_PARAM);
}

static void test_doctor_set_status(void)
{
    assert(doctor_set_status("D001", 1) == ERR_PARAM);
}

static void test_doctor_query_by_id(void)
{
    doctor_t entry;
    unsigned char empty_bytes[sizeof(entry)] = {0};

    memset(&entry, 0x5a, sizeof(entry));

    assert(doctor_query_by_id("D001", &entry) == ERR_PARAM);
    assert(memcmp(&entry, empty_bytes, sizeof(entry)) == 0);
}

static void test_doctor_query_all(void)
{
    doctor_t empty_entry = {0};
    doctor_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(doctor_query_all(NULL, 1, &entries, &out_n, &out_total) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_doctor_query_by_dept(void)
{
    doctor_t empty_entry = {0};
    doctor_t *entries = &empty_entry;
    int out_n = 7;

    assert(doctor_query_by_dept("smoke", &entries, &out_n) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
}

static void test_doctor_list_free(void)
{
    doctor_t *entries = malloc(sizeof(*entries));

    assert(entries != NULL);
    doctor_list_free(entries);
    doctor_list_free(NULL);
}

static void test_medicine_add(void)
{
    medicine_t input = {0};
    char out_med_id[TEXT_LEN] = "sentinel";

    assert(medicine_add(&input, out_med_id) == ERR_PARAM);
    assert(out_med_id[0] == '\0');
}

static void test_medicine_update_price(void)
{
    assert(medicine_update_price("MED001", 1) == ERR_PARAM);
}

static void test_medicine_change_stock(void)
{
    assert(medicine_change_stock("MED001", 1, 1, "smoke", "admin") == ERR_PERM);
}

static void test_medicine_set_status(void)
{
    assert(medicine_set_status("MED001", 1) == ERR_PARAM);
}

static void test_medicine_stock_in(void)
{
    assert(medicine_stock_in("MED001", 1, "admin") == ERR_PERM);
}

static void test_medicine_stock_out(void)
{
    assert(medicine_stock_out("MED001", 1, "admin") == ERR_PERM);
}

static void test_medicine_check_stock(void)
{
    assert(medicine_check_stock("MED001", 1, "smoke", "admin") == ERR_PARAM);
}

static void test_medicine_query_by_id(void)
{
    medicine_t entry;
    unsigned char empty_bytes[sizeof(entry)] = {0};

    memset(&entry, 0x5a, sizeof(entry));

    assert(medicine_query_by_id("MED001", &entry) == ERR_PARAM);
    assert(memcmp(&entry, empty_bytes, sizeof(entry)) == 0);
}

static void test_medicine_query_all(void)
{
    medicine_t empty_entry = {0};
    medicine_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(medicine_query_all(NULL, 1, &entries, &out_n, &out_total) == ERR_PERM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_medicine_query_low_stock(void)
{
    medicine_t empty_entry = {0};
    medicine_t *entries = &empty_entry;
    int out_n = 7;

    assert(medicine_query_low_stock(&entries, &out_n) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
}

static void test_medicine_query_stock_log(void)
{
    stock_log_t empty_entry = {0};
    stock_log_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(medicine_query_stock_log("MED001", 1, NULL, NULL, 1, &entries, &out_n, &out_total) ==
           ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_medicine_list_free(void)
{
    medicine_t *entries = malloc(sizeof(*entries));

    assert(entries != NULL);
    medicine_list_free(entries);
    medicine_list_free(NULL);
}

static void test_stock_log_list_free(void)
{
    stock_log_t *entries = malloc(sizeof(*entries));

    assert(entries != NULL);
    stock_log_list_free(entries);
    stock_log_list_free(NULL);
}

static void test_export_field_list(void)
{
    const char *selected_fields[2] = {"sentinel", "sentinel"};

    assert(export_field_list("patient", selected_fields, 2) == ERR_PARAM);
    assert(selected_fields[0] == NULL);
    assert(selected_fields[1] == NULL);
}

static void test_export_table(void)
{
    const char *selected_fields[1] = {"name"};
    char out_path[TEXT_LEN] = "sentinel";

    assert(export_table("patient", NULL, selected_fields, 1, out_path, sizeof(out_path)) ==
           ERR_PARAM);
    assert(out_path[0] == '\0');
}

static void test_todo_admin(void)
{
    char lines[TODO_MAX_LINES][TODO_LINE_LEN];
    char empty_lines[TODO_MAX_LINES][TODO_LINE_LEN] = {{0}};
    int count = 7;

    memset(lines, 0x5a, sizeof(lines));
    assert(todo_admin(lines, &count) == ERR_PARAM);
    assert(count == 0);
    assert(memcmp(lines, empty_lines, sizeof(lines)) == 0);
    assert(todo_admin(NULL, NULL) == ERR_PARAM);
}

static void test_null_outputs(void)
{
    assert(doctor_add(NULL, NULL) == ERR_PARAM);
    assert(doctor_query_by_id(NULL, NULL) == ERR_PARAM);
    assert(doctor_query_all(NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(doctor_query_by_dept(NULL, NULL, NULL) == ERR_PARAM);
    assert(medicine_add(NULL, NULL) == ERR_PARAM);
    assert(medicine_query_by_id(NULL, NULL) == ERR_PARAM);
    assert(medicine_query_all(NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(medicine_query_low_stock(NULL, NULL) == ERR_PARAM);
    assert(medicine_query_stock_log(NULL, 0, NULL, NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(export_field_list(NULL, NULL, 0) == ERR_PARAM);
    assert(export_table(NULL, NULL, NULL, 0, NULL, 0) == ERR_PARAM);
}

static void test_export_capacity(void)
{
    const char *field = "sentinel";
    char path[1] = {'x'};

    assert(export_field_list("patient", &field, 0) == ERR_PARAM);
    assert(field != NULL);
    assert(export_field_list("patient", &field, -1) == ERR_PARAM);
    assert(field != NULL);
    assert(export_table("patient", NULL, NULL, 0, path, 0) == ERR_PARAM);
    assert(path[0] == 'x');
    assert(export_table("patient", NULL, NULL, 0, path, -1) == ERR_PARAM);
    assert(path[0] == 'x');
    assert(export_table("patient", NULL, NULL, 0, path, 1) == ERR_PARAM);
    assert(path[0] == '\0');
}

int main(void)
{
    test_null_outputs();
    test_export_capacity();
    test_doctor_add();
    test_doctor_update();
    test_doctor_set_status();
    test_doctor_query_by_id();
    test_doctor_query_all();
    test_doctor_query_by_dept();
    test_doctor_list_free();
    test_medicine_add();
    test_medicine_update_price();
    test_medicine_change_stock();
    test_medicine_set_status();
    test_medicine_stock_in();
    test_medicine_stock_out();
    test_medicine_check_stock();
    test_medicine_query_by_id();
    test_medicine_query_all();
    test_medicine_query_low_stock();
    test_medicine_query_stock_log();
    test_medicine_list_free();
    test_stock_log_list_free();
    test_export_field_list();
    test_export_table();
    test_todo_admin();
    return 0;
}
