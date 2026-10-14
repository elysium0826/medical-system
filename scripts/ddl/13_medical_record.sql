-- D 起草，待第 0 天由表 owner 对照既有 v1.0 核定。
-- 仓库缺少 v1.0 原始 schema；以下据当前 common.h / SRS 起草，不代表既有数据库。
-- 不带 JOIN 显示字段；暂不臆造跨表外键，孤儿数据由 check_data.sql 检查。
CREATE TABLE IF NOT EXISTS t_medical_record (
    record_id INT NOT NULL AUTO_INCREMENT,
    visit_id INT NOT NULL,
    patient_id VARCHAR(20) NOT NULL,
    doctor_id VARCHAR(10) NOT NULL,
    past_history VARCHAR(1023) NOT NULL DEFAULT '',
    personal_history VARCHAR(1023) NOT NULL DEFAULT '',
    family_history VARCHAR(1023) NOT NULL DEFAULT '',
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (record_id),
    UNIQUE KEY uk_medrec_visit (visit_id),
    KEY idx_medrec_patient (patient_id),
    KEY idx_medrec_doctor (doctor_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='病历（D草案）';
