# 社区医疗服务中心信息管理系统 v1.1 需求规格说明书

版本：v1.1　文档状态：待评审　编制日期：2026-03-02
适用对象：四名开发成员（A/B/C/D）及其使用的编码 agent
基线文档：《需求说明书与 Git 协同开发》v1.0（下称《v1.0 说明书》，入库后置于 `docs/需求说明书_v1.0.md`）

---

## 0. 文档说明

### 0.1 本文件的用途

本文件是 v1.1 的**实施依据**，不是介绍材料。它把 v1.0 说明书的功能范围扩展到 47 个功能点，并把扩展部分、分工边界、接口契约、数据结构变更和验收标准写成可以直接执行的条目。

阅读本文件的前提：已经读过 v1.0 说明书。本文件**不重复**描述 M01～M08 的基线行为，只规定：

1. 新增模块 M09、M10、M11 的全部需求（第 6.4～6.6 节）
2. 既有模块的增量需求（第 6.1～6.3 节，条目号在基线之后续编）
3. 全局约定：错误码、常量、返回值、事务归属、SQL 书写（第 4 章）
4. 数据结构变更：新表完整 DDL、既有表字段变更（第 5 章）
5. 跨模块接口契约（第 7 章）
6. 验收用例与追溯矩阵（第 9、10 章）
7. 协作流程、CI、里程碑（第 8、11、13 章）

### 0.2 文档效力顺序

发生冲突时按以下顺序裁定，序号小者优先：

| 序 | 文档 | 仓库位置 | 约束对象 |
|----|------|---------|---------|
| 1 | 本需求规格说明书 v1.1 | `docs/SRS_v1.1.md` | 功能范围、数据结构、接口、验收 |
| 2 | 编码规范（AI 辅助开发） | `.ai/style.md` | 代码形态、注释、命名、禁止清单 |
| 3 | v1.0 说明书 | `docs/需求说明书_v1.0.md` | 基线功能行为、原型界面 |
| 4 | 功能扩展与四人分工方案 | `docs/方案报告_v1.1.md` | 决策理由与论证过程（背景材料） |

第 4 项只解释"为什么这么定"，不作为实现依据。

### 0.3 需求条目的书写约定

每条需求形如：

```
FR-Mxx-yy  功能名称（此处为格式示例，不是需求条目；提取需求时跳过本块）
- owner / 优先级 / 涉及表
- 函数: 完整签名（含参数类型与出参）
- 输入：参数及格式约束
- 规则：处理逻辑，逐条可判定
- 输出：返回码与出参
- 事务：是否开事务、由谁开
- 验收：TC 编号
```

用词含义（与 RFC 2119 一致）：

- **必须 / MUST**：不满足则该功能点不通过验收，PR 不予合并
- **应当 / SHOULD**：偏离需在 PR 描述中说明理由，评审同意即可
- **可以 / MAY**：可选实现，不影响验收
- **禁止 / MUST NOT**：违反视为缺陷，`make lint` 或评审会拦下

功能点的计数口径：**一个功能点 = 一项用户可见的能力**，不等于一个 C 函数。个别条目包含两个函数（如 FR-M05-05 含模板保存与模板套用），仍计 1 点。计数汇总见附录 B。

### 0.4 交给编码 agent 的输入

单个任务包**必须**包含下列全部材料，缺一件就不要开工：

1. 本文件第 0.3 节（书写约定）+ 第 4 章（全局约定）+ 目标 FR 条目全文
2. 目标 FR 涉及的表的 DDL（第 5 章对应小节）
3. `include/` 下相关头文件的当前内容（`common.h`、`errcode.h`、目标模块头文件）
4. `.ai/style.md` 全文
5. 本文件第 12 章（agent 输入输出契约）
6. 目标 FR 的验收用例 TC 全文（第 9 章）

任务包模板见附录 A。**禁止**把 v1.0 说明书整本连同本文件一起丢给 agent 让它"自行决定范围"——范围由 FR 条目界定，越界实现的功能一律回退。

---

## 1. 项目概述与范围

### 1.1 项目性质

本科课程设计项目，四人在三周内完成，全程使用 AI agent 辅助编码。交付物是一个可在 Linux 下编译运行的终端程序 `medical_system`，以及配套的数据库脚本、测试用例和文档。

因此本文件的取舍标准是：**能演示、能答辩、能讲清每一行代码**，而不是能上生产。

### 1.2 运行环境

| 项 | 要求 |
|----|------|
| 操作系统 | 任意 Linux 发行版（Ubuntu 20.04+、Debian 11+、Rocky/CentOS 9 均可） |
| 编译器 | gcc，支持 C99，开启 `-Wall -Werror -g` |
| 数据库 | MySQL 8.x，客户端库 `libmysqlclient-dev`（或 `mysql-devel`） |
| 构建 | GNU Make |
| 界面 | 终端字符界面，ANSI 转义序列，不使用 ncurses |
| 部署 | 单机，不依赖任何云服务；CI 使用 GitHub Actions 的 ubuntu-latest |

仓库可附 `docker-compose.yml` 起一个本机 MySQL 8 容器，这**只是**开发期便利（省掉四人各自装库），交付形态仍是"单台 Linux + 本机 MySQL"。CI 不使用 docker-compose，用 Actions 的 `services: mysql:8`。README 中同时给出"直接装 MySQL"和"用 compose 起容器"两条路径。

v1.0 说明书中"Rocky Linux 9.8 / glibc 2.34"的具体版本要求放宽为"任意 Linux"，但 `Makefile` **必须**在 Ubuntu 与 RHEL 系两种包名下都能装依赖（CI 用 Ubuntu，成员本机可能用 RHEL 系，README 里两套命令都写）。

### 1.3 v1.1 范围

在 v1.0 的 8 个模块 35 个功能点基础上，新增 3 个模块、扩展 2 个既有模块，共 12 个新功能点，合计 47 点。

| 模块 | 名称 | 点数 | 变化 |
|------|------|------|------|
| M01 | 患者档案管理 | 6 | 基线不变 |
| M02 | 医生信息管理 | 5 | 基线不变 |
| M03 | 预约挂号管理 | 5 → 6 | +1（改期与爽约） |
| M04 | 就诊记录管理 | 3 | 基线不变 |
| M05 | 处方管理 | 4 → 5 | +1（处方模板） |
| M06 | 药品库存管理 | 6 | 基线不变 |
| M07 | 病历管理 | 3 | 基线不变 |
| M08 | 登录与用户管理 | 3 | 基线不变 |
| M09 | 排班与号源管理 | 4 | 新增 |
| M10 | 库存流水与预警 | 3 | 新增 |
| M11 | 三端待办提醒 | 3 | 新增 |

计数核对：35 + 12 = 47，增幅 (47−35)/35 = **34.3%**；模块 8 → 11，增幅 **37.5%**。两项均落在 30%～40% 区间。

### 1.4 不计入功能点数的改动

以下改动是为了让新增功能可用、或修补 v1.0 的遗漏，**不计入 47 点**，但仍必须实现：

| 编号 | 改动 | 原因 |
|------|------|------|
| CH-01 | 列表查询支持模糊匹配（`LIKE '%关键字%'`）与分页（`LIMIT/OFFSET`，每页 `PAGE_SIZE` = 20） | 47 条患者记录滚一屏没法演示 |
| CH-02 | 病历打印视图（按固定版式输出一份完整病历到终端，可重定向到文件） | 演示效果，无新表无新逻辑 |
| CH-03 | 登录失败计数落库（`fail_count` + `lock_until`），替代进程内静态变量 | v1.0 的锁定在程序退出后失效，且静态变量在单进程多角色切换时会串号 |
| CH-04 | 会话超时 15 分钟无操作自动退回登录界面 | 终端程序的基本安全习惯 |
| CH-05 | 导出支持选择字段，输出 CSV（UTF-8，含表头） | v1.0 只导出固定字段的 txt |
| CH-06 | `t_medicine` 增加 `status` 字段（1 在售 / 0 停用） | v1.0 的药品表无软删除，停用药仍会出现在开方列表 |
| CH-07 | `t_medical_record.update_time` 补 `ON UPDATE CURRENT_TIMESTAMP` | v1.0 的 DDL 只有 `DEFAULT CURRENT_TIMESTAMP`，更新时不刷新，导致"24 小时内可改"规则永远成立 |
| CH-08 | `t_patient` 增加紧急联系人 `contact_name` / `contact_phone` | 社区医疗的实际字段，患者自行维护 |

若评审老师把这些也算作新功能点，总数将升至约 55 点（+57%），超出 40% 上限。届时的削减顺序为：CH-02 病历打印视图 → CH-05 导出可选字段 → CH-01 的分页（保留模糊查询）。**必须**在开工前与老师确认计数口径，确认结果记入 Issue #1。

### 1.5 明确排除范围

v1.0 说明书 1.1.3 列出的排除项**全部保留排除**，v1.1 不做：

收费与退费、系统日志与监控告警、进程间通信 / 子进程 / 多线程、统计报表与图表、独立科室管理模块、系统参数配置界面、互联网医院与在线问诊、移动端与 Web 端、医保联网结算、短信与邮件通知、数据库集群与读写分离。

额外排除（v1.1 新增的排除项）：

- **复杂算法**：不引入排序算法实现、图算法、动态规划、机器学习、表达式解析、自研加密或哈希、自研容器。所有新增功能的实现手段限定为：条件 SQL 更新 + 影响行数判断、聚合查询（`COUNT/SUM/GROUP BY`）、日期函数（`DATEDIFF/TIMESTAMPDIFF`）、单表或两表 `JOIN`、一次算术减法、行复制。详见附录 C 的逐项对照。
- **性能指标**：v1.0 的"单表 10 万行查询 ≤500ms""常规操作 ≤200ms""10 并发终端"**不作为 v1.1 的验收项**。数据规模按演示数据量（患者 ≤50、药品 ≤100、预约 ≤200）设计，不为大数据量建索引、不做分表、不做缓存。允许添加的索引仅限第 5 章 DDL 中明确写出的那些。
- **密码加密**：沿用 v1.0，密码明文存储，`hash_password()` 保留空桩函数（原样返回输入）。不接 OpenSSL。

### 1.6 并发与一致性口径

程序是单进程单连接的终端应用，不存在真正的多线程并发。但为了正确性和答辩可讲性，以下两类操作**必须**写成"条件更新 + 判断影响行数"，不得写成"先 SELECT 判断，再 UPDATE"：

1. 号源占用与释放（`quota_used` 增减）
2. 库存增减（`stock` 变化）

理由：这样即使两个终端同时操作，也不会出现余号为负或库存为负；同时它是本项目唯一需要讲清的"并发处理"，答辩时是个加分点，且实现成本是一条 SQL。

---

## 2. 术语

| 术语 | 含义 |
|------|------|
| 号源 | 某医生某日期某时段（上午/下午）可接诊的名额，一条 `t_schedule` 记录 |
| 余号 | `quota_total - quota_used` |
| 占号 | 挂号成功使 `quota_used + 1` |
| 放号 | 取消或改期使 `quota_used - 1` |
| 接诊 | 医生把待就诊预约转为就诊记录，预约状态 0 → 1 |
| 爽约 | 患者未到场，医生或管理员把预约状态置 3，号源不释放 |
| 开方 | 医生为一次就诊写处方主表 + 明细，并扣减库存 |
| 库存流水 | 每一次库存变化的一条记录，`t_stock_log` |
| 盘点 | 输入实盘数量，与账面库存比较，差额生成盘盈或盘亏流水并更新库存 |
| 软删除 | 把 `status` 置 0，记录仍在表中，列表默认不显示 |
| 待办 | 按角色聚合的"今天该做什么"清单，全部由 SQL 聚合得出，不落库 |
| owner | 某个表或某个函数的唯一负责人，只有 owner 能改 |
| 唯一写入口 | 跨模块修改某张表时必须调用的那个函数，其他模块禁止自己写 UPDATE |

---

## 3. 角色与权限

### 3.1 角色

沿用 v1.0 的三种角色，不新增：`admin`（管理员）、`doctor`（医生）、`patient`（患者）。默认账号：管理员 `admin`、医生 `D001`，患者通过注册产生。

### 3.2 权限矩阵增量

下表只列 v1.1 新增或变更的权限项。基线权限见 v1.0 说明书 3.3 节，保持不变。

| 功能 | admin | doctor | patient | 说明 |
|------|:-----:|:------:|:-------:|------|
| 排班录入（按周生成） | ✔ | — | — | 医生不能给自己排班 |
| 排班查询 | ✔ 全部 | ✔ 本人 | ✔ 可挂号的医生 | 患者端只显示未来 7 天且 `status=1` 且有余号的记录 |
| 号源数调整 | ✔ | — | — | 只能调未来日期，已占用数不得超过新号源总数 |
| 停诊 / 复诊 | ✔ | — | — | `t_schedule.status` 置 0 / 1；停诊时段若已有预约，**必须**提示条数并拒绝停诊 |
| 预约改期 | — | — | ✔ 本人 | 只能改 `status=0` 的预约，只能改到未来 7 天内 |
| 爽约标记 | ✔ | ✔ 本人当日 | — | 只对 `status=0` 且日期早于今天的预约生效 |
| 库存流水查询 | ✔ | ✔ 与本人处方相关 | — | 医生只能看 `change_type=3` 且 `ref_id` 属于本人处方的记录 |
| 低库存预警列表 | ✔ | — | — | 与 v1.0 的"低于 LOW_STOCK 在列表标注"并存 |
| 库存盘点 | ✔ | — | — | v1.0 已有，v1.1 改为生成流水 |
| 处方模板保存 | — | ✔ 本人 | — | |
| 处方模板套用 | — | ✔ 本人 | — | 只能套用自己保存的模板 |
| 待办查看 | ✔ 管理端 | ✔ 医生端 | ✔ 患者端 | 每端只返回本角色的数据 |

### 3.3 越权处理

任何跨角色的数据访问，**必须**在函数内部按当前登录身份校验，不得依赖菜单不显示来保证安全。校验失败返回 `ERR_PERM`，UI 层统一提示"无权限执行此操作"。

当前登录身份通过 `common.c` 的全局会话结构 `g_session`（见第 7.3 节）获取，**禁止**各模块自己再存一份用户名。

---

## 4. 全局约定

### 4.1 返回码

所有业务函数返回 `int`：`0` 成功，负值为错误码。错误码集中定义在 `include/errcode.h`，数值**必须**与下表完全一致（agent 不得自行编号）：

```c
#ifndef MED_ERRCODE_H
#define MED_ERRCODE_H

#define ERR_DB_CONN    (-1)   /* 数据库连接失败 */
#define ERR_DB_EXEC    (-2)   /* SQL 执行失败 */
#define ERR_PARAM      (-3)   /* 参数非法 */
#define ERR_NOT_FOUND  (-4)   /* 记录不存在或已注销 */
#define ERR_DUPLICATE  (-5)   /* 唯一约束冲突：身份证、账号、同日同医生挂号 */
#define ERR_BAD_STATE  (-6)   /* 状态不允许该操作：非法状态迁移 */
#define ERR_PERM       (-7)   /* 越权 */
#define ERR_NO_QUOTA   (-8)   /* 无余号 */
#define ERR_STOCK_LOW  (-9)   /* 库存不足 */
#define ERR_LOCKED     (-10)  /* 账号锁定，或病历超过可修改时限 */
#define ERR_TIMEOUT    (-11)  /* 会话超时 */
#define ERR_TX         (-12)  /* 事务开启或提交失败 */
#define ERR_NOMEM      (-13)  /* 内存分配失败 */

const char *err_str(int code);   /* 返回中文提示语，未知码返回"未知错误" */

#endif
```

`ERR_DB_CONN`、`ERR_DB_EXEC`、`ERR_DUPLICATE` 是 v1.0 已有的名字，本节为它们指定了数值。

出参约定：返回非 0 时，出参内容**不作保证**，调用方不得读取。

### 4.2 常量

集中定义在 `include/common.h`，**禁止**在 .c 文件里写魔法数字：

| 宏 | 值 | 含义 |
|----|-----|------|
| `LOW_STOCK` | 50 | 库存预警阈值（v1.0 已有） |
| `LOCK_FAIL_MAX` | 5 | 连续登录失败次数上限 |
| `LOCK_MINUTES` | 10 | 锁定时长（分钟） |
| `SESSION_TIMEOUT_MIN` | 15 | 会话超时（分钟） |
| `MEDREC_EDIT_HOURS` | 24 | 病历可修改时限（小时） |
| `PAGE_SIZE` | 20 | 列表每页条数 |
| `SCHEDULE_DAYS` | 7 | 按周排班的天数 |
| `APPOINT_AHEAD_DAYS` | 7 | 可预约/可改期的最大提前天数 |
| `SQL_BUF` | 4096 | SQL 语句缓冲区长度 |
| `PID_LEN` | 20 | 患者编号缓冲区长度 |
| `DID_LEN` | 10 | 医生编号缓冲区长度 |
| `NAME_LEN` | 64 | 姓名等短字符串长度 |
| `TEXT_LEN` | 1024 | 主诉、现病史等长文本长度 |
| `SLOT_AM` | `"上午"` | 时段取值 |
| `SLOT_PM` | `"下午"` | 时段取值 |

预约状态、库存变更类型等取值用 `enum`，写在对应模块头文件里（见第 7.4 节）。

### 4.3 编号规则

| 对象 | 格式 | 示例 | 生成方式 |
|------|------|------|---------|
| 患者编号 | `P` + 4 位年 + 4 位流水 | `P20260001` | 查当年最大号 + 1，无记录则从 0001 起 |
| 医生编号 | `D` + 3 位流水 | `D001` | 查最大号 + 1 |
| 药品编号 | 沿用 v1.0 | `MED001` | 同上 |
| 预约号 / 就诊号 / 处方号 / 病历号 / 排班号 / 流水号 | 数据库自增 | — | 不自造编号 |

编号生成**必须**与插入在同一事务内，避免并发产生重号。

### 4.4 事务归属规则

这是本规格中最重要的一条约定，违反会直接导致数据不一致：

1. **只有发起业务动作的函数**可以调用 `db_tx_begin` / `db_tx_commit` / `db_tx_rollback`
2. 被调用的 owner 函数（`medicine_change_stock`、`appointment_set_status`、`schedule_take_quota`、`schedule_release_quota`、`patient_set_status`）**内部禁止**出现任何事务控制语句，它们只执行 SQL 并返回错误码
3. 事务发起函数在**任一**子步骤失败时**必须**回滚，且回滚后不得留下部分写入
4. 一个用户操作对应一个事务，**禁止**嵌套开启事务
5. 事务发起函数白名单（`make lint` 据此检查）：`patient_register`、`patient_delete`、`appointment_add`、`appointment_cancel`、`appointment_reschedule`、`appointment_mark_noshow`、`visit_take`、`prescription_add`、`prescription_void`、`medicine_check_stock`、`medicine_add`、`schedule_add_week`、`schedule_set_quota`、`schedule_set_status`、`presc_template_add`

跨模块调用的事务归属见第 7.2 节的具体标注。

### 4.5 SQL 与数据访问约定

1. 业务 SQL **写在各业务模块**里，`db.c` 只提供连接、执行、查询、转义和事务原语，不含任何业务表的 SQL
2. 所有拼接进 SQL 的用户输入字符串**必须**先经 `db_escape`
3. **禁止** `SELECT *`，字段显式列出，结果按列序号解析
4. 数量与名额的增减**必须**用条件 UPDATE + 影响行数判断（第 1.6 节）
5. **不建物理外键**。表间关系由应用层保证，孤儿数据由 `scripts/check_data.sql`（`make check-data`）检查
6. 时间字段：`create_time` 只在插入时写；`update_time` 在 DDL 中带 `ON UPDATE CURRENT_TIMESTAMP`，且更新语句显式写 `NOW()`（双保险，避免依赖 MySQL 版本行为）
7. 日期与时段：日期字符串统一 `YYYY-MM-DD`，时段只允许 `SLOT_AM` / `SLOT_PM` 两个取值

### 4.6 唯一写入口

一张表只允许一个模块写。跨模块需要改别人的表时，**必须**调用对方提供的唯一写入口函数，禁止自己写 UPDATE。

| 表 | owner | 唯一写入口 | 谁可以调 |
|----|-------|-----------|---------|
| `t_patient` | A | `patient_register`、`patient_update_self`、`patient_set_status` | C 调 `patient_set_status` 注销 |
| `t_doctor` | C | `doctor_add`、`doctor_update`、`doctor_set_status` | 仅 C |
| `t_admin` | D | `user_change_password` | 仅 D |
| `t_appointment` | A | `appointment_add`、`appointment_set_status`、`appointment_reschedule` | B 调 `appointment_set_status` 接诊/爽约；C 调它取消 |
| `t_visit_record` | B | `visit_add` | 仅 B |
| `t_prescription` / `t_presc_detail` | B | `prescription_add`、`prescription_void` | 仅 B |
| `t_medicine` | C | `medicine_add`、`medicine_update_price`、`medicine_change_stock`、`medicine_set_status` | B 开方时调 `medicine_change_stock` 扣减 |
| `t_medical_record` | B | `medical_record_add`、`medical_record_update` | 仅 B |
| `t_schedule` | A | `schedule_add_week`、`schedule_set_quota`、`schedule_set_status`、`schedule_take_quota`、`schedule_release_quota` | B 接诊不改号源；A 自己内部调用 |
| `t_stock_log` | C | `medicine_change_stock` 内部写入 | 无对外接口，B 不得直接插流水 |
| `t_presc_template` / `_item` | B | `presc_template_add`、`presc_template_delete` | 仅 B |
| 登录失败计数与锁定 | D | `user_login` 内部处理 | 无对外接口 |

只读跨表**不受限**：任何模块都可以 SELECT 任何表（权限校验另算）。

---

## 5. 数据模型

### 5.1 表清单与归属

v1.1 共 13 张表。加粗为新增。

| 表 | owner | 建表脚本 |
|----|-------|---------|
| `t_patient` | A | `scripts/ddl/01_patient.sql` |
| `t_doctor` | C | `scripts/ddl/02_doctor.sql` |
| `t_admin` | D | `scripts/ddl/03_admin.sql` |
| `t_medicine` | C | `scripts/ddl/04_medicine.sql` |
| **`t_stock_log`** | C | `scripts/ddl/05_stock_log.sql` |
| `t_appointment` | A | `scripts/ddl/06_appointment.sql` |
| **`t_schedule`** | A | `scripts/ddl/07_schedule.sql` |
| `t_visit_record` | B | `scripts/ddl/08_visit_record.sql` |
| `t_prescription` | B | `scripts/ddl/09_prescription.sql` |
| `t_presc_detail` | B | `scripts/ddl/10_presc_detail.sql` |
| **`t_presc_template`** | B | `scripts/ddl/11_presc_template.sql` |
| **`t_presc_template_item`** | B | `scripts/ddl/12_presc_template_item.sql` |
| `t_medical_record` | B | `scripts/ddl/13_medical_record.sql` |

DDL 按表分文件的原因：`init_db.sql` 用 `SOURCE` 按序号引入，四人各自改自己的文件，不会在同一个文件上冲突。

### 5.2 新增表 DDL

以下语句是最终版本，**必须**原样使用（可按 `.clang-format` 无关，SQL 不做格式化）。所有表 `ENGINE=InnoDB DEFAULT CHARSET=utf8mb4`。

```sql
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
```

```sql
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
```

```sql
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
```

```sql
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
```

### 5.3 既有表字段变更

变更脚本 `scripts/migrate_v1.1.sql`，**必须**幂等（可重复执行不报错）。

| 表 | 变更 | 类型与默认值 | 用途 |
|----|------|-------------|------|
| `t_patient` | 新增 `contact_name` | `VARCHAR(20) NOT NULL DEFAULT ''` | CH-08 紧急联系人 |
| `t_patient` | 新增 `contact_phone` | `VARCHAR(20) NOT NULL DEFAULT ''` | CH-08 |
| `t_patient` | 新增 `fail_count` | `TINYINT NOT NULL DEFAULT 0` | CH-03 登录失败计数 |
| `t_patient` | 新增 `lock_until` | `DATETIME NULL` | CH-03 锁定截止时刻 |
| `t_doctor` | 新增 `fail_count` | `TINYINT NOT NULL DEFAULT 0` | CH-03 |
| `t_doctor` | 新增 `lock_until` | `DATETIME NULL` | CH-03 |
| `t_admin` | 新增 `fail_count` | `TINYINT NOT NULL DEFAULT 0` | CH-03 |
| `t_admin` | 新增 `lock_until` | `DATETIME NULL` | CH-03 |
| `t_medicine` | 新增 `status` | `TINYINT NOT NULL DEFAULT 1` | CH-06 停用药 |
| `t_appointment` | 新增 `schedule_id` | `INT NULL` | FR-M09-04 关联号源 |
| `t_appointment` | 新增 `change_time` | `DATETIME NULL` | FR-M03-06 最近改期时刻 |
| `t_appointment` | 新增 `change_count` | `TINYINT NOT NULL DEFAULT 0` | FR-M03-06 改期次数 |
| `t_medical_record` | 修改 `update_time` | 补 `ON UPDATE CURRENT_TIMESTAMP` | CH-07 |

`t_appointment.status` 的取值范围由 `{0,1,2}` 扩展为 `{0,1,2,3}`，字段类型不变（`TINYINT`），无需 DDL，但注释**必须**更新为"0待就诊 1已就诊 2已取消 3爽约"。

幂等写法模板（每个字段一段，agent 按此生成，不要自创写法）：

```sql
SET @db := DATABASE();

SELECT COUNT(*) INTO @c FROM information_schema.COLUMNS
 WHERE TABLE_SCHEMA = @db AND TABLE_NAME = 't_patient' AND COLUMN_NAME = 'contact_name';
SET @s := IF(@c = 0,
    'ALTER TABLE t_patient ADD COLUMN contact_name VARCHAR(20) NOT NULL DEFAULT '''' AFTER phone',
    'DO 0');
PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;
```

`update_time` 的修改用 `information_schema.COLUMNS.EXTRA` 是否包含 `on update CURRENT_TIMESTAMP` 判断。

### 5.4 初始化与数据脚本

| 脚本 | 内容 | 负责人 |
|------|------|--------|
| `scripts/init_db.sql` | `CREATE DATABASE IF NOT EXISTS medical_db`，按序号 `SOURCE` 全部 DDL，预置 `admin/123456` 与 `D001/123456` | D |
| `scripts/migrate_v1.1.sql` | 5.3 节全部变更，幂等 | D 汇总，字段由各 owner 提供片段 |
| `scripts/seed_demo.sql` | 演示数据：10 名患者、4 名医生、30 种药品（其中 3 种库存低于 50）、当周排班、20 条历史预约与就诊 | D 建骨架，A/B/C 各填自己表的数据 |
| `scripts/seed_test.sql` | 灌入 `medical_test` 库，覆盖第 9 章全部 TC 的前置状态；末尾附 `DROP DATABASE medical_test` 供清理 | 各 owner 写自己模块的部分，D 汇总 |
| `scripts/check_data.sql` | 孤儿数据检查：预约指向不存在的患者/医生/号源、明细指向不存在的处方/药品、就诊记录指向不存在的预约、流水指向不存在的药品；每条查询返回 0 行为合格 | D |
| `scripts/demo.sh` | 全流程演示脚本，走完 v1.0 说明书 4.7 的六步主流程 + v1.1 三个新模块 | D |

`make db-init` = 执行 `init_db.sql`；`make db-migrate` = 执行 `migrate_v1.1.sql`；`make db-seed` = 执行 `seed_demo.sql`；`make check-data` = 执行 `check_data.sql` 并要求全部计数为 0。数据库连接参数从环境变量 `DB_HOST` / `DB_USER` / `DB_PASS` / `DB_NAME` 读取，默认 `localhost` / `root` / 空 / `medical_db`，CI 中为 `medical_test`。

---

## 6. 功能需求

条目号在 v1.0 基线之后续编：M01 基线 6 点，故增量从 `FR-M01-07` 起；M03 基线 5 点，增量从 `FR-M03-06` 起；M05 基线 4 点，增量从 `FR-M05-05` 起；M06 基线 6 点，增量从 `FR-M06-07` 起；M07 基线 3 点，增量从 `FR-M07-04` 起；M08 基线 3 点，增量从 `FR-M08-04` 起。M09/M10/M11 为新增模块，从 01 起。

优先级：**P0** = 核心链路，任何情况下不得裁剪；**P1** = 主要扩展功能，时间不足时按第 13.3 节顺序裁剪；**P2** = 可裁剪。

### 6.1 M03 预约挂号（增量）

```
FR-M03-06  预约改期与爽约标记
- owner: A    优先级: P1    涉及表: t_appointment, t_schedule
- 计数: 1 个功能点（含 appointment_reschedule 与 appointment_mark_noshow 两个函数）

输入（改期）：appoint_id, new_date(YYYY-MM-DD), new_slot(上午/下午), operator_role
规则（改期）：
  1. 预约必须存在；不存在返回 ERR_NOT_FOUND
  2. 只有 status=0 可改期；否则返回 ERR_BAD_STATE
  3. 患者角色调用时，appoint_id 必须属于 g_session 中的当前患者；否则返回 ERR_PERM。
     管理员可对任意预约改期
  4. new_date 必须在今天起 APPOINT_AHEAD_DAYS 天内（含今天）；否则返回 ERR_PARAM
  5. 目标 (doctor_id, new_date, new_slot) 的排班必须存在且 status=1；否则返回 ERR_NOT_FOUND
  6. 改期后不得与其他 status=0 的预约冲突（同患者同日同医生唯一）；冲突返回 ERR_DUPLICATE
  7. change_count 不得超过 3；超过返回 ERR_BAD_STATE，提示"改期次数已达上限，请联系前台"
  8. 事务内顺序：释放原号源 → 占用新号源 → 更新 t_appointment 的
     appoint_date / time_slot / schedule_id / change_time=NOW() / change_count+1
  9. 任一步失败全部回滚，原预约与原号源保持不变
  10. 新旧时段相同时跳过第 8 步的释放与占用，只更新 change_time 与 change_count

输入（爽约）：appoint_id, operator（管理员或该预约的接诊医生）
规则（爽约）：
  1. 预约必须存在，且 status=0，且 appoint_date 早于今天；否则返回 ERR_BAD_STATE
  2. 医生角色调用时，appoint_id 的 doctor_id 必须等于当前医生；否则返回 ERR_PERM
  3. 调 appointment_set_status(appoint_id, 3, "爽约")
  4. 号源不释放（quota_used 不减），这是与取消的区别

输出：0 成功；负错误码。改期成功时出参 out_new_appoint 返回更新后的预约结构
事务：改期与爽约各为一个事务，由本模块函数开启
验收：TC-M03-01, TC-M03-02, TC-M03-03, TC-M03-04
```

```
FR-M03-07  挂号时占用号源（对基线 appointment_add 的修改）
- owner: A    优先级: P0    涉及表: t_appointment, t_schedule
- 计数: 不单独计数，属于 FR-M09-04

对 v1.0 的 appointment_add 做如下修改，其余行为不变：
  1. 新增入参 schedule_id（由 UI 层从排班列表选定后传入）
  2. 排班必须存在、status=1、work_date 与 time_slot 与入参一致；否则返回 ERR_NOT_FOUND
  3. 事务内先调 schedule_take_quota(schedule_id)，返回 ERR_NO_QUOTA 时整个挂号失败，
     不得写入 t_appointment
  4. 插入 t_appointment 时写入 schedule_id
  5. 取消（appointment_cancel）在事务内调 schedule_release_quota(appoint->schedule_id)；
     schedule_id 为 NULL 的历史数据跳过放号，不报错
  6. 接诊与爽约不释放号源
  7. 基线的"同患者同日同医生不可重复挂号"规则保留，检查顺序在占号之前
验收：TC-M09-03, TC-M09-04, TC-M03-05
```

### 6.2 M05 处方（增量）

```
FR-M05-05  处方模板
- owner: B    优先级: P1    涉及表: t_presc_template, t_presc_template_item, t_prescription,
  t_presc_detail, t_medicine
- 计数: 1 个功能点（含 presc_template_add / presc_template_apply / presc_template_query /
  presc_template_delete，其中后两个为配套查询与删除，不单独计数）

输入（保存模板）：tpl_name, remark, 明细数组 [{med_id, quantity, dosage}], n
规则（保存模板）：
  1. 模板归属当前登录医生（g_session.doctor_id），不接受外部传入的 doctor_id
  2. 同一医生下 tpl_name 唯一，重复返回 ERR_DUPLICATE
  3. n 必须在 1～20 之间，否则返回 ERR_PARAM
  4. 每条明细的 med_id 必须存在且 t_medicine.status=1；否则返回 ERR_NOT_FOUND
  5. quantity 必须大于 0；dosage 允许空串
  6. 同一模板内 med_id 不得重复；重复返回 ERR_DUPLICATE
  7. 事务内先插主表取自增 tpl_id，再逐条插明细；任一条失败全部回滚
  8. 保存模板不检查库存，也不扣库存（与开方的区别）

输入（套用模板）：tpl_id, visit_id, diagnosis, 可覆盖的明细修改（数量调整）
规则（套用模板）：
  1. 模板必须存在且属于当前医生；否则返回 ERR_PERM
  2. visit_id 必须存在且属于当前医生；否则返回 ERR_PERM
  3. 套用只是把模板明细复制到内存中的待提交处方明细列表，
     由医生在界面上确认或调整数量后，再调 prescription_add 正式开方
  4. 库存校验与扣减仍由 prescription_add 承担，本函数不碰库存
  5. 复制时按 med_id 查当前 t_medicine 的 price 作为 unit_price，
     不使用模板保存时的历史价格
  6. 已停用（status=0）的药品在套用列表中跳过，并在返回的出参 out_skipped 中给出条数，
     UI 提示"其中 N 种药品已停用，未加入处方"

输出：0 成功；出参 out_items 为填充好的明细数组，out_n 为条数，out_skipped 为跳过条数
事务：保存模板开事务；套用模板只读，不开事务
验收：TC-M05-01, TC-M05-02, TC-M05-03
```

```
FR-M05-06  处方作废的流水留痕（对基线 prescription_void 的修改）
- owner: B    优先级: P1    涉及表: t_prescription, t_stock_log
- 计数: 不单独计数，属于 FR-M10-01

对 v1.0 的 prescription_void 做如下修改，其余行为不变：
  1. 沿用基线规则：status 置 0，库存不回滚
  2. 事务内对每条明细调 medicine_change_stock(med_id, 0, 6, presc_id, operator)，
     delta 传 0 表示库存不变，只留一条 change_type=6 的流水，
     before_stock 与 after_stock 相同，remark 写"处方作废留痕"
  3. 只有处方 status=1 时可作废；重复作废返回 ERR_BAD_STATE
  4. 只有开方医生本人或管理员可作废；否则返回 ERR_PERM
验收：TC-M10-04
```

### 6.3 基线模块的通用增量

```
FR-CMN-01  列表模糊查询与分页
- owner: 各模块自持（A: M01/M03, B: M04/M05/M07, C: M02/M06, D: 提供公共分页函数）
- 优先级: P1    计数: 不计入功能点（CH-01）

规则：
  1. 所有 query_all 类函数增加两个入参：const char *keyword（可为 NULL 或空串表示不过滤）、
     int page（从 1 开始）、int *out_total（总条数，可为 NULL）
  2. keyword 非空时，对该模块的主要文本字段做 LIKE '%关键字%' 匹配：
     M01 匹配 name / phone / patient_id；M02 匹配 name / dept / title；
     M06 匹配 med_name / manufacturer；M03/M04/M05/M07 匹配患者姓名（需 JOIN t_patient）
  3. keyword 中的 % 和 _ 必须转义（用 db_escape 后再替换），避免用户输入通配符改变语义
  4. 分页用 LIMIT PAGE_SIZE OFFSET (page-1)*PAGE_SIZE；page 小于 1 时按 1 处理
  5. out_total 用一条 SELECT COUNT(*) 单独查，与列表查询使用相同的 WHERE 条件
  6. 排序字段固定，保证翻页结果稳定：M01 按 patient_id，M02 按 doctor_id，
     M06 按 med_id，M03 按 appoint_date DESC, appoint_id DESC，其余按主键 DESC
  7. UI 层显示"第 X/Y 页，共 Z 条"，提供"下一页 / 上一页 / 退出列表"三个选项
验收：TC-CMN-01, TC-CMN-02
```

```
FR-CMN-02  登录失败计数落库与会话超时
- owner: D    优先级: P1    计数: 不计入功能点（CH-03, CH-04）

规则（失败计数）：
  1. 三种角色的表都新增 fail_count 与 lock_until（见 5.3）
  2. 登录时先判 lock_until > NOW()，是则返回 ERR_LOCKED，
     提示"账号已锁定，请 N 分钟后重试"，N = TIMESTAMPDIFF(MINUTE, NOW(), lock_until) + 1
  3. 密码错误：fail_count + 1；达到 LOCK_FAIL_MAX 时同时写
     lock_until = NOW() + INTERVAL LOCK_MINUTES MINUTE，并把 fail_count 归零
  4. 密码正确：fail_count 归零、lock_until 置 NULL
  5. 计数按"账号 + 密码是否正确"落库，不使用进程内静态变量
规则（会话超时）：
  6. g_session 记录 last_action（time_t），每次菜单操作前检查距今是否超过
     SESSION_TIMEOUT_MIN 分钟；超过则清空会话、返回登录界面，提示"会话已超时，请重新登录"
  7. 超时判断放在 main.c 的菜单循环入口一处，不在各模块重复
验收：TC-M08-01, TC-M08-02, TC-CMN-03
```

```
FR-CMN-03  导出可选字段与 CSV
- owner: C（导出功能实现）+ 各模块提供字段清单    优先级: P2    计数: 不计入功能点（CH-05）

规则：
  1. 导出前让用户勾选字段，默认全选；至少选 1 个，否则返回 ERR_PARAM
  2. 输出 UTF-8 CSV，首行为表头（中文字段名），值中含逗号或双引号时按 CSV 规则加引号转义
  3. 文件名 data/<模块>_export_YYYYMMDD_HHMMSS.csv，目录不存在则创建
  4. 密码字段永远不可导出，字段清单中不包含它
  5. 患者导出只有管理员可用；医生导出本人相关数据；患者不可导出
验收：TC-CMN-04
```

```
FR-CMN-04  病历打印视图
- owner: B    优先级: P2    计数: 不计入功能点（CH-02）

规则：
  1. 按固定版式输出一次就诊的完整信息：患者基本信息、就诊记录（主诉、现病史、体征）、
     诊断、处方明细（药名/规格/数量/用法/小计/合计）、病历三段史、医生签名行
  2. 只读，不修改任何数据
  3. 权限：本人医生、管理员、本人患者可见；其他人返回 ERR_PERM
  4. 支持重定向到文件（程序内用 fopen 写 data/record_<visit_id>.txt，
     同时输出到终端）
验收：TC-CMN-05
```

### 6.4 M09 排班与号源管理（新增，owner: A）

```
FR-M09-01  按周生成排班
- owner: A    优先级: P1    涉及表: t_schedule, t_doctor
- 函数: int schedule_add_week(const char *doctor_id, const char *monday,
                              int quota_am, int quota_pm, int *out_created);

输入：doctor_id, monday(周一日期 YYYY-MM-DD), quota_am, quota_pm
前置条件：调用者为 admin 角色
规则：
  1. doctor_id 必须存在且 t_doctor.status=1；否则返回 ERR_NOT_FOUND
  2. monday 必须是合法日期，且是周一（用 MySQL 的 DAYOFWEEK 或 C 侧 tm_wday 判断）；
     不是周一返回 ERR_PARAM，提示"请输入周一的日期"
  3. monday 不得早于今天；否则返回 ERR_PARAM
  4. quota_am 与 quota_pm 均在 0～200 之间；为 0 表示该时段不出诊，不生成记录
  5. 从 monday 起连续 SCHEDULE_DAYS(7) 天，每天生成上午、下午两条记录，
     共最多 14 条
  6. (doctor_id, work_date, time_slot) 已存在的记录跳过，不覆盖、不报错，
     也不计入 out_created
  7. 单事务：本次要插入的记录全部成功才提交；任一条失败全部回滚
输出：0 成功，out_created 返回实际新建条数（0～14）；负错误码
事务：本函数开启（在白名单内）
副作用：只写 t_schedule
验收：TC-M09-01, TC-M09-02
```

```
FR-M09-02  排班查询
- owner: A    优先级: P1    涉及表: t_schedule, t_doctor
- 函数: int schedule_query_by_doctor(const char *doctor_id, const char *date_from,
                                     const char *date_to, int only_available,
                                     schedule_t **out, int *out_n);

输入：doctor_id（NULL 表示不限医生）, date_from, date_to, only_available
规则：
  1. date_from / date_to 均可为 NULL；同时为 NULL 时默认查今天起 SCHEDULE_DAYS 天
  2. date_from 不得晚于 date_to；否则返回 ERR_PARAM
  3. only_available=1 时追加 status=1 AND quota_used < quota_total AND work_date >= CURDATE()
     （患者端挂号选号用这个模式）
  4. JOIN t_doctor 带出医生姓名与科室，供列表显示
  5. 排序：work_date ASC, time_slot ASC（上午在前，用 FIELD(time_slot,'上午','下午')）
  6. 结果由调用方用 schedule_list_free 释放；out 与 out_n 必须非 NULL
  7. 权限：patient 角色调用时 only_available 强制为 1，且返回结果不含已停诊医生
输出：0 成功（无记录时 out_n=0，out 为 NULL，仍返回 0）；负错误码
事务：只读，不开事务
验收：TC-M09-05, TC-M09-06
```

```
FR-M09-03  号源数调整与停诊
- owner: A    优先级: P1    涉及表: t_schedule, t_appointment
- 函数: int schedule_set_quota(int schedule_id, int new_total);
        int schedule_set_status(int schedule_id, int new_status, int *out_blocked);

规则（调整号源）：
  1. 仅 admin 可调用；否则返回 ERR_PERM
  2. 排班必须存在；否则返回 ERR_NOT_FOUND
  3. work_date 必须不早于今天；已过去的排班不可调整，返回 ERR_BAD_STATE
  4. new_total 在 0～200 之间；且不得小于当前 quota_used；
     小于时返回 ERR_PARAM，提示"已挂出 N 个号，号源总数不能少于 N"
  5. 条件 UPDATE：SET quota_total=? WHERE schedule_id=? AND quota_used <= ?，
     影响行数为 0 时返回 ERR_BAD_STATE（并发下被抢占的兜底）
规则（停诊 / 复诊）：
  6. new_status 只允许 0 或 1
  7. 停诊（置 0）前必须统计该时段 status=0 的预约条数；
     大于 0 时返回 ERR_BAD_STATE，out_blocked 给出条数，
     UI 提示"该时段已有 N 位患者预约，请先处理预约再停诊"
  8. 复诊（置 1）无附加条件
输出：0 成功；负错误码
事务：调整号源不开事务（单条 UPDATE）；停诊开事务（先查计数再更新）
验收：TC-M09-07, TC-M09-08, TC-M09-09
```

```
FR-M09-04  号源占用与释放
- owner: A    优先级: P0    涉及表: t_schedule
- 函数: int schedule_take_quota(int schedule_id);
        int schedule_release_quota(int schedule_id);

规则（占用）：
  1. 条件 UPDATE：UPDATE t_schedule SET quota_used = quota_used + 1
     WHERE schedule_id = ? AND status = 1 AND quota_used < quota_total
  2. 影响行数 = 1 表示占号成功，返回 0
  3. 影响行数 = 0 时区分原因：再查一次该记录，
     status != 1 返回 ERR_BAD_STATE（已停诊），
     记录不存在返回 ERR_NOT_FOUND，
     其余返回 ERR_NO_QUOTA（号已满）
  4. 本函数是 t_schedule.quota_used 增加的**唯一**途径
规则（释放）：
  5. 条件 UPDATE：SET quota_used = quota_used - 1
     WHERE schedule_id = ? AND quota_used > 0
  6. 影响行数 = 0 时（quota_used 已为 0 或记录不存在）返回 0 并写一条警告到 stderr，
     不视为失败——历史数据的 schedule_id 为 NULL 或不匹配时不能阻塞取消流程
  7. 本函数是 quota_used 减少的**唯一**途径
事务：两个函数都**不开**事务，由调用方（appointment_add / appointment_cancel /
      appointment_reschedule）的事务包裹。这是 CR-18 的强制要求。
验收：TC-M09-03, TC-M09-04, TC-M09-10, TC-M03-05
```

**A 线的 UI 要求**：管理员端"排班管理"菜单含四项——按周排班、查询排班、调整号源、停诊/复诊；患者端挂号流程改为"选科室 → 选医生 → 选日期时段（只显示有余号的）→ 填症状 → 确认"。号源已满的时段在列表中显示`[已满]`且不可选。

### 6.5 M10 库存流水与预警（新增，owner: C）

```
FR-M10-01  库存变化与流水记录
- owner: C    优先级: P0    涉及表: t_medicine, t_stock_log
- 函数: int medicine_change_stock(const char *med_id, int delta, int change_type,
                                  const char *ref_id, const char *operator);

输入：med_id, delta（正数增加、负数减少、0 表示只留痕）, change_type, ref_id, operator
change_type 取值（enum stock_change_t，定义在 include/medicine.h）：
  1 = 入库, 2 = 出库, 3 = 处方扣减, 4 = 盘盈, 5 = 盘亏, 6 = 作废留痕
规则：
  1. med_id 必须存在；否则返回 ERR_NOT_FOUND（含 status=0 的停用药，历史流水仍可写）
  2. change_type 必须在 1～6 之间；delta 为 0 时 change_type 必须是 6；
     否则返回 ERR_PARAM
  3. 先查出 before_stock（SELECT stock FROM t_medicine WHERE med_id=?）；
     查不到返回 ERR_NOT_FOUND
  4. 条件 UPDATE：UPDATE t_medicine SET stock = stock + delta
     WHERE med_id = ? AND stock + delta >= 0
  5. 影响行数 = 0 时：delta < 0 返回 ERR_STOCK_LOW，delta >= 0 返回 ERR_DB_EXEC
  6. 再查 after_stock（不复用 before_stock + delta，以数据库实际值为准），
     插入 t_stock_log 一条记录，字段全部填满，remark 由 change_type 自动生成
     （如"处方扣减"），operator 为当前登录账号
  7. 本函数是 t_medicine.stock 变化的**唯一**途径。v1.0 的 medicine_stock_in /
     medicine_stock_out 改为内部调用本函数（change_type 分别为 1 和 2），
     对外签名保持不变
  8. 出库（含处方扣减）不得使库存变为负数，由第 4 步的条件保证
事务：**不开**事务，由调用方包裹。B 的 prescription_add、C 的 medicine_check_stock
      在自己的事务里调用它。
验收：TC-M10-01, TC-M10-02, TC-M10-04, TC-M05-04
```

```
FR-M10-02  库存流水查询
- owner: C    优先级: P1    涉及表: t_stock_log, t_medicine
- 函数: int medicine_query_stock_log(const char *med_id, int change_type,
                                     const char *date_from, const char *date_to,
                                     int page, stock_log_t **out, int *out_n,
                                     int *out_total);

规则：
  1. 四个筛选条件均可为空/0，为空表示不限；组合成 WHERE 子句
  2. change_type 非 0 时必须在 1～6 之间，否则返回 ERR_PARAM
  3. date_from / date_to 作用于 create_time 的日期部分
  4. JOIN t_medicine 带出 med_name 与 spec
  5. 分页规则同 FR-CMN-01；排序 create_time DESC, log_id DESC
  6. 权限：admin 可查全部；doctor 调用时**强制**追加条件
     change_type=3 AND EXISTS(SELECT 1 FROM t_prescription p
     WHERE p.presc_id = t_stock_log.ref_id AND p.doctor_id = 当前医生)，
     外部传入的 change_type 若不为 3 则直接返回 ERR_PERM；patient 一律返回 ERR_PERM
  7. UI 显示变化量时带符号（+50 / -3），并显示 before → after
输出：0 成功；负错误码；out_total 为筛选条件下的总条数
事务：只读
验收：TC-M10-03, TC-M10-05
```

```
FR-M10-03  低库存预警列表
- owner: C    优先级: P1    涉及表: t_medicine
- 函数: int medicine_query_low_stock(medicine_t **out, int *out_n);

规则：
  1. 条件固定为 stock < LOW_STOCK AND status = 1
  2. 排序 stock ASC（最缺的排最前），同库存按 med_id
  3. 不分页，一次返回全部；预期数量不超过 50 条
  4. 权限：仅 admin；其他角色返回 ERR_PERM
  5. UI 输出时对 stock = 0 的行标注`[缺货]`，其余标注`[偏低]`
  6. 与 v1.0 的 medicine_query_all 中"低于 LOW_STOCK 标注"功能并存，
     不互相替代
输出：0 成功（无记录时 out_n=0）；负错误码
事务：只读
验收：TC-M10-06
```

```
FR-M10-04  盘点生成流水（对基线 medicine_check_stock 的修改）
- owner: C    优先级: P1    涉及表: t_medicine, t_stock_log
- 计数: 不单独计数，属于 FR-M10-01 的应用

对 v1.0 的盘点功能做如下修改：
  1. 输入实盘数量 actual，账面数量 book 从 t_medicine 读出
  2. diff = actual - book；diff = 0 时提示"账实相符"，不写流水、不开事务、直接返回 0
  3. diff > 0 时调 medicine_change_stock(med_id, diff, 4, batch, operator)
     diff < 0 时调 medicine_change_stock(med_id, diff, 5, batch, operator)
     （delta 传带符号的 diff，不需要取绝对值）
  4. batch 为盘点批次号，格式 YYYYMMDDHHMMSS，一次盘点操作中所有药品共用同一个批次号，
     由 UI 层生成后传入
  5. 多药品连续盘点时，每个药品一个独立事务（在盘点主函数中逐个调用，
     单个失败不影响已完成的，失败项在结果汇总中列出）
  6. 盘点后 stock 必须等于流水的 after_stock，且等于 actual
验收：TC-M10-07, TC-M10-08
```

**C 线的 UI 要求**：管理员端"库存管理"菜单在 v1.0 基础上增加两项——库存流水（可按药品、类型、日期筛选）、低库存预警（进入即列出，顶部显示"共 N 种药品库存低于 50"）。药品列表增加`状态`列（在售/停用），并提供停用与恢复操作。

### 6.6 M11 三端待办提醒（新增，owner: D 建框架，A/B/C 填 SQL）

待办不落库，每次进入菜单时实时查询。三个函数签名统一，返回一个字符串数组，UI 层逐行打印。

```
FR-M11-01  医生端待办
- owner: B（SQL 与业务判定）+ D（框架与 UI 挂载）    优先级: P1
- 函数: int todo_doctor(char lines[][TODO_LINE_LEN], int *out_n);

规则：以当前登录医生为条件，按下列顺序最多返回 10 条，每条形如"[类别] 内容"：
  1. 今日待接诊：今日 status=0 的预约条数与最近一位患者姓名
     → "[接诊] 今日还有 N 位患者待接诊，下一位：张三（09:30 上午）"
  2. 昨日爽约未处理：appoint_date < CURDATE() 且 status=0 的条数
     → "[爽约] 有 N 条过期未到诊的预约待标记爽约"
  3. 24 小时内可修改的病历：本人书写且 update_time 在 24 小时内的条数
     → "[病历] 有 N 份病历仍在可修改期内"
  4. 本人模板数量：t_presc_template 中本人的条数
     → "[模板] 已保存 N 个处方模板"
  5. 无任何待办时 out_n = 0，UI 显示"暂无待办事项"
实现手段：4 条 COUNT 查询 + 1 条取最近患者的查询，允许用一条 UNION ALL 合并计数
事务：只读
验收：TC-M11-01, TC-M11-04
```

```
FR-M11-02  患者端待办
- owner: A（SQL 与业务判定）+ D（框架与 UI 挂载）    优先级: P1
- 函数: int todo_patient(char lines[][TODO_LINE_LEN], int *out_n);

规则：以当前登录患者为条件，按下列顺序最多返回 10 条：
  1. 未来预约：status=0 且 appoint_date >= CURDATE()，按日期升序取前 3 条，
     每条一行 → "[预约] 3月15日 上午 心内科 李医生（还有 2 天）"
     天数用 DATEDIFF(appoint_date, CURDATE())，当天显示"今天"
  2. 可改期提示：上述预约中 change_count < 3 的，追加"（可改期 N 次）"
  3. 未取报告的就诊：最近一次 status=1 的预约对应的就诊记录，
     提示"[就诊] 3月10日 的就诊记录与处方已生成，可查看"
  4. 病历完善提示：t_patient 中 contact_phone 为空时
     → "[资料] 尚未填写紧急联系人，建议在个人信息中补充"
  5. 无待办时 out_n = 0
事务：只读
验收：TC-M11-02, TC-M11-04
```

```
FR-M11-03  管理员端待办
- owner: C（SQL 与业务判定）+ D（框架与 UI 挂载）    优先级: P1
- 函数: int todo_admin(char lines[][TODO_LINE_LEN], int *out_n);

规则：按下列顺序最多返回 10 条：
  1. 低库存：stock < LOW_STOCK AND status=1 的药品条数，
     前 3 种药品名 → "[库存] 5 种药品库存低于 50：阿莫西林(12)、布洛芬(0)、..."
     stock=0 的药名后加"(缺货)"
  2. 今日预约总量与已接诊量："[预约] 今日 23 条预约，已接诊 8 条"
  3. 未来 7 天未排班的在职医生：t_doctor.status=1 且在未来 7 天内无 t_schedule 记录
     → "[排班] 2 位医生未来一周未排班：王医生、赵医生"
     实现：LEFT JOIN t_schedule ... WHERE s.schedule_id IS NULL，
     或对每位医生做一次 NOT EXISTS 计数（医生数量少，循环查询可接受）
  4. 爽约待处理总量："[爽约] 全院有 N 条过期预约待标记"
  5. 注销患者数（本月）："[档案] 本月新增注销患者 N 人"
  6. 无待办时 out_n = 0
事务：只读
验收：TC-M11-03, TC-M11-04
```

**D 的框架责任**：在 `include/todo.h` 定义 `TODO_LINE_LEN`（128）、`TODO_MAX_LINES`（10），提供 `int todo_print(int role)` 统一入口——按 role 分派到上述三个函数之一，打印标题"待办事项"、逐行输出、无待办时输出"暂无待办事项"，末尾 `pause_wait()`。三个角色主菜单的**第一项**固定为"今日待办"。

**实现约束**：待办的每一条**必须**能用不超过两表的 JOIN 加聚合函数写出来。禁止在 C 侧做循环内的多次查询后再在内存里排序或分组（第 3 条的未排班医生查询除外，且医生总数不超过 20）。若某条待办写不出简单 SQL，删掉这条待办，不要引入复杂逻辑。

### 6.7 功能点计数汇总

见附录 B。

---

## 7. 接口契约

### 7.1 数据库访问层（`src/db.c` → `lib/libdb.so`，owner: D）

```c
/* include/db.h */
#ifndef MED_DB_H
#define MED_DB_H

#include <stddef.h>

typedef struct {
    int     rows;      /* 结果行数，0 表示空结果集 */
    int     cols;      /* 列数 */
    char  ***data;     /* data[row][col]，数据库 NULL 存为 C 的 NULL 指针 */
} db_result_t;

/* 连接与关闭。参数为 NULL 时取环境变量 DB_HOST/DB_USER/DB_PASS/DB_NAME，
   默认 localhost / root / 空密码 / medical_db。失败返回 ERR_DB_CONN。 */
int  db_connect(const char *host, const char *user, const char *pass,
                const char *dbname, unsigned int port);
void db_close(void);

/* 执行不返回结果集的语句（INSERT/UPDATE/DELETE）。
   affected 可为 NULL。SQL 失败返回 ERR_DB_EXEC，连接断开返回 ERR_DB_CONN。 */
int  db_exec(const char *sql, unsigned long long *affected);

/* 执行查询。成功返回 0，*out 指向结果集（空结果集时 rows=0，data 非 NULL）。
   失败时 *out 置 NULL 并返回错误码。结果集必须用 db_result_free 释放。 */
int  db_query(const char *sql, db_result_t **out);
void db_result_free(db_result_t *rs);

/* 按行列取值的便捷访问器：越界或该位置为数据库 NULL 时返回空串 ""，
   调用方因此不需要判 NULL。返回的指针在 db_result_free 后失效。 */
const char *db_get(const db_result_t *rs, int row, int col);

/* 事务原语。嵌套调用 db_tx_begin 返回 ERR_TX。
   只有第 4.4 节白名单中的函数可以调用这三个接口。 */
int  db_tx_begin(void);
int  db_tx_commit(void);
int  db_tx_rollback(void);

/* SQL 字符串转义，内部是 mysql_real_escape_string。
   outlen 必须大于 strlen(in)*2+1。连接未建立时退化为手工转义
   （只处理 ' " \ NUL \n \r 和 Ctrl-Z），并在 stderr 写一条警告。 */
void db_escape(const char *in, char *out, size_t outlen);

#endif
```

`db.c` 中**禁止**出现任何业务表名。`db_get` 是第 9 个接口，属于查询访问器组，与"6 个封装"的说法不冲突（连接、执行、查询、释放、事务、转义六类能力）。

### 7.2 跨模块唯一写入口

以下函数是跨模块调用的契约。签名与语义**必须**与此处一致，改动需 owner 与 D 双方同意并通知全员。

```c
/* include/appointment.h    owner: A */
/* 预约状态迁移的唯一入口。合法迁移见附录 B。
   内部禁止事务控制。非法迁移返回 ERR_BAD_STATE。
   reason 可为 NULL，写入日志字段（本项目仅用于 UI 回显，不落库）。 */
int appointment_set_status(int appoint_id, int new_status, const char *reason);

/* include/schedule.h    owner: A */
int schedule_take_quota(int schedule_id);      /* 见 FR-M09-04，内部禁止事务控制 */
int schedule_release_quota(int schedule_id);   /* 同上 */

/* include/medicine.h    owner: C */
/* 库存变化的唯一入口，同时写 t_stock_log。内部禁止事务控制。
   delta 为带符号变化量，0 表示只留痕（此时 change_type 必须为 6）。
   库存将变负时返回 ERR_STOCK_LOW。 */
int medicine_change_stock(const char *med_id, int delta, int change_type,
                          const char *ref_id, const char *operator);

/* 药品状态：1 在售，0 停用。停用药不出现在开方选药列表。 */
int medicine_set_status(const char *med_id, int new_status);

/* include/patient.h    owner: A */
/* 患者状态：1 正常，0 注销。注销后不可登录、不可挂号，历史记录保留。 */
int patient_set_status(const char *patient_id, int new_status);
```

**调用方必须遵守**（评审重点检查项）：

| 调用方 | 调用 | 时机 | 事务 |
|--------|------|------|------|
| B `visit_take`（接诊） | `appointment_set_status(id, 1, "接诊")` | 写完 t_visit_record 之后 | B 的事务内 |
| B `visit_take` | 不调用号源函数 | 接诊不释放号源 | — |
| B `appointment_mark_noshow` 的 UI 入口 | 调 A 的 `appointment_mark_noshow` | — | A 的事务内 |
| B `prescription_add` | `medicine_change_stock(mid, -qty, 3, presc_id, op)` | 每条明细插入之后 | B 的事务内 |
| B `prescription_void` | `medicine_change_stock(mid, 0, 6, presc_id, op)` | 状态置 0 之后 | B 的事务内 |
| C 患者注销 UI | `patient_set_status(pid, 0)` | — | C 的事务内 |
| A `appointment_add` | `schedule_take_quota(sid)` | 插入 t_appointment 之前 | A 的事务内 |
| A `appointment_cancel` | `schedule_release_quota(sid)` | 状态置 2 之后 | A 的事务内 |
| A `appointment_reschedule` | `release` 后 `take` | 更新字段之前 | A 的事务内 |
| C `medicine_check_stock` | `medicine_change_stock(mid, diff, 4或5, batch, op)` | — | C 的事务内 |

### 7.3 公共层（`src/common.c` → `lib/libcommon.a`，owner: D）

```c
/* include/common.h 中的会话结构 */
typedef struct {
    int   role;                 /* 0 未登录, 1 admin, 2 doctor, 3 patient */
    char  username[NAME_LEN];
    char  user_id[DID_LEN];     /* admin_id / doctor_id / patient_id */
    char  user_name[NAME_LEN];  /* 真实姓名，用于界面显示 */
    time_t login_at;
    time_t last_action;         /* 每次菜单操作更新，用于会话超时 */
} session_t;

extern session_t g_session;     /* 在 common.c 中定义，其他文件只 extern */
```

会话相关函数：`void session_clear(void)`、`int session_is_login(void)`、`int session_check_timeout(void)`（超时返回 `ERR_TIMEOUT` 并自动 `session_clear`）、`void session_touch(void)`。

角色常量：`ROLE_NONE 0`、`ROLE_ADMIN 1`、`ROLE_DOCTOR 2`、`ROLE_PATIENT 3`。权限检查宏：`int session_require_role(int role)`，不匹配返回 `ERR_PERM`。

UI 与工具函数（owner: D，签名固定）：

```c
void clear_screen(void);
void print_title(const char *title);          /* 40 个 = 的上下边框，含当前用户与角色 */
void print_table_header(const char *fmt, ...); /* 变参，输出表头行与分隔线 */
void print_table_row(const char *fmt, ...);
void print_table_end(void);
void pause_wait(void);                         /* "按回车键继续..." */
int  read_line(const char *prompt, char *buf, int buflen);   /* 去换行，返回 0 成功 */
int  read_int(const char *prompt, int *out);                 /* 非法输入重试，最多 3 次 */
int  read_decimal(const char *prompt, double *out);
int  read_date(const char *prompt, char *out);               /* 校验 YYYY-MM-DD 合法性 */
int  read_choice(const char *prompt, int min, int max);      /* 返回选项值，-1 表示放弃 */
void read_password(const char *prompt, char *buf, int buflen); /* termios 关回显 */

int  date_valid(const char *s);                  /* 1 合法 0 非法 */
int  date_diff_days(const char *d1, const char *d2);  /* d1 - d2，天 */
void now_str(char *out, int outlen);             /* "YYYY-MM-DD HH:MM:SS" */
void today_str(char *out, int outlen);           /* "YYYY-MM-DD" */
const char *err_str(int code);
int  str_empty(const char *s);                   /* NULL 或 "" 返回 1 */

/* 断言宏，供 tests/ 使用；实现为失败时打印文件行号与期望/实际值并 abort */
#define ASSERT_EQ(actual, expect)      ...
#define ASSERT_STR_EQ(actual, expect)  ...
#define ASSERT_TRUE(cond)              ...
#define ASSERT_RC(actual)              ...   /* 断言返回码为 0，否则打印 err_str */
```

### 7.4 枚举定义位置

| 枚举 | 头文件 | 取值 |
|------|--------|------|
| `appt_status_t` | `include/appointment.h` | `APPT_WAITING 0`、`APPT_VISITED 1`、`APPT_CANCELED 2`、`APPT_NOSHOW 3` |
| `stock_change_t` | `include/medicine.h` | `STOCK_IN 1`、`STOCK_OUT 2`、`STOCK_PRESC 3`、`STOCK_GAIN 4`、`STOCK_LOSS 5`、`STOCK_VOID 6` |
| `role_t` | `include/common.h` | `ROLE_NONE 0`、`ROLE_ADMIN 1`、`ROLE_DOCTOR 2`、`ROLE_PATIENT 3` |
| `med_status_t` | `include/medicine.h` | `MED_ON_SALE 1`、`MED_OFF_SALE 0` |

### 7.5 主要结构体

```c
typedef struct {
    int  schedule_id;
    char doctor_id[DID_LEN];
    char doctor_name[NAME_LEN];
    char dept[NAME_LEN];
    char work_date[11];        /* YYYY-MM-DD */
    char time_slot[11];
    int  quota_total;
    int  quota_used;
    int  status;
} schedule_t;

typedef struct {
    long long log_id;
    char med_id[PID_LEN];
    char med_name[NAME_LEN];
    char spec[NAME_LEN];
    int  change_type;
    int  quantity;             /* 带符号 */
    int  before_stock;
    int  after_stock;
    char ref_id[PID_LEN];
    char operator[NAME_LEN];
    char remark[101];
    char create_time[20];
} stock_log_t;

typedef struct {
    int  item_id;
    char med_id[PID_LEN];
    char med_name[NAME_LEN];
    int  quantity;
    char dosage[50];
    double unit_price;
    double subtotal;
} presc_item_t;
```

其余结构体（`patient_t`、`doctor_t`、`appoint_t`、`visit_t`、`presc_t`、`medrec_t`、`medicine_t`）沿用 v1.0 说明书 2.3 节的定义，按 5.3 的字段变更补充新成员，成员名与数据库字段名一致。

列表类函数的出参统一为"数组指针 + 条数"，并配一个 `xxx_list_free` 释放函数，命名规则：`schedule_list_free`、`stock_log_list_free`、`presc_item_list_free` 等。

---

## 8. 目录结构、构建与文件归属

### 8.1 目录结构

```
medical-system/
├── Makefile                    D
├── docker-compose.yml          D（可选：本机起一个 MySQL 8 容器，只是开发便利，交付形态仍是单机 + 本机 MySQL）
├── README.md                   D（W4 全员补充）
├── CHANGELOG.md                C
├── .gitignore                  D
├── .clang-format               D
├── CODEOWNERS                  D
├── .github/
│   ├── workflows/ci.yml        D
│   ├── PULL_REQUEST_TEMPLATE.md  D
│   └── ISSUE_TEMPLATE/         D
├── .ai/
│   ├── style.md                D（内容 = 编码规范）
│   ├── schema.md               D（内容 = 本文件第 5 章）
│   ├── conventions.md          D（内容 = 本文件第 4、7 章）
│   ├── prompt-patient.md       A
│   ├── prompt-doctor.md        B
│   ├── prompt-admin.md         C
│   └── prompt-core.md          D
├── docs/
│   ├── 需求说明书_v1.0.md       D（拷入仓库）
│   ├── SRS_v1.1.md             D（本文件）
│   ├── 方案报告_v1.1.md         D
│   └── design/                 每人一份设计说明，W4 提交
│       ├── A-患者线.md
│       ├── B-医生线.md
│       ├── C-管理员线.md
│       └── D-统合线.md
├── include/
│   ├── common.h                D
│   ├── errcode.h               D
│   ├── db.h                    D
│   ├── ui.h                    D
│   ├── todo.h                  D
│   ├── patient.h               A
│   ├── appointment.h           A
│   ├── schedule.h              A
│   ├── visit.h                 B
│   ├── prescription.h          B
│   ├── medical_record.h        B
│   ├── doctor.h                C
│   ├── medicine.h              C
│   └── user.h                  D
├── src/
│   ├── main.c                  D
│   ├── common.c                D
│   ├── db.c                    D
│   ├── user.c                  D
│   ├── todo.c                  D（框架）+ A/B/C（各自函数体）
│   ├── patient.c               A
│   ├── appointment.c           A
│   ├── schedule.c              A
│   ├── visit.c                 B
│   ├── prescription.c          B
│   ├── medical_record.c        B
│   ├── doctor.c                C
│   ├── medicine.c              C
│   ├── ui_admin.c              C
│   ├── ui_doctor.c             B
│   ├── ui_patient.c            A
│   └── export.c                C
├── tests/
│   ├── harness.c               D
│   ├── run_all.sh              D
│   ├── test_patient.c          A
│   ├── test_appointment.c      A
│   ├── test_schedule.c         A
│   ├── test_visit.c            B
│   ├── test_prescription.c     B
│   ├── test_medical_record.c   B
│   ├── test_doctor.c           C
│   ├── test_medicine.c         C
│   └── test_user.c             D
├── scripts/
│   ├── init_db.sql             D
│   ├── migrate_v1.1.sql        D
│   ├── seed_demo.sql           D
│   ├── seed_test.sql           D
│   ├── check_data.sql          D
│   ├── demo.sh                 D
│   ├── lint.sh                 D
│   └── ddl/01..13_*.sql        各 owner
├── obj/                        构建产物，不入库
├── lib/                        libcommon.a、libdb.so，不入库
└── data/                       导出文件，不入库
```

`todo.c` 是四人共写的唯一文件，冲突风险最高，因此约定：**函数体按固定顺序排列**（`todo_doctor` → `todo_patient` → `todo_admin` → `todo_print`），每人只改自己那一个函数的内部，不改顺序、不改其他函数、不在文件顶部加 include。首次提交由 D 建骨架（四个空函数 + include），三人各自 PR 填自己的函数体。

### 8.2 构建产物

| 产物 | 来源 | 说明 |
|------|------|------|
| `lib/libcommon.a` | `src/common.c` | 静态库 |
| `lib/libdb.so` | `src/db.c` | 动态库，编译加 `-fPIC` |
| `medical_system` | `src/` 下其余全部 .c | 链接上述两个库 + `-lmysqlclient` |
| `obj/test_*` | `tests/*.c` | 每个测试文件一个可执行 |

业务 .c 文件**不各自编成库**，直接链进 `medical_system`。

### 8.3 Makefile 目标

```
make              编译全部，-Wall -Werror -g
make lib          只编 libcommon.a 与 libdb.so
make test         编译并运行 tests/ 下全部用例，任一失败即非 0 退出
make lint         执行 scripts/lint.sh，见编码规范第 7 章
make format       clang-format -i 就地格式化
make format-check 只检查不改动，有差异则非 0 退出（CI 用）
make asan         -fsanitize=address,undefined 重新编译并跑 demo.sh 与 test
make db-init      执行 init_db.sql
make db-up        docker compose up -d（可选目标，本机未装 docker 时报一句提示并返回 0，不阻塞其他目标）
make db-down      docker compose down（可选目标，同上）
make db-migrate   执行 migrate_v1.1.sql
make db-seed      执行 seed_demo.sql
make db-reset     DROP + CREATE + init + migrate + seed
make check-data   执行 check_data.sql，任何一项计数非 0 则失败
make demo         执行 demo.sh
make clean        删 obj/ lib/ medical_system
make distclean    clean + 删 data/*.csv data/*.txt
make help         列出全部目标
```

源文件列表**必须**用 `$(wildcard src/*.c)` 与 `$(wildcard tests/test_*.c)` 生成，禁止硬编码文件名——否则每加一个 .c 都要改 Makefile，四人必然冲突。

### 8.4 减少冲突的三条硬约定

1. `main.c` 不放任何菜单文字。它只做：连接数据库 → 登录 → 按 `g_session.role` 跳到 `ui_admin_main()` / `ui_doctor_main()` / `ui_patient_main()` → 退出时关闭连接。菜单在 `ui_*.c` 里，三人各写各的文件
2. DDL 按表分文件，`init_db.sql` 只有一串 `SOURCE`，新增表由 D 加一行
3. `common.h` 的修改**必须**走 PR 且 D 审批；新增结构体成员追加在结构体末尾，不插在中间（避免他人正在编译的代码错位）

---

## 9. 验收用例

用例在 `tests/` 下实现，用 `scripts/seed_test.sql` 灌入前置数据，`make test` 一键运行。断言用 `common.h` 的宏。数据库为 `medical_test`，每个测试文件开头清库重灌，末尾不清理（由 `run_all.sh` 统一 DROP）。

| 编号 | 场景 | 期望结果 | owner |
|------|------|---------|-------|
| TC-M03-01 | 对 status=0 的预约改期到三天后的有余号时段 | 返回 0；预约的 date/slot/schedule_id 更新；change_count=1；change_time 非空 | A |
| TC-M03-02 | 改期到已满时段 | 返回 `ERR_NO_QUOTA`；原预约字段不变；原号源 quota_used 不变；新号源 quota_used 不变 | A |
| TC-M03-03 | 对 status=1（已就诊）的预约改期 | 返回 `ERR_BAD_STATE`；无任何数据变化 | A |
| TC-M03-04 | change_count 已为 3 时再改期 | 返回 `ERR_BAD_STATE`；数据不变 | A |
| TC-M03-05 | 取消 status=0 的预约 | 返回 0；status=2；对应号源 quota_used 减 1 | A |
| TC-M03-06 | 患者用自己的账号改他人预约 | 返回 `ERR_PERM` | A |
| TC-M09-01 | 对在职医生按周排班，周一日期正确，上午 20 下午 10 | 返回 0；out_created=14；14 条记录字段正确 | A |
| TC-M09-02 | 重复执行同一次按周排班 | 返回 0；out_created=0；记录仍为 14 条，未被覆盖 | A |
| TC-M09-03 | 号源总数为 1，两名患者先后挂同一时段 | 第一例返回 0 且 quota_used=1；第二例返回 `ERR_NO_QUOTA`，且 t_appointment 中不存在第二例记录 | A |
| TC-M09-04 | 挂满后取消一个，再挂一个 | 取消返回 0 且 quota_used 回到 0；再挂返回 0 且 quota_used=1 | A |
| TC-M09-05 | 传入非周一日期排班 | 返回 `ERR_PARAM`；无记录写入 | A |
| TC-M09-06 | 患者端查询排班，only_available=1 | 结果中不含 status=0 的排班、不含 quota_used=quota_total 的排班、不含过去日期 | A |
| TC-M09-07 | 把 quota_total 调到小于 quota_used | 返回 `ERR_PARAM`；quota_total 不变 | A |
| TC-M09-08 | 对已有 2 条待就诊预约的时段停诊 | 返回 `ERR_BAD_STATE`；out_blocked=2；status 仍为 1 | A |
| TC-M09-09 | 对无预约的时段停诊，再复诊 | 两次均返回 0；status 依次为 0、1 | A |
| TC-M09-10 | 对已停诊的号源占号 | 返回 `ERR_BAD_STATE` | A |
| TC-M05-01 | 保存含 3 条明细的模板 | 返回 0；主表 1 条、明细 3 条；doctor_id 为当前医生 | B |
| TC-M05-02 | 同名模板重复保存 | 返回 `ERR_DUPLICATE`；明细未写入 | B |
| TC-M05-03 | 套用模板，其中 1 种药已停用 | 返回 0；out_n=2；out_skipped=1；unit_price 为当前药品价格而非模板保存时价格；t_medicine.stock 不变 | B |
| TC-M05-04 | 开方 3 条明细，第 2 条库存不足 | 返回 `ERR_STOCK_LOW`；处方主表无记录、明细无记录、三种药库存全部不变、t_stock_log 无 change_type=3 的记录 | B |
| TC-M05-05 | 套用他人模板 | 返回 `ERR_PERM` | B |
| TC-M05-06 | 正常开方 2 条明细 | 返回 0；主表 1 条、明细 2 条；两种药库存各减对应数量；t_stock_log 新增 2 条 change_type=3；明细 subtotal = quantity × unit_price；主表 status=1 | B |
| TC-M07-01 | 医生修改本人 2 小时内的病历 | 返回 0；update_time 刷新 | B |
| TC-M07-02 | 医生修改本人 25 小时前的病历 | 返回 `ERR_LOCKED`；内容不变 | B |
| TC-M07-03 | 医生修改他人书写的病历 | 返回 `ERR_PERM` | B |
| TC-M10-01 | 入库 100（原库存 20） | 返回 0；stock=120；流水 change_type=1，quantity=100，before=20，after=120 | C |
| TC-M10-02 | 出库量大于库存 | 返回 `ERR_STOCK_LOW`；stock 不变；无流水写入 | C |
| TC-M10-03 | 按药品 + 日期区间查流水 | 只返回该药品该区间内的记录；按 create_time DESC 排序；out_total 正确 | C |
| TC-M10-04 | 作废一张已开处方 | 返回 0；处方 status=0；全部药品库存不变；每种药新增一条 change_type=6、quantity=0、before=after 的流水 | C |
| TC-M10-05 | 医生查询流水：先以 change_type=0（全院）查，再以 change_type=3 查 | 第一次返回 `ERR_PERM`；第二次返回 0，且结果中每条记录的 ref_id 对应的处方 doctor_id 都等于当前医生 | C |
| TC-M10-06 | 三种药库存分别为 120、30、0，查低库存 | 返回 0；out_n=2；顺序为 stock=0 的药在前 | C |
| TC-M10-07 | 盘点：账面 50，实盘 42 | 返回 0；stock=42；流水 change_type=5、quantity=-8、before=50、after=42；ref_id 为批次号 | C |
| TC-M10-08 | 盘点：账面 50，实盘 50 | 返回 0；无流水写入；stock 不变 | C |
| TC-M10-09 | 停用某药品后查询开方选药列表 | 该药不在列表中；历史处方明细仍能查出该药名 | C |
| TC-M11-01 | 医生有 3 条今日待接诊、1 条过期未标记 | 返回 0；lines 中含"[接诊] 今日还有 3 位"与"[爽约] 有 1 条" | B |
| TC-M11-02 | 患者有 1 条 2 天后的预约、未填紧急联系人 | 返回 0；lines 中含"[预约]"与"还有 2 天"、含"[资料]" | A |
| TC-M11-03 | 2 种药低于阈值、1 位医生未来一周无排班 | 返回 0；lines 中含"[库存] 2 种"与"[排班] 1 位" | C |
| TC-M11-04 | 无任何数据的空库 | 三个函数均返回 0，out_n=0，不崩溃 | D |
| TC-M01-01 | 用重复身份证注册 | 返回 `ERR_DUPLICATE`；表中无新记录 | A |
| TC-M01-02 | 管理员注销患者，该患者再登录 | 注销返回 0；登录返回 `ERR_NOT_FOUND`（status=0 视为账号不存在） | A/D |
| TC-M08-01 | 连续 5 次错误密码，第 6 次即使密码正确 | 第 5 次后 fail_count 归零且 lock_until 约为 10 分钟后；第 6 次返回 `ERR_LOCKED` | D |
| TC-M08-02 | 锁定期间用正确密码登录 | 返回 `ERR_LOCKED`，提示中含剩余分钟数 | D |
| TC-M08-03 | 密码错误 3 次后用正确密码登录 | 返回 0；fail_count=0；lock_until 为 NULL | D |
| TC-CMN-01 | 50 名患者，keyword 匹配其中 7 名，PAGE_SIZE=20 | 第 1 页返回 7 条，out_total=7，第 2 页返回 0 条 | A |
| TC-CMN-02 | keyword 含 `%` | 按字面匹配，不被当作通配符（若库中存在名字含 % 的记录则匹配到它，否则 out_total=0） | A |
| TC-CMN-03 | 会话超时（把 last_action 手动改为 20 分钟前） | `session_check_timeout` 返回 `ERR_TIMEOUT`，g_session.role 变为 0 | D |
| TC-CMN-04 | 导出患者列表，只勾选姓名与电话 | 生成的 CSV 首行为两个中文表头，行数 = 记录数 + 1，不含密码列 | C |
| TC-CMN-05 | 病历打印视图，本人患者查看 | 返回 0；输出含患者姓名、诊断、药品明细与合计金额 | B |
| TC-DATA-01 | 全库执行 check_data.sql | 全部查询计数为 0 | D |
| TC-SEC-01 | 姓名输入 `张三'; DROP TABLE t_patient; --` 后注册 | 返回 0 或 `ERR_PARAM`；t_patient 表仍存在；库中该记录的 name 字段为原始输入串 | A |
| TC-SEC-02 | 患者端调用管理员菜单入口函数 | 返回 `ERR_PERM` | D |
| TC-ASAN-01 | `make asan` 跑完 demo.sh 与全部 test | ASan/UBSan 无 error，无泄漏报告 | D |

用例总数 52 条。P0 相关用例（TC-M03-05、TC-M05-04、TC-M05-06、TC-M09-03、TC-M09-04、TC-M10-01、TC-M10-02、TC-M01-01、TC-SEC-01、TC-ASAN-01）**必须**在 W2 结束前通过，其余在 W3 结束前通过。

---

## 10. 追溯矩阵

| FR | 实现文件 | 头文件 | owner | TC |
|----|---------|--------|-------|-----|
| FR-M03-06 | `src/appointment.c` | `appointment.h` | A | TC-M03-01～04, 06 |
| FR-M03-07 | `src/appointment.c` | `appointment.h`、`schedule.h` | A | TC-M09-03, 04, TC-M03-05 |
| FR-M05-05 | `src/prescription.c` | `prescription.h` | B | TC-M05-01～03, 05 |
| FR-M05-06 | `src/prescription.c` | `prescription.h`、`medicine.h` | B | TC-M10-04 |
| FR-M09-01 | `src/schedule.c` | `schedule.h` | A | TC-M09-01, 02, 05 |
| FR-M09-02 | `src/schedule.c`、`src/ui_patient.c` | `schedule.h` | A | TC-M09-06 |
| FR-M09-03 | `src/schedule.c`、`src/ui_admin.c` | `schedule.h` | A | TC-M09-07～09 |
| FR-M09-04 | `src/schedule.c` | `schedule.h` | A | TC-M09-03, 04, 10 |
| FR-M10-01 | `src/medicine.c` | `medicine.h` | C | TC-M10-01, 02, 04 |
| FR-M10-02 | `src/medicine.c`、`src/ui_admin.c` | `medicine.h` | C | TC-M10-03, 05 |
| FR-M10-03 | `src/medicine.c`、`src/ui_admin.c` | `medicine.h` | C | TC-M10-06 |
| FR-M10-04 | `src/medicine.c` | `medicine.h` | C | TC-M10-07, 08 |
| FR-M11-01 | `src/todo.c`（`todo_doctor`） | `todo.h` | B | TC-M11-01, 04 |
| FR-M11-02 | `src/todo.c`（`todo_patient`） | `todo.h` | A | TC-M11-02, 04 |
| FR-M11-03 | `src/todo.c`（`todo_admin`） | `todo.h` | C | TC-M11-03, 04 |
| FR-CMN-01 | 各模块 query 函数 + `src/common.c` | `common.h` | A/B/C | TC-CMN-01, 02 |
| FR-CMN-02 | `src/user.c`、`src/main.c` | `user.h`、`common.h` | D | TC-M08-01～03, TC-CMN-03 |
| FR-CMN-03 | `src/export.c` | `common.h` | C | TC-CMN-04 |
| FR-CMN-04 | `src/medical_record.c` | `medical_record.h` | B | TC-CMN-05 |
| CH-06 | `src/medicine.c` | `medicine.h` | C | TC-M10-09 |
| CH-07 | `scripts/ddl/13_medical_record.sql` | — | B | TC-M07-01, 02 |
| CH-08 | `src/patient.c` | `patient.h` | A | TC-M11-02 |

每条 FR 关闭时，对应 Issue 必须在描述中列出通过的 TC 编号；TC 未实现的 FR 不得关闭 Issue。

---

## 11. 协作流程

### 11.1 远端与分支

主远端 GitHub，镜像远端 Gitee（`git remote add gitee ...`，只在 GitHub 不可达时推送，防校园网波动）。

| 分支 | 用途 | 保护 |
|------|------|------|
| `master` | 可交付状态，只接受来自 `develop` 的 PR | 禁 force push、禁直接 push、需 2 approve |
| `develop` | 集成分支 | 禁 force push、禁直接 push、需 1 approve + CI 通过 |
| `feature/<role>-<module>-<简述>` | 功能开发 | 无 |
| `hotfix/<简述>` | 从 master 拉出修紧急问题，同时合回 master 与 develop | 无 |

角色代号：`patient`（A）、`doctor`（B）、`admin`（C）、`core`（D）。示例：`feature/patient-m09-schedule`、`feature/doctor-m05-template`、`feature/admin-m10-stocklog`、`feature/core-ci`。

分支保护规则在 W1 由 D 配置：`develop` 与 `master` 开启 Require a pull request before merging、Require status checks to pass（check 名为 `ci`）、Require branches to be up to date before merging、Do not allow force pushes、Include administrators。

### 11.2 CODEOWNERS

```
/include/          @D
/src/db.c          @D
/src/common.c      @D
/src/main.c        @D
/Makefile          @D
/scripts/          @D
/.github/          @D
/tests/harness.c   @D
/src/patient.c     @A
/src/appointment.c @A
/src/schedule.c    @A
/src/ui_patient.c  @A
/src/visit.c       @B
/src/prescription.c @B
/src/medical_record.c @B
/src/ui_doctor.c   @B
/src/doctor.c      @C
/src/medicine.c    @C
/src/ui_admin.c    @C
/src/export.c      @C
/CHANGELOG.md      @C
```

CODEOWNERS 使对应 owner 自动成为必需审阅者。（实际使用时把 @A～@D 替换为四人的 GitHub 用户名。）

### 11.3 PR 规则

1. 一个 PR 只做一件事，描述中写 `Fixes #N` 关联 Issue
2. diff 不超过 400 行（不含自动生成的 SQL 数据）。超出必须拆分
3. 标题用 conventional commits：`feat(M09): 按周生成排班号源`、`fix(M05): 明细插入失败时库存未回滚`、`refactor(C): 盘点差异改走 medicine_change_stock`、`test(M10): 补盘亏流水用例`、`chore(ci): 加 mysql 服务容器`
4. 描述按四段填写（模板已生成）：改了什么 / 怎么测的（列出通过的 TC 编号）/ 影响谁的接口（改了头文件或唯一写入口时必须点名）/ AI 辅助说明（哪些部分由 agent 生成、自己做了哪些判断与修改）
5. 至少 1 个 approve；改动 `include/*.h`、`src/db.c`、`src/common.c`、`src/main.c`、`Makefile`、`scripts/**`、`.github/**` 时需 2 个 approve，且其中必须包含 D
6. 禁止自审自合
7. `develop` 与 `master` 禁止 force push；个人分支 rebase 后用 `git push --force-with-lease`
8. CI 未绿不得合并；本地**必须**先跑 `make && make lint && make format-check && make test`

### 11.4 评审分工

交叉评审：A ↔ B 互评，C ↔ D 互评。核心文件（第 11.2 节中 @D 的部分）D 必审。评审关注点按顺序：

1. 是否越界改了不属于自己的文件
2. 事务边界是否符合第 4.4 节（被调函数里出现 `db_tx_*` 直接打回）
3. SQL 是否全部转义、是否有 `SELECT *`
4. 返回码是否用了 `errcode.h` 中的既有码
5. 是否实现了 FR 之外的功能（有则要求拆出或删掉）
6. 编码规范第 6 章禁止清单
7. 逻辑正确性与测试覆盖

评审**不**纠结个人写法偏好（见编码规范第 8 章）。

### 11.5 Issue 与看板

- 47 个功能点，每人在 W1 为自己负责的点建 Issue，标题格式 `[M09-01] 按周生成排班`
- 标签：`module/M01`～`module/M11`、`owner/A`～`owner/D`、`priority/P0`～`P2`、`type/feature|bug|test|docs|chore`
- Projects 看板四列：待办 / 进行中 / 待评审 / 已完成。"待评审"表示 PR 已提，合并后移到"已完成"并关闭 Issue
- Milestones：`W1-地基`、`W2-核心链路(v1.0)`、`W3-扩展(v1.1)`、`W4-收尾`
- C 兼管看板整理与 `CHANGELOG.md` 更新（每合并一个 feat/fix PR 补一行）

### 11.6 CI

`.github/workflows/ci.yml`：

```yaml
name: ci
on:
  pull_request:
    branches: [ develop, master ]
  push:
    branches: [ develop, master ]
jobs:
  build:
    runs-on: ubuntu-latest
    services:
      mysql:
        image: mysql:8
        env:
          MYSQL_ROOT_PASSWORD: root
          MYSQL_DATABASE: medical_test
        ports: [ '3306:3306' ]
        options: >-
          --health-cmd="mysqladmin ping -h localhost -uroot -proot"
          --health-interval=10s --health-timeout=5s --health-retries=10
    env:
      DB_HOST: 127.0.0.1
      DB_USER: root
      DB_PASS: root
      DB_NAME: medical_test
    steps:
      - uses: actions/checkout@v4
      - run: sudo apt-get update && sudo apt-get install -y gcc make libmysqlclient-dev clang-format
      - run: make            # -Wall -Werror，任何警告都失败
      - run: make lint
      - run: make format-check
      - run: make db-init && make db-migrate
      - run: make test
      - run: make check-data
      - run: make asan
```

`make asan` 在 CI 中跑 `demo.sh`，需要 seed 数据，因此 asan 步骤**必须**排在 db-init 之后，并在步骤内先执行 `make db-seed`。

### 11.7 提交纪律

- 小步提交，一次一个改动。禁止把 agent 一次生成的几百行代码打成单个 commit
- 提交信息带模块号与动词，说清做了什么。禁止 `update`、`修改`、`完善代码`
- 每次提交前本地跑 `make && make lint`
- 合并 `develop` 到自己的分支用 `git pull --rebase origin develop`，避免无意义的 merge commit

---

## 12. AI 辅助开发的输入输出契约

本章规定"人做什么、agent 做什么、产出物长什么样"，是四人使用 agent 的统一口径。

### 12.1 分工边界

**人负责**（不得交给 agent 决定）：

1. 接口签名、数据结构、错误码取值——以本文件第 7 章为准，agent 不得修改
2. 业务规则的判定——本文件第 6 章的规则条目由人读懂后转述给 agent，agent 提出疑问时由人裁定
3. 事务边界的划分——第 4.4 节已定，agent 不得自行增减 `db_tx_*` 调用
4. 需求范围的取舍——agent 建议加功能时一律拒绝，改为记 Issue 由人评审
5. 代码评审与合并决定
6. 答辩演示与设计说明文档

**agent 可以做**：

1. 按 FR 条目与接口契约生成 .c 文件初稿
2. 生成 `tests/test_*.c` 用例骨架与断言
3. 生成 `migrate_v1.1.sql` 的幂等片段（按 5.3 的模板）
4. 生成 seed 数据 SQL
5. 挑 diff 的毛病：未判断的返回值、可能的缓冲区溢出、未释放的内存、遗漏的错误分支
6. 解释一段 SQL 的执行计划或 MySQL 语法
7. 按 `.ai/style.md` 重写某段代码的形态（不改逻辑）

### 12.2 agent 的产出物约束

**必须**：

- 只产出被要求的文件内容。要求 `src/schedule.c` 就只给 `src/schedule.c`
- 函数签名与本文件第 7 章完全一致，包括参数顺序、const 修饰、出参类型
- 错误码只用 `errcode.h` 中已定义的
- 结构体成员名与数据库字段名一致
- 输出完整的可编译文件，不用 `...` 或 `// 其余代码同上` 省略

**禁止**：

- 生成需求之外的文件（`IMPLEMENTATION_NOTES.md`、`SUMMARY.md`、`README_*.md`、示例 main、演示用 .c）
- 修改本文件已定的接口、DDL、错误码、常量值
- 实现 FR 之外的功能，或"顺手优化"既有代码
- 引入第三方库、头文件之外的 include
- 生成 `TODO` / `FIXME` / 被注释掉的代码
- 在回复正文里输出与代码无关的长篇说明——说明写在 PR 描述里，由人写

### 12.3 提交前的人工检查

agent 产出后，提交人**必须**逐行读过再提 PR，并回答三个问题（答辩时会被问到同样的问题）：

1. 这段代码为什么这样写？换成另一种写法会出什么问题？
2. 事务在哪里开、在哪里提交、失败时哪些数据会回滚？
3. 哪几行是安全或一致性的关键？（转义、条件 UPDATE 的影响行数判断、状态检查）

答不上来就不要提交。这一条是本项目使用 AI 辅助的底线要求。

### 12.4 PR 描述中的 AI 辅助说明

固定写清三件事，一到三句话即可，不要写成长篇声明：

- 哪些部分由 agent 生成（如"SQL 拼接与错误分支由 agent 生成"）
- 自己做了哪些判断与修改（如"事务边界和状态机判定是我定的，agent 初稿把 release 放在了 update 之后，我调整了顺序"）
- 哪些地方自己重写或删掉了（如"agent 生成的参数校验有八条，删到四条"）

---

## 13. 里程碑与交付物

### 13.1 里程碑

只排到周，不做人日估算。每个里程碑的判定标准是"某个命令跑通"，不是"某个百分比完成"。

| 里程碑 | 时间 | 内容 | 判定标准 |
|--------|------|------|---------|
| **W1 地基** | 第 1 周 | GitHub 仓库与分支保护、CODEOWNERS、Issue/PR 模板、CI、Makefile、`db.c` 六类封装、`common.c` 与 `ui.h` 全套公共函数、`errcode.h`、13 张表的 DDL 与 `init_db.sql`、全部头文件定稿、三端菜单空壳、`.ai/` 目录 | `make` 通过且 CI 绿；`make db-init` 建库成功；程序能登录并进入三个空菜单；`make lint` 通过 |
| **W2 核心链路** | 第 2 周 | v1.0 的 35 个功能点全部实现（M01～M08 基线行为）；P0 用例通过 | `bash scripts/demo.sh` 走完 v1.0 说明书 4.7 的六步主流程无报错；P0 用例全绿；打 tag `v1.0` |
| **W3 扩展** | 第 3 周上半 | 12 个新功能点（M09 4 点、M10 3 点、M11 3 点、M03-06、M05-05）；CH-01～CH-08 | 对应 Issue 全部关闭；TC-M09-*/TC-M10-*/TC-M11-*/TC-M03-*/TC-M05-* 全绿；`make check-data` 通过 |
| **W4 收尾** | 第 3 周下半 | 联调、README、CHANGELOG、四份设计说明、答辩演示脚本排练、`make asan` 清零 | CI 全绿；`make asan` 无报告；tag `v1.1`；四份设计说明入库 |

W1 的四件事按此顺序做（前后有依赖）：① D 建仓库、配保护分支与 CI，推空 Makefile；② D 提交 `errcode.h` + `db.h` + `common.h` 骨架（只有声明），**当天**合并，让三人能开始写实现；③ 四人各写自己表的 DDL 并由 D 汇总 `init_db.sql`；④ 三人开始写各自模块，D 补 `common.c` 与 `ui.h` 的实现。第 ② 步是关键路径，D 若在第一天没交出头文件，其余三人当天无法开工——这一条是 v1.0 说明书暴露的瓶颈，v1.1 用"头文件先行 + db.c 收窄为通用封装"解决。

### 13.2 交付物清单

| 交付物 | 负责人 | 时间 |
|--------|--------|------|
| 可运行程序 `medical_system` + Makefile | 全员 | W4 |
| 数据库脚本（init / migrate / seed / check） | D 汇总 | W1～W3 |
| 测试用例 52 条 + `make test` | 各 owner | W2～W3 |
| CI 工作流（绿） | D | W1 |
| README（环境、编译、初始化、演示、常见问题） | D | W4 |
| CHANGELOG | C | 持续 |
| 四份设计说明 `docs/design/[A-D]-*.md` | 各人 | W4 |
| 答辩演示脚本 `scripts/demo.sh` + 讲稿要点 | D | W4 |
| 本 SRS 与编码规范入库 | D | W1 |

设计说明的要求：每人 2～4 页，写自己那条线的模块划分、关键数据结构、事务边界、遇到的两个具体问题与解决过程。**不要**写成需求复述，也**不要**贴大段代码。

### 13.3 功能裁剪顺序

进度落后时按此顺序砍，砍掉的功能在 Issue 上标 `type/descoped` 并说明原因：

1. FR-CMN-04 病历打印视图（P2）
2. FR-CMN-03 导出可选字段（P2）
3. FR-M11-03 管理员端待办（P1）
4. FR-M05-05 处方模板（P1）
5. FR-M03-06 预约改期（P1，保留爽约标记）
6. FR-M09-03 号源数调整与停诊（P1，保留按周生成与占放号）

**永不裁剪**：核心链路（注册 → 排班 → 挂号占号 → 接诊 → 开方扣库存 → 写病历 → 患者查询）、FR-M09-01/02/04、FR-M10-01/02、FR-M11-01/02、FR-CMN-01/02、全部 P0 用例。

### 13.4 风险与应对

| 风险 | 应对 |
|------|------|
| W1 头文件定稿延迟，三人无法开工 | D 在第一天只交声明不交实现；三人可先用桩函数编译 |
| `todo.c` 四人共写冲突 | 函数体固定顺序 + 各自只改自己的函数 + 骨架由 D 先建 |
| agent 生成代码风格漂移 | 三道闸：`-Werror`、`make lint` 禁词、`make format-check` |
| 事务边界被 agent 改坏 | `make lint` 检查 `db_tx_*` 只出现在白名单函数中；评审第 2 项 |
| 数据库版本或字符集差异 | CI 固定 `mysql:8`；README 写明本机需要的版本与 `utf8mb4` 设置 |
| 老师不认可功能点计数口径 | 开工前确认（Issue #1），准备第 1.4 节的削减方案 |
| GitHub 访问不稳定 | Gitee 镜像远端；每次合并后同步推送 |
| 成员对 agent 产出理解不足，答辩讲不清 | 第 12.3 节的三问 + 每人写设计说明 + W4 交叉演示排练（A 演示 B 的线，讲不清就补） |

---

## 14. 非功能性要求

### 14.1 纳入验收的

| 项 | 要求 | 验证方式 |
|----|------|---------|
| 可编译性 | `-Wall -Werror` 零警告通过 | CI |
| 代码形态 | 符合编码规范第 7 章全部机器检查项 | `make lint`、`make format-check` |
| 内存安全 | ASan + UBSan 无 error、无泄漏 | `make asan` |
| 数据一致性 | 无孤儿数据；事务失败后无部分写入 | `make check-data`、TC-M05-04 |
| SQL 注入防护 | 全部用户输入经转义 | TC-SEC-01、评审 |
| 权限隔离 | 三个角色互不越权 | TC-SEC-02、TC-M03-06、TC-M05-05、TC-M07-03 |
| 可移植性 | Ubuntu 与 RHEL 系均可编译 | README 两套依赖命令；CI 用 Ubuntu |
| 可测试性 | 52 条用例可一键运行 | `make test` |
| 可演示性 | `demo.sh` 无交互跑完全流程 | W2、W4 判定标准 |
| 可维护性 | 单文件 ≤600 行、单函数 ≤60 行、无重复的表格打印代码 | `make lint` |

### 14.2 明确不纳入验收的

| 项 | v1.0 的原要求 | v1.1 的处理 |
|----|--------------|------------|
| 查询性能 | 单表 10 万行 ≤500ms | **不验收**。演示数据量为患者 ≤50、药品 ≤100、预约 ≤200。不做索引优化、不分表、不缓存 |
| 响应时间 | 常规操作 ≤200ms | **不验收**。终端程序在小数据量下天然满足，不为此写代码 |
| 并发能力 | 10 个并发终端 | **不验收**。程序是单进程单连接。第 1.6 节的条件更新只为数据正确性，不为并发压测 |
| 密码安全 | 明文存储 + 预留 hash | 沿用明文。`hash_password()` 保留空桩，不接 OpenSSL |
| 操作系统版本 | Rocky Linux 9.8 / glibc 2.34 | 放宽为任意 Linux |
| 工期精度 | 按小时排一天时间表 | 只排到周，判定标准是命令跑通 |

### 14.3 质量闸门

三道闸，全部通过才能提 PR：

1. `make`（`-Wall -Werror`）——编译期问题
2. `make lint` + `make format-check`——形态与禁止清单
3. `make test` + `make asan` + `make check-data`——运行期与数据一致性

---

## 附录 A：交给编码 agent 的任务包模板

复制以下模板，填空后作为 agent 的输入。

```
# 任务：实现 FR-M09-01 按周生成排班

## 你的产出
只输出一个文件的完整内容：src/schedule.c 中的 schedule_add_week 函数
（如该文件已有其他函数，输出完整文件并保留它们）。
不要输出说明文档，不要输出 Makefile 改动，不要输出测试文件。

## 需求条目
[粘贴 FR-M09-01 全文]

## 相关表结构
[粘贴 5.2 节 t_schedule 的 DDL、5.3 中 t_appointment 的变更、t_doctor 的 DDL]

## 必须使用的接口（签名不得修改）
[粘贴 7.1 db.h、7.3 common.h 中本次会用到的声明]

## 全局约定
[粘贴第 4 章全文]

## 编码规范
[粘贴 .ai/style.md 全文]

## 验收用例
[粘贴 TC-M09-01、TC-M09-02、TC-M09-05 三行]

## 禁止事项
- 不要修改任何接口签名、错误码取值、常量值、表结构
- 不要实现需求条目之外的功能，包括你认为"应该有"的功能
- 不要在函数内调用 db_tx_begin/commit/rollback 之外的模块的写入口以外的写操作
- 不要生成 TODO、被注释掉的代码、装饰性注释块
- 不要用 sprintf/strcpy/strcat，SQL 拼接一律 snprintf + db_escape
- 不要 SELECT *
- 不要引入任何第三方库
```

任务包的粒度：**一次一个 FR 条目**。不要一次让 agent 实现整个模块，那样产出的代码无法逐行审查，也无法与 Issue 和 PR 对应。

---

## 附录 B：功能点计数

### B.1 v1.0 基线（35 点）

| 模块 | 点数 | 功能 |
|------|:----:|------|
| M01 | 6 | 注册、修改本人信息、按编号查询、列表查询、注销、导出 |
| M02 | 5 | 新增、删除（软）、修改、按编号查询、列表查询 |
| M03 | 5 | 挂号、取消、按患者查询、按医生查今日、接诊 |
| M04 | 3 | 新增就诊记录、按患者查询、按医生查询 |
| M05 | 4 | 开方、作废、按患者查询、按处方号查询 |
| M06 | 6 | 新增药品、入库、出库、调价、列表查询、盘点 |
| M07 | 3 | 新增病历、修改病历、按患者查询 |
| M08 | 3 | 登录、患者注册、修改密码 |
| 合计 | **35** | |

### B.2 v1.1 新增（12 点）

| 编号 | 模块 | 功能 | owner | 优先级 |
|------|------|------|-------|--------|
| FR-M09-01 | M09 | 按周生成排班 | A | P1 |
| FR-M09-02 | M09 | 排班查询（三端视图） | A | P1 |
| FR-M09-03 | M09 | 号源调整与停诊复诊 | A | P1 |
| FR-M09-04 | M09 | 号源占用与释放（挂号/取消联动） | A | P0 |
| FR-M10-01 | M10 | 库存变化与流水记录 | C | P0 |
| FR-M10-02 | M10 | 库存流水查询 | C | P1 |
| FR-M10-03 | M10 | 低库存预警列表 | C | P1 |
| FR-M11-01 | M11 | 医生端待办 | B | P1 |
| FR-M11-02 | M11 | 患者端待办 | A | P1 |
| FR-M11-03 | M11 | 管理员端待办 | C | P1 |
| FR-M03-06 | M03 | 预约改期与爽约标记 | A | P1 |
| FR-M05-05 | M05 | 处方模板 | B | P1 |
| 合计 | | **12 点** | | |

计数说明：FR-M03-06 含 `appointment_reschedule` 与 `appointment_mark_noshow` 两个函数，FR-M05-05 含 `presc_template_add` / `apply` / `query` / `delete` 四个函数，各计 1 点，因为对用户而言分别是"改期这件事"和"模板这件事"。FR-M09-04 与 FR-M10-01 虽以库函数形式存在，但用户可见（挂号时提示"号已满"、库存变化必有流水可查），计入点数。

### B.3 增幅

- 功能点：35 → 47，增幅 **34.3%**
- 模块：8 → 11，增幅 **37.5%**
- 数据表：9 → 13，增幅 44.4%（表数量不是计数口径，仅作规模参考）
- 不计入点数的改动 8 项（CH-01～CH-08）

两项主口径均落在 30%～40% 区间。

### B.4 工作量配平

| 成员 | 负责范围 | 粗估占比 |
|------|---------|:--------:|
| A 患者线 | M01、M03（含改期与爽约）、M09 全部、M08 患者注册半边、FR-M11-02、`ui_patient.c`、CH-08 | 26% |
| B 医生线 | M04、M05（含模板）、M07、FR-M11-01、`ui_doctor.c`、CH-02 | 27% |
| C 管理员线 | M02、M06、M10 全部、FR-M11-03、`ui_admin.c`、`export.c`、CHANGELOG、看板管理、CH-03 的字段部分 | 22% |
| D 统合线 | M08 登录与会话、M11 框架、`db.c`、`common.c`、`main.c`、Makefile、scripts、CI、测试框架、集成测试、CH-04 | 25% |

互备：A ↔ B、C ↔ D。互备的要求只是"能读懂对方代码 + 能评审对方 PR"，不要求能独立重写。

---

## 附录 C：无复杂算法对照表

逐项说明每个新功能的实现手段，供评审与答辩核对"没有复杂算法"。

| 功能 | 实现手段 | 复杂度 |
|------|---------|--------|
| 号源占用 | 一条带 `WHERE quota_used < quota_total` 的 UPDATE，看影响行数 | 一条 SQL |
| 号源释放 | 一条带 `WHERE quota_used > 0` 的 UPDATE | 一条 SQL |
| 按周排班 | 一个 7 次 × 2 时段的循环，每次一条 INSERT | 双层定长循环，共 14 次 |
| 排班查询 | SELECT + JOIN t_doctor + `FIELD(time_slot,'上午','下午')` 排序 | 一条 SQL |
| 停诊前检查 | 一条 `SELECT COUNT(*) FROM t_appointment WHERE schedule_id=? AND status=0` | 一条 SQL |
| 库存变化 | 一条带 `WHERE stock + delta >= 0` 的 UPDATE，加一条流水 INSERT | 两条 SQL |
| 流水查询 | SELECT + JOIN t_medicine + 动态 WHERE + LIMIT/OFFSET | 一条 SQL |
| 低库存 | `SELECT ... WHERE stock < 50 AND status = 1 ORDER BY stock` | 一条 SQL |
| 盘点差异 | `diff = actual - book` 一次减法，符号决定 change_type | 一次算术运算 |
| 处方模板套用 | 查模板明细，逐行复制到内存数组，同时查当前药价 | 一个循环 + 一条 IN 查询 |
| 预约改期 | 一个事务内：release → take → UPDATE 三个字段 | 三次数据库调用 |
| 医生端待办 | 4 条 COUNT 查询 + 1 条取最近患者 | 5 条 SQL，可合并为 UNION ALL |
| 患者端待办 | 1 条预约列表（LIMIT 3）+ 2 条 COUNT + 1 次字段判空 | 4 条 SQL |
| 管理员端待办 | 1 条低库存（LIMIT 3）+ 1 条今日预约聚合 + 1 条未排班医生（NOT EXISTS）+ 2 条 COUNT | 5 条 SQL |
| 分页 | `LIMIT PAGE_SIZE OFFSET (page-1)*PAGE_SIZE` + 一条 `COUNT(*)` | 算术 + 一条 SQL |
| 会话超时 | `time(NULL) - last_action > SESSION_TIMEOUT_MIN * 60` | 一次减法比较 |
| 登录锁定 | `fail_count + 1`，达到 5 时写 `lock_until = NOW() + INTERVAL 10 MINUTE` | 一次比较 + 一条 UPDATE |
| 密码 | 明文比对，`hash_password()` 原样返回 | 一次 `strcmp` |

明确不实现的东西：任何排序算法（全部用 `ORDER BY`）、任何查找算法（全部用 `WHERE`）、哈希表、图、递归、动态规划、状态机引擎（预约状态迁移用一张合法迁移表 + `switch` 判断，见附录 D）、表达式求值、正则（模糊查询用 SQL `LIKE`，不用 regex 库）、自研字符串或容器库、任何浮点数值计算（金额用 `DECIMAL(10,2)` 由数据库计算小计与合计，C 侧只用 `double` 显示）。

金额小计与合计**必须**在 SQL 中算：`subtotal = quantity * unit_price`，合计用 `SELECT SUM(subtotal)`。C 侧不做金额累加，避免浮点误差。

---

## 附录 D：预约状态机

`t_appointment.status` 的合法迁移。`appointment_set_status` 是唯一的迁移入口，用一张静态表 + 查表判断实现，不用状态机框架。

```c
/* 合法迁移表：from -> 允许的 to 集合 */
static const int legal[][2] = {
    { APPT_WAITING,  APPT_VISITED  },   /* 0 -> 1  接诊 */
    { APPT_WAITING,  APPT_CANCELED },   /* 0 -> 2  取消 */
    { APPT_WAITING,  APPT_NOSHOW   },   /* 0 -> 3  爽约 */
};
```

| from \ to | 0 待就诊 | 1 已就诊 | 2 已取消 | 3 爽约 |
|-----------|:-------:|:-------:|:-------:|:-----:|
| **0 待就诊** | —（改期不改状态） | ✔ 接诊（B） | ✔ 取消（A 本人 / C） | ✔ 爽约（B / C） |
| **1 已就诊** | ✘ | — | ✘ | ✘ |
| **2 已取消** | ✘ | ✘ | — | ✘ |
| **3 爽约** | ✘ | ✘ | ✘ | — |

- 0 是唯一可迁出的状态；1、2、3 都是终态
- 任何非法迁移返回 `ERR_BAD_STATE`，UI 提示当前状态与目标状态的中文名
- 改期不改变 status，因此不在迁移表中
- 只有 0 → 2（取消）释放号源；0 → 1 和 0 → 3 都不释放
- 迁移的附加条件（不是状态机的一部分，是业务规则）：0 → 2 要求操作者是本人患者或管理员；0 → 1 要求操作者是接诊医生本人；0 → 3 要求 `appoint_date < CURDATE()`

实现形态（评审时按此检查，不要写成嵌套 if-else）：

```c
int appointment_set_status(int appoint_id, int new_status, const char *reason)
{
    /* 1. 查出当前 status，不存在返回 ERR_NOT_FOUND
       2. 查 legal 表，未命中返回 ERR_BAD_STATE
       3. 条件 UPDATE：SET status=? WHERE appoint_id=? AND status=?
          影响行数为 0 返回 ERR_BAD_STATE（并发兜底）
       4. 若 new_status == APPT_CANCELED，调 schedule_release_quota */
}
```

第 3 步的 UPDATE 带上 `AND status = <旧值>`，这样即使两步之间状态被别处改掉，也不会覆盖。这是本项目处理"先查后改"竞态的统一手法。
