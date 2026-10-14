/* common.h  公共常量、会话、跨文件结构体与工具函数（SRS 4.2 / 7.3 / 7.5）
 * 作者：D   创建：2026-10-14 */
#ifndef MED_COMMON_H
#define MED_COMMON_H

#include <time.h>

#include "errcode.h"

/* 全局常量（SRS 4.2，禁止在 .c 文件里写魔法数字） */
#define LOW_STOCK 50           /* 库存预警阈值 */
#define LOCK_FAIL_MAX 5        /* 连续登录失败次数上限 */
#define LOCK_MINUTES 10        /* 锁定时长（分钟） */
#define SESSION_TIMEOUT_MIN 15 /* 会话超时（分钟） */
#define MEDREC_EDIT_HOURS 24   /* 病历可修改时限（小时） */
#define PAGE_SIZE 20           /* 列表每页条数 */
#define SCHEDULE_DAYS 7        /* 按周排班的天数 */
#define APPOINT_AHEAD_DAYS 7   /* 可预约/可改期的最大提前天数 */
#define SQL_BUF 4096           /* SQL 语句缓冲区长度 */
#define PID_LEN 20             /* 患者编号缓冲区长度 */
#define DID_LEN 10             /* 医生编号缓冲区长度 */
#define NAME_LEN 64            /* 姓名等短字符串长度 */
#define TEXT_LEN 1024          /* 主诉、现病史等长文本长度 */
#define SLOT_AM "上午"
#define SLOT_PM "下午"

/* 角色（SRS 7.4） */
typedef enum { ROLE_NONE = 0, ROLE_ADMIN = 1, ROLE_DOCTOR = 2, ROLE_PATIENT = 3 } role_t;

/* 会话（SRS 7.3） */
typedef struct {
    int role; /* 0 未登录, 1 admin, 2 doctor, 3 patient */
    char username[NAME_LEN];
    char user_id[DID_LEN];    /* admin_id / doctor_id / patient_id */
    char user_name[NAME_LEN]; /* 真实姓名，用于界面显示 */
    time_t login_at;
    time_t last_action; /* 每次菜单操作更新，用于会话超时 */
} session_t;

extern session_t g_session; /* 在 common.c 中定义，其他文件只 extern */

/* 以下结构体中 schedule_t、stock_log_t、presc_item_t 与 SRS 7.5 完全一致；
   其余七个沿用 v1.0 说明书 2.3 节定义并按 SRS 5.3 补充新成员，
   当前为 D 起草版本，第 0 天由各 owner 对照 v1.0 说明书核定后定稿。
   成员名与数据库字段名一致，新增成员一律追加在结构体末尾（SRS 8.4）。 */

typedef struct {
    char patient_id[PID_LEN];
    char password[NAME_LEN]; /* 明文保存，hash_password 目前为空桩 */
    char name[NAME_LEN];
    char gender[8];      /* 男 / 女 */
    char birth_date[11]; /* YYYY-MM-DD */
    char id_card[24];    /* 18 位身份证，唯一 */
    char phone[24];
    char address[128];
    int status; /* 1 正常，0 注销 */
    char create_time[20];
    char update_time[20];
    char contact_name[24];  /* SRS 5.3 新增：紧急联系人 */
    char contact_phone[24]; /* SRS 5.3 新增 */
} patient_t;

typedef struct {
    char doctor_id[DID_LEN];
    char password[NAME_LEN];
    char name[NAME_LEN];
    char dept[NAME_LEN];  /* 科室，t_doctor 的字符串字段，不建科室表 */
    char title[NAME_LEN]; /* 职称 */
    char phone[24];
    int status; /* 1 在职，0 离职 */
    char create_time[20];
    char update_time[20];
} doctor_t;

typedef struct {
    char med_id[PID_LEN];
    char med_name[NAME_LEN];
    char spec[NAME_LEN]; /* 规格 */
    char manufacturer[NAME_LEN];
    double price;
    int stock;
    int status; /* SRS 5.3 新增：1 在售，0 停用 */
    char create_time[20];
    char update_time[20];
} medicine_t;

typedef struct {
    int appoint_id;
    char patient_id[PID_LEN];
    char patient_name[NAME_LEN]; /* JOIN t_patient 带出，供列表显示 */
    char doctor_id[DID_LEN];
    char doctor_name[NAME_LEN];
    char dept[NAME_LEN];
    int schedule_id; /* SRS 5.3 新增：关联号源，0 表示历史数据无号源 */
    char appoint_date[11];
    char time_slot[11];
    char symptom[256];
    int status; /* 0 待就诊，1 已就诊，2 已取消，3 爽约 */
    char create_time[20];
    char update_time[20];
    char change_time[20]; /* SRS 5.3 新增：最近改期时刻，空串表示未改期 */
    int change_count;     /* SRS 5.3 新增：改期次数 */
} appoint_t;

typedef struct {
    int visit_id;
    int appoint_id;
    char patient_id[PID_LEN];
    char patient_name[NAME_LEN];
    char doctor_id[DID_LEN];
    char doctor_name[NAME_LEN];
    char chief_complaint[TEXT_LEN]; /* 主诉 */
    char present_illness[TEXT_LEN]; /* 现病史 */
    char physical_exam[TEXT_LEN];   /* 体征 */
    char diagnosis[TEXT_LEN];
    char visit_time[20];
    char create_time[20];
    char update_time[20];
} visit_t;

typedef struct {
    int presc_id;
    int visit_id;
    char patient_id[PID_LEN];
    char patient_name[NAME_LEN];
    char doctor_id[DID_LEN];
    char doctor_name[NAME_LEN];
    double total_price;
    int status; /* 1 有效，0 已作废 */
    char create_time[20];
    char update_time[20];
} presc_t;

typedef struct {
    int item_id;
    char med_id[PID_LEN];
    char med_name[NAME_LEN];
    int quantity;
    char dosage[50];
    double unit_price;
    double subtotal;
} presc_item_t;

typedef struct {
    int record_id;
    int visit_id;
    char patient_id[PID_LEN];
    char doctor_id[DID_LEN];
    char past_history[TEXT_LEN];     /* 既往史 */
    char personal_history[TEXT_LEN]; /* 个人史 */
    char family_history[TEXT_LEN];   /* 家族史 */
    char create_time[20];
    char update_time[20]; /* 24 小时可改窗口的判断依据 */
} medrec_t;

typedef struct {
    int schedule_id;
    char doctor_id[DID_LEN];
    char doctor_name[NAME_LEN];
    char dept[NAME_LEN];
    char work_date[11]; /* YYYY-MM-DD */
    char time_slot[11];
    int quota_total;
    int quota_used;
    int status;
} schedule_t;

typedef struct {
    long long log_id;
    char med_id[PID_LEN];
    char med_name[NAME_LEN];
    char spec[NAME_LEN];
    int change_type;
    int quantity; /* 带符号 */
    int before_stock;
    int after_stock;
    char ref_id[PID_LEN];
    char operator[NAME_LEN];
    char remark[101];
    char create_time[20];
} stock_log_t;

typedef struct {
    int tpl_id;
    char tpl_name[NAME_LEN]; /* 数据库 VARCHAR(50) */
    char doctor_id[DID_LEN];
    char remark[101];
    int item_count; /* 明细条数，查询时统计，非表字段 */
    char create_time[20];
} presc_tpl_t;

/* 会话函数（SRS 7.3，owner: D） */
void session_clear(void);
int session_is_login(void);
int session_check_timeout(void); /* 超时返回 ERR_TIMEOUT 并自动 session_clear */
void session_touch(void);
int session_require_role(int role); /* 不匹配返回 ERR_PERM */

/* 工具函数（SRS 7.3，owner: D） */
int date_valid(const char *s);                      /* 1 合法 0 非法 */
int date_diff_days(const char *d1, const char *d2); /* d1 - d2，天 */
void now_str(char *out, int outlen);                /* "YYYY-MM-DD HH:MM:SS" */
void today_str(char *out, int outlen);              /* "YYYY-MM-DD" */
int str_empty(const char *s);                       /* NULL 或 "" 返回 1 */

/* 分页与模糊查询的公共支持（FR-CMN-01，owner: D） */
/* page 小于 1 时按 1 处理 */
int page_normalize(int page);
/* (page-1)*PAGE_SIZE */
int page_offset(int page);
/* 总页数，total 小于 1 时返回 1 */
int page_count(int total);
void like_escape(const char *in, char *out, int outlen); /* db_escape 后再转义 % 和 _ */

/* 断言支持函数，实现在 common.c，供 tests/ 通过下面的宏使用 */
void assert_eq_int(const char *file, int line, const char *expr, long long actual,
                   long long expect);
void assert_eq_str(const char *file, int line, const char *expr, const char *actual,
                   const char *expect);
void assert_true(const char *file, int line, const char *expr, int cond);
void assert_rc(const char *file, int line, const char *expr, int rc);

#define ASSERT_EQ(actual, expect)                                                                  \
    assert_eq_int(__FILE__, __LINE__, #actual, (long long)(actual), (long long)(expect))
#define ASSERT_STR_EQ(actual, expect) assert_eq_str(__FILE__, __LINE__, #actual, (actual), (expect))
#define ASSERT_TRUE(cond) assert_true(__FILE__, __LINE__, #cond, (cond) ? 1 : 0)
#define ASSERT_RC(actual) assert_rc(__FILE__, __LINE__, #actual, (actual))

#endif
