import os
import re
from pathlib import Path
from PIL import Image

output_dir = Path("images")
output_dir.mkdir(exist_ok=True)

missing_dims = []

# 正则表达式
dim_pattern = re.compile(r"//\s*w(\d+)h(\d+)", re.IGNORECASE)
var_pattern = re.compile(
    r"(//\s*w\d+h\d+\s*)?"                                  # 可选尺寸注释
    r"(static\s+const\s+unsigned\s+char\s+(\w+)\[\]\s*=\s*\{)",  # 变量声明开头
    re.MULTILINE)

hex_data_pattern = re.compile(r"0x[0-9a-fA-F]{2}")

def extract_variable_block(content, start_index):
    # 如果文件中出现Auto-generated则跳过
    if "Auto-generated" in content:
        return None 
    brace_count = 0
    end_index = start_index
    while end_index < len(content):
        if content[end_index] == '{':
            brace_count += 1
        elif content[end_index] == '}':
            brace_count -= 1
            if brace_count == 0:
                return content[start_index:end_index+1]
        end_index += 1
    return None

def cpp_to_bmp(folder):
    # 支持cpp和h文件
    cpp_files = [f for f in Path(folder).rglob("*") if f.suffix in [".cpp", ".h"]]
    count = 0

    for cpp_path in cpp_files:
        with open(cpp_path, "r", encoding="utf-8") as f:
            content = f.read()

        for match in var_pattern.finditer(content):
            full_match = match.group(0)
            comment = match.group(1)
            var_name = match.group(3)
            var_start = match.end(2)
            var_block = extract_variable_block(content, var_start)

            if not var_block:
                continue

            hex_bytes = hex_data_pattern.findall(var_block)
            byte_values = [int(h, 16) for h in hex_bytes]
            byte_data = bytes(byte_values)

            if comment:
                dim_match = dim_pattern.match(comment.strip())
                if dim_match:
                    width, height = int(dim_match.group(1)), int(dim_match.group(2))
                else:
                    missing_dims.append((cpp_path, var_name))
                    continue
            else:
                missing_dims.append((cpp_path, var_name))
                continue

            # 构建 XBM 文件内容
            xbm_name = f"{cpp_path.stem}_{var_name}"
            xbm_text = f"#define {xbm_name}_width {width}\n"
            xbm_text += f"#define {xbm_name}_height {height}\n"
            xbm_text += f"static char {xbm_name}_bits[] = {{\n"
            for i in range(0, len(byte_values), 12):
                xbm_text += "   " + ", ".join(f"0x{b:02x}" for b in byte_values[i:i+12]) + ",\n"
            xbm_text = xbm_text.rstrip(",\n") + "\n};\n"

            xbm_path = output_dir / f"{xbm_name}.xbm"
            bmp_path = output_dir / f"{xbm_name}.bmp"

            with open(xbm_path, "w") as f:
                f.write(xbm_text)

            try:
                img = Image.open(xbm_path)
                img.save(bmp_path)
                count += 1
            except Exception as e:
                print(f"[Error] Failed to convert {xbm_path.name}: {e}")

    print(f"\n✅ 总共生成了 {count} 张 BMP 图片，保存在 {output_dir.resolve()}")

    if missing_dims:
        print(f"\n⚠️ 以下变量缺少尺寸注释：")
        for path, name in missing_dims:
            print(f" - 文件: {path} 中变量名: {name}")

if __name__ == "__main__":
    target_dir = input("请输入要扫描的目录路径：").strip()
    if not os.path.isdir(target_dir):
        print("无效目录！")
    else:
        cpp_to_bmp(target_dir)
