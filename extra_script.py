import os
from SCons.Script import Import

Import("env")

print("\n>>> Running extra_script.py")

# Install missed package
try:
    import pillow
except ImportError:
    env.Execute("$PYTHONEXE -m pip install pillow")

def before_buildfs(source, target, env):
    print("\n>>>  before_buildfs: build-www")
    env.Execute("ruby shared/build-www.rb")

    print("\n>>> before_buildfs: copy assets")
    env.Execute("robocopy /MIR shared\\assets data\\assets")

    print("\n>>> before_buildfs: gzip assets")
    env.Execute("powershell -Command \"Get-ChildItem data\\ -Recurse -Include *.css,*.js,*.html | ForEach-Object { gzip -Force $_.FullName }\"")

    print("\n>>> before_buildfs: total size")
    env.Execute("dir /s data")

env.AddPreAction("buildfs", before_buildfs)

def after_buildfs(source, target, env):
    print("\n>>> after_buildfs: unzip assets")
    # 如果需要在构建后解压缩文件，可以在这里添加相应的解压缩命令
    pass

env.AddPostAction("buildfs", after_buildfs)

# 在build之前generate_images
def before_build(source, target, env):
    from tools import generate_images
    from tools import normalize_image_name
    print("\n>>> before_build: normalize_and_group_images")
    normalize_image_name.normalize_and_group_images()
    print("\n>>> before_build: convert_jpg_to_bmp")
    generate_images.convert_jpg_to_bmp()
    print("\n>>> before_build: generate_cpp_and_header")
    generate_images.generate_cpp_and_header()
    print("\n>>> before_build: delete_xbm_files")
    generate_images.delete_xbm_files()
    print("\n>>> before_build: auto_replace_generated_vars")

env.AddPreAction("build", before_build)