import os
from pathlib import Path
import shutil

def normalize_and_group_images(image_dir="images"):
    image_dir = Path(image_dir)
    renamed_files = 0
    moved_files = 0
    prefix_removed = 0

    # Step 1: Rename *_bits.* -> *.*
    for file in image_dir.iterdir():
        if file.is_file() and "_bits" in file.stem:
            new_stem = file.stem.replace("_bits", "")
            new_path = file.with_name(new_stem + file.suffix)

            if not new_path.exists():
                file.rename(new_path)
                print(f"✅ 重命名: {file.name} -> {new_path.name}")
                renamed_files += 1

    # Step 2 & 3: Move by prefix and remove prefix in name
    for file in image_dir.iterdir():
        if file.is_file() and file.suffix.lower() == ".bmp":
            parts = file.stem.split('_', 1)
            if len(parts) == 2:
                prefix, rest = parts
            else:
                prefix = parts[0]
                rest = parts[0]

            target_dir = image_dir / prefix
            target_dir.mkdir(exist_ok=True)

            new_name = rest + file.suffix
            target_path = target_dir / new_name

            if not target_path.exists():
                shutil.move(str(file), target_path)
                print(f"📂 移动: {file.name} -> {target_dir.name}/{new_name}")
                moved_files += 1
                if new_name != file.name:
                    prefix_removed += 1
            else:
                print(f"⚠️ 已存在: {target_path}, 跳过")

    print(f"\n📊 总结:")
    print(f"✅ 重命名文件数: {renamed_files}")
    print(f"📁 移动文件数: {moved_files}")
    print(f"🔤 去除前缀文件数: {prefix_removed}")

if __name__ == "__main__":
    normalize_and_group_images()
