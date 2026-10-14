/* main.c  应用生命周期与角色路由（owner: D）
 * 作者：D   创建：2026-10-14 */
#include "common.h"
#include "db.h"
#include "ui.h"

#include <stdio.h>

int main(void)
{
    int rc;

    rc = db_connect(NULL, NULL, NULL, NULL, 0);
    if (rc != 0) {
        fprintf(stderr, "%s\n", err_str(rc));
        return 1;
    }
    while (ui_login() != ROLE_NONE) {
        switch (g_session.role) {
        case ROLE_ADMIN:
            ui_admin_main();
            break;
        case ROLE_DOCTOR:
            ui_doctor_main();
            break;
        case ROLE_PATIENT:
            ui_patient_main();
            break;
        default:
            break;
        }
        session_clear();
    }
    db_close();
    return 0;
}
