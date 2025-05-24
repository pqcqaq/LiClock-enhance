import re
from pathlib import Path
from PIL import Image
import shutil

# --- 配置 ---
SRC_DIR = Path("src")
IMAGES_ROOT_DIR = Path("images") # 存储所有图片子文件夹的根目录
IMAGES_H_PATH = IMAGES_ROOT_DIR / "images.h"
# -----------------

print("🚀 开始处理图像二进制定义和源代码更新...")

def extract_size_info(file_content: str) -> dict:
    """
    从文件内容中提取尺寸信息。
    支持两种模式：
    1. `name, width, height`
    2. `image = name` (默认为 32x32)
    """
    size_info = {}
    size_pattern = re.compile(r"(\w+),\s*(\d+),\s*(\d+)")
    app_icon_pattern = re.compile(r'image\s*=\s*(\w+)\s*;')

    matches = size_pattern.findall(file_content)
    for name, w, h in matches:
        size_info[name] = {'width': int(w), 'height': int(h)}

    icon_matches = app_icon_pattern.findall(file_content)
    for name in icon_matches:
        if name not in size_info:
            size_info[name] = {'width': 32, 'height': 32} # 默认 32x32

    return size_info

def extract_variable_block(content: str, start_index: int) -> str | None:
    """
    从给定的起始索引开始提取C数组的大括号内容。
    """
    brace_count = 0
    end_index = start_index
    while end_index < len(content):
        if content[end_index] == '{':
            brace_count += 1
        elif content[end_index] == '}':
            brace_count -= 1
            if brace_count == 0:
                # 找到匹配的闭合大括号，返回从开始到闭合大括号（包括）的完整块
                return content[start_index:end_index + 1]
        end_index += 1
    return None

def process_file(file_path: Path, all_size_info: dict, extracted_bmp_data: dict):
    """
    处理单个 .cpp 或 .h 文件：
    1. 查找并提取二进制图片数据。
    2. 在 images/file_stem 文件夹中生成 BMP 文件，并去除 _bits 后缀。
    3. 从源代码中删除定义。
    4. 替换引用。
    """
    if "Auto-generated" in file_path.read_text(encoding="utf-8", errors="ignore"):
        print(f"⏩ 跳过自动生成文件: {file_path}")
        return False

    original_content = file_path.read_text(encoding="utf-8", errors="ignore")
    modified_content = original_content
    file_modified = False

    # 为当前文件创建对应的图片子文件夹
    file_image_dir = IMAGES_ROOT_DIR / file_path.stem
    file_image_dir.mkdir(parents=True, exist_ok=True)

    # 正则表达式
    # 匹配 `// wXXhYY` 注释和 `extern const uint8_t name[] = {...};` 这种定义
    var_declaration_pattern = re.compile(
        r'(?P<comment>//\s*w(\d+)h(\d+)\s*)?'  # 可选尺寸注释
        r'(?:(extern|static)\s*)?'             # 可选 extern 或 static
        r'(const\s+(?:unsigned\s+char|uint8_t))\s+' # 类型声明
        r'(?P<name>\w+)\s*\[\]\s*=\s*\{'       # 变量名 + 数组 + 初始化开始
    )
    hex_data_pattern = re.compile(r"0x[0-9a-fA-F]{2}")

    # 倒序遍历匹配，以便删除内容时不会影响后续匹配的索引
    matches_to_process = list(var_declaration_pattern.finditer(modified_content))
    matches_to_process.reverse()

    for match in matches_to_process:
        comment_group = match.group('comment')
        var_name = match.group('name')
        declaration_start = match.start(0)
        var_block_start = match.end(0) - 1 # '{' 的起始位置

        full_declaration_block = extract_variable_block(modified_content, declaration_start)
        if not full_declaration_block:
            continue

        var_block_content = extract_variable_block(modified_content, var_block_start)
        if not var_block_content:
            continue

        width, height = None, None
        if comment_group:
            dim_match = re.search(r'w(\d+)h(\d+)', comment_group)
            if dim_match:
                width, height = int(dim_match.group(1)), int(dim_match.group(2))
        
        # 如果注释中没有尺寸信息，尝试从 all_size_info 中获取
        if not (width and height) and var_name in all_size_info:
            width = all_size_info[var_name]['width']
            height = all_size_info[var_name]['height']

        if width and height:
            hex_bytes = hex_data_pattern.findall(var_block_content)
            byte_values = [int(h, 16) for h in hex_bytes]
            
            # --- 直接生成 BMP 文件，处理文件名和路径 ---
            # 去除 _bits 后缀
            clean_var_name = var_name.replace("_bits", "")
            bmp_filename = f"{clean_var_name}.bmp"
            bmp_filepath = file_image_dir / bmp_filename
            
            # 确保父目录存在
            bmp_filepath.parent.mkdir(parents=True, exist_ok=True)

            try:
                # XBM 数据通常是单色位图，PIL 可以直接从字节数据创建 '1' 模式图像
                img = Image.frombytes('1', (width, height), bytes(byte_values), 'raw', '1;IR', 0, 1)
                img.save(bmp_filepath)
                # 记录原始变量名及其尺寸，用于生成 images.h
                extracted_bmp_data[var_name] = {'width': width, 'height': height}
                print(f"🖼️ 已生成 BMP 文件: {bmp_filepath}")
            except Exception as e:
                print(f"⚠️ 无法为 {var_name} ({width}x{height}) 生成 BMP 文件到 {bmp_filepath}: {e}")
                continue # 如果生成失败，不删除源代码定义

            # 从源代码中删除二进制图片定义
            # 这里的切片操作需要注意 match.end() 是 '{' 之后的索引
            # 所以我们需要从 match.start() 到 full_declaration_block 的结束
            end_of_block = match.start() + len(full_declaration_block)
            modified_content = modified_content[:match.start()] + modified_content[end_of_block:]
            file_modified = True
            print(f"🗑️ 已从 {file_path} 中删除定义: {var_name}")
        else:
            print(f"⚠️ 变量 {var_name} 在 {file_path} 中缺少长宽信息，无法生成 BMP。")

    # 替换引用：battery_empty_bits,12,12 -> battery_empty_bits,battery_empty_bits_width,battery_empty_bits_height
    # 注意：这里我们假设在 all_size_info 中已经有了替换所需的所有尺寸信息
    for name, dims in all_size_info.items():
        # 创建更精确的正则表达式以避免误替换
        # 匹配 `name, 数字, 数字`，并确保 `name` 是一个完整的单词
        replace_pattern = re.compile(rf'\b{re.escape(name)}\s*,\s*(\d+)\s*,\s*(\d+)\b')
        replacement_string = rf'{re.escape(name)},{re.escape(name)}_width,{re.escape(name)}_height'
        
        # 只有当实际匹配到旧格式时才进行替换
        if replace_pattern.search(modified_content):
            modified_content = replace_pattern.sub(replacement_string, modified_content)
            file_modified = True
            print(f"🔄 已在 {file_path} 中替换引用: {name}")

    # 在 include 的最下面添加 #include "images/images.h"
    # 这里我们统一使用 images/images.h，因为所有的宏定义都会在这个文件中
    if file_modified and "#include \"images/images.h\"" not in modified_content:
        # 查找最后一个 include 语句
        last_include_match = None
        # 搜索所有以 #include 开头的行
        for include_match in re.finditer(r'^#include\s+["<].*?[">]', modified_content, re.MULTILINE):
            last_include_match = include_match

        if last_include_match:
            # 在最后一个 include 后面插入
            insert_index = last_include_match.end()
            # 确保新 include 语句在新的一行，并且与前一个 include 有一个空行隔开（可选，为了格式美观）
            # 判断插入点后是否有换行，如果没有则添加
            if modified_content[insert_index:insert_index+1] != '\n':
                modified_content = modified_content[:insert_index] + "\n" + "#include \"images/images.h\"\n" + modified_content[insert_index:]
            else:
                modified_content = modified_content[:insert_index] + "\n#include \"images/images.h\"\n" + modified_content[insert_index:]
            print(f"➕ 已在 {file_path} 中添加 include 语句。")
        else:
            # 如果文件中没有 include 语句，就在文件顶部添加
            modified_content = "#include \"images/images.h\"\n\n" + modified_content
            print(f"➕ 已在 {file_path} 顶部添加 include 语句。")

    if file_modified:
        file_path.write_text(modified_content, encoding="utf-8")
        print(f"✅ 文件已更新: {file_path}")
    return file_modified

def generate_images_h(bmp_data: dict):
    """
    根据生成的 BMP 文件信息创建 images/images.h 文件。
    """
    header_content = """#ifndef IMAGES_H
#define IMAGES_H

#include <stdint.h> // for uint8_t

// Auto-generated by script. Do not modify directly.

"""
    for name, dims in bmp_data.items():
        header_content += f"#define {name}_width {dims['width']}\n"
        header_content += f"#define {name}_height {dims['height']}\n"
        # 声明 extern const uint8_t name_bits[];
        header_content += f"extern const uint8_t {name}_bits[];\n\n"

    header_content += "#endif // IMAGES_H\n"

    IMAGES_ROOT_DIR.mkdir(parents=True, exist_ok=True)
    IMAGES_H_PATH.write_text(header_content, encoding="utf-8")
    print(f"\n🎉 已生成 {IMAGES_H_PATH}")


# --- 主程序流程 ---
if not SRC_DIR.is_dir():
    print(f"错误: 源代码目录 '{SRC_DIR}' 不存在。请确保脚本在项目根目录下运行。")
else:
    source_files = list(SRC_DIR.rglob("*.cpp")) + list(SRC_DIR.rglob("*.h"))
    if not source_files:
        print(f"在 '{SRC_DIR}' 目录中没有找到 .cpp 或 .h 文件。")
    else:
        print(f"📄 共找到 {len(source_files)} 个源文件。")

        # 第一遍扫描，收集所有尺寸信息
        print("\n🔍 第一遍扫描：收集所有尺寸信息...")
        all_size_info = {}
        for file in source_files:
            try:
                content = file.read_text(encoding="utf-8", errors="ignore")
                file_size_info = extract_size_info(content)
                all_size_info.update(file_size_info)
                if file_size_info:
                    print(f"✅ 从 {file} 中提取到 {len(file_size_info)} 条尺寸信息。")
            except Exception as e:
                print(f"⚠️ 读取文件 {file} 时出错：{e}")
        print(f"📦 共提取尺寸信息 {len(all_size_info)} 条。")

        # 第二遍扫描，处理文件并生成 BMP
        print("\n⚙️ 第二遍扫描：处理文件，生成 BMP，并修改源代码...")
        modified_files_count = 0
        extracted_bmp_data = {} # 存储已成功提取和生成 BMP 的数据，用于生成 images.h

        for file in source_files:
            try:
                if process_file(file, all_size_info, extracted_bmp_data):
                    modified_files_count += 1
            except Exception as e:
                print(f"⚠️ 处理文件 {file} 时出错：{e}")

        print(f"\n🎉 处理完成，共修改了 {modified_files_count} 个文件。")
        print(f"🖼️ 共生成了 {len(extracted_bmp_data)} 张 BMP 图片。")

        # 生成 images.h 文件
        generate_images_h(extracted_bmp_data)

print("\n✨ 所有任务完成。")