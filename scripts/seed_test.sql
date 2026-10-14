-- 独立 medical_test 空库的非破坏性前置夹具，不 DROP/DELETE/TRUNCATE；不是52条业务测试的实现。

-- 仅合成演示姓名、虚构地区码 000000 的身份证与虚构电话，不含真实个人隐私。

-- 所有账号密码 123456 为明文课程演示；不得用于生产，不在已有业务库覆盖或清空数据。

-- 只允许指定的空库；已有任何业务数据即拒绝。重复运行不会覆盖账户或修改已有库存。

-- 请通过 db.sh 执行；每条 INSERT 额外检查 @seed_allowed，直接 SOURCE 失败后继续也不会误写。

SET NAMES utf8mb4;
SET @seed_allowed := 0;

SET @seed_allowed := (DATABASE() = 'medical_test' AND NOT EXISTS (SELECT 1 FROM t_patient LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_doctor LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_admin LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_medicine LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_schedule LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_appointment LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_stock_log LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_visit_record LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_prescription LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_presc_detail LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_presc_template LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_presc_template_item LIMIT 1) AND NOT EXISTS (SELECT 1 FROM t_medical_record LIMIT 1));

DROP TEMPORARY TABLE IF EXISTS seed_empty_database_guard;

CREATE TEMPORARY TABLE seed_empty_database_guard (ok TINYINT NOT NULL CHECK (ok = 1));

INSERT INTO seed_empty_database_guard (ok) VALUES (IF(@seed_allowed, 1, 0));

START TRANSACTION;

-- TC-M09-01/02：D004 尚无排班，可给它选择未来周一；TC-M09-05 非周一/非法参数由测试传入。
-- D001今日3待接诊/1过期；P20260001仅有2天后的待诊且紧急联系人空。
-- 在售低库存仅MED002(30)/MED003(0)；MED004(20)是停用库存管理夹具，不参与低库存预警。
-- D002有明日号源，未来7天未排班的在职医生恰为D004。
-- TC-M05-04的失败开方须使用新visit_id=1102，选择MED001/MED003/MED005（第2项零库存）；
-- 原有1101处方及其流水属于作废/历史查询夹具，用例必须按新单据ID判定无残留，不能断言全库无旧处方。

-- TC-M11-04 空库：测试应在初始化后、灌种前执行；会话超时、权限、注入、CSV、ASan须业务测试驱动。

-- 固定夹具ID如下；各用例应事务回滚或独立重建测试库，不共享有状态用例结果。

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
SELECT 'P20260001', '123456', '检索命中01', '男', '1990-01-01', '000000199001010001', '00000000001', '虚构地址：课程演示区', '', '', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260002', '123456', '检索命中02', '女', '1990-01-01', '000000199001010002', '00000000002', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260003', '123456', '检索命中03', '男', '1990-01-01', '000000199001010003', '00000000003', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260004', '123456', '检索命中04', '女', '1990-01-01', '000000199001010004', '00000000004', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260005', '123456', '检索命中05', '男', '1990-01-01', '000000199001010005', '00000000005', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260006', '123456', '检索命中06', '女', '1990-01-01', '000000199001010006', '00000000006', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260007', '123456', '检索命中07', '男', '1990-01-01', '000000199001010007', '00000000007', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260008', '123456', '测试%字面量', '女', '1990-01-01', '000000199001010008', '00000000008', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260009', '123456', '张三''; DROP TABLE t_patient; --', '男', '1990-01-01', '000000199001010009', '00000000009', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260010', '123456', '测试患者10', '女', '1990-01-01', '000000199001010010', '00000000010', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260011', '123456', '测试患者11', '男', '1990-01-01', '000000199001010011', '00000000011', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260012', '123456', '测试患者12', '女', '1990-01-01', '000000199001010012', '00000000012', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260013', '123456', '测试患者13', '男', '1990-01-01', '000000199001010013', '00000000013', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260014', '123456', '测试患者14', '女', '1990-01-01', '000000199001010014', '00000000014', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260015', '123456', '测试患者15', '男', '1990-01-01', '000000199001010015', '00000000015', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260016', '123456', '测试患者16', '女', '1990-01-01', '000000199001010016', '00000000016', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260017', '123456', '测试患者17', '男', '1990-01-01', '000000199001010017', '00000000017', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260018', '123456', '测试患者18', '女', '1990-01-01', '000000199001010018', '00000000018', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260019', '123456', '测试患者19', '男', '1990-01-01', '000000199001010019', '00000000019', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260020', '123456', '测试患者20', '女', '1990-01-01', '000000199001010020', '00000000020', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260021', '123456', '测试患者21', '男', '1990-01-01', '000000199001010021', '00000000021', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260022', '123456', '测试患者22', '女', '1990-01-01', '000000199001010022', '00000000022', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260023', '123456', '测试患者23', '男', '1990-01-01', '000000199001010023', '00000000023', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260024', '123456', '测试患者24', '女', '1990-01-01', '000000199001010024', '00000000024', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260025', '123456', '测试患者25', '男', '1990-01-01', '000000199001010025', '00000000025', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260026', '123456', '测试患者26', '女', '1990-01-01', '000000199001010026', '00000000026', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260027', '123456', '测试患者27', '男', '1990-01-01', '000000199001010027', '00000000027', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260028', '123456', '测试患者28', '女', '1990-01-01', '000000199001010028', '00000000028', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260029', '123456', '测试患者29', '男', '1990-01-01', '000000199001010029', '00000000029', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260030', '123456', '测试患者30', '女', '1990-01-01', '000000199001010030', '00000000030', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260031', '123456', '测试患者31', '男', '1990-01-01', '000000199001010031', '00000000031', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260032', '123456', '测试患者32', '女', '1990-01-01', '000000199001010032', '00000000032', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260033', '123456', '测试患者33', '男', '1990-01-01', '000000199001010033', '00000000033', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260034', '123456', '测试患者34', '女', '1990-01-01', '000000199001010034', '00000000034', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260035', '123456', '测试患者35', '男', '1990-01-01', '000000199001010035', '00000000035', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260036', '123456', '测试患者36', '女', '1990-01-01', '000000199001010036', '00000000036', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260037', '123456', '测试患者37', '男', '1990-01-01', '000000199001010037', '00000000037', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260038', '123456', '测试患者38', '女', '1990-01-01', '000000199001010038', '00000000038', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260039', '123456', '测试患者39', '男', '1990-01-01', '000000199001010039', '00000000039', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260040', '123456', '测试患者40', '女', '1990-01-01', '000000199001010040', '00000000040', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260041', '123456', '测试患者41', '男', '1990-01-01', '000000199001010041', '00000000041', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260042', '123456', '测试患者42', '女', '1990-01-01', '000000199001010042', '00000000042', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260043', '123456', '测试患者43', '男', '1990-01-01', '000000199001010043', '00000000043', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260044', '123456', '测试患者44', '女', '1990-01-01', '000000199001010044', '00000000044', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260045', '123456', '测试患者45', '男', '1990-01-01', '000000199001010045', '00000000045', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260046', '123456', '测试患者46', '女', '1990-01-01', '000000199001010046', '00000000046', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260047', '123456', '测试患者47', '男', '1990-01-01', '000000199001010047', '00000000047', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260048', '123456', '测试患者48', '女', '1990-01-01', '000000199001010048', '00000000048', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 3, NULL WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260049', '123456', '测试患者49', '男', '1990-01-01', '000000199001010049', '00000000049', '虚构地址：课程演示区', '合成联系人', '00000000000', 1, 0, DATE_ADD(NOW(), INTERVAL 10 MINUTE) WHERE @seed_allowed = 1;

INSERT INTO t_patient (patient_id, password, name, gender, birth_date, id_card, phone, address, contact_name, contact_phone, status, fail_count, lock_until)
SELECT 'P20260050', '123456', '测试患者50', '女', '1990-01-01', '000000199001010050', '00000000050', '虚构地址：课程演示区', '合成联系人', '00000000000', 0, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED001', '合成药品001', '课程规格', '虚构演示药厂', 1.0, 120, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED001', 1, 120, 0, 120, 'TEST-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 72 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED002', '合成药品002', '课程规格', '虚构演示药厂', 2.0, 30, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED002', 1, 30, 0, 30, 'TEST-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 72 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED003', '合成药品003', '课程规格', '虚构演示药厂', 3.0, 0, 1 WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED004', '合成药品004', '课程规格', '虚构演示药厂', 4.0, 20, 0 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED004', 1, 20, 0, 20, 'TEST-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 72 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED005', '合成药品005', '课程规格', '虚构演示药厂', 5.0, 50, 1 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED005', 1, 50, 0, 50, 'TEST-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 72 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medicine (med_id, med_name, spec, manufacturer, price, stock, status)
SELECT 'MED006', '合成药品006', '课程规格', '虚构演示药厂', 6.0, 50, 0 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED006', 1, 51, 0, 51, 'TEST-OPEN', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 72 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1001, 'D001', DATE_ADD(CURDATE(), INTERVAL 0 DAY), '上午', 20, 2, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1001, 'P20260020', 'D001', DATE_ADD(CURDATE(), INTERVAL 0 DAY), '上午', '合成症状，仅课程演示', 0, 1001, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1002, 'P20260002', 'D001', DATE_ADD(CURDATE(), INTERVAL 0 DAY), '上午', '合成症状，仅课程演示', 0, 1001, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1002, 'D001', DATE_ADD(CURDATE(), INTERVAL 0 DAY), '下午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1003, 'P20260003', 'D001', DATE_ADD(CURDATE(), INTERVAL 0 DAY), '下午', '合成症状，仅课程演示', 0, 1002, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1003, 'D001', DATE_ADD(CURDATE(), INTERVAL -1 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1004, 'P20260004', 'D001', DATE_ADD(CURDATE(), INTERVAL -1 DAY), '上午', '合成症状，仅课程演示', 0, 1003, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1010, 'D001', DATE_ADD(CURDATE(), INTERVAL 2 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1010, 'P20260001', 'D001', DATE_ADD(CURDATE(), INTERVAL 2 DAY), '上午', '合成症状，仅课程演示', 0, 1010, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1011, 'D001', DATE_ADD(CURDATE(), INTERVAL 3 DAY), '上午', 20, 0, 1 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1012, 'D001', DATE_ADD(CURDATE(), INTERVAL 4 DAY), '上午', 1, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1012, 'P20260002', 'D001', DATE_ADD(CURDATE(), INTERVAL 4 DAY), '上午', '合成症状，仅课程演示', 0, 1012, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1013, 'D001', DATE_ADD(CURDATE(), INTERVAL 5 DAY), '上午', 20, 0, 0 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1014, 'D001', DATE_ADD(CURDATE(), INTERVAL 6 DAY), '上午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1014, 'P20260003', 'D001', DATE_ADD(CURDATE(), INTERVAL 6 DAY), '上午', '合成症状，仅课程演示', 0, 1014, 3, DATE_SUB(NOW(), INTERVAL 1 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1015, 'D001', DATE_ADD(CURDATE(), INTERVAL 1 DAY), '上午', 20, 0, 1 WHERE @seed_allowed = 1;

-- TC-M09-03：未占用的单名额号源，两名不同患者依次竞争。
INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1016, 'D001', DATE_ADD(CURDATE(), INTERVAL 1 DAY), '下午', 1, 0, 1 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1020, 'D003', DATE_ADD(CURDATE(), INTERVAL 1 DAY), '上午', 20, 2, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1021, 'P20260010', 'D003', DATE_ADD(CURDATE(), INTERVAL 1 DAY), '上午', '合成症状，仅课程演示', 0, 1020, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1022, 'P20260011', 'D003', DATE_ADD(CURDATE(), INTERVAL 1 DAY), '上午', '合成症状，仅课程演示', 0, 1020, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1021, 'D003', DATE_ADD(CURDATE(), INTERVAL 1 DAY), '下午', 20, 0, 1 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1030, 'D002', DATE_ADD(CURDATE(), INTERVAL 1 DAY), '上午', 20, 0, 1 WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1101, 'D001', DATE_ADD(CURDATE(), INTERVAL -1 DAY), '下午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1101, 'P20260001', 'D001', DATE_ADD(CURDATE(), INTERVAL -1 DAY), '下午', '合成症状，仅课程演示', 1, 1101, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 1101, 1101, 'P20260001', 'D001', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 2 HOUR), DATE_SUB(NOW(), INTERVAL 2 HOUR), DATE_SUB(NOW(), INTERVAL 2 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medical_record (record_id, visit_id, patient_id, doctor_id, past_history, personal_history, family_history, create_time, update_time)
SELECT 1101, 1101, 'P20260001', 'D001', '合成既往史', '合成个人史', '合成家族史', DATE_SUB(NOW(), INTERVAL 2 HOUR), DATE_SUB(NOW(), INTERVAL 2 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1102, 'D001', DATE_ADD(CURDATE(), INTERVAL -2 DAY), '下午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1102, 'P20260002', 'D001', DATE_ADD(CURDATE(), INTERVAL -2 DAY), '下午', '合成症状，仅课程演示', 1, 1102, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 1102, 1102, 'P20260002', 'D001', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 25 HOUR), DATE_SUB(NOW(), INTERVAL 25 HOUR), DATE_SUB(NOW(), INTERVAL 25 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medical_record (record_id, visit_id, patient_id, doctor_id, past_history, personal_history, family_history, create_time, update_time)
SELECT 1102, 1102, 'P20260002', 'D001', '合成既往史', '合成个人史', '合成家族史', DATE_SUB(NOW(), INTERVAL 25 HOUR), DATE_SUB(NOW(), INTERVAL 25 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1103, 'D002', DATE_ADD(CURDATE(), INTERVAL -3 DAY), '下午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1103, 'P20260003', 'D002', DATE_ADD(CURDATE(), INTERVAL -3 DAY), '下午', '合成症状，仅课程演示', 1, 1103, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_visit_record (visit_id, appoint_id, patient_id, doctor_id, chief_complaint, present_illness, physical_exam, diagnosis, visit_time, create_time, update_time)
SELECT 1103, 1103, 'P20260003', 'D002', '合成主诉', '合成现病史', '合成体征', '演示诊断（非医疗建议）', DATE_SUB(NOW(), INTERVAL 2 HOUR), DATE_SUB(NOW(), INTERVAL 2 HOUR), DATE_SUB(NOW(), INTERVAL 2 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_medical_record (record_id, visit_id, patient_id, doctor_id, past_history, personal_history, family_history, create_time, update_time)
SELECT 1103, 1103, 'P20260003', 'D002', '合成既往史', '合成个人史', '合成家族史', DATE_SUB(NOW(), INTERVAL 2 HOUR), DATE_SUB(NOW(), INTERVAL 2 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_prescription (presc_id, visit_id, patient_id, doctor_id, total_price, status)
SELECT 1101, 1101, 'P20260001', 'D001', 7, 1 WHERE @seed_allowed = 1;

INSERT INTO t_presc_detail (presc_id, med_id, quantity, dosage, unit_price, subtotal)
SELECT 1101, 'MED001', 1, '合成用法', 1, 1 WHERE @seed_allowed = 1;

-- 已停用MED006仍保留在历史处方中，覆盖TC-M10-09历史查询前置。
INSERT INTO t_presc_detail (presc_id, med_id, quantity, dosage, unit_price, subtotal)
SELECT 1101, 'MED006', 1, '合成用法', 6, 6 WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED006', 3, -1, 51, 50, '1101', 'admin', '合成历史处方扣减', DATE_SUB(NOW(), INTERVAL 2 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED001', 1, 1, 120, 121, 'TEST-EXTRA', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 3 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_stock_log (med_id, change_type, quantity, before_stock, after_stock, ref_id, operator, remark, create_time)
SELECT 'MED001', 3, -1, 121, 120, '1101', 'admin', '合成库存示例', DATE_SUB(NOW(), INTERVAL 2 HOUR) WHERE @seed_allowed = 1;

INSERT INTO t_presc_template (tpl_id, tpl_name, doctor_id, remark)
SELECT 1101, '合成三药模板', 'D001', 'MED006停用；应用模板应跳过1项' WHERE @seed_allowed = 1;

INSERT INTO t_presc_template_item (tpl_id, med_id, quantity, dosage)
SELECT 1101, 'MED001', 1, '合成用法' WHERE @seed_allowed = 1;

INSERT INTO t_presc_template_item (tpl_id, med_id, quantity, dosage)
SELECT 1101, 'MED002', 1, '合成用法' WHERE @seed_allowed = 1;

INSERT INTO t_presc_template_item (tpl_id, med_id, quantity, dosage)
SELECT 1101, 'MED006', 1, '合成用法' WHERE @seed_allowed = 1;

INSERT INTO t_presc_template (tpl_id, tpl_name, doctor_id, remark)
SELECT 1102, '合成三药模板', 'D002', 'MED006停用；应用模板应跳过1项' WHERE @seed_allowed = 1;

INSERT INTO t_presc_template_item (tpl_id, med_id, quantity, dosage)
SELECT 1102, 'MED001', 1, '合成用法' WHERE @seed_allowed = 1;

INSERT INTO t_presc_template_item (tpl_id, med_id, quantity, dosage)
SELECT 1102, 'MED002', 1, '合成用法' WHERE @seed_allowed = 1;

INSERT INTO t_presc_template_item (tpl_id, med_id, quantity, dosage)
SELECT 1102, 'MED006', 1, '合成用法' WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1201, 'D003', DATE_ADD(CURDATE(), INTERVAL -3 DAY), '下午', 20, 0, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1201, 'P20260004', 'D003', DATE_ADD(CURDATE(), INTERVAL -3 DAY), '下午', '合成症状，仅课程演示', 2, 1201, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_schedule (schedule_id, doctor_id, work_date, time_slot, quota_total, quota_used, status)
SELECT 1202, 'D003', DATE_ADD(CURDATE(), INTERVAL -4 DAY), '下午', 20, 1, 1 WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1202, 'P20260005', 'D003', DATE_ADD(CURDATE(), INTERVAL -4 DAY), '下午', '合成症状，仅课程演示', 3, 1202, 0, NULL WHERE @seed_allowed = 1;

INSERT INTO t_appointment (appoint_id, patient_id, doctor_id, appoint_date, time_slot, symptom, status, schedule_id, change_count, change_time)
SELECT 1203, 'P20260006', 'D003', DATE_ADD(CURDATE(), INTERVAL -5 DAY), '下午', '合成症状，仅课程演示', 3, NULL, 0, NULL WHERE @seed_allowed = 1;

COMMIT;

DROP TEMPORARY TABLE seed_empty_database_guard;

SELECT IF(@seed_allowed, 'SEED_OK', 'SEED_REFUSED_NONEMPTY_OR_WRONG_DATABASE') AS seed_status;
