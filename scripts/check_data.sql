-- SRS 5.4 / TC-DATA-01：业务数据只读检查，每个 failure_count 必须为 0。
-- 最后一行 TOTAL_FAILURES 是机器判定口径；bash scripts/db.sh check 将非零转为退出码 1。
-- 只校验当前状态，无法从快照证明历史状态迁移合法或回滚/权限/并发正确；仍须业务测试。
-- 附录D：0待就诊/1已就诊/3爽约占号，只有2已取消放号；旧历史允许NULL号源。
-- 流水检查不要求迁移前历史必有初始流水，只检查已有流水的链及最新余额。
SET NAMES utf8mb4;
WITH checks AS (
SELECT 'appointment_patient_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_appointment a LEFT JOIN t_patient p ON p.patient_id=a.patient_id WHERE p.patient_id IS NULL
UNION ALL
SELECT 'appointment_doctor_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_appointment a LEFT JOIN t_doctor d ON d.doctor_id=a.doctor_id WHERE d.doctor_id IS NULL
UNION ALL
SELECT 'appointment_schedule_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_appointment a LEFT JOIN t_schedule s ON s.schedule_id=a.schedule_id WHERE a.schedule_id IS NOT NULL AND s.schedule_id IS NULL
UNION ALL
SELECT 'appointment_schedule_mismatch' AS check_name, COUNT(*) AS failure_count
  FROM t_appointment a JOIN t_schedule s ON s.schedule_id=a.schedule_id WHERE a.doctor_id<>s.doctor_id OR a.appoint_date<>s.work_date OR a.time_slot<>s.time_slot
UNION ALL
SELECT 'appointment_state_or_change_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_appointment WHERE status NOT IN (0,1,2,3) OR change_count NOT BETWEEN 0 AND 3 OR (change_count>0 AND change_time IS NULL) OR time_slot NOT IN ('上午','下午') OR (status=3 AND appoint_date>=CURDATE())
UNION ALL
SELECT 'appointment_active_duplicate' AS check_name, COUNT(*) AS failure_count
  FROM (SELECT patient_id,doctor_id,appoint_date FROM t_appointment WHERE status<>2 GROUP BY patient_id,doctor_id,appoint_date HAVING COUNT(*)>1) duplicates
UNION ALL
SELECT 'schedule_doctor_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_schedule s LEFT JOIN t_doctor d ON d.doctor_id=s.doctor_id WHERE d.doctor_id IS NULL
UNION ALL
SELECT 'schedule_quota_or_state_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_schedule WHERE quota_total NOT BETWEEN 0 AND 200 OR quota_used<0 OR quota_used>quota_total OR status NOT IN (0,1) OR time_slot NOT IN ('上午','下午')
UNION ALL
SELECT 'schedule_consumed_quota_mismatch' AS check_name, COUNT(*) AS failure_count
  FROM t_schedule s LEFT JOIN (SELECT schedule_id, COUNT(*) AS consumed FROM t_appointment WHERE status IN (0,1,3) AND schedule_id IS NOT NULL GROUP BY schedule_id) a ON a.schedule_id=s.schedule_id WHERE s.quota_used<>COALESCE(a.consumed,0)
UNION ALL
SELECT 'stopped_schedule_has_waiting' AS check_name, COUNT(*) AS failure_count
  FROM t_schedule s JOIN t_appointment a ON a.schedule_id=s.schedule_id WHERE s.status=0 AND a.status=0
UNION ALL
SELECT 'visit_appointment_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_visit_record v LEFT JOIN t_appointment a ON a.appoint_id=v.appoint_id WHERE a.appoint_id IS NULL
UNION ALL
SELECT 'visit_owner_or_state_mismatch' AS check_name, COUNT(*) AS failure_count
  FROM t_visit_record v JOIN t_appointment a ON a.appoint_id=v.appoint_id WHERE a.status<>1 OR v.patient_id<>a.patient_id OR v.doctor_id<>a.doctor_id
UNION ALL
SELECT 'visited_appointment_missing_visit' AS check_name, COUNT(*) AS failure_count
  FROM t_appointment a LEFT JOIN t_visit_record v ON v.appoint_id=a.appoint_id WHERE a.status=1 AND v.visit_id IS NULL
UNION ALL
SELECT 'visit_patient_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_visit_record v LEFT JOIN t_patient p ON p.patient_id=v.patient_id WHERE p.patient_id IS NULL
UNION ALL
SELECT 'visit_doctor_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_visit_record v LEFT JOIN t_doctor d ON d.doctor_id=v.doctor_id WHERE d.doctor_id IS NULL
UNION ALL
SELECT 'prescription_visit_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_prescription p LEFT JOIN t_visit_record v ON v.visit_id=p.visit_id WHERE v.visit_id IS NULL
UNION ALL
SELECT 'prescription_owner_mismatch' AS check_name, COUNT(*) AS failure_count
  FROM t_prescription p JOIN t_visit_record v ON v.visit_id=p.visit_id WHERE p.patient_id<>v.patient_id OR p.doctor_id<>v.doctor_id
UNION ALL
SELECT 'prescription_state_or_total_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_prescription WHERE status NOT IN (0,1) OR total_price<0
UNION ALL
SELECT 'prescription_detail_missing' AS check_name, COUNT(*) AS failure_count
  FROM t_prescription p LEFT JOIN t_presc_detail i ON i.presc_id=p.presc_id WHERE i.item_id IS NULL
UNION ALL
SELECT 'prescription_total_mismatch' AS check_name, COUNT(*) AS failure_count
  FROM t_prescription p JOIN (SELECT presc_id,SUM(subtotal) AS total FROM t_presc_detail GROUP BY presc_id) i ON i.presc_id=p.presc_id WHERE p.total_price<>i.total
UNION ALL
SELECT 'detail_prescription_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_presc_detail i LEFT JOIN t_prescription p ON p.presc_id=i.presc_id WHERE p.presc_id IS NULL
UNION ALL
SELECT 'detail_medicine_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_presc_detail i LEFT JOIN t_medicine m ON m.med_id=i.med_id WHERE m.med_id IS NULL
UNION ALL
SELECT 'detail_quantity_or_price_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_presc_detail WHERE quantity<=0 OR unit_price<0 OR subtotal<>ROUND(quantity*unit_price,2)
UNION ALL
SELECT 'medicine_stock_price_or_status_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_medicine WHERE stock<0 OR price<0 OR status NOT IN (0,1)
UNION ALL
SELECT 'stock_log_medicine_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_stock_log l LEFT JOIN t_medicine m ON m.med_id=l.med_id WHERE m.med_id IS NULL
UNION ALL
SELECT 'stock_log_arithmetic_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_stock_log WHERE before_stock<0 OR after_stock<0 OR after_stock<>before_stock+quantity
UNION ALL
SELECT 'stock_log_type_or_sign_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_stock_log WHERE change_type NOT IN (1,2,3,4,5,6) OR (change_type IN (1,4) AND quantity<=0) OR (change_type IN (2,3,5) AND quantity>=0) OR (change_type=6 AND (quantity<>0 OR before_stock<>after_stock))
UNION ALL
SELECT 'stock_log_prescription_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_stock_log l LEFT JOIN t_prescription p ON CAST(p.presc_id AS CHAR)=l.ref_id WHERE l.change_type IN (3,6) AND p.presc_id IS NULL
UNION ALL
SELECT 'stock_log_void_state_mismatch' AS check_name, COUNT(*) AS failure_count
  FROM t_stock_log l JOIN t_prescription p ON CAST(p.presc_id AS CHAR)=l.ref_id WHERE l.change_type=6 AND p.status<>0
UNION ALL
SELECT 'stock_log_chain_mismatch' AS check_name, COUNT(*) AS failure_count
  FROM (SELECT before_stock,LAG(after_stock) OVER (PARTITION BY med_id ORDER BY create_time,log_id) AS previous_stock FROM t_stock_log) l WHERE previous_stock IS NOT NULL AND before_stock<>previous_stock
UNION ALL
SELECT 'stock_log_latest_mismatch' AS check_name, COUNT(*) AS failure_count
  FROM t_medicine m JOIN (SELECT med_id,after_stock,ROW_NUMBER() OVER (PARTITION BY med_id ORDER BY create_time DESC,log_id DESC) AS rn FROM t_stock_log) l ON l.med_id=m.med_id AND l.rn=1 WHERE m.stock<>l.after_stock
UNION ALL
SELECT 'template_doctor_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_presc_template t LEFT JOIN t_doctor d ON d.doctor_id=t.doctor_id WHERE d.doctor_id IS NULL
UNION ALL
SELECT 'template_item_template_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_presc_template_item i LEFT JOIN t_presc_template t ON t.tpl_id=i.tpl_id WHERE t.tpl_id IS NULL
UNION ALL
SELECT 'template_item_medicine_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_presc_template_item i LEFT JOIN t_medicine m ON m.med_id=i.med_id WHERE m.med_id IS NULL
UNION ALL
SELECT 'template_item_quantity_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_presc_template_item WHERE quantity<=0
UNION ALL
SELECT 'template_item_count_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_presc_template t LEFT JOIN (SELECT tpl_id,COUNT(*) AS n FROM t_presc_template_item GROUP BY tpl_id) i ON i.tpl_id=t.tpl_id WHERE COALESCE(i.n,0) NOT BETWEEN 1 AND 20
UNION ALL
SELECT 'medical_record_visit_orphan' AS check_name, COUNT(*) AS failure_count
  FROM t_medical_record r LEFT JOIN t_visit_record v ON v.visit_id=r.visit_id WHERE v.visit_id IS NULL
UNION ALL
SELECT 'medical_record_owner_mismatch' AS check_name, COUNT(*) AS failure_count
  FROM t_medical_record r JOIN t_visit_record v ON v.visit_id=r.visit_id WHERE r.patient_id<>v.patient_id OR r.doctor_id<>v.doctor_id
UNION ALL
SELECT 't_patient_login_state_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_patient WHERE status NOT IN (0,1) OR fail_count NOT BETWEEN 0 AND 4 OR (lock_until>NOW() AND fail_count<>0)
UNION ALL
SELECT 't_doctor_login_state_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_doctor WHERE status NOT IN (0,1) OR fail_count NOT BETWEEN 0 AND 4 OR (lock_until>NOW() AND fail_count<>0)
UNION ALL
SELECT 't_admin_login_state_invalid' AS check_name, COUNT(*) AS failure_count
  FROM t_admin WHERE status NOT IN (0,1) OR fail_count NOT BETWEEN 0 AND 4 OR (lock_until>NOW() AND fail_count<>0)
)
SELECT check_name, failure_count FROM checks
UNION ALL
SELECT 'TOTAL_FAILURES', COALESCE(SUM(failure_count),0) FROM checks;
