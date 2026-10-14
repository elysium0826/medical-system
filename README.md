# 社区医疗服务中心信息管理系统 v1.1 开发包

Linux C99 + MySQL 8 + ANSI终端，单机课程项目。D负责统合需求与接口骨架，发给A患者线、B医生线、C管理员线按FR开发。目标是35基线点+12新增点=47点；**这是开发包骨架，不是已实现的业务系统，不是已发布v1.1**。

## 项目材料

实施依据是 [完整SRS](docs/SRS_v1.1.md)，[分工方案](docs/方案报告_v1.1.md) 作为需求与分工背景。源码、公共头文件、数据库脚本和测试分别位于 `src/`、`include/`、`scripts/` 和 `tests/`。

**v1.0原稿未提供**：不伪造`docs/需求说明书_v1.0.md`。基线结构体/九表DDL为D草案，须第0天由owner对照原稿核定；现有头文件是下发快照，agent不得改接口。已在WSL Linux和隔离MySQL 8验证公共层与菜单骨架，具体范围见下文；未完成52条TC业务验收，不能把骨架自检或桩当业务完成。

## Linux依赖

Ubuntu/Debian（客户端开发包提供mysqlclient头与链接库；直接安装MySQL8服务与此分开）：

```sh
sudo apt-get update
sudo apt-get install -y gcc make libmysqlclient-dev default-mysql-client clang-format python3
# 直接装库路径：确认发行版仓库提供MySQL8；若不是，使用官方MySQL8仓库
sudo apt-get install -y mysql-server
sudo systemctl enable --now mysql
```

Rocky/RHEL系（必须选择MySQL8，不用MariaDB替代验收环境）：

```sh
sudo dnf install -y gcc make mysql-devel mysql clang-tools-extra python3
# 如发行版源中包名/版本不匹配，先启用适配发行版的MySQL8官方源
sudo dnf install -y mysql-server
sudo systemctl enable --now mysqld
# 首次root密码按发行版安装流程设置，不能假定为空
```

验证工具实际存在：`gcc --version`、`make --version`、`mysql_config --version`、`mysql --version`、`clang-format --version`、`python3 --version`。服务须MySQL8，表使用InnoDB/utf8mb4；RHEL仓库若未提供mysql-devel/clang-format则补官方或发行版支持的仓库。终端使用UTF-8；不引入ncurses、OpenSSL或第三方C库。

### 两种数据库路径

**路径一：直接安装本机MySQL8。** 设置root密码后通过下面DB环境变量连接；系统root socket认证可能不能用TCP登录，需由本机管理员创建开发账号/赋予仅开发库的权限，勿把空密码例子当生产配置。

**路径二：开发期Docker Compose MySQL8。** 安装Docker和Compose插件后：

```sh
cd medical-system
export DB_HOST=127.0.0.1 DB_PORT=3306 DB_USER=root DB_PASS=medical123 DB_NAME=medical_db
docker compose up -d
# 或 make db-up；等待mysql容器健康，再执行下面初始化命令
```

Docker路径密码为**medical123**。compose只是开发便利，CI用Actions mysql:8服务而非compose，交付仍是Linux+本机MySQL。

## 环境、构建与初始化

构建、SQL、lint与开发包演示脚本已提供。初始化不删除已有数据；种子只允许对应的空库，重复灌种会拒绝。`make db-reset`有破坏性，必须显式设置`CONFIRM_RESET=yes`，且只允许`medical_db`/`medical_test`。**只用于专用开发/测试库**，不得连接真实患者数据库。

```sh
cd medical-system
export DB_HOST=127.0.0.1
export DB_PORT=3306
export DB_USER=root
export DB_PASS='你的本机MySQL密码'   # Docker改为medical123
export DB_NAME=medical_db

make lib              # libcommon.a + libdb.so
make                  # -std=c99 -Wall -Werror -g
make lint
make format-check     # 只检查；CI禁止用make format自动修改文件
make db-init
make db-migrate       # 增量脚本幂等；勿以db-reset替代升级
make db-seed          # 仅专用演示库：合成演示数据
make check-data
./medical_system
```

默认连接是localhost/root/空密码/medical_db；显式环境更清楚。共享库若报找不到libdb.so，检查构建rpath；仅作本机诊断可用`LD_LIBRARY_PATH="$PWD/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" ./medical_system`。不要把链接问题误认为业务通过。

### 演示账号（合成seed目标，灌种成功后才可用）

| 角色 | 账号 | 密码 |
|---|---|---|
| 管理员 | admin | 123456 |
| 医生 | D001 | 123456 |
| 患者 | P20260001 | 123456 |

三个账号都来自合成灌种；`db-init`只建库/表，不预置账号，不是生产凭据。密码按课程SRS明文存储，hash_password是原样返回空桩，**不可生产使用**；不发布真实身份、电话、诊疗信息。

## 测试与CI：骨架不等于验收

```sh
# 真数据库测试只允许独立medical_test，不清库、不DROP，不得用真实库
export DB_HOST=127.0.0.1 DB_USER=root DB_PASS='测试MySQL密码' DB_NAME=medical_test
export RUN_DB_TESTS=1
make db-init
make db-migrate
make db-seed          # 空medical_test用独立测试夹具；已有夹具不得重复灌种
make test
make check-data
make asan            # 重新构建，执行测试和只读开发包demo
# 不设置RUN_DB_TESTS时只跑结构/公共工具测试，明确打印DB测试SKIP
```

`make test`必须报告测试层次：公共层/mock骨架自检、真实MySQL8集成TC、尚未实现TC。桩仅供先行链接，不得把桩返回0/无菜单异常算FR验收；目标是SRS第9章52TC，不宣称已覆盖。ASan/UBSan目标须跑demo和test并开启泄漏检查；未实际跑过只记未验证。

[CI](.github/workflows/ci.yml) 在ubuntu-latest使用临时medical_test和合成数据：make→lint→format-check→db-init/db-migrate→test→check-data→重新db-init/db-migrate/db-seed→asan。asan必须在seed之后；格式只检查不改文件。工作流在 `main`、`develop`、`master` 分支的推送和拉取请求上触发；分支保护需由仓库维护者在GitHub设置。

## 本次已验证的骨架范围（2026-10-14）

- 环境：WSL Ubuntu、GCC 15.2.0、隔离Docker MySQL **8.0.46**；格式使用clang-format 22.1.8检查。原始根目录数据库封装不修改，项目适配层使用`dbm_`命名空间。
- 已通过C99 `-Wall -Wextra -Werror`完整构建、lint、格式检查和shell语法检查。
- 真实MySQL验证：建库/十三表、幂等迁移、独立测试种子/演示种子、`check-data`零异常；四个测试程序实际运行。
- 公共层测试包含16KiB长字符串/空值/空结果、参数绑定、事务提交与回滚/禁止嵌套、公共工具、会话超时，以及管理员登录失败计数、锁定和成功清零。
- 三端实际登录、待办未实现提示、业务桩菜单跳转及退出已通过冒烟测试。
- `make asan`实际执行测试与只读demo，开启AddressSanitizer/UndefinedBehaviorSanitizer及泄漏检查，最终通过；三端菜单另在同一sanitizer构建下通过。
- 上述不代表A/B/C业务实现、不代表52条SRS TC全通过；GitHub Actions的执行结果以仓库实际运行记录为准，分支保护尚待维护者配置。格式工具版本不同若产生纯格式差异，交D统一版本，不擅改业务逻辑。

## 协作与实现范围

A：M01/M03/M09、患者注册数据入口、患者待办；B：M04/M05/M07、模板与医生待办；C：M02/M06/M10、导出/管理员UI与待办；D：M08、common/db、main、todo框架、构建/scripts/CI/集成测试。每个任务对应一个FR，修改公共接口前须由维护者确认。

一个表一个owner；跨模块写用唯一入口，调用者管事务。接诊/爽约不放号；取消只放一次；开方扣库存并记流水整体回滚；作废只零变化留痕，不返库存。关键口径矛盾先交D裁定。

核心演示目标：注册→查排班→挂号占号→接诊→开方扣库存→写病历→患者查询，再演示改期/爽约/模板/预警/待办。当前`make demo`/`bash scripts/demo.sh`只做只读数据检查并列出待实现演示清单，明确不是上述业务链路的端到端演示。

## 限制与常见问题

- Windows本机不保证具备Linux POSIX/MySQL客户端环境；用WSL2或Linux验证，不伪造本机通过。
- 缺v1.0使基线结构/DDL暂不能核实；无目标TC实现就不能关闭FR Issue。
- 数据库连接失败先查DB_HOST/PORT/USER/PASS/NAME与服务健康；容器端口占用需人工协调，不顺手改接口或私开替代服务。
- 不做收费医保、Web/移动端、多线程、复杂算法、生产密码安全与压测。**课程环境下未做性能测试**，不声明10万行500ms或10并发达标。
- 当前版本为开发包骨架；发布tag、CI通过和业务验收均需实际验证记录。
