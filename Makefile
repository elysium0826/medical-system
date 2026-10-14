# Linux C / MySQL 8 开发骨架（owner: D）
.DEFAULT_GOAL := all
CC := gcc
AR := ar
MYSQL_CONFIG ?= mysql_config
MYSQL_CFLAGS ?= $(shell $(MYSQL_CONFIG) --cflags 2>/dev/null)
MYSQL_LIBS ?= $(shell $(MYSQL_CONFIG) --libs 2>/dev/null)
ifeq ($(strip $(MYSQL_LIBS)),)
MYSQL_LIBS := -lmysqlclient
endif
CPPFLAGS := -Iinclude $(MYSQL_CFLAGS) -D_POSIX_C_SOURCE=200809L
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -g
LDFLAGS ?=
SOURCES := $(wildcard src/*.c)
TEST_SOURCES := $(wildcard tests/test_*.c)
HEADERS := $(wildcard include/*.h)
COMMON_SRC := src/common.c
DB_SRC := src/db.c src/db_mysql.c
APP_SRC := $(filter-out $(COMMON_SRC) $(DB_SRC),$(SOURCES))
APP_OBJS := $(patsubst src/%.c,obj/%.o,$(APP_SRC))
DB_OBJS := $(patsubst src/%.c,obj/%.pic.o,$(DB_SRC))
TEST_BINS := $(patsubst tests/%.c,obj/%,$(TEST_SOURCES))
TEST_LINK_OBJS := $(filter-out obj/main.o,$(APP_OBJS)) obj/app_main.o obj/harness.o
CLANG_FORMAT ?= clang-format

.PHONY: all lib test lint format format-check asan db-init db-up db-down db-migrate \
        db-seed db-reset check-data demo clean distclean help
all: medical_system
lib: lib/libcommon.a lib/libdb.so

obj/.stamp lib/.stamp data/.stamp:
	mkdir -p $(@D)
	touch $@
obj/%.o: src/%.c $(HEADERS) | obj/.stamp
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@
obj/%.pic.o: src/%.c $(HEADERS) | obj/.stamp
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -MMD -MP -c $< -o $@
obj/app_main.o: src/main.c $(HEADERS) | obj/.stamp
	$(CC) $(CPPFLAGS) $(CFLAGS) -Dmain=medical_app_main -c $< -o $@
obj/harness.o: tests/harness.c $(HEADERS) | obj/.stamp
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
lib/libcommon.a: obj/common.o | lib/.stamp
	$(AR) rcs $@ $^
lib/libdb.so: $(DB_OBJS) | lib/.stamp
	$(CC) -shared $(LDFLAGS) -o $@ $^ $(MYSQL_LIBS)
medical_system: $(APP_OBJS) lib/libcommon.a lib/libdb.so
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJS) -Llib -lcommon -ldb $(MYSQL_LIBS) \
	    -Wl,-rpath,'$$ORIGIN/lib'
obj/test_%: tests/test_%.c $(TEST_LINK_OBJS) lib/libcommon.a lib/libdb.so
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $< $(TEST_LINK_OBJS) -Llib \
	    -lcommon -ldb $(MYSQL_LIBS) -Wl,-rpath,'$$ORIGIN/../lib' -o $@
test: $(TEST_BINS)
	bash tests/run_all.sh
lint:
	bash scripts/lint.sh
format:
	$(CLANG_FORMAT) -i $(SOURCES) $(HEADERS) $(wildcard tests/*.c)
format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES) $(HEADERS) $(wildcard tests/*.c)
asan:
	$(MAKE) clean
	$(MAKE) CFLAGS='-std=c99 -Wall -Wextra -Werror -g -fsanitize=address,undefined -fno-omit-frame-pointer' \
	    LDFLAGS='-fsanitize=address,undefined' all test
	ASAN_OPTIONS=detect_leaks=1 bash scripts/demo.sh

db-init:
	bash scripts/db.sh init
db-up:
	@if command -v docker >/dev/null 2>&1; then docker compose up -d; \
	else echo '未安装 Docker，跳过可选数据库容器'; fi
db-down:
	@if command -v docker >/dev/null 2>&1; then docker compose down; \
	else echo '未安装 Docker，跳过可选数据库容器'; fi
db-migrate:
	bash scripts/db.sh migrate
db-seed:
	@if [ "$${DB_NAME:-medical_db}" = medical_test ]; then bash scripts/db.sh seed-test; \
	else bash scripts/db.sh seed; fi
db-reset:
	bash scripts/db.sh reset
	$(MAKE) db-init db-migrate db-seed
check-data:
	bash scripts/db.sh check
demo: all
	bash scripts/demo.sh
clean:
	rm -f obj/*.o obj/*.d obj/test_* lib/*.a lib/*.so medical_system
distclean: clean
	rm -f data/*.csv data/*.txt
help:
	@printf '%s\n' 'make/lib/test/lint/format/format-check/asan' \
	'db-init/db-up/db-down/db-migrate/db-seed/db-reset/check-data/demo/clean/distclean/help' \
	'业务为未实现桩；RUN_DB_TESTS=1 DB_NAME=medical_test make test 启用独立测试库验证。'

-include $(wildcard obj/*.d)
