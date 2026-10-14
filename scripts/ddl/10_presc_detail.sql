-- D 起草，待第 0 天由表 owner 对照既有 v1.0 核定。
-- 仓库缺少 v1.0 原始 schema；以下据当前 common.h / SRS 起草，不代表既有数据库。
-- 不带 JOIN 显示字段；暂不臆造跨表外键，孤儿数据由 check_data.sql 检查。
CREATE TABLE IF NOT EXISTS t_presc_detail (
    item_id INT NOT NULL AUTO_INCREMENT,
    presc_id INT NOT NULL,
    med_id VARCHAR(20) NOT NULL,
    quantity INT NOT NULL,
    dosage VARCHAR(50) NOT NULL DEFAULT '',
    unit_price DECIMAL(10,2) NOT NULL,
    subtotal DECIMAL(12,2) NOT NULL,
    PRIMARY KEY (item_id),
    UNIQUE KEY uk_presc_med (presc_id, med_id),
    KEY idx_detail_med (med_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='处方明细（D草案）';
