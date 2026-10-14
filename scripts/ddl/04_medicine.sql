-- D 起草，待第 0 天由表 owner 对照既有 v1.0 核定。
-- 仓库缺少 v1.0 原始 schema；以下据当前 common.h / SRS 起草，不代表既有数据库。
-- 不带 JOIN 显示字段；暂不臆造跨表外键，孤儿数据由 check_data.sql 检查。
CREATE TABLE IF NOT EXISTS t_medicine (
    med_id VARCHAR(20) NOT NULL,
    med_name VARCHAR(63) NOT NULL,
    spec VARCHAR(63) NOT NULL DEFAULT '',
    manufacturer VARCHAR(63) NOT NULL DEFAULT '',
    price DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    stock INT NOT NULL DEFAULT 0,
    status TINYINT NOT NULL DEFAULT 1 COMMENT '1在售 0停用',
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (med_id),
    KEY idx_medicine_name (med_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='药品档案（D草案）';
