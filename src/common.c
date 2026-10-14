/* common.c  公共工具、会话与终端输入输出（owner: D）
 * 作者：D   创建：2026-10-14 */
#include "common.h"
#include "ui.h"
#include "user.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

session_t g_session;

const char *err_str(int code)
{
    switch (code) {
    case 0:
        return "操作成功";
    case ERR_DB_CONN:
        return "数据库连接失败";
    case ERR_DB_EXEC:
        return "SQL 执行失败";
    case ERR_PARAM:
        return "参数非法";
    case ERR_NOT_FOUND:
        return "记录不存在或已注销";
    case ERR_DUPLICATE:
        return "记录重复";
    case ERR_BAD_STATE:
        return "状态不允许该操作";
    case ERR_PERM:
        return "无权执行该操作";
    case ERR_NO_QUOTA:
        return "无余号";
    case ERR_STOCK_LOW:
        return "库存不足";
    case ERR_LOCKED:
        return "账号锁定或超过可修改时限";
    case ERR_TIMEOUT:
        return "会话已超时，请重新登录";
    case ERR_TX:
        return "事务状态错误";
    case ERR_NOMEM:
        return "内存分配失败";
    default:
        return "未知错误";
    }
}

void session_clear(void)
{
    memset(&g_session, 0, sizeof(g_session));
}

int session_is_login(void)
{
    return g_session.role >= ROLE_ADMIN && g_session.role <= ROLE_PATIENT;
}

int session_check_timeout(void)
{
    if (!session_is_login()) {
        return ERR_PERM;
    }
    if (difftime(time(NULL), g_session.last_action) > SESSION_TIMEOUT_MIN * 60) {
        session_clear();
        return ERR_TIMEOUT;
    }
    return 0;
}

void session_touch(void)
{
    if (session_is_login()) {
        g_session.last_action = time(NULL);
    }
}

int session_require_role(int role)
{
    return session_is_login() && g_session.role == role ? 0 : ERR_PERM;
}

int str_empty(const char *s)
{
    return s == NULL || s[0] == '\0';
}

int date_valid(const char *s)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int year, month, day, limit;
    size_t i;

    if (s == NULL || strlen(s) != 10 || s[4] != '-' || s[7] != '-') {
        return 0;
    }
    for (i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit((unsigned char)s[i])) {
            return 0;
        }
    }
    if (sscanf(s, "%4d-%2d-%2d", &year, &month, &day) != 3 || year < 1 || month < 1 || month > 12 ||
        day < 1) {
        return 0;
    }
    limit = days[month - 1];
    if (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        limit++;
    }
    return day <= limit;
}

/* 公历日期转整数天数，不经过本地时区，避免夏令时导致的 23/25 小时日。 */
static int date_day_number(const char *s)
{
    int year, month, day, era, yoe, doy, doe;

    sscanf(s, "%4d-%2d-%2d", &year, &month, &day);
    year -= month <= 2;
    era = year / 400;
    yoe = year - era * 400;
    doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe;
}

int date_diff_days(const char *d1, const char *d2)
{
    if (!date_valid(d1) || !date_valid(d2)) {
        return 0;
    }
    return date_day_number(d1) - date_day_number(d2);
}

static void format_time(char *out, int outlen, const char *format)
{
    time_t value = time(NULL);
    struct tm *local;

    if (out == NULL || outlen <= 0) {
        return;
    }
    out[0] = '\0';
    local = localtime(&value);
    if (local != NULL && strftime(out, (size_t)outlen, format, local) == 0) {
        out[0] = '\0';
    }
}

void now_str(char *out, int outlen)
{
    format_time(out, outlen, "%Y-%m-%d %H:%M:%S");
}

void today_str(char *out, int outlen)
{
    format_time(out, outlen, "%Y-%m-%d");
}

int page_normalize(int page)
{
    return page < 1 ? 1 : page;
}

int page_offset(int page)
{
    page = page_normalize(page);
    return page - 1 > INT_MAX / PAGE_SIZE ? INT_MAX : (page - 1) * PAGE_SIZE;
}

int page_count(int total)
{
    return total < 1 ? 1 : 1 + (total - 1) / PAGE_SIZE;
}

/* 入参已经 db_escape；双反斜杠经 SQL 字面量解析后保留 LIKE 转义字符。 */
void like_escape(const char *in, char *out, int outlen)
{
    size_t i, n = 0;

    if (out == NULL || outlen <= 0) {
        return;
    }
    out[0] = '\0';
    if (in == NULL) {
        return;
    }
    for (i = 0; in[i] != '\0'; i++) {
        n += (in[i] == '%' || in[i] == '_') ? 3 : 1;
    }
    if (n >= (size_t)outlen) {
        fprintf(stderr, "like_escape: 输出缓冲区不足\n");
        return;
    }
    n = 0;
    for (i = 0; in[i] != '\0'; i++) {
        if (in[i] == '%' || in[i] == '_') {
            out[n++] = '\\';
            out[n++] = '\\';
        }
        out[n++] = in[i];
    }
    out[n] = '\0';
}

void assert_eq_int(const char *file, int line, const char *expr, long long actual, long long expect)
{
    if (actual != expect) {
        fprintf(stderr, "%s:%d %s: actual=%lld expected=%lld\n", file, line, expr, actual, expect);
        abort();
    }
}

void assert_eq_str(const char *file, int line, const char *expr, const char *actual,
                   const char *expect)
{
    if (actual == NULL || expect == NULL || strcmp(actual, expect) != 0) {
        fprintf(stderr, "%s:%d %s: 字符串断言失败\n", file, line, expr);
        abort();
    }
}

void assert_true(const char *file, int line, const char *expr, int cond)
{
    assert_eq_int(file, line, expr, cond != 0, 1);
}

void assert_rc(const char *file, int line, const char *expr, int rc)
{
    assert_eq_int(file, line, expr, rc, 0);
}

void clear_screen(void)
{
    if (isatty(STDOUT_FILENO)) {
        printf("\033[2J\033[H");
    }
}

static const char *role_name(int role)
{
    switch (role) {
    case ROLE_ADMIN:
        return "管理员";
    case ROLE_DOCTOR:
        return "医生";
    case ROLE_PATIENT:
        return "患者";
    default:
        return "未登录";
    }
}

void print_title(const char *title)
{
    printf("========================================\n%s\n", title != NULL ? title : "");
    if (session_is_login()) {
        printf("当前用户：%s（%s）  账号：%s\n", g_session.user_name, role_name(g_session.role),
               g_session.user_id);
    }
    printf("========================================\n");
}

void print_table_header(const char *fmt, ...)
{
    va_list args;

    if (fmt == NULL) {
        return;
    }
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n----------------------------------------\n");
}

void print_table_row(const char *fmt, ...)
{
    va_list args;

    if (fmt == NULL) {
        return;
    }
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void print_table_end(void)
{
    printf("----------------------------------------\n");
}

void print_page_info(int page, int total)
{
    printf("第 %d/%d 页，共 %d 条\n", page_normalize(page), page_count(total),
           total < 0 ? 0 : total);
}

/* 允许空行；超长输入整行丢弃，不接受截断字段。EOF 返回 ERR_PARAM。 */
int read_line(const char *prompt, char *buf, int buflen)
{
    size_t n;
    int c;

    if (buf == NULL || buflen < 2) {
        return ERR_PARAM;
    }
    buf[0] = '\0';
    if (prompt != NULL) {
        printf("%s", prompt);
        fflush(stdout);
    }
    if (fgets(buf, buflen, stdin) == NULL) {
        return ERR_PARAM;
    }
    n = strlen(buf);
    if (n > 0 && buf[n - 1] == '\n') {
        buf[--n] = '\0';
    } else {
        c = getchar();
        if (c != '\n' && c != EOF) {
            while ((c = getchar()) != '\n' && c != EOF) {
            }
            buf[0] = '\0';
            return ERR_PARAM;
        }
    }
    if (n > 0 && buf[n - 1] == '\r') {
        buf[n - 1] = '\0';
    }
    return 0;
}

void pause_wait(void)
{
    char line[8];

    read_line("按回车键继续...", line, sizeof(line));
}

static int at_end(const char *s)
{
    while (*s != '\0' && isspace((unsigned char)*s)) {
        s++;
    }
    return *s == '\0';
}

int read_int(const char *prompt, int *out)
{
    char line[64], *end;
    long value;
    int i;

    if (out == NULL) {
        return ERR_PARAM;
    }
    for (i = 0; i < 3; i++) {
        if (read_line(prompt, line, sizeof(line)) != 0) {
            if (feof(stdin)) {
                return ERR_PARAM;
            }
            continue;
        }
        errno = 0;
        value = strtol(line, &end, 10);
        if (end != line && at_end(end) && errno == 0 && value >= INT_MIN && value <= INT_MAX) {
            *out = (int)value;
            return 0;
        }
        printf("输入无效，请输入整数\n");
    }
    return ERR_PARAM;
}

int read_decimal(const char *prompt, double *out)
{
    char line[64], *end;
    double value;
    int i;

    if (out == NULL) {
        return ERR_PARAM;
    }
    for (i = 0; i < 3; i++) {
        if (read_line(prompt, line, sizeof(line)) != 0) {
            if (feof(stdin)) {
                return ERR_PARAM;
            }
            continue;
        }
        errno = 0;
        value = strtod(line, &end);
        if (end != line && at_end(end) && errno == 0 && isfinite(value)) {
            *out = value;
            return 0;
        }
        printf("输入无效，请输入有限小数\n");
    }
    return ERR_PARAM;
}

int read_date(const char *prompt, char *out)
{
    char line[32];
    int i;

    if (out == NULL) {
        return ERR_PARAM;
    }
    out[0] = '\0';
    for (i = 0; i < 3; i++) {
        if (read_line(prompt, line, sizeof(line)) == 0 && date_valid(line)) {
            memcpy(out, line, 11);
            return 0;
        }
        if (feof(stdin)) {
            return ERR_PARAM;
        }
        printf("日期无效，请按 YYYY-MM-DD 输入\n");
    }
    return ERR_PARAM;
}

int read_choice(const char *prompt, int min, int max)
{
    int value, i;
    char line[64], *end;
    long parsed;

    if (min > max) {
        return -1;
    }
    for (i = 0; i < 3; i++) {
        if (read_line(prompt, line, sizeof(line)) != 0) {
            if (feof(stdin)) {
                return -1;
            }
            continue;
        }
        errno = 0;
        parsed = strtol(line, &end, 10);
        if (end != line && at_end(end) && errno == 0 && parsed >= min && parsed <= max) {
            value = (int)parsed;
            return value;
        }
        printf("选项无效，请输入 %d-%d\n", min, max);
    }
    return -1;
}

/* 登录 UI 属公共终端层，main.c 不包含菜单文字。 */
int ui_login(void)
{
    char name[NAME_LEN], password[NAME_LEN];
    int role, rc, remain;

    for (;;) {
        clear_screen();
        print_title("社区医疗系统 v1.1（开发骨架）");
        printf("1. 管理员登录\n2. 医生登录\n3. 患者登录\n0. 退出系统\n");
        role = read_choice("请输入选项[0-3]：", 0, 3);
        if (role <= 0) {
            return ROLE_NONE;
        }
        if (read_line("账号：", name, sizeof(name)) != 0) {
            if (feof(stdin)) {
                return ROLE_NONE;
            }
            continue;
        }
        read_password("密码：", password, sizeof(password));
        rc = user_login(role, name, password);
        memset(password, 0, sizeof(password));
        if (rc == 0) {
            return role;
        }
        if (rc == ERR_LOCKED && user_lock_remain(role, name, &remain) == 0) {
            printf("账号已锁定，请 %d 分钟后重试\n", remain);
        } else {
            printf("%s\n", err_str(rc));
        }
        if (feof(stdin)) {
            return ROLE_NONE;
        }
        pause_wait();
    }
}

void read_password(const char *prompt, char *buf, int buflen)
{
    struct termios old, hidden;
    int changed = 0;

    if (buf == NULL || buflen < 2) {
        return;
    }
    buf[0] = '\0';
    if (isatty(STDIN_FILENO)) {
        if (tcgetattr(STDIN_FILENO, &old) != 0) {
            return;
        }
        hidden = old;
        hidden.c_lflag &= (tcflag_t)~ECHO;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &hidden) != 0) {
            return;
        }
        changed = 1;
    }
    if (read_line(prompt, buf, buflen) != 0) {
        buf[0] = '\0';
    }
    if (changed) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        printf("\n");
    }
}
