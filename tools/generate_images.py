import os
from pathlib import Path
from PIL import Image
from io import BytesIO

# Paths
IMAGE_DIR = Path("images")
SRC_DIR = Path("src/images")
JPG_DIR = IMAGE_DIR / "jpg"
HEADER_PATH = SRC_DIR / "images.h"
SOURCE_PATH = SRC_DIR / "images.cpp"

# STRUCT_DEF = "typedef struct {\n    const unsigned char* data;\n    int width;\n    int height;\n}} image_desc;\n"

def convert_jpg_to_bmp():
    """Convert JPG files in images/jpg to BMP in images/"""
    for jpg_file in JPG_DIR.glob("*.jp*g"):
        bmp_file = IMAGE_DIR / (jpg_file.stem + ".bmp")
        with Image.open(jpg_file) as img:
            img.convert("1").save(bmp_file, "BMP")
        print(f"[JPG->BMP] {jpg_file.name} -> {bmp_file.name}")

def bmp_to_xbm_data(bmp_path):
    """Convert BMP to XBM data components"""
    with Image.open(bmp_path) as img:
        img_bw = img.convert("1")
        name = bmp_path.stem.replace("-", "_").replace(".", "_")
        width, height = img_bw.size
        var_name = f"{name}_bits"

        # Write to in-memory .xbm file
        buffer = BytesIO()
        img_bw.save(buffer, "xbm")
        xbm_text = buffer.getvalue().decode("utf-8")

        # Extract hex array from the XBM
        hex_start = xbm_text.find("{")
        hex_end = xbm_text.find("};")
        hex_data = xbm_text[hex_start + 1:hex_end].strip()

        # Clean up
        define_w = f"#define {var_name}_width {width}"
        define_h = f"#define {var_name}_height {height}"
        commit_wh = f"// w{width}h{height} name:{name}"
        array_def = f"const unsigned char {var_name}[] = {{\n    {hex_data}\n}};"

        return var_name, width, height, define_w, define_h, commit_wh, array_def

def generate_cpp_and_header():
    # 这里要从除了jpg的目录中读取bmp文件
    bmp_files = sorted(IMAGE_DIR.glob("**/*.bmp"), key=lambda x: x.stem)
    if not bmp_files:
        print("❌ No BMP files found.")
        return

    header_lines = [
        "// Auto-generated images.h",
        '#include <A_Config.h>',
        "",
        # STRUCT_DEF,
        ""
    ]
    cpp_lines = [
        "// Auto-generated images.cpp",
        '#include "images.h"',
        ""
    ]
    image_entries = []

    for bmp in bmp_files:
        var_name, w, h, define_w, define_h, commit_wh, array_def = bmp_to_xbm_data(bmp)

        # 完整路径，文件名等信息
        header_lines.append(f"// {commit_wh}, file: {bmp}")
        header_lines.append(f"extern const unsigned char {var_name}[];")
        header_lines.append(f"#define {var_name}_width {w}")
        header_lines.append(f"#define {var_name}_height {h}")
        header_lines.append(f"\n")
        cpp_lines.extend([commit_wh, array_def, ""])
        image_entries.append(f"    {{{var_name}, {w}, {h}}},")

    header_lines.append("")
    header_lines.append("extern const image_desc images[];")
    header_lines.append(f"#define IMAGE_COUNT {len(image_entries)}")

    cpp_lines.append("const image_desc images[] = {")
    cpp_lines.extend(image_entries)
    cpp_lines.append("};")

    HEADER_PATH.write_text("\n".join(header_lines))
    SOURCE_PATH.write_text("\n".join(cpp_lines))

    print(f"✅ Header written to {HEADER_PATH}")
    print(f"✅ Source written to {SOURCE_PATH}")
    print(f"📦 Total images: {len(image_entries)}")

def delete_xbm_files():
    """Delete all XBM files in the images directory."""
    xbm_files = list(IMAGE_DIR.glob("*.xbm"))
    for xbm_file in xbm_files:
        try:
            xbm_file.unlink()
        except Exception as e:
            print(f"Error deleting {xbm_file.name}: {e}")
if __name__ == "__main__":
    convert_jpg_to_bmp()
    generate_cpp_and_header()
    delete_xbm_files()
    print("✅ All operations completed successfully.")
