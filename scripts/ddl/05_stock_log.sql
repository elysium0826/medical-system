-- scripts/ddl/05_stock_log.sql    库存流水表（owner: C）
CREATE TABLE IF NOT EXISTS t_stock_log (
    log_id        BIGINT       NOT NULL AUTO_INCREMENT,
    med_id        VARCHAR(20)  NOT NULL COMMENT '药品编号',
    change_type   TINYINT      NOT NULL COMMENT '1入库 2出库 3处方扣减 4盘盈 5盘亏 6作废留痕',
    quantity      INT          NOT NULL COMMENT '变化量，正数增加负数减少',
    before_stock  INT          NOT NULL COMMENT '变化前库存',
    after_stock   INT          NOT NULL COMMENT '变化后库存',
    ref_id        VARCHAR(20)  NOT NULL DEFAULT '' COMMENT '关联单据号：处方号/盘点批次',
    operator      VARCHAR(20)  NOT NULL COMMENT '操作人账号',
    remark        VARCHAR(100) NOT NULL DEFAULT '',
    create_time   DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (log_id),
    KEY idx_med_time (med_id, create_time),
    KEY idx_type_time (change_type, create_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='库存变化流水';
