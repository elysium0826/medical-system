/*
 * test_visit.c  医生角色结构冒烟（接诊/处方/病历/待办，owner: B）
 * 作者：B   创建：2026-10-14
 */

#include "medical_record.h"
#include "prescription.h"
#include "todo.h"
#include "visit.h"

#ifdef NDEBUG
#error "Structural smoke tests require active assertions"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* 仅验证未实现桩报 ERR_PARAM、出参清空及释放接口；不代表业务验收用例通过。 */

static void test_visit_take(void)
{
    visit_t input = {0};
    int out_visit_id = 7;

    assert(visit_take(1, &input, &out_visit_id) == ERR_PARAM);
    assert(out_visit_id == 0);
}

static void test_visit_add(void)
{
    visit_t input = {0};
    int out_visit_id = 7;

    assert(visit_add(&input, &out_visit_id) == ERR_PARAM);
    assert(out_visit_id == 0);
}

static void test_visit_query_by_id(void)
{
    visit_t entry;
    unsigned char empty_bytes[sizeof(entry)] = {0};

    memset(&entry, 0x5a, sizeof(entry));

    assert(visit_query_by_id(1, &entry) == ERR_PARAM);
    assert(memcmp(&entry, empty_bytes, sizeof(entry)) == 0);
}

static void test_visit_query_by_appoint(void)
{
    visit_t entry;
    unsigned char empty_bytes[sizeof(entry)] = {0};

    memset(&entry, 0x5a, sizeof(entry));

    assert(visit_query_by_appoint(1, &entry) == ERR_PARAM);
    assert(memcmp(&entry, empty_bytes, sizeof(entry)) == 0);
}

static void test_visit_query_by_patient(void)
{
    visit_t empty_entry = {0};
    visit_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(visit_query_by_patient("P20260001", 1, &entries, &out_n, &out_total) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_visit_query_by_doctor(void)
{
    visit_t empty_entry = {0};
    visit_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(visit_query_by_doctor("D001", 1, &entries, &out_n, &out_total) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_visit_list_free(void)
{
    visit_t *entries = malloc(sizeof(*entries));

    assert(entries != NULL);
    visit_list_free(entries);
    visit_list_free(NULL);
}

static void test_prescription_add(void)
{
    presc_item_t input = {0};
    int out_presc_id = 7;

    assert(prescription_add(1, &input, 1, &out_presc_id) == ERR_PARAM);
    assert(out_presc_id == 0);
}

static void test_prescription_void(void)
{
    assert(prescription_void(1) == ERR_PARAM);
}

static void test_prescription_query_by_id(void)
{
    presc_t entry;
    unsigned char empty_bytes[sizeof(entry)] = {0};

    memset(&entry, 0x5a, sizeof(entry));

    assert(prescription_query_by_id(1, &entry) == ERR_PARAM);
    assert(memcmp(&entry, empty_bytes, sizeof(entry)) == 0);
}

static void test_prescription_query_by_visit(void)
{
    presc_t empty_entry = {0};
    presc_t *entries = &empty_entry;
    int out_n = 7;

    assert(prescription_query_by_visit(1, &entries, &out_n) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
}

static void test_prescription_query_by_patient(void)
{
    presc_t empty_entry = {0};
    presc_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(prescription_query_by_patient("P20260001", 1, &entries, &out_n, &out_total) ==
           ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_prescription_query_items(void)
{
    presc_item_t empty_entry = {0};
    presc_item_t *entries = &empty_entry;
    int out_n = 7;

    assert(prescription_query_items(1, &entries, &out_n) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
}

static void test_prescription_list_free(void)
{
    presc_t *entries = malloc(sizeof(*entries));

    assert(entries != NULL);
    prescription_list_free(entries);
    prescription_list_free(NULL);
}

static void test_presc_item_list_free(void)
{
    presc_item_t *entries = malloc(sizeof(*entries));

    assert(entries != NULL);
    presc_item_list_free(entries);
    presc_item_list_free(NULL);
}

static void test_presc_template_add(void)
{
    presc_item_t input = {0};

    assert(presc_template_add("smoke", "smoke", &input, 1) == ERR_PARAM);
}

static void test_presc_template_apply(void)
{
    presc_item_t empty_entry = {0};
    presc_item_t *entries = &empty_entry;
    int out_n = 7;
    int out_skipped = 7;

    assert(presc_template_apply(1, 1, "smoke", &entries, &out_n, &out_skipped) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_skipped == 0);
}

static void test_presc_template_delete(void)
{
    assert(presc_template_delete(1) == ERR_PARAM);
}

static void test_presc_template_query_all(void)
{
    presc_tpl_t empty_entry = {0};
    presc_tpl_t *entries = &empty_entry;
    int out_n = 7;

    assert(presc_template_query_all(&entries, &out_n) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
}

static void test_presc_tpl_list_free(void)
{
    presc_tpl_t *entries = malloc(sizeof(*entries));

    assert(entries != NULL);
    presc_tpl_list_free(entries);
    presc_tpl_list_free(NULL);
}

static void test_medical_record_add(void)
{
    medrec_t input = {0};
    int out_record_id = 7;

    assert(medical_record_add(&input, &out_record_id) == ERR_PARAM);
    assert(out_record_id == 0);
}

static void test_medical_record_update(void)
{
    medrec_t input = {0};

    assert(medical_record_update(&input) == ERR_PARAM);
}

static void test_medical_record_query_by_visit(void)
{
    medrec_t entry;
    unsigned char empty_bytes[sizeof(entry)] = {0};

    memset(&entry, 0x5a, sizeof(entry));

    assert(medical_record_query_by_visit(1, &entry) == ERR_PARAM);
    assert(memcmp(&entry, empty_bytes, sizeof(entry)) == 0);
}

static void test_medical_record_query_by_doctor(void)
{
    medrec_t empty_entry = {0};
    medrec_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(medical_record_query_by_doctor("D001", 1, &entries, &out_n, &out_total) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_medical_record_print(void)
{
    char out_path[TEXT_LEN] = "sentinel";

    assert(medical_record_print(1, out_path, sizeof(out_path)) == ERR_PARAM);
    assert(out_path[0] == '\0');
}

static void test_todo_doctor(void)
{
    char lines[TODO_MAX_LINES][TODO_LINE_LEN];
    char empty_lines[TODO_MAX_LINES][TODO_LINE_LEN] = {{0}};
    int count = 7;

    memset(lines, 0x5a, sizeof(lines));
    assert(todo_doctor(lines, &count) == ERR_PARAM);
    assert(count == 0);
    assert(memcmp(lines, empty_lines, sizeof(lines)) == 0);
    assert(todo_doctor(NULL, NULL) == ERR_PARAM);
}

static void test_null_outputs(void)
{
    assert(visit_take(1, NULL, NULL) == ERR_PARAM);
    assert(visit_add(NULL, NULL) == ERR_PARAM);
    assert(visit_query_by_id(1, NULL) == ERR_PARAM);
    assert(visit_query_by_appoint(1, NULL) == ERR_PARAM);
    assert(visit_query_by_patient(NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(visit_query_by_doctor(NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(prescription_add(1, NULL, 1, NULL) == ERR_PARAM);
    assert(prescription_query_by_id(1, NULL) == ERR_PARAM);
    assert(prescription_query_by_visit(1, NULL, NULL) == ERR_PARAM);
    assert(prescription_query_by_patient(NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(prescription_query_items(1, NULL, NULL) == ERR_PARAM);
    assert(presc_template_apply(1, 1, NULL, NULL, NULL, NULL) == ERR_PARAM);
    assert(presc_template_query_all(NULL, NULL) == ERR_PARAM);
    assert(medical_record_add(NULL, NULL) == ERR_PARAM);
    assert(medical_record_query_by_visit(1, NULL) == ERR_PARAM);
    assert(medical_record_query_by_doctor(NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(medical_record_print(1, NULL, 0) == ERR_PARAM);
}

static void test_print_capacity(void)
{
    char path[1] = {'x'};

    assert(medical_record_print(1, path, 0) == ERR_PARAM);
    assert(path[0] == 'x');
    assert(medical_record_print(1, path, -1) == ERR_PARAM);
    assert(path[0] == 'x');
    assert(medical_record_print(1, path, 1) == ERR_PARAM);
    assert(path[0] == '\0');
}

int main(void)
{
    test_null_outputs();
    test_print_capacity();
    test_visit_take();
    test_visit_add();
    test_visit_query_by_id();
    test_visit_query_by_appoint();
    test_visit_query_by_patient();
    test_visit_query_by_doctor();
    test_visit_list_free();
    test_prescription_add();
    test_prescription_void();
    test_prescription_query_by_id();
    test_prescription_query_by_visit();
    test_prescription_query_by_patient();
    test_prescription_query_items();
    test_prescription_list_free();
    test_presc_item_list_free();
    test_presc_template_add();
    test_presc_template_apply();
    test_presc_template_delete();
    test_presc_template_query_all();
    test_presc_tpl_list_free();
    test_medical_record_add();
    test_medical_record_update();
    test_medical_record_query_by_visit();
    test_medical_record_query_by_doctor();
    test_medical_record_print();
    test_todo_doctor();
    return 0;
}
