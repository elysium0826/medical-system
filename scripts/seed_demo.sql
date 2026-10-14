-- 演示数据：10患者 / 4医生 / 30药品 / 当周排班 / 20历史预约与就诊。

-- 仅合成演示姓名、虚构地区码 000000 的身份证与虚构电话，不含真实个人隐私。

-- 所有账号密码 123456 为明文课程演示；不得用于生产，不在已有业务库覆盖或清空数据。

-- 只允许指定的空库；已有任何业务数据即拒绝。重复运行不会覆盖账户或修改已有库存。

-- 请通过 db.sh 执行；每条 INSERT 额外检查 @seed_allowed，直接 SOURCE 失败后继续也不会误写。

SET NAMES utf8mb4;
SET @seed_allowed := 0;

SET @seed_allowed := (DATABASE() = 'medical_db' AND NOT EXISTS (SELECT 1 FROM t_patient LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_doctor LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_admin LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_medicine LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_schedule LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_appointment LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_stock_log LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_visit_record LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_prescription LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_presc_detail LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_presc_template LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_presc_template_item LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_medical_record LIMIT 1));

DROP TEMPORARY TABLE IF EXISTS seed_empty_database_guard;

CREATE TEMPORARY TABLE seed_empty_database_guard (ok TINYINT NOT NULL CHECK (ok = 1));

INSERT INTO seed_empty_database_guard (ok) VALUES (IF(@seed_allowed, 1, 0));

START TRANSACTION;

INSERT INTO t_admin (admin_id, password, name)
SELECT 'admin', '123456', '合成管理员' WHERE @seed_allowed = 1;

INSERT INTO t_doctor (doctor_id, password, name, dept, title, phone)
SELECT 'D001', '123456', '合成医生01', '合成全科', '课程示例职称', '00000000001' WHERE @seed_allowed = 1;

INSERT INTO t_doctor (doctor_id, password, name, dept, title, phone)
SELECT 'D002', '123456', '合成医生02', '合成全科', '课程示例职称', '00000000002' WHERE @seed_allowed = 1;

INSERT INTO t_doctor (doctor_id, password, name, dept, title, phone)
SELECT 'D003', '123456', '合成医生03', '合成内科', '课程示例职称', '00000000003' WHERE @seed_allowed = 1;

INSERT INTO t_doctor (doctor_id, password, name, dept, title, phone)
SELECT 'D004', '123456', '合成医生04', '合成内科', '课程示例职称', '00000000004' WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260001', '123456', '合成患者01', '男', '1990-01-01', '000000199001010001', '00000000001', '虚构地址：课程演示区', '', '', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260002', '123456', '合成患者02', '女', '1990-01-01', '000000199001010002', '00000000002', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260003', '123456', '合成患者03', '男', '1990-01-01', '000000199001010003', '00000000003', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260004', '123456', '合成患者04', '女', '1990-01-01', '000000199001010004', '00000000004', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260005', '123456', '合成患者05', '男', '1990-01-01', '000000199001010005', '00000000005', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260006', '123456', '合成患者06', '女', '1990-01-01', '000000199001010006', '00000000006', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260007', '123456', '合成患者07', '男', '1990-01-01', '000000199001010007', '00000000007', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260008', '123456', '合成患者08', '女', '1990-01-01', '000000199001010008', '00000000008', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260009', '123456', '合成患者09', '男', '1990-01-01', '000000199001010009', '00000000009', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260010', '123456', '合成患者10', '女', '1990-01-01', '000000199001010010', '00000000010', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED001', '合成药品001', '课程规格', '虚构演示药厂', 1.0, 0, 1 WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED002', '合成药品002', '课程规格', '虚构演示药厂', 2.0, 20, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED002', 1, 20, 0, 20, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED003', '合成药品003', '课程规格', '虚构演示药厂', 3.0, 30, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED003', 1, 30, 0, 30, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED004', '合成药品004', '课程规格', '虚构演示药厂', 4.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED004', 1, 101, 0, 101, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED005', '合成药品005', '课程规格', '虚构演示药厂', 5.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED005', 1, 101, 0, 101, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED006', '合成药品006', '课程规格', '虚构演示药厂', 6.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED006', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED007', '合成药品007', '课程规格', '虚构演示药厂', 7.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED007', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED008', '合成药品008', '课程规格', '虚构演示药厂', 8.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED008', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED009', '合成药品009', '课程规格', '虚构演示药厂', 9.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED009', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED010', '合成药品010', '课程规格', '虚构演示药厂', 10.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED010', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED011', '合成药品011', '课程规格', '虚构演示药厂', 11.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED011', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED012', '合成药品012', '课程规格', '虚构演示药厂', 12.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED012', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED013', '合成药品013', '课程规格', '虚构演示药厂', 13.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED013', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED014', '合成药品014', '课程规格', '虚构演示药厂', 14.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED014', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED015', '合成药品015', '课程规格', '虚构演示药厂', 15.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED015', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED016', '合成药品016', '课程规格', '虚构演示药厂', 16.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED016', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED017', '合成药品017', '课程规格', '虚构演示药厂', 17.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED017', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED018', '合成药品018', '课程规格', '虚构演示药厂', 18.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED018', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED019', '合成药品019', '课程规格', '虚构演示药厂', 19.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED019', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED020', '合成药品020', '课程规格', '虚构演示药厂', 20.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED020', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED021', '合成药品021', '课程规格', '虚构演示药厂', 21.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED021', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED022', '合成药品022', '课程规格', '虚构演示药厂', 22.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED022', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED023', '合成药品023', '课程规格', '虚构演示药厂', 23.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED023', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED024', '合成药品024', '课程规格', '虚构演示药厂', 24.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED024', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED025', '合成药品025', '课程规格', '虚构演示药厂', 25.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED025', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED026', '合成药品026', '课程规格', '虚构演示药厂', 26.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED026', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED027', '合成药品027', '课程规格', '虚构演示药厂', 27.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED027', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED028', '合成药品028', '课程规格', '虚构演示药厂', 28.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED028', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED029', '合成药品029', '课程规格', '虚构演示药厂', 29.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED029', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED030', '合成药品030', '课程规格', '虚构演示药厂', 30.0, 100, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED030', 1, 100, 0, 100, 'DEMO-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1440 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1100, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 0 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1101, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 0 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1102, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 1 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1103, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 1 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1104, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 2 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1105, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 2 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1106, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 3 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1107, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 3 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1108, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 4 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1109, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 4 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1110, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 5 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1111, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 5 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1112, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 6 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1113, 'D001', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 6 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1200, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 0 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1201, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 0 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1202, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 1 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1203, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 1 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1204, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 2 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1205, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 2 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1206, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 3 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1207, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 3 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1208, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 4 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1209, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 4 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1210, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 5 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1211, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 5 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1212, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 6 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1213, 'D002', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 6 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1300, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 0 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1301, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 0 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1302, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 1 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1303, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 1 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1304, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 2 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1305, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 2 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1306, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 3 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1307, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 3 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1308, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 4 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1309, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 4 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1310, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 5 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1311, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 5 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1312, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 6 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1313, 'D003', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 6 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1400, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 0 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1401, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 0 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1402, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 1 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1403, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 1 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1404, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 2 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1405, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 2 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1406, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 3 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1407, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 3 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1408, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 4 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1409, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 4 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1410, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 5 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1411, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 5 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1412, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 6 DAY), '上午', 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used)
SELECT 1413, 'D004', DATE_ADD(DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY), INTERVAL 6 DAY), '下午', 10, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2001, 'D001', DATE_ADD(CURDATE(), INTERVAL -59 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1, 'P20260001', 'D001', DATE_ADD(CURDATE(), INTERVAL -59 DAY), '上午', '合成症状，仅课程演示', 1, 2001, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 1, 1, 'P20260001', 'D001', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1416 HOUR), DATE_SUB(NOW(), INTERVAL 1416 HOUR), DATE_SUB(NOW(), INTERVAL 1416 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2002, 'D002', DATE_ADD(CURDATE(), INTERVAL -58 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 2, 'P20260002', 'D002', DATE_ADD(CURDATE(), INTERVAL -58 DAY), '上午', '合成症状，仅课程演示', 1, 2002, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 2, 2, 'P20260002', 'D002', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1392 HOUR), DATE_SUB(NOW(), INTERVAL 1392 HOUR), DATE_SUB(NOW(), INTERVAL 1392 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2003, 'D003', DATE_ADD(CURDATE(), INTERVAL -57 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 3, 'P20260003', 'D003', DATE_ADD(CURDATE(), INTERVAL -57 DAY), '上午', '合成症状，仅课程演示', 1, 2003, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 3, 3, 'P20260003', 'D003', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1368 HOUR), DATE_SUB(NOW(), INTERVAL 1368 HOUR), DATE_SUB(NOW(), INTERVAL 1368 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2004, 'D004', DATE_ADD(CURDATE(), INTERVAL -56 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 4, 'P20260004', 'D004', DATE_ADD(CURDATE(), INTERVAL -56 DAY), '上午', '合成症状，仅课程演示', 1, 2004, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 4, 4, 'P20260004', 'D004', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1344 HOUR), DATE_SUB(NOW(), INTERVAL 1344 HOUR), DATE_SUB(NOW(), INTERVAL 1344 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2005, 'D001', DATE_ADD(CURDATE(), INTERVAL -55 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 5, 'P20260005', 'D001', DATE_ADD(CURDATE(), INTERVAL -55 DAY), '上午', '合成症状，仅课程演示', 1, 2005, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 5, 5, 'P20260005', 'D001', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1320 HOUR), DATE_SUB(NOW(), INTERVAL 1320 HOUR), DATE_SUB(NOW(), INTERVAL 1320 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2006, 'D002', DATE_ADD(CURDATE(), INTERVAL -54 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 6, 'P20260006', 'D002', DATE_ADD(CURDATE(), INTERVAL -54 DAY), '上午', '合成症状，仅课程演示', 1, 2006, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 6, 6, 'P20260006', 'D002', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1296 HOUR), DATE_SUB(NOW(), INTERVAL 1296 HOUR), DATE_SUB(NOW(), INTERVAL 1296 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2007, 'D003', DATE_ADD(CURDATE(), INTERVAL -53 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 7, 'P20260007', 'D003', DATE_ADD(CURDATE(), INTERVAL -53 DAY), '上午', '合成症状，仅课程演示', 1, 2007, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 7, 7, 'P20260007', 'D003', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1272 HOUR), DATE_SUB(NOW(), INTERVAL 1272 HOUR), DATE_SUB(NOW(), INTERVAL 1272 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2008, 'D004', DATE_ADD(CURDATE(), INTERVAL -52 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 8, 'P20260008', 'D004', DATE_ADD(CURDATE(), INTERVAL -52 DAY), '上午', '合成症状，仅课程演示', 1, 2008, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 8, 8, 'P20260008', 'D004', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1248 HOUR), DATE_SUB(NOW(), INTERVAL 1248 HOUR), DATE_SUB(NOW(), INTERVAL 1248 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2009, 'D001', DATE_ADD(CURDATE(), INTERVAL -51 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 9, 'P20260009', 'D001', DATE_ADD(CURDATE(), INTERVAL -51 DAY), '上午', '合成症状，仅课程演示', 1, 2009, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 9, 9, 'P20260009', 'D001', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1224 HOUR), DATE_SUB(NOW(), INTERVAL 1224 HOUR), DATE_SUB(NOW(), INTERVAL 1224 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2010, 'D002', DATE_ADD(CURDATE(), INTERVAL -50 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 10, 'P20260010', 'D002', DATE_ADD(CURDATE(), INTERVAL -50 DAY), '上午', '合成症状，仅课程演示', 1, 2010, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 10, 10, 'P20260010', 'D002', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1200 HOUR), DATE_SUB(NOW(), INTERVAL 1200 HOUR), DATE_SUB(NOW(), INTERVAL 1200 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2011, 'D003', DATE_ADD(CURDATE(), INTERVAL -49 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 11, 'P20260001', 'D003', DATE_ADD(CURDATE(), INTERVAL -49 DAY), '上午', '合成症状，仅课程演示', 1, 2011, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 11, 11, 'P20260001', 'D003', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1176 HOUR), DATE_SUB(NOW(), INTERVAL 1176 HOUR), DATE_SUB(NOW(), INTERVAL 1176 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2012, 'D004', DATE_ADD(CURDATE(), INTERVAL -48 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 12, 'P20260002', 'D004', DATE_ADD(CURDATE(), INTERVAL -48 DAY), '上午', '合成症状，仅课程演示', 1, 2012, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 12, 12, 'P20260002', 'D004', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1152 HOUR), DATE_SUB(NOW(), INTERVAL 1152 HOUR), DATE_SUB(NOW(), INTERVAL 1152 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2013, 'D001', DATE_ADD(CURDATE(), INTERVAL -47 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 13, 'P20260003', 'D001', DATE_ADD(CURDATE(), INTERVAL -47 DAY), '上午', '合成症状，仅课程演示', 1, 2013, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 13, 13, 'P20260003', 'D001', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1128 HOUR), DATE_SUB(NOW(), INTERVAL 1128 HOUR), DATE_SUB(NOW(), INTERVAL 1128 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2014, 'D002', DATE_ADD(CURDATE(), INTERVAL -46 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 14, 'P20260004', 'D002', DATE_ADD(CURDATE(), INTERVAL -46 DAY), '上午', '合成症状，仅课程演示', 1, 2014, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 14, 14, 'P20260004', 'D002', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1104 HOUR), DATE_SUB(NOW(), INTERVAL 1104 HOUR), DATE_SUB(NOW(), INTERVAL 1104 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2015, 'D003', DATE_ADD(CURDATE(), INTERVAL -45 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 15, 'P20260005', 'D003', DATE_ADD(CURDATE(), INTERVAL -45 DAY), '上午', '合成症状，仅课程演示', 1, 2015, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 15, 15, 'P20260005', 'D003', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1080 HOUR), DATE_SUB(NOW(), INTERVAL 1080 HOUR), DATE_SUB(NOW(), INTERVAL 1080 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2016, 'D004', DATE_ADD(CURDATE(), INTERVAL -44 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 16, 'P20260006', 'D004', DATE_ADD(CURDATE(), INTERVAL -44 DAY), '上午', '合成症状，仅课程演示', 1, 2016, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 16, 16, 'P20260006', 'D004', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1056 HOUR), DATE_SUB(NOW(), INTERVAL 1056 HOUR), DATE_SUB(NOW(), INTERVAL 1056 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2017, 'D001', DATE_ADD(CURDATE(), INTERVAL -43 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 17, 'P20260007', 'D001', DATE_ADD(CURDATE(), INTERVAL -43 DAY), '上午', '合成症状，仅课程演示', 1, 2017, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 17, 17, 'P20260007', 'D001', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1032 HOUR), DATE_SUB(NOW(), INTERVAL 1032 HOUR), DATE_SUB(NOW(), INTERVAL 1032 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2018, 'D002', DATE_ADD(CURDATE(), INTERVAL -42 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 18, 'P20260008', 'D002', DATE_ADD(CURDATE(), INTERVAL -42 DAY), '上午', '合成症状，仅课程演示', 1, 2018, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 18, 18, 'P20260008', 'D002', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 1008 HOUR), DATE_SUB(NOW(), INTERVAL 1008 HOUR), DATE_SUB(NOW(), INTERVAL 1008 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2019, 'D003', DATE_ADD(CURDATE(), INTERVAL -41 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 19, 'P20260009', 'D003', DATE_ADD(CURDATE(), INTERVAL -41 DAY), '上午', '合成症状，仅课程演示', 1, 2019, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 19, 19, 'P20260009', 'D003', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 984 HOUR), DATE_SUB(NOW(), INTERVAL 984 HOUR), DATE_SUB(NOW(), INTERVAL 984 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 2020, 'D004', DATE_ADD(CURDATE(), INTERVAL -40 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 20, 'P20260010', 'D004', DATE_ADD(CURDATE(), INTERVAL -40 DAY), '上午', '合成症状，仅课程演示', 1, 2020, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 20, 20, 'P20260010', 'D004', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 960 HOUR), DATE_SUB(NOW(), INTERVAL 960 HOUR), DATE_SUB(NOW(), INTERVAL 960 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medical_record (record_id, visit_id, patient_id, doctor_id, past_history, personal_history, family_history, create_time, update_time)
SELECT 1, 1, 'P20260001', 'D001', '合成既往史', '合成个人史', '合成家族史', DATE_SUB(NOW(), INTERVAL 1416 HOUR), DATE_SUB(NOW(), INTERVAL 1416 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_prescription (presc_id, visit_id, patient_id, doctor_id, total_price, status)
SELECT 1, 1, 'P20260001', 'D001', 9, 1 WHERE @seed_allowed = 1;

INSERT INTO t_presc_detail (presc_id, med_id, quantity, dosage, unit_price, subtotal)
SELECT 1, 'MED004', 1, '合成用法，不供临床使用', 4, 4 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED004', 3, -1, 101, 100, '1', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1416 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_presc_detail (presc_id, med_id, quantity, dosage, unit_price, subtotal)
SELECT 1, 'MED005', 1, '合成用法，不供临床使用', 5, 5 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED005', 3, -1, 101, 100, '1', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 1416 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_presc_template (tpl_id, tpl_name, doctor_id, remark)
SELECT 1, '合成模板', 'D001', '示例，不供临床使用' WHERE @seed_allowed = 1;

INSERT INTO t_presc_template_item (tpl_id, med_id, quantity, dosage)
SELECT 1, 'MED004', 1, '演示用法' WHERE @seed_allowed = 1;

INSERT INTO t_presc_template_item (tpl_id, med_id, quantity, dosage)
SELECT 1, 'MED005', 1, '演示用法' WHERE @seed_allowed = 1;

COMMIT;

DROP TEMPORARY TABLE seed_empty_database_guard;

SELECT IF(@seed_allowed, 'SEED_OK', 'SEED_REFUSED_NONEMPTY_OR_WRONG_DATABASE') AS seed_status;
