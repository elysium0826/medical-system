-- scripts/ddl/07_schedule.sql    排班号源表（owner: A）
CREATE TABLE IF NOT EXISTS t_schedule (
    schedule_id  INT          NOT NULL AUTO_INCREMENT,
    doctor_id    VARCHAR(10)  NOT NULL COMMENT '医生编号',
    work_date    DATE         NOT NULL COMMENT '出诊日期',
    time_slot    VARCHAR(10)  NOT NULL COMMENT '上午/下午',
    quota_total  INT          NOT NULL DEFAULT 20 COMMENT '号源总数',
    quota_used   INT          NOT NULL DEFAULT 0  COMMENT '已占用',
    status       TINYINT      NOT NULL DEFAULT 1  COMMENT '1正常 0停诊',
    create_time  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (schedule_id),
    UNIQUE KEY uk_doc_date_slot (doctor_id, work_date, time_slot),
    KEY idx_date_status (work_date, status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='医生排班与号源';
