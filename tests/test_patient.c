/*
 * test_patient.c  患者角色结构冒烟
 * 作者：A   创建：2026-10-14
 */

#include "appointment.h"
#include "patient.h"
#include "schedule.h"
#include "todo.h"

#ifdef NDEBUG
#error "Structural smoke tests require active assertions"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* 仅验证未实现桩报 ERR_PARAM、出参清空及释放接口；不代表业务验收用例通过。 */

static void test_patient_register(void)
{
    patient_t input = {0};
    char out_patient_id[TEXT_LEN] = "sentinel";

    assert(patient_register(&input, out_patient_id) == ERR_PARAM);
    assert(out_patient_id[0] == '\0');
}

static void test_patient_update_self(void)
{
    patient_t input = {0};

    assert(patient_update_self(&input) == ERR_PARAM);
}

static void test_patient_set_status(void)
{
    assert(patient_set_status("P20260001", 1) == ERR_PARAM);
}

static void test_patient_delete(void)
{
    assert(patient_delete("P20260001") == ERR_PARAM);
}

static void test_patient_query_by_id(void)
{
    patient_t entry;
    unsigned char empty_bytes[sizeof(entry)] = {0};

    memset(&entry, 0x5a, sizeof(entry));

    assert(patient_query_by_id("P20260001", &entry) == ERR_PARAM);
    assert(memcmp(&entry, empty_bytes, sizeof(entry)) == 0);
}

static void test_patient_query_all(void)
{
    patient_t empty_entry = {0};
    patient_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(patient_query_all(NULL, 1, &entries, &out_n, &out_total) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_patient_list_free(void)
{
    patient_t *entries = malloc(sizeof(*entries));

    assert(entries != NULL);
    patient_list_free(entries);
    patient_list_free(NULL);
}

static void test_appointment_add(void)
{
    assert(appointment_add("P20260001", "D001", 1, "2026-10-15", SLOT_AM, "smoke") == ERR_PARAM);
}

static void test_appointment_set_status(void)
{
    assert(appointment_set_status(1, 1, "smoke") == ERR_PARAM);
}

static void test_appointment_cancel(void)
{
    assert(appointment_cancel(1) == ERR_PARAM);
}

static void test_appointment_reschedule(void)
{
    appoint_t entry;
    unsigned char empty_bytes[sizeof(entry)] = {0};

    memset(&entry, 0x5a, sizeof(entry));

    assert(appointment_reschedule(1, "2026-10-19", SLOT_PM, ROLE_ADMIN, &entry) == ERR_PARAM);
    assert(memcmp(&entry, empty_bytes, sizeof(entry)) == 0);
}

static void test_appointment_mark_noshow(void)
{
    assert(appointment_mark_noshow(1) == ERR_PARAM);
}

static void test_appointment_query_by_id(void)
{
    appoint_t entry;
    unsigned char empty_bytes[sizeof(entry)] = {0};

    memset(&entry, 0x5a, sizeof(entry));

    assert(appointment_query_by_id(1, &entry) == ERR_PARAM);
    assert(memcmp(&entry, empty_bytes, sizeof(entry)) == 0);
}

static void test_appointment_query_all(void)
{
    appoint_t empty_entry = {0};
    appoint_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(appointment_query_all(NULL, 1, &entries, &out_n, &out_total) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_appointment_query_by_patient(void)
{
    appoint_t empty_entry = {0};
    appoint_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(appointment_query_by_patient("P20260001", 1, &entries, &out_n, &out_total) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_appointment_query_by_doctor(void)
{
    appoint_t empty_entry = {0};
    appoint_t *entries = &empty_entry;
    int out_n = 7;
    int out_total = 7;

    assert(appointment_query_by_doctor("D001", NULL, 1, &entries, &out_n, &out_total) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
    assert(out_total == 0);
}

static void test_appointment_list_free(void)
{
    appoint_t *entries = malloc(sizeof(*entries));

    assert(entries != NULL);
    appointment_list_free(entries);
    appointment_list_free(NULL);
}

static void test_schedule_add_week(void)
{
    int out_created = 7;

    assert(schedule_add_week("D001", "2026-10-19", 1, 1, &out_created) == ERR_PARAM);
    assert(out_created == 0);
}

static void test_schedule_query_by_doctor(void)
{
    schedule_t empty_entry = {0};
    schedule_t *entries = &empty_entry;
    int out_n = 7;

    assert(schedule_query_by_doctor("D001", NULL, NULL, 1, &entries, &out_n) == ERR_PARAM);
    assert(entries == NULL);
    assert(out_n == 0);
}

static void test_schedule_query_by_id(void)
{
    schedule_t entry;
    unsigned char empty_bytes[sizeof(entry)] = {0};

    memset(&entry, 0x5a, sizeof(entry));

    assert(schedule_query_by_id(1, &entry) == ERR_PARAM);
    assert(memcmp(&entry, empty_bytes, sizeof(entry)) == 0);
}

static void test_schedule_set_quota(void)
{
    assert(schedule_set_quota(1, 1) == ERR_PARAM);
}

static void test_schedule_set_status(void)
{
    int out_blocked = 7;

    assert(schedule_set_status(1, 1, &out_blocked) == ERR_PARAM);
    assert(out_blocked == 0);
}

static void test_schedule_take_quota(void)
{
    assert(schedule_take_quota(1) == ERR_PARAM);
}

static void test_schedule_release_quota(void)
{
    assert(schedule_release_quota(1) == ERR_PARAM);
}

static void test_schedule_list_free(void)
{
    schedule_t *entries = malloc(sizeof(*entries));

    assert(entries != NULL);
    schedule_list_free(entries);
    schedule_list_free(NULL);
}

static void test_todo_patient(void)
{
    char lines[TODO_MAX_LINES][TODO_LINE_LEN];
    char empty_lines[TODO_MAX_LINES][TODO_LINE_LEN] = {{0}};
    int count = 7;

    memset(lines, 0x5a, sizeof(lines));
    assert(todo_patient(lines, &count) == ERR_PARAM);
    assert(count == 0);
    assert(memcmp(lines, empty_lines, sizeof(lines)) == 0);
    assert(todo_patient(NULL, NULL) == ERR_PARAM);
}

static void test_null_outputs(void)
{
    assert(patient_register(NULL, NULL) == ERR_PARAM);
    assert(patient_query_by_id(NULL, NULL) == ERR_PARAM);
    assert(patient_query_all(NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(appointment_reschedule(1, NULL, NULL, ROLE_PATIENT, NULL) == ERR_PARAM);
    assert(appointment_query_by_id(1, NULL) == ERR_PARAM);
    assert(appointment_query_all(NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(appointment_query_by_patient(NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(appointment_query_by_doctor(NULL, NULL, 1, NULL, NULL, NULL) == ERR_PARAM);
    assert(schedule_add_week(NULL, NULL, 1, 1, NULL) == ERR_PARAM);
    assert(schedule_query_by_doctor(NULL, NULL, NULL, 1, NULL, NULL) == ERR_PARAM);
    assert(schedule_query_by_id(1, NULL) == ERR_PARAM);
    assert(schedule_set_status(1, 1, NULL) == ERR_PARAM);
}

int main(void)
{
    test_null_outputs();
    test_patient_register();
    test_patient_update_self();
    test_patient_set_status();
    test_patient_delete();
    test_patient_query_by_id();
    test_patient_query_all();
    test_patient_list_free();
    test_appointment_add();
    test_appointment_set_status();
    test_appointment_cancel();
    test_appointment_reschedule();
    test_appointment_mark_noshow();
    test_appointment_query_by_id();
    test_appointment_query_all();
    test_appointment_query_by_patient();
    test_appointment_query_by_doctor();
    test_appointment_list_free();
    test_schedule_add_week();
    test_schedule_query_by_doctor();
    test_schedule_query_by_id();
    test_schedule_set_quota();
    test_schedule_set_status();
    test_schedule_take_quota();
    test_schedule_release_quota();
    test_schedule_list_free();
    test_todo_patient();
    return 0;
}
