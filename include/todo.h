/* todo.h  三端登录后首屏待办提醒（SRS 6.6 / 8.1，owner: D 框架 + A/B/C 各填函数体）
 * 作者：D   创建：2026-10-14 */
#ifndef MED_TODO_H
#define MED_TODO_H

#define TODO_LINE_LEN 128
#define TODO_MAX_LINES 10

/* 三个函数分别由 B（医生）、A（患者）、C（管理员）实现，只读查询，
   每条待办一行，格式 "[类别] 内容"，最多 TODO_MAX_LINES 条，条数写入 *out_n。
   无待办时 *out_n = 0 并返回 0。 */
int todo_doctor(char lines[][TODO_LINE_LEN], int *out_n);
int todo_patient(char lines[][TODO_LINE_LEN], int *out_n);
int todo_admin(char lines[][TODO_LINE_LEN], int *out_n);

/* 按角色分派并输出待办（D 实现，三端菜单第一项调用） */
void todo_print(int role);

#endif
