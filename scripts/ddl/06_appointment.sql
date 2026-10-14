-- D 起草，待第 0 天由表 owner 对照既有 v1.0 核定。
-- 仓库缺少 v1.0 原始 schema；以下据当前 common.h / SRS 起草，不代表既有数据库。
-- 不带 JOIN 显示字段；暂不臆造跨表外键，孤儿数据由 check_data.sql 检查。
CREATE TABLE IF NOT EXISTS t_appointment (
    appoint_id INT NOT NULL AUTO_INCREMENT,
    patient_id VARCHAR(20) NOT NULL,
    doctor_id VARCHAR(10) NOT NULL,
    appoint_date DATE NOT NULL,
    time_slot VARCHAR(10) NOT NULL,
    symptom VARCHAR(255) NOT NULL DEFAULT '',
    status TINYINT NOT NULL DEFAULT 0 COMMENT '0待就诊 1已就诊 2已取消 3爽约',
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    schedule_id INT NULL COMMENT 'NULL为无号源的v1.0历史数据',
    change_time DATETIME NULL,
    change_count TINYINT NOT NULL DEFAULT 0,
    PRIMARY KEY (appoint_id),
    KEY idx_appoint_patient_date (patient_id, appoint_date),
    KEY idx_appoint_doctor_date (doctor_id, appoint_date),
    KEY idx_appoint_schedule (schedule_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='预约挂号（D草案，重复规则由业务核定）';
