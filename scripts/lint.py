#!/usr/bin/env python3
"""源码静态门禁，不代替业务、SQL事务边界与权限人工评审。"""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
errors = []
# SRS 显式多参契约优先；export 接口是 D 的草案，待第0天审核。
param_exceptions = {'medicine_query_stock_log', 'export_table'}
tx_owners = {
    'patient_register', 'patient_set_status', 'patient_delete',
    'appointment_add', 'appointment_cancel', 'appointment_reschedule',
    'appointment_mark_noshow', 'visit_take', 'prescription_add', 'prescription_void',
    'schedule_add_week', 'schedule_set_quota', 'schedule_set_status',
    'medicine_check_stock', 'presc_template_add', 'presc_template_delete',
}


def issue(path, line, message):
    errors.append(f'{path.relative_to(root)}:{line}: {message}')


def masked(text):
    pattern = r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    return re.sub(pattern, lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]),
                  text, flags=re.S)


for path in sorted(list((root / 'src').glob('*.c')) +
                   list((root / 'include').glob('*.h')) + list((root / 'tests').glob('*.c'))):
    text = path.read_text(encoding='utf-8')
    clean = masked(text)
    lines = text.splitlines()
    if path.suffix == '.c' and len(lines) > 600:
        issue(path, 1, 'CR-10 文件超过600行')
    if not text.startswith('/*') or '作者：' not in text[:300]:
        issue(path, 1, 'CR-01 缺文件头/作者')
    if path.suffix == '.h' and not re.search(r'^#ifndef MED_\w+_H$', text, re.M):
        issue(path, 1, 'CR-13 include guard')
    for number, line in enumerate(lines, 1):
        if '\t' in line:
            issue(path, number, '禁止Tab')
        if len(line) > 100:
            issue(path, number, '行超过100字符')
        if re.search(r'\b(TODO|FIXME|XXX)\b|@(brief|param|return)\b', line):
            issue(path, number, 'CR-02/05 禁止过程标记或Doxygen标签')
    for match in re.finditer(r'\b(sprintf|strcpy|strcat|gets|scanf)\s*\(', clean):
        issue(path, clean[:match.start()].count('\n') + 1, 'CR-20 禁止不安全函数')
    if path.parent.name == 'src' and path.name != 'common.c' and not path.name.startswith('ui_'):
        for match in re.finditer(r'\b(printf|vprintf)\s*\(', clean):
            issue(path, clean[:match.start()].count('\n') + 1, 'CR-20 非UI打印')
    if path.parent.name == 'src' and path.name != 'main.c' and re.search(r'\bexit\s*\(', clean):
        issue(path, 1, 'CR-20 非main退出进程')
    if re.search(r'\bSELECT\s+\*', text, re.I):
        issue(path, 1, 'CR-23 禁SELECT *')
    definition = r'\b([a-z][a-z0-9_]*)\s*\(([^;{}]*)\)\s*\{'
    for match in re.finditer(definition, clean):
        name, params = match.group(1), match.group(2)
        if name in {'if', 'for', 'while', 'switch'}:
            continue
        depth, end = 1, match.end()
        while end < len(clean) and depth:
            depth += (clean[end] == '{') - (clean[end] == '}')
            end += 1
        body = clean[match.end():end]
        number = clean[:match.start()].count('\n') + 1
        if sum(bool(line.strip()) for line in body.splitlines()) > 60:
            issue(path, number, f'CR-10 {name} 超过60代码行')
        if params.count(',') + 1 > 6 and name not in param_exceptions:
            issue(path, number, f'CR-10 {name} 超过6参数')
        if path.parent.name == 'src' and path.name != 'db.c':
            if re.search(r'\bdb_tx_(begin|commit|rollback)\s*\(', body) and name not in tx_owners:
                issue(path, number, f'CR-18 {name} 不在事务白名单')

if errors:
    print('\n'.join(errors), file=sys.stderr)
    sys.exit(1)
print('lint通过：禁用API/过程标记/guard/行长/文件与函数规模/参数/事务白名单。')
print('权限、SQL转义完整性、返回值检查、嵌套层数与业务一致性仍须人工评审。')
