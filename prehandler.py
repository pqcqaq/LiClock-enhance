import os
import importlib.util

def load_module_from_file(name, relative_path):
    project_root = os.getcwd()
    module_path = os.path.join(project_root, relative_path)
    spec = importlib.util.spec_from_file_location(name, module_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module

print("\n>>> before_build: loading tools modules")

# 动态加载每个模块
generate_images = load_module_from_file("generate_images", "tools/generate_images.py")
normalize_image_name = load_module_from_file("normalize_image_name", "tools/normalize_image_name.py")
auto_replace_generated_vars = load_module_from_file("auto_replace_generated_vars", "tools/auto_replace_generated_vars.py")

# 执行各步骤
print("\n>>> before_build: normalize_and_group_images")
normalize_image_name.normalize_and_group_images()

generate_images.convert_jpg_to_bmp()
generate_images.generate_struct_file()
subfolder_data = generate_images.generate_subfolder_files()
generate_images.generate_main_image_files(subfolder_data)
generate_images.clean_up_files()
print("\n🎉 所有操作已成功完成！")

print("\n>>> before_build: auto_replace_generated_vars")
auto_replace_generated_vars.main()