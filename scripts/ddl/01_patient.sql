-- D 起草，待第 0 天由表 owner 对照既有 v1.0 核定。
-- 仓库缺少 v1.0 原始 schema；以下据当前 common.h / SRS 起草，不代表既有数据库。
-- 不带 JOIN 显示字段；暂不臆造跨表外键，孤儿数据由 check_data.sql 检查。
CREATE TABLE IF NOT EXISTS t_patient (
    patient_id VARCHAR(20) NOT NULL,
    password VARCHAR(63) NOT NULL COMMENT '课程演示明文密码，不得用于生产',
    name VARCHAR(63) NOT NULL,
    gender VARCHAR(8) NOT NULL,
    birth_date DATE NOT NULL,
    id_card VARCHAR(18) NOT NULL,
    phone VARCHAR(20) NOT NULL DEFAULT '',
    address VARCHAR(127) NOT NULL DEFAULT '',
    status TINYINT NOT NULL DEFAULT 1 COMMENT '1正常 0注销',
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    contact_name VARCHAR(20) NOT NULL DEFAULT '',
    contact_phone VARCHAR(20) NOT NULL DEFAULT '',
    fail_count TINYINT NOT NULL DEFAULT 0,
    lock_until DATETIME NULL,
    PRIMARY KEY (patient_id),
    UNIQUE KEY uk_patient_id_card (id_card),
    KEY idx_patient_name (name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='患者档案（D草案）';
