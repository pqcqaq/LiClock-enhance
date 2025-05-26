import re
from pathlib import Path
from datetime import datetime

# ------------ 生成函数注册区（你可以自定义） ------------
def gen_timestamp():
    # 要是时分秒
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    return f"#define built_at \"{now}\""

def gen_build_user():
    return '#define BUILD_USER "dev_user"'

def gen_build_version():
    return '#define code_version "2.0.11"'

GENERATORS = {
    "timestamp": gen_timestamp,
    "user": gen_build_user,
    "version": gen_build_version,
    # 添加更多生成器...
}

# ------------ 替换记录 ------------
replacements = []


# ------------ 核心替换逻辑 ------------
def replace_lines(file_path: Path):
    lines = file_path.read_text(encoding="utf-8").splitlines()
    updated = False
    i = 0

    while i < len(lines):
        line = lines[i].strip()
        match = re.match(r"//\s*pre-build-auto-generated:\s*(\w+)", line)
        if match:
            key = match.group(1)
            if key in GENERATORS and i + 1 < len(lines):
                original_line = lines[i + 1].strip()
                new_line = GENERATORS[key]()  # 直接替换为生成器返回的整行
                lines[i + 1] = new_line
                replacements.append((str(file_path), key, original_line, new_line))
                updated = True
                i += 1  # skip next line
        i += 1

    if updated:
        file_path.write_text("\n".join(lines), encoding="utf-8")

# ------------ 主程序 ------------
def main():
    src_dir = Path("include")
    print("🚀 开始自动替换生成的变量...")
    if not src_dir.exists():
        print("❌ 源代码目录不存在，请检查路径。")
        return
    cpp_h_files = list(src_dir.rglob("*.cpp")) + list(src_dir.rglob("*.h"))
    print(f"📂 共找到 {len(cpp_h_files)} 个源文件。")
    for file_path in cpp_h_files:
        replace_lines(file_path)

    # ------------ 输出替换信息 ------------
    print("\n📦 替换统计：")
    if not replacements:
        print("无替换内容")
        return

    for file, key, old, new in replacements:
        print(f"文件: {file}\n  🔧 键: {key}\n  ⏪ 原: {old}\n  ✅ 新: {new}\n")

if __name__ == "__main__":
    main()
