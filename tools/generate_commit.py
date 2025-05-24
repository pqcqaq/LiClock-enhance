import re
from pathlib import Path

print("🔍 开始扫描 src 目录下的 .cpp 和 .h 文件...")

# 递归扫描 src 目录下的所有 .cpp 和 .h 文件
source_files = list(Path("src").rglob("*.cpp")) + list(Path("src").rglob("*.h"))
print(f"📄 共找到 {len(source_files)} 个源文件。")

# 步骤 1：提取所有尺寸信息：格式如 defaultAppIcon, 32, 32
size_info = {}
size_pattern = re.compile(r"(\w+),\s*(\d+),\s*(\d+)")
# 在提取尺寸信息的部分之后添加新的匹配逻辑
app_icon_pattern = re.compile(r'image\s*=\s*(\w+)\s*;')

for file in source_files:
    if "Auto-generated" in file.read_text(encoding="utf-8", errors="ignore"):
        continue
    try:
        text = file.read_text(encoding="utf-8", errors="ignore")
        
        # 原有尺寸匹配
        matches = size_pattern.findall(text)
        for name, w, h in matches:
            size_info[name] = f"// w{w}h{h}"

        # 新增 app icon 匹配，分辨率默认为 32x32
        icon_matches = app_icon_pattern.findall(text)
        for name in icon_matches:
            if name not in size_info:
                size_info[name] = "// w32h32"

        if matches or icon_matches:
            print(f"✅ 在文件 {file} 中提取了 {len(matches) + len(icon_matches)} 个尺寸信息。")
    except Exception as e:
        print(f"⚠️ 读取文件 {file} 时出错：{e}")

print(f"📦 共提取尺寸信息 {len(size_info)} 条。\n")

# 步骤 2：匹配 extern const uint8_t xxx[] = {...}; 并插入注释
extern_pattern = re.compile(
    r'((?:extern\s+|static\s+|extern\s+static\s+|static\s+extern\s+)?const\s+uint8_t\s+(\w+)\s*\[\]\s*=\s*\{[\s\S]*?\};)',
    re.DOTALL
)

def insert_comments(content: str, filename: str) -> str:
    modified = False

    def replacer(match):
        nonlocal modified
        full_decl = match.group(1)
        name = match.group(2)
        print(f"🔎 检查到声明：{name}")
        comment = size_info.get(name)
        if comment:
            if comment not in content[:match.start()]:
                print(f"📝 为 {filename} 中的图标 `{name}` 插入注释：{comment}")
                modified = True
                return f"{comment}\n{full_decl}"
        return full_decl

    new_content = extern_pattern.sub(replacer, content)
    print(f"🔄 文件 {filename} 处理完成")
    return new_content if modified else content

# 步骤 3：处理每个文件
modified_files = 0
for file in source_files:
    try:
        content = file.read_text(encoding="utf-8", errors="ignore")
        new_content = insert_comments(content, str(file))
        if new_content != content:
            file.write_text(new_content, encoding="utf-8")
            print(f"✅ 文件已更新：{file}")
            modified_files += 1
    except Exception as e:
        print(f"⚠️ 处理文件 {file} 时出错：{e}")

print(f"\n🎉 处理完成，共修改了 {modified_files} 个文件。")
