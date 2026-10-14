/* ui.h  终端界面公共函数与三端主循环声明（SRS 7.3，owner: D）
 * 作者：D   创建：2026-10-14 */
#ifndef MED_UI_H
#define MED_UI_H

void clear_screen(void);
/* 40 个 = 的上下边框，含当前用户与角色 */
void print_title(const char *title);
void print_table_header(const char *fmt, ...); /* 变参，输出表头行与分隔线 */
void print_table_row(const char *fmt, ...);
void print_table_end(void);
/* "第 X/Y 页，共 Z 条"（FR-CMN-01） */
void print_page_info(int page, int total);
/* "按回车键继续..." */
void pause_wait(void);
int read_line(const char *prompt, char *buf, int buflen); /* 去换行，返回 0 成功 */
int read_int(const char *prompt, int *out);               /* 非法输入重试，最多 3 次 */
int read_decimal(const char *prompt, double *out);
/* 校验 YYYY-MM-DD 合法性 */
int read_date(const char *prompt, char *out);
/* 返回选项值，-1 表示放弃 */
int read_choice(const char *prompt, int min, int max);
void read_password(const char *prompt, char *buf, int buflen); /* termios 关回显 */

/* D 的登录界面在 common.c；三端循环在菜单入口及读取选项后检查会话超时。 */
int ui_login(void);

/* 三端主循环，由 main.c 按 g_session.role 调用（SRS 8.4） */
void ui_admin_main(void);   /* owner: C，src/ui_admin.c */
void ui_doctor_main(void);  /* owner: B，src/ui_doctor.c */
void ui_patient_main(void); /* owner: A，src/ui_patient.c */

#endif
