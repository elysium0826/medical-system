-- SRS 5.3；D 汇总。执行前备份并由各 owner 核对既有 v1.0 schema、列类型与默认值。
-- 本脚本不把草案当作既有 schema，不建库、不删数据；缺少基线表须停止并核对。
-- 12 个新增列 + 1 个 update_time 修改；另更新预约 status 注释（类型仍为 TINYINT）。
SET @db := DATABASE();

-- 01/13 t_patient.contact_name
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_patient' AND COLUMN_NAME = 'contact_name';
SET @s := IF(@c = 0,
    'ALTER TABLE t_patient ADD COLUMN contact_name VARCHAR(20) NOT NULL DEFAULT '''' AFTER phone',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 02/13 t_patient.contact_phone
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_patient' AND COLUMN_NAME = 'contact_phone';
SET @s := IF(@c = 0,
    'ALTER TABLE t_patient ADD COLUMN contact_phone VARCHAR(20) NOT NULL DEFAULT '''' AFTER contact_name',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 03/13 t_patient.fail_count
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_patient' AND COLUMN_NAME = 'fail_count';
SET @s := IF(@c = 0,
    'ALTER TABLE t_patient ADD COLUMN fail_count TINYINT NOT NULL DEFAULT 0',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 04/13 t_patient.lock_until
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_patient' AND COLUMN_NAME = 'lock_until';
SET @s := IF(@c = 0,
    'ALTER TABLE t_patient ADD COLUMN lock_until DATETIME NULL',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 05/13 t_doctor.fail_count
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_doctor' AND COLUMN_NAME = 'fail_count';
SET @s := IF(@c = 0,
    'ALTER TABLE t_doctor ADD COLUMN fail_count TINYINT NOT NULL DEFAULT 0',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 06/13 t_doctor.lock_until
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_doctor' AND COLUMN_NAME = 'lock_until';
SET @s := IF(@c = 0,
    'ALTER TABLE t_doctor ADD COLUMN lock_until DATETIME NULL',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 07/13 t_admin.fail_count
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_admin' AND COLUMN_NAME = 'fail_count';
SET @s := IF(@c = 0,
    'ALTER TABLE t_admin ADD COLUMN fail_count TINYINT NOT NULL DEFAULT 0',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 08/13 t_admin.lock_until
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_admin' AND COLUMN_NAME = 'lock_until';
SET @s := IF(@c = 0,
    'ALTER TABLE t_admin ADD COLUMN lock_until DATETIME NULL',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 09/13 t_medicine.status
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_medicine' AND COLUMN_NAME = 'status';
SET @s := IF(@c = 0,
    'ALTER TABLE t_medicine ADD COLUMN status TINYINT NOT NULL DEFAULT 1',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 10/13 t_appointment.schedule_id
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_appointment' AND COLUMN_NAME = 'schedule_id';
SET @s := IF(@c = 0,
    'ALTER TABLE t_appointment ADD COLUMN schedule_id INT NULL',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 11/13 t_appointment.change_time
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_appointment' AND COLUMN_NAME = 'change_time';
SET @s := IF(@c = 0,
    'ALTER TABLE t_appointment ADD COLUMN change_time DATETIME NULL',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 12/13 t_appointment.change_count
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_appointment' AND COLUMN_NAME = 'change_count';
SET @s := IF(@c = 0,
    'ALTER TABLE t_appointment ADD COLUMN change_count TINYINT NOT NULL DEFAULT 0',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 13/13 t_medical_record.update_time；EXTRA 已有 ON UPDATE 则不修改。
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_medical_record' AND COLUMN_NAME = 'update_time'
   AND LOWER(EXTRA) LIKE '%on update current_timestamp%';
SET @s := IF(@c = 0,
    'ALTER TABLE t_medical_record MODIFY COLUMN update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;

-- 注释是契约的一部分；第 0 天确认既有 status 确为 TINYINT 再运行。
SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_appointment' AND COLUMN_NAME = 'status'
   AND COLUMN_COMMENT = '0待就诊 1已就诊 2已取消 3爽约';
SET @s := IF(@c = 0,
    'ALTER TABLE t_appointment MODIFY COLUMN status TINYINT NOT NULL DEFAULT 0 COMMENT ''0待就诊 1已就诊 2已取消 3爽约''',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;
