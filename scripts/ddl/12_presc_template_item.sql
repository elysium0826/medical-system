-- scripts/ddl/12_presc_template_item.sql    处方模板明细（owner: B）
CREATE TABLE IF NOT EXISTS t_presc_template_item (
    item_id   INT          NOT NULL AUTO_INCREMENT,
    tpl_id    INT          NOT NULL,
    med_id    VARCHAR(20)  NOT NULL,
    quantity  INT          NOT NULL COMMENT '数量，必须大于 0',
    dosage    VARCHAR(50)  NOT NULL DEFAULT '' COMMENT '用法用量',
    PRIMARY KEY (item_id),
    UNIQUE KEY uk_tpl_med (tpl_id, med_id),
    KEY idx_tpl (tpl_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='处方模板明细';
