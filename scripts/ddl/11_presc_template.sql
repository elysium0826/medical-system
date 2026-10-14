-- scripts/ddl/11_presc_template.sql    处方模板主表（owner: B）
CREATE TABLE IF NOT EXISTS t_presc_template (
    tpl_id       INT          NOT NULL AUTO_INCREMENT,
    tpl_name     VARCHAR(50)  NOT NULL COMMENT '模板名，同一医生下唯一',
    doctor_id    VARCHAR(10)  NOT NULL COMMENT '所属医生',
    remark       VARCHAR(100) NOT NULL DEFAULT '',
    create_time  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (tpl_id),
    UNIQUE KEY uk_doc_name (doctor_id, tpl_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='处方模板';
