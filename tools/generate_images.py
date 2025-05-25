import os
from pathlib import Path
from PIL import Image
from io import BytesIO

# 路径设置
IMAGE_DIR = Path("images")
SRC_DIR = Path("src/images")
JPG_DIR = IMAGE_DIR / "jpg"

# 确保源目录存在
SRC_DIR.mkdir(parents=True, exist_ok=True)

def convert_jpg_to_bmp():
    """将 JPG 文件转换为 BMP 文件"""
    print("--- 开始将 JPG 转换为 BMP ---")
    for jpg_file in JPG_DIR.glob("*.jp*g"):
        relative_path = jpg_file.relative_to(IMAGE_DIR)
        target_dir = IMAGE_DIR / relative_path.parent
        target_dir.mkdir(parents=True, exist_ok=True)

        bmp_file = target_dir / (jpg_file.stem + ".bmp")
        try:
            with Image.open(jpg_file) as img:
                img.convert("1").save(bmp_file, "BMP")
            print(f"[JPG->BMP] {jpg_file.name} 转换为 {bmp_file}")
        except Exception as e:
            print(f"❌ 转换失败：{jpg_file.name}，错误信息：{e}")

def bmp_to_xbm_data(bmp_path):
    """将 BMP 转换为 XBM 格式的数据"""
    with Image.open(bmp_path) as img:
        img_bw = img.convert("1")
        name = bmp_path.stem.replace("-", "_").replace(".", "_")
        width, height = img_bw.size
        var_name = f"{name}_bits"

        buffer = BytesIO()
        img_bw.save(buffer, "xbm")
        xbm_text = buffer.getvalue().decode("utf-8")

        hex_start = xbm_text.find("{")
        hex_end = xbm_text.find("};")
        hex_data = xbm_text[hex_start + 1:hex_end].strip()

        comment_wh = f"// w{width}h{height}"
        array_def = f"const uint8_t {var_name}[] = {{\n    {hex_data}\n}};"

        return name, var_name, width, height, comment_wh, array_def

def generate_struct_file():
    """生成 images_struct.h 头文件"""
    print("--- 正在生成 images_struct.h ---")
    struct_header_path = SRC_DIR / "images_struct.h"

    header_lines = [
        "// Auto-generated images_struct.h",
        "#ifndef IMAGES_STRUCT_H",
        "#define IMAGES_STRUCT_H",
        '#include <stdint.h>',
        '',
        """
typedef struct
{
    const uint8_t *data;
    uint16_t width;
    uint16_t height;
} image_desc;
        """,
        # "extern const image_desc images[];",
        # "extern const uint16_t images_count;",
        # "#define IMAGE_COUNT images_count", # 兼容旧代码
        "#endif // IMAGES_STRUCT_H"
    ]
    struct_header_path.write_text("\n".join(header_lines))
    print(f"📄 头文件已写入：{struct_header_path}")

def generate_subfolder_files():
    """为每个包含 BMP 图像的子文件夹生成头文件和源文件"""
    print("--- 正在生成子文件夹图像文件 ---")
    all_subfolder_image_arrays = []

    subfolders = set()
    for bmp_file in IMAGE_DIR.glob("**/*.bmp"):
        if JPG_DIR not in bmp_file.parents:
            subfolders.add(bmp_file.parent)

    for subfolder in sorted(list(subfolders)):
        folder_name = "root" if subfolder == IMAGE_DIR else subfolder.name.lower().replace("-", "_").replace(" ", "_")
        sub_header_path = SRC_DIR / f"{folder_name}.h"
        sub_source_path = SRC_DIR / f"{folder_name}.cpp"

        bmp_files_in_subfolder = sorted(list(subfolder.glob("*.bmp")), key=lambda x: x.stem)
        if not bmp_files_in_subfolder:
            continue

        header_lines = [
            f"// Auto-generated {folder_name}.h",
            f"#ifndef {folder_name.upper()}_IMAGES_H",
            f"#define {folder_name.upper()}_IMAGES_H",
            '#include "images_struct.h"',
            f'#include <stdint.h> // for uint8_t',
            '',
        ]
        cpp_lines = [
            f"// Auto-generated {folder_name}.cpp",
            f'#include "{folder_name}.h"',
            ''
        ]
        image_entries = []

        for bmp in bmp_files_in_subfolder:
            name, var_name, w, h, comment_wh, array_def = bmp_to_xbm_data(bmp)
            commit = f"// {comment_wh}, file: {bmp}"
            header_lines.append(commit)
            header_lines.append(f"extern const uint8_t {var_name}[];")
            header_lines.append(f"#define {name}_width {w}")
            header_lines.append(f"#define {name}_height {h}")
            header_lines.append(f"\n")
            cpp_lines.extend([commit, array_def, ""])
            image_entries.append(f"    {{{var_name}, {name}_width, {name}_height}},")

        header_lines.append(f"extern const image_desc {folder_name}_images[];")
        header_lines.append(f"#define {folder_name.upper()}_IMAGE_COUNT {len(image_entries)}")
        header_lines.append(f"#endif // {folder_name.upper()}_IMAGES_H")

        cpp_lines.append(f"const image_desc {folder_name}_images[] = {{")
        cpp_lines.extend(image_entries)
        cpp_lines.append("};")

        sub_header_path.write_text("\n".join(header_lines))
        sub_source_path.write_text("\n".join(cpp_lines))
        print(f"✅ 已生成：{sub_header_path} 和 {sub_source_path}")
        all_subfolder_image_arrays.append((folder_name, len(image_entries)))

    return all_subfolder_image_arrays

def generate_main_image_files(subfolder_data):
    """生成 images.h 和 images.cpp 总入口文件"""
    print("--- 正在生成主图像文件（images.h/cpp）---")
    main_header_path = SRC_DIR / "images.h"
    main_source_path = SRC_DIR / "images.cpp"

    header_includes = []
    header_externs = []
    main_image_entries = []

    main_header_lines = [
        "// Auto-generated images.h",
        "#ifndef IMAGES_H",
        "#define IMAGES_H",
        '#include "images_struct.h"',
        f'#include <stdint.h> // for uint8_t',
        "",
    ]

    main_cpp_lines = [
        "// Auto-generated images.cpp",
        ''
    ]

    for folder_name, count in subfolder_data:
        header_includes.append(f'#include "{folder_name}.h"')
        header_externs.append(f"extern const image_desc {folder_name}_images[];")
        header_externs.append(f"#define {folder_name.upper()}_IMAGE_COUNT {count}")
        header_externs.append("")
        main_cpp_lines.append(f'#include "{folder_name}.h"')

    main_header_lines.extend(header_includes)
    main_header_lines.append("")
    main_header_lines.extend(header_externs)
    main_header_lines.append("#endif // IMAGES_H")

    main_cpp_lines.append("")

    all_bmp_files = sorted(list(IMAGE_DIR.glob("**/*.bmp")), key=lambda x: x.stem)
    all_bmp_files = [f for f in all_bmp_files if JPG_DIR not in f.parents]

    for bmp in all_bmp_files:
        name = bmp.stem.replace("-", "_").replace(".", "_")
        var_name = f"{name}_bits"
        main_image_entries.append(f"    {{{var_name}, {name}_width, {name}_height}},")

    main_cpp_lines.append("\nconst image_desc images[] = {")
    main_cpp_lines.extend(main_image_entries)
    main_cpp_lines.append("};")
    main_cpp_lines.append(f"const uint16_t images_count = {len(main_image_entries)};")

    if main_image_entries:
        main_header_lines.insert(-2, "\nextern const image_desc images[];")
        main_header_lines.insert(-2, "extern const uint16_t images_count;")
        main_header_lines.insert(-2, f"#define IMAGE_COUNT {len(main_image_entries)}")

    main_header_path.write_text("\n".join(main_header_lines))
    main_source_path.write_text("\n".join(main_cpp_lines))

    print(f"📁 主头文件已写入：{main_header_path}")
    print(f"📄 主源文件已写入：{main_source_path}")
    print(f"📦 合并 images[] 总数：{len(main_image_entries)} 张图像")

def clean_up_files():
    """清理中间文件，如 XBM 和 JPG（如有需要）"""
    print("--- 正在清理临时文件 ---")

    # 如果需要删除 JPG 源文件，可以取消以下注释
    # for jpg_file in JPG_DIR.glob("*.jp*g"):
    #     try:
    #         jpg_file.unlink()
    #         print(f"🗑️ 已删除 JPG：{jpg_file.name}")
    #     except Exception as e:
    #         print(f"⚠️ 删除失败 {jpg_file.name}：{e}")

    print("✅ 清理完成。")

if __name__ == "__main__":
    convert_jpg_to_bmp()
    generate_struct_file()
    subfolder_data = generate_subfolder_files()
    generate_main_image_files(subfolder_data)
    clean_up_files()
    print("\n🎉 所有操作已成功完成！")
