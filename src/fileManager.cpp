#include <A_Config.h>
#include <GUI.h>

#include "images/images.h"

struct s_fileicondict {
    const char *extension;
    const uint8_t *ico;
};
static const struct s_fileicondict fileicondict[] = {
    {"bmp", imgfile_bits},  {"jpg", imgfile_bits}, {"png", imgfile_bits},
    {"xbm", imgfile_bits},  {"lua", luafile_bits}, {"buz", musicfile_bits},
    {"json", sysfile_bits}, {"bin", sysfile_bits}, {"txt", textfile_bits},
    {"lbm", imgfile_bits},  // 自定义的图片格式（实际上是XBM编码）
    {NULL, NULL},
};
/**
 *
“lbm”格式定义：
扩展名.lbm
单色位图
小端
2字节宽度
2字节高度
之后是数据
*/
static const uint8_t *getFileIcon(const char *extension) {
    for (int i = 0; fileicondict[i].extension != NULL; i++) {
        if (strcmp(fileicondict[i].extension, extension) == 0) {
            return fileicondict[i].ico;
        }
    }
    return otherfile_bits;
}
namespace GUI {
char filedialog_buffer[300];
void push_buffer();
void pop_buffer();
const char *fileDialog(const char *title, bool isApp, const char *endsWidth,
                       const char *gotoendsWidth) {
    // 首先选择文件系统
    bool useSD = false;
    if (isApp == false) {
        if (peripherals.isSDLoaded()) {
            if (msgbox_yn("请选择文件系统", "左：LittleFS\n右：TF 卡", "TF 卡",
                          "LittleFS")) {
                useSD = true;
            }
        }
    } else {
        useSD = true;
    }
    String cwd = "/";
    File root;
    File file;
    int16_t total_entries = 0;
    menu_item entries[128];
    char *titles[128];
    memset(entries, 0, sizeof(entries));
    memset(titles, 0, sizeof(titles));
    entries[0].icon = folder_bits;
    entries[0].title = "..";
    while (1) {
        Serial.print("[文件浏览] 当前工作目录：");
        Serial.println(cwd);
        // 首先清除
        total_entries = 1;
        while (titles[total_entries] != NULL) {
            free(titles[total_entries]);
            titles[total_entries] = NULL;
            entries[total_entries].icon = NULL;
            entries[total_entries].title = NULL;
            ++total_entries;
        }
        total_entries = 1;
        if (useSD) {
            root = SD.open(cwd);
        } else {
            root = LittleFS.open(cwd);
        }
        if (cwd != "/") cwd += "/";
        if (!root) {
            Serial.println("[文件] root未打开");
        }
        file = root.openNextFile();
        while (file) {
            String tmp = file.name();
            String ext;
            if (tmp.lastIndexOf('.') != -1) {
                ext = tmp.substring(tmp.lastIndexOf('.') + 1);
            } else {
                ext = "";
            }
            Serial.println(tmp);
            if (gotoendsWidth != NULL) {
                if (tmp.endsWith(gotoendsWidth)) {
                    file.close();
                    file = root.openNextFile();
                    continue;
                }
            }
            if (isApp == false) {
                if (file.isDirectory()) {
                    entries[total_entries].icon = folder_bits;
                } else {
                    if (endsWidth != NULL) {
                        if (strcmp(endsWidth, ext.c_str()) != 0) {
                            file.close();
                            file = root.openNextFile();
                            continue;
                        }
                    }
                    entries[total_entries].icon = getFileIcon(ext.c_str());
                }
                titles[total_entries] = (char *)malloc(strlen(file.name()) + 1);
                if (titles[total_entries] == NULL) {
                    GUI::msgbox("严重错误", "[文件管理] 动态内存不足");
                    ESP.restart();
                }
                strcpy(titles[total_entries], file.name());
                entries[total_entries].title = titles[total_entries];
                ++total_entries;
            } else {
                if (file.isDirectory()) {
                    if (ext == "app") {
                        entries[total_entries].icon = luafile_bits;
                        titles[total_entries] =
                            (char *)malloc(strlen(file.name()) + 1);
                        if (titles[total_entries] == NULL) {
                            GUI::msgbox("严重错误", "[文件管理] 动态内存不足");
                            ESP.restart();
                        }
                        strcpy(titles[total_entries], file.name());
                        entries[total_entries].title = titles[total_entries];
                        ++total_entries;
                    }
                }
            }
            if (total_entries >= 128) {
                GUI::msgbox("严重错误", "文件数超过128个，即将重启");
                ESP.restart();
            }
            file.close();
            file = root.openNextFile();
        }
        int selected = menu(title, entries, 12, 12);
        if (selected == 0) {
            // 上一级目录
            display.display(false);  // 全局刷新一次
            if (cwd == "/") {
                total_entries = 1;
                while (titles[total_entries] != NULL) {
                    free(titles[total_entries]);
                    titles[total_entries] = NULL;
                    ++total_entries;
                }
                return NULL;
            } else {
                if (cwd.lastIndexOf('/', cwd.length() - 2) == 0) {
                    // 下一个是根目录
                    cwd = "/";
                    root.close();
                } else {
                    cwd = cwd.substring(0,
                                        cwd.lastIndexOf('/', cwd.length() - 2));
                    root.close();
                }
                continue;
            }
        } else if (entries[selected].icon == folder_bits) {
            // 是文件夹
            cwd += entries[selected].title;
            root.close();
            continue;
        } else {
            sprintf(filedialog_buffer, "%s%s%s", useSD ? "/sd" : "/littlefs",
                    cwd.c_str(), entries[selected].title);
            break;
        }
    }
    total_entries = 1;
    while (titles[total_entries] != NULL) {
        free(titles[total_entries]);
        titles[total_entries] = NULL;
        ++total_entries;
    }
    return filedialog_buffer;
}

const char *folderDialog(const char *title) {
    static char selected_path[256];
    String cwd = "/";
    bool useSD = false;

    // 选择文件系统
    if (peripherals.isSDLoaded()) {
        if (msgbox_yn("请选择文件系统", "左：LittleFS\n右：TF 卡", "TF 卡",
                      "LittleFS")) {
            useSD = true;
        }
    }

    // 状态结构体
    struct ButtonStates {
        bool btnr_short = false;
        bool btnr_long = false;
        bool btnl_short = false;
        bool btnl_long = false;
        bool btnc_short = false;
        bool btnc_long = false;
    } states;

    // 回调函数
    auto onBtnrClick = [](void *ptr) {
        static_cast<ButtonStates *>(ptr)->btnr_short = true;
    };
    auto onBtnrLong = [](void *ptr) {
        static_cast<ButtonStates *>(ptr)->btnr_long = true;
    };
    auto onBtnlClick = [](void *ptr) {
        static_cast<ButtonStates *>(ptr)->btnl_short = true;
    };
    auto onBtnlLong = [](void *ptr) {
        static_cast<ButtonStates *>(ptr)->btnl_long = true;
    };
    auto onBtncClick = [](void *ptr) {
        static_cast<ButtonStates *>(ptr)->btnc_short = true;
    };
    auto onBtncLong = [](void *ptr) {
        static_cast<ButtonStates *>(ptr)->btnc_long = true;
    };

    // 初始化按钮状态
    hal.hookButton();
    hal.btnr.attachClick(onBtnrClick, &states);
    hal.btnr.attachLongPressStart(onBtnrLong, &states);
    hal.btnl.attachClick(onBtnlClick, &states);
    hal.btnl.attachLongPressStart(onBtnlLong, &states);
    hal.btnc.attachClick(onBtncClick, &states);
    hal.btnc.attachLongPressStart(onBtncLong, &states);

    while (true) {
        File root = useSD ? SD.open(cwd) : LittleFS.open(cwd);
        if (!root) {
            GUI::msgbox("错误", "无法打开目录");
            return nullptr;
        }

        std::vector<String> folders;
        while (true) {
            File f = root.openNextFile();
            if (!f) break;
            if (f.isDirectory()) {
                folders.push_back(f.name());
            }
            f.close();
        }
        root.close();

        // 构建 menu_item 列表
        std::vector<menu_item> items(folders.size() + 1);
        items[0].title = "..";
        items[0].icon = folder_bits;
        for (size_t i = 0; i < folders.size(); ++i) {
            items[i + 1].title = folders[i].c_str();
            items[i + 1].icon = folder_bits;
        }

        int selected = GUI::menu(title, items.data());
        if (selected < 0) return nullptr;

        // 重置按钮状态
        states.btnr_short = states.btnr_long = false;
        states.btnl_short = states.btnl_long = false;
        states.btnc_short = states.btnc_long = false;

        // 等待按键操作
        while (true) {
            if (states.btnc_short) {
                states.btnc_short = false;

                if (selected == 0) {
                    if (cwd != "/") {
                        cwd = cwd.substring(0, cwd.lastIndexOf('/'));
                        if (cwd == "") cwd = "/";
                    }
                } else {
                    cwd += (cwd.endsWith("/") ? "" : "/");
                    cwd += folders[selected - 1];
                }
                break;
            }

            if (states.btnc_long) {
                states.btnc_long = false;
                // 返回选择的路径
                snprintf(selected_path, sizeof(selected_path), "%s%s",
                         useSD ? "/sd" : "/littlefs", cwd.c_str());
                return selected_path;
            }

            delay(20);  // 防抖
        }
    }
}

}  // namespace GUI
