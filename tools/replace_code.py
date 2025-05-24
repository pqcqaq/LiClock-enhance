import re
from pathlib import Path

src_dir = Path("src")

var_pattern = re.compile(
    r"(//\s*w\d+h\d+\s*)?"                                      # 可选尺寸注释
    r"(?:(extern|static)\s+)?"                                  # 可选 extern 或 static
    r"(const\s+(?:unsigned\s+char|uint8_t))\s+"                 # 类型声明：const unsigned char 或 const uint8_t
    r"(\w+)\s*\[\]\s*=\s*\{",                                   # 变量名 + 数组 + 初始化开始
    re.MULTILINE
)

# 找变量定义块，用来删除：从var_pattern匹配末尾的 '{' 开始，到对应的匹配 '}' 结束
def remove_variable_definition(content, start_pos):
    if start_pos < 0 or start_pos >= len(content):
        return None
    brace_count = 0
    i = start_pos
    while i < len(content):
        if content[i] == '{':
            brace_count += 1
        elif content[i] == '}':
            brace_count -= 1
            if brace_count == 0:
                return i + 1
        i += 1
    return None

include_line = '#include "images/images.h"'

def process_file(path: Path):
    content = path.read_text(encoding="utf-8")
        # 如果文件中出现Auto-generated则跳过
    if "Auto-generated" in content:
        return None 
    original_content = content

    # 1. 添加 include 指令（如果不存在）
    includes = list(re.finditer(r'^\s*#include\s+[<"].+[>"].*$', content, re.MULTILINE))
    if includes:
        last_include = includes[-1]
        insert_pos = last_include.end()
        if include_line not in content:
            content = content[:insert_pos] + "\n" + include_line + content[insert_pos:]
    else:
        # 文件没有任何include，就插入到开头
        if include_line not in content:
            content = include_line + "\n" + content

    # 2. 删除变量定义
    offset = 0  # 删除内容导致的位置偏移
    for match in var_pattern.finditer(content):
        start = match.start() + offset
        init_start = match.end(0) - 1 + offset  # 退回到 '{' 位置
        end_pos = remove_variable_definition(content, init_start)
        if end_pos is None:
            continue
        content = content[:start] + content[end_pos:]
        offset -= (end_pos - start)

    # 3. 替换变量名,数字宽高 为 变量名,变量名_width,变量名_height
    # 目标形式类似：variable_name,12,12
    # 先写个正则，匹配形如：变量名,数字,数字，数字可以是多位整数
    pattern_replace = re.compile(r"(\b\w+_bits)\s*,\s*\d+\s*,\s*\d+")
    content = pattern_replace.sub(lambda m: f"{m.group(1)},{m.group(1)}_width,{m.group(1)}_height", content)

    # 写回文件
    if content != original_content:
        path.write_text(content, encoding="utf-8")
        print(f"[修改] {path}")

def main():
    for file_path in src_dir.rglob("*"):
        if file_path.suffix in (".cpp", ".h"):
            process_file(file_path)

if __name__ == "__main__":
    main()
