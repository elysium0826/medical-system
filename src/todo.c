/* todo.c  待办公共框架，按函数体共写（SRS 8.1，owner: A/B/C/D）
 * 作者：D   创建：2026-10-14 */
#include "todo.h"
#include "common.h"
#include "ui.h"
#include <string.h>

/* owner: B，仅替换本函数内部。业务待实现，不能将空待办作为FR验收通过。 */
int todo_doctor(char lines[][TODO_LINE_LEN], int *out_n)
{
    if (lines != NULL) {
        memset(lines, 0, TODO_MAX_LINES * TODO_LINE_LEN);
    }
    if (out_n != NULL) {
        *out_n = 0;
    }
    return ERR_PARAM;
}

/* owner: A，仅替换本函数内部。 */
int todo_patient(char lines[][TODO_LINE_LEN], int *out_n)
{
    if (lines != NULL) {
        memset(lines, 0, TODO_MAX_LINES * TODO_LINE_LEN);
    }
    if (out_n != NULL) {
        *out_n = 0;
    }
    return ERR_PARAM;
}

/* owner: C，仅替换本函数内部。 */
int todo_admin(char lines[][TODO_LINE_LEN], int *out_n)
{
    if (lines != NULL) {
        memset(lines, 0, TODO_MAX_LINES * TODO_LINE_LEN);
    }
    if (out_n != NULL) {
        *out_n = 0;
    }
    return ERR_PARAM;
}

/* owner: D，三端只调用此入口，不复制待办打印逻辑。 */
void todo_print(int role)
{
    char lines[TODO_MAX_LINES][TODO_LINE_LEN] = {{0}};
    int n = 0, i, rc;

    if (session_require_role(role) != 0) {
        return;
    }
    switch (role) {
    case ROLE_DOCTOR:
        rc = todo_doctor(lines, &n);
        break;
    case ROLE_PATIENT:
        rc = todo_patient(lines, &n);
        break;
    case ROLE_ADMIN:
        rc = todo_admin(lines, &n);
        break;
    default:
        return;
    }
    print_title("待办事项");
    if (rc != 0) {
        print_table_row("%s", "待办业务尚未实现，请由对应 owner 替换骨架桩");
    } else if (n == 0) {
        print_table_row("%s", "暂无待办事项");
    } else if (n < 0 || n > TODO_MAX_LINES) {
        print_table_row("%s", err_str(ERR_PARAM));
    } else {
        for (i = 0; i < n; i++) {
            lines[i][TODO_LINE_LEN - 1] = '\0';
            print_table_row("%s", lines[i]);
        }
    }
    pause_wait();
}
