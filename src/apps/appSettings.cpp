#include "AppManager.h"
#include "DS3231.h"
#include "images/images.h"

DS3231 Srtc;

static const menu_item settings_menu_main[] = {
    {NULL, "退出"},     {NULL, "时间设置"},     {NULL, "闹钟设置"},
    {NULL, "网络设置"}, {NULL, "重新扫描外设"}, {NULL, "SD卡信息"},
    {NULL, "其它设置"}, {NULL, "关于"},         {NULL, NULL},
};

static const menu_item settings_menu_time[] = {
    {NULL, "返回上一级"},
    {NULL, "手动触发NTP"},
    {NULL, "时间同步间隔设置"},
    {NULL, "RTC线性偏移修正"},
    {NULL, "离线模式"},
    {NULL, "在复位为“power on”时自动对时"},
    {NULL, NULL},
};
static menu_item settings_menu_network[] = {
    {NULL, "返回上一级"},
    {NULL, "当前连接"},
    {NULL, "搜索周围的WIFI"},
    {NULL, "ESPTouch配网"},
    {NULL, "启动HTTP服务器"},
    {NULL, "ESPNow设备扫描"},
    {NULL, "蓝牙扫描"},
    {NULL, "退出Bilibili账号"},
    {NULL, "从文件加载WIFI配置"},
    {NULL, "保存当前网络"},
    {NULL, NULL},
};

static const menu_item settings_menu_peripheral[] = {
    {NULL, "返回上一级"}, {NULL, "重新扫描外设"}, {NULL, "AHT20"},
    {NULL, "BMP280"},     {NULL, "SGP30"},        {NULL, "DS3231"},
    {NULL, "SD卡"},       {NULL, NULL},
};

static const menu_item settings_menu_other[] = {
    {NULL, "返回上一级"},   {NULL, "屏幕方向"},        {NULL, "天气更新间隔"},
    {NULL, "立即更新天气"}, {NULL, "主屏幕应用选择"},  {NULL, "应用管理"},
    {NULL, "TF加载方式"},   {NULL, "恢复出厂设置"},    {NULL, "格式化Littlefs"},
    {NULL, "电池电压校准"}, {NULL, "sd_clk_freq set"}, {NULL, "cpufreq set"},
    {NULL, "自动休眠电压"}, {NULL, "长按时间"},        {NULL, "按键音设置"},
    {NULL, NULL},
};

class AppSettings : public AppBase {
   private:
    /* data */
    String toApp = "";
    bool hasToApp = false;

   public:
    AppSettings() {
        name = "settings";
        title = "设置";
        description = "简单的设置";
        image = settings_bits;
        noDefaultEvent = true;
        peripherals_requested = PERIPHERALS_SD_BIT;
    }
    void set();
    void setup();
    void menu_time();
    void menu_alarm();
    void menu_network();
    void menu_peripheral();
    void menu_other();
    int binToDec(int bin);
    int decToBin(int dec);
    void cheak_config(char *a);
};
static AppSettings app;

void AppSettings::set() {
    _showInList = hal.pref.getBool(hal.get_char_sha_key(title), true);
}
void AppSettings::setup() {
    display.clearScreen();
    GUI::drawWindowsWithTitle("设置", 0, 0, 296, 128);
    u8g2Fonts.drawUTF8(120, 75, "请稍等...");
    // 下面是设置菜单
    int res = 0;
    bool end = false;
    while (end == false && hasToApp == false) {
        display.display(false);  // 每次进入设置全局刷新一次
        res = GUI::menu("设置", settings_menu_main);
        switch (res) {
            case 0:
                end = true;
                break;
            case 1:
                // 时间设置
                menu_time();
                break;
            case 2:
                // 闹钟设置
                menu_alarm();
                break;
            case 3:
                // 网络设置
                menu_network();
                break;
            case 4:
                // 重新扫描外设
                peripherals.check();
                break;
            case 5:
                // SD卡信息
                {
                    display.clearScreen();
                    GUI::drawWindowsWithTitle("SD卡信息", 0, 0, 296, 128);
                    // u8g2Fonts.setCursor(5,30);
                    // u8g2Fonts.printf("类型：%s",SD.cardType());
                    u8g2Fonts.setCursor(5, 30);
                    float cardSizeKB = (float)SD.cardSize() / 1024.0 / 1024.0;
                    u8g2Fonts.printf("大小：%uBytes %.2fMB ", SD.cardSize(),
                                     cardSizeKB);
                    u8g2Fonts.setCursor(5, 45);
                    u8g2Fonts.printf("扇区数量：%u", SD.numSectors());
                    u8g2Fonts.setCursor(5, 60);
                    u8g2Fonts.printf("扇区大小：%u Bytes", SD.sectorSize());
                    u8g2Fonts.setCursor(5, 75);
                    float cardSizeuse = (float)SD.usedBytes() / 1024.0 / 1024.0;
                    float cardSizetotal =
                        (float)SD.totalBytes() / 1024.0 / 1024.0;
                    u8g2Fonts.printf("空间使用:%d%%(%.2f/%.2f)MB",
                                     SD.usedBytes() * 100 / SD.totalBytes(),
                                     cardSizeuse, cardSizetotal);
                    u8g2Fonts.setCursor(5, 90);
                    u8g2Fonts.printf("可用空间：%.2fMB",
                                     cardSizetotal - cardSizeuse);
                    display.display();
                    hal.wait_input();
                }
                break;
            case 6:
                // 其它设置
                menu_other();
                break;
            case 7: {
                display.display();
                display.setFullWindow();
                display.clearScreen();
                GUI::drawWindowsWithTitle("关于本设备", 0, 0, 296, 128);
                u8g2Fonts.setCursor(5, 30);
                u8g2Fonts.printf("设备名称:LiClock 版本:%s   by QCQCQC",
                                 code_version);
                u8g2Fonts.drawUTF8(5, 45,
                                   "CPU:Xtensa@32-bit LX6 @0.24GHz X2+ULP");
                u8g2Fonts.setCursor(5, 60);
                u8g2Fonts.printf("内存:520KB SRAM+16KB RTC SRAM   存储:%dMB",
                                 ESP.getFlashChipSize() / 1024 / 1024);
                u8g2Fonts.setCursor(5, 75);
                u8g2Fonts.printf(
                    "文件系统(已用/总空间):%d%% %d/%d kB",
                    LittleFS.usedBytes() * 100 / LittleFS.totalBytes(),
                    LittleFS.usedBytes() / 1024, LittleFS.totalBytes() / 1024);
                u8g2Fonts.setCursor(5, 90);
                u8g2Fonts.printf(
                    "屏幕类型:EPD  屏幕分辨率:%dX%d CPU_freq:%uMHz",
                    SCREEN_WIDTH, SCREEN_HEIGHT, getCpuFrequencyMhz());
                u8g2Fonts.setCursor(5, 105);
                u8g2Fonts.printf("源代码编译时间：%s", built_at);
                display.display();
                hal.wait_input();
            } break;
            default:
                break;
        }
    }
    if (hasToApp == true) {
        hasToApp = false;
        if (toApp != "") {
            appManager.gotoApp(toApp.c_str());
        }
        return;
    }
    appManager.goBack();
}

// 时间设置
void AppSettings::menu_time() {
    int res = 0;
    bool end = false;
    while (end == false && hasToApp == false) {
        res = GUI::menu("时间设置", settings_menu_time);
        switch (res) {
            case 0:
                end = true;
                break;
            case 1:
                // 手动触发NTP
                if (GUI::msgbox_yn("手动触发NTP", "将连接WiFi并同步时间") ==
                    true) {
                    // 同步时间
                    hal.autoConnectWiFi();
                    NTPSync();
                    GUI::msgbox("手动触发NTP", "同步完成");
                }
                break;
            case 2:
                // 时间同步间隔设置
                {
                    const menu_item menu[] = {
                        {NULL, "取消"},   {NULL, "禁用时间同步"},
                        {NULL, "2小时"},  {NULL, "4小时"},
                        {NULL, "6小时"},  {NULL, "12小时"},
                        {NULL, "24小时"}, {NULL, "36小时"},
                        {NULL, "48小时"}, {NULL, NULL},
                    };
                    res = GUI::menu("时间同步间隔设置", menu);
                    if (res > 0) {
                        // 设置时间同步间隔
                        hal.pref.putUChar(SETTINGS_PARAM_NTP_INTERVAL, res - 1);
                        GUI::msgbox("时间同步间隔设置", "设置完成");
                    }
                }
                break;
            case 3:
                // RTC线性偏移修正
                toApp = "rtcoffset";
                hasToApp = true;
                end = true;
                break;
            case 4:
                // 离线模式
                if (GUI::msgbox_yn("仅时钟模式", "是否启用仅时钟模式？",
                                   "仅时钟", "天气时钟")) {
                    // 启用
                    config[PARAM_CLOCKONLY] = "1";
                    hal.saveConfig();
                } else {
                    config[PARAM_CLOCKONLY] = "0";
                    hal.saveConfig();
                }
                break;
            case 5:
                // 离线模式
                if (GUI::msgbox_yn("时间设置",
                                   "在芯片上电复位时自动联网更新ESP32的时间",
                                   "启用", "禁用")) {
                    // 启用
                    config[autontpsync] = "1";
                    hal.saveConfig();
                } else {
                    config[autontpsync] = "0";
                    hal.saveConfig();
                }
                break;
            default:
                break;
        }
    }
}

// 闹钟设置
void AppSettings::menu_alarm() {
    int res = 0;
    bool end = false;
    menu_item settings_menu_alarm[] = {
        {NULL, "返回上一级"}, {NULL, NULL}, {NULL, NULL},       {NULL, NULL},
        {NULL, NULL},         {NULL, NULL}, {NULL, "闹钟铃声"}, {NULL, NULL},
    };
    const menu_item settings_menu_alarm_sub[] = {
        {NULL, "返回"},
        {NULL, "时间"},
        {NULL, "重复周期"},
        {NULL, NULL},
    };
    const menu_item settings_menu_alarm_time[] = {
        {NULL, "返回"},       {NULL, "关闭"},   {NULL, "单次"},
        {NULL, "周一到周五"}, {NULL, "周六日"}, {NULL, "周一"},
        {NULL, "周二"},       {NULL, "周三"},   {NULL, "周四"},
        {NULL, "周五"},       {NULL, "周六"},   {NULL, "周日"},
        {NULL, "手动输入"},   {NULL, NULL},
    };
    char alarm_buf[5][30];
    char alarm_buf_week[25];
    char bit_week[7] = {0};
    while (end == false && hasToApp == false) {
        // 读取闹钟设置
        for (int i = 0; i < 5; ++i) {
            if (alarms.alarm_table[i].enable == 0) {
                sprintf(alarm_buf[i], "%d：%02d:%02d，关闭", i + 1,
                        alarms.alarm_table[i].time / 60,
                        alarms.alarm_table[i].time % 60,
                        alarms.getEnable(alarms.alarm_table + i).c_str());
            } else {
                sprintf(alarm_buf[i], "%d：%02d:%02d,%s", i + 1,
                        alarms.alarm_table[i].time / 60,
                        alarms.alarm_table[i].time % 60,
                        alarms.getEnable(alarms.alarm_table + i).c_str());
            }
            settings_menu_alarm[i + 1].title = alarm_buf[i];
        }
        res = GUI::menu("闹钟设置", settings_menu_alarm);
        if (res == 0) break;
        if (res == 6) {
            const char *str =
                GUI::fileDialog("请选择闹钟铃声文件", false, "buz");
            if (str) {
                hal.pref.putString(SETTINGS_PARAM_ALARM_TONE, String(str));
            } else {
                if (GUI::msgbox_yn("你选择了返回",
                                   "是否使用默认铃声，或者保留之前的设置",
                                   "使用默认", "取消")) {
                    hal.pref.remove(SETTINGS_PARAM_ALARM_TONE);
                }
            }
        }
        int selected = res - 1;
        res = GUI::menu(alarm_buf[selected], settings_menu_alarm_sub);
        switch (res) {
            case 0:
                break;
            case 1: {
                alarms.alarm_table[selected].time = GUI::msgbox_time(
                    "请输入闹钟时间", alarms.alarm_table[selected].time);
                if (alarms.alarm_table[selected].enable == 0)
                    alarms.alarm_table[selected].enable = ALARM_ENABLE_ONCE;
                break;
            }
            case 2: {
                int res;
                res = GUI::menu("请选择重复周期", settings_menu_alarm_time);
                enum alarm_enable_enum res_table[] = {
                    ALARM_DISABLE,
                    ALARM_ENABLE_ONCE,
                    (enum alarm_enable_enum)0b00111110,
                    (enum alarm_enable_enum)0b01000001,
                    ALARM_ENABLE_MONDAY,
                    ALARM_ENABLE_TUESDAY,
                    ALARM_ENABLE_WEDNESDAY,
                    ALARM_ENABLE_THURSDAY,
                    ALARM_ENABLE_FRIDAY,
                    ALARM_ENABLE_SATDAY,
                    ALARM_ENABLE_SUNDAY,
                };
                switch (res) {
                    case 0:
                        break;
                    case 12: {
                        int time = binToDec(GUI::msgbox_number(
                            "六五四三二一日", 7,
                            decToBin(alarms.alarm_table[selected].enable)));
                        alarms.alarm_table[selected].enable =
                            (enum alarm_enable_enum)(time % 256);
                    } break;
                    default:
                        alarms.alarm_table[selected].enable =
                            (enum alarm_enable_enum)(res_table[(res - 1) % 11]);
                        break;
                }
            } break;
            default:
                break;
        }
    }
    alarms.save();
}

// 网络设置
void AppSettings::menu_network() {
    int res = 0;
    bool end = false;

    while (end == false && hasToApp == false) {
        menu_item current_connection;
        // 如果wifi连接则显示：当前：SSID，否则显示“未连接
        current_connection.icon = NULL;
        if (WiFi.status() == WL_CONNECTED) {
            current_connection.title = (String("当前：") + WiFi.SSID()).c_str();
        } else {
            current_connection.title = "未连接";
        }
        settings_menu_network[1] = current_connection;
        res = GUI::menu("网络设置", settings_menu_network);
        switch (res) {
            case 0:
                end = true;
                break;
            case 2: {
                u8g2Fonts.setCursor(20, 120);
                u8g2Fonts.printf("正在搜索WIFI...");
                display.display();
                WiFi.mode(WIFI_STA);
                hal.searchWiFi();
                Serial.printf("搜索到的个数:%d", hal.numNetworks);
                char winfo[hal.numNetworks][64];
                char _ssid[hal.numNetworks][64];
                if (hal.numNetworks != 0) {
                    for (int i = 0; i < hal.numNetworks; i++) {
                        String ssid = WiFi.SSID(i);
                        char ssidArray[ssid.length() +
                                       1];  // +1 是为了包含字符串末尾的 null
                                            // 字符
                        ssid.toCharArray(ssidArray, sizeof(ssidArray));
                        sprintf(winfo[i], "%s %d", ssidArray, WiFi.RSSI(i));
                        sprintf(_ssid[i], "%s", ssidArray);
                    }
                }
                menu_item *WiFi_list = new menu_item[hal.numNetworks + 2];
                WiFi_list[0].title = "返回";
                WiFi_list[0].icon = NULL;
                for (int i = 1; i < hal.numNetworks + 1; i++) {
                    WiFi_list[i].title = winfo[i - 1];
                    WiFi_list[i].icon = NULL;
                }
                WiFi_list[hal.numNetworks + 1].title = NULL;
                WiFi_list[hal.numNetworks + 1].icon = NULL;
                int res = 0;
                bool end = false;
                while (end == false && hasToApp == false) {
                    res = GUI::menu("部分扫描到的WIFI", WiFi_list);
                    if (res == 0) {
                        delete[] WiFi_list;
                        end = true;
                        break;
                    } else {
                        delete[] WiFi_list;
                        cheak_config(_ssid[res - 1]);
                        break;
                    }
                }
            } break;
            case 3:
                // ESPTouch配网
                hal.WiFiConfigSmartConfig();
                break;
            case 4:
                // 启动HTTP服务器
                toApp = "webserver";
                hasToApp = true;
                end = true;
                break;
            case 5:
                // ESPNow设备扫描
                break;
            case 6:
                // 蓝牙扫描
                break;
            case 7:
                // 退出Bilibili账号
                if (LittleFS.exists("/blCookies.txt")) {
                    LittleFS.remove("/blCookies.txt");
                    GUI::msgbox("完成", "Bilibili账户登录信息已删除");
                    break;
                } else {
                    GUI::msgbox("提示", "Bilibili Cookies不存在");
                    break;
                }
                break;
            case 8:
                char configFileName[256];
                // 从文件加载配置
                strcpy(configFileName, GUI::fileDialog("请选择WIFI配置文件",
                                                       false, "wificonfig"));
                if (configFileName == NULL) {
                    GUI::msgbox("提示", "没有选择文件");
                    end = true;
                    break;
                } else {
                    HAL::FilePair f = hal.fileOpen(configFileName, "r");
                    // 第一行是SSID，第二行是密码
                    if (f.file.available()) {
                        String ssid = f.file.readStringUntil('\n');
                        String password = f.file.readStringUntil('\n');
                        f.file.close();
                        // 先处理一下这两个字符串，trim
                        ssid.trim();
                        password.trim();
                        // 检查是否有空格
                        config[PARAM_PASS] = password.c_str();
                        config[PARAM_SSID] = ssid.c_str();
                        // 尝试连接Wifi
                        int result = hal.tryConnectWiFi();
                        if (result == 0) {
                            GUI::msgbox("连接成功", (String("已连接到") + ssid +
                                                     "\n请重启设备以使设置生效")
                                                        .c_str());
                            ESP.restart();
                        } else if (result == -1) {
                            GUI::msgbox("连接失败", "密码错误或SSID不存在");
                        }

                    } else {
                        GUI::msgbox("错误", "无法打开文件");
                        Serial.printf("无法打开文件%s\n", configFileName);
                    }
                }
                end = true;
                break;
            case 9:
                char toSavePath[256];
                if (strlen(config[PARAM_SSID].as<const char *>()) == 0) {
                    GUI::msgbox("错误", "当前没有WIFI配置");
                    break;
                }
                // 保存当前网络
                strcpy(toSavePath, GUI::fileDialog("选择保存目录"));
                if (toSavePath == NULL) {
                    GUI::msgbox("提示", "没有选择目录");
                    end = true;
                    break;
                } else {
                    strcat(toSavePath, "/");
                    strcat(toSavePath, config[PARAM_SSID].as<const char *>());
                    strcat(toSavePath, ".wificonfig");
                    // 如果文件已经存在，则提示覆盖
                    if (hal.fileExists(toSavePath)) {
                        if (GUI::msgbox_yn("文件已存在",
                                           "是否覆盖？") == false) {
                            GUI::msgbox("提示", "已取消保存");
                            end = true;
                            break;
                        }
                    } else {
                        // 如果文件不存在，则创建
                        Serial.printf("正在保存到%s\n", toSavePath);
                    }
                    // 打开文件
                    HAL::FilePair f = hal.fileOpen(toSavePath, "w");
                    if (f.file.availableForWrite()) {
                        f.file.println(config[PARAM_SSID].as<const char *>());
                        f.file.println(config[PARAM_PASS].as<const char *>());
                        f.file.close();
                        GUI::msgbox("保存成功", "已将当前网络配置保存到文件");
                    } else {
                        GUI::msgbox("错误", "无法打开文件");
                        Serial.printf("无法打开文件%s\n", toSavePath);
                    }
                }
                break;
            default:
                break;
        }
    }
}

#include <nvs_flash.h>
// 其它设置
void AppSettings::menu_other() {
    int res = 0;
    bool end = false;
    while (end == false && hasToApp == false) {
        res = GUI::menu("其它设置", settings_menu_other);
        switch (res) {
            case 0:
                end = true;
                break;
            case 1:
                // 屏幕方向
                if (GUI::msgbox_yn("屏幕方向选择",
                                   "正常意为开关在屏幕左上，否则为右下",
                                   "正常（右）", "反转（左）")) {
                    hal.pref.putUChar(SETTINGS_PARAM_SCREEN_ORIENTATION, 3);
                } else {
                    hal.pref.putUChar(SETTINGS_PARAM_SCREEN_ORIENTATION, 1);
                }
                display.display();
                GUI::msgbox(
                    "提示",
                    "按键控制方式请修改GPIO宏定义");  // 为了节省内存并加快速度
                break;
            case 2:
                // 天气更新间隔
                {
                    int res = GUI::msgbox_number(
                        "请输入天气更新间隔", 2,
                        atoi(config[PARAM_FULLUPDATE].as<const char *>()));
                    if (res < 5 || res > 40) {
                        res = 20;
                    }
                    char tmp[4];
                    sprintf(tmp, "%d", res);
                    config[PARAM_FULLUPDATE] = tmp;
                    hal.saveConfig();
                    break;
                }
            case 3:
                // 立即更新天气
                {
                    int res = weather.refresh();
                    if (res == 0) {
                        GUI::msgbox(
                            "更新完成",
                            "已将天气信息保存至/littlefs/System/weather.bin");
                    } else if (res == -2) {
                        GUI::msgbox("发生错误", "错误原因：http错误或异常");
                    } else if (res == -3) {
                        GUI::msgbox("发生错误", "错误原因：彩云天气API失效");
                    }
                    break;
                }
            case 4:
                // 主屏幕应用选择
                {
                    AppBase *tmp = appManager.appSelector(true);
                    if (tmp) {
                        Serial.println(tmp->name);
                        if (GUI::msgbox_yn("警告",
                                           "选择不兼容的App可能会导致无法开机，"
                                           "是否确认？") == true) {
                            if (strcmp(tmp->name, "clock") == 0) {
                                config[PARAM_CLOCKONLY] = "0";
                                hal.saveConfig();
                                hal.pref.putString(SETTINGS_PARAM_HOME_APP,
                                                   "clock");
                            } else if (strcmp(tmp->name, "clockonly") == 0) {
                                config[PARAM_CLOCKONLY] = "0";
                                hal.saveConfig();
                                hal.pref.putString(SETTINGS_PARAM_HOME_APP,
                                                   "clock");
                            } else {
                                hal.pref.putString(SETTINGS_PARAM_HOME_APP,
                                                   tmp->name);
                            }
                            GUI::msgbox("设置成功", "重启或下次唤醒后生效");
                        }
                    }
                    break;
                }
            case 5:
                // 已安装应用管理
                toApp = "installer";
                hasToApp = true;
                end = true;
                return;
                break;
            case 6:
                if (GUI::msgbox_yn("修改TF卡加载模式",
                                   "模式\n1.当卸载后才断电\n2.休眠后就断电",
                                   "模式1", "模式2")) {
                    config[TFmode] = "1";
                    hal.saveConfig();
                    Serial.printf("修改TF卡加载模式,当卸载后才断电");
                } else {
                    config[TFmode] = "0";
                    hal.saveConfig();
                    Serial.printf("修改TF卡加载模式,休眠后就断电");
                }
                break;
            case 7:
                // 恢复出厂设置
                {
                    if (GUI::msgbox_yn("此操作不可撤销",
                                       "是否恢复出厂设置？")) {
                        if (GUI::msgbox_yn("这是最后一次提示",
                                           "将格式化nvs和LittleFS存储区",
                                           "取消（右）",
                                           "确认（左）") == false) {
                            display.clearScreen();
                            u8g2Fonts.drawUTF8(30, 40, "正在格式化NVS存储");
                            display.display();
                            nvs_flash_erase();
                            display.clearScreen();
                            u8g2Fonts.drawUTF8(30, 40,
                                               "正在格式化LittleFS存储");
                            display.display(true);
                            LittleFS.end();
                            LittleFS.format();
                            display.clearScreen();
                            u8g2Fonts.drawUTF8(30, 40, "完成，正在重启");
                            display.display(true);
                            ESP.restart();
                        }
                    }
                }
                break;
            case 8: {
                if (GUI::msgbox_yn("警告", "格式化将丢失所有文件，包括配置文件",
                                   "确定", "取消")) {
                    if (GUI::msgbox_yn("警告",
                                       "这是最后一次提醒，是否仍要格式化",
                                       "取消", "确定")) {
                    } else {
                        if (LittleFS.format()) {
                            GUI::msgbox("提示", "LiClock成功进行了格式化");
                        } else {
                            for (int i = 0; i < 3; i++) {
                                buzzer.append(3000, 200);
                                delay(350);
                            }
                            GUI::msgbox(
                                "提示",
                                "LiClock在格式化时发生了错误,格式化未能完成");
                        }
                    }
                }
            } break;
            case 9: {
                int vcc;
                if (GUI::msgbox_yn(
                        "校准方式",
                        "1."
                        "使用芯片ADC校准数据\n在使用1前，请使用esptool\n确定芯"
                        "片有正确校准数据\n2.使用外部仪表读数",
                        "1", "2")) {
                    vcc = 2 * analogReadMilliVolts(PIN_ADC);
                } else {
                    vcc = GUI::msgbox_number("外部仪表读数", 4, hal.VCC);
                }
                int ppc;
                int adc = analogRead(PIN_ADC);
                ppc = vcc * 4096 / adc;
                hal.pref.putInt("ppc", ppc);
                char buf[40];
                sprintf(buf, "新的分压系数: %d\n%d->%d mV", ppc, vcc, hal.VCC);
                GUI::msgbox("提示", buf);
            } break;
            case 10: {
                float new_clk_freq = (float)GUI::msgbox_number(
                    "SD卡时钟", 8, hal.pref.getInt("sd_clk_freq", 500000));
                hal.pref.putInt("sd_clk_freq", new_clk_freq);
            } break;
            case 11: {
                GUI::msgbox("提示",
                            "频率只能为以下数值\n240,160,80,40,20,10MHz\n");
                int new_freq =
                    GUI::msgbox_number("new freq", 3, getCpuFrequencyMhz());
                hal.pref.putInt("CpuFreq", new_freq);
                int freq = ESP.getCpuFreqMHz();
                if (freq != new_freq) {
                    bool cpuset = setCpuFrequencyMhz(new_freq);
                    Serial.begin(115200);
                    Serial.printf("CpuFreq: %dMHZ -> %dMHZ ......", freq,
                                  new_freq);
                    if (cpuset) {
                        Serial.print("ok\n");
                        GUI::msgbox("提示", "频率修改成功");
                    } else {
                        Serial.print("err\n");
                        GUI::msgbox("错误", "频率未能修改");
                        F_LOG("CPU频率修改失败,设置的值:%d", new_freq);
                    }
                }
            } break;
            case 12: {
                int auto_sleep_mv = GUI::msgbox_number(
                    "自动休眠电压", 4, hal.pref.getInt("auto_sleep_mv", 2800));
                hal.pref.putInt("auto_sleep_mv", auto_sleep_mv);
            } break;
            case 13: {
                int long_pres_time = GUI::msgbox_number(
                    "长按时间", 4, hal.pref.getInt("lpt", 25) * 10);
                hal.pref.putInt("lpt", long_pres_time / 10);
            } break;
            case 14: {
                static const menu_select settings_btn_buz[] = {
                    {false, "返回"},
                    {true, "按键音"},
                    {false, "声音频率"},
                    {false, "声音长度"},
                    {false, NULL}};
                int res = 0;
                bool end = false;
                while (!end) {
                    res = GUI::select_menu("按键音设置", settings_btn_buz);
                    switch (res) {
                        case 0:
                            end = true;
                            break;
                        case 1:
                            break;
                        case 2:
                            hal.pref.putInt(
                                "btn_buz_freq",
                                GUI::msgbox_number(
                                    "输入频率Hz", 5,
                                    hal.pref.getInt("btn_buz_freq", 150)));
                            break;
                        case 3:
                            hal.pref.putInt(
                                "btn_buz_time",
                                GUI::msgbox_number(
                                    "输入时长ms", 5,
                                    hal.pref.getInt("btn_buz_time", 100)));
                            break;
                        default:
                            break;
                    }
                }
            } break;
            default:
                break;
        }
    }
}

int AppSettings::decToBin(int dec) {
    if (dec < 0 || dec > 255) {
        log_e("非法的输入值");
        return 0;
    }
    int bin = 0;
    int base = 1;
    for (int i = 0; i < 8; ++i) {
        if (dec & 1) {  // 检查最低位是否为1
            bin += base;
        }
        dec >>= 1;   // 右移一位
        base *= 10;  // 更新权重
    }
    return bin;
}
int AppSettings::binToDec(int bin) {
    int decimal = 0;
    int base = 1;
    while (bin > 0) {
        int digit = bin % 10;
        decimal += digit * base;
        bin /= 10;
        base *= 2;
    }
    return decimal;
}

void AppSettings::cheak_config(char *a) {
    if (GUI::msgbox_yn("提示写入选中WIFI", a, "确定", "取消")) {
        config[PARAM_SSID] = a;
        hal.saveConfig();
    }
    if (GUI::msgbox_yn("立即输入密码？", a, "是", "否")) {
        char pass[256];
        sprintf(pass, "%s", GUI::englishInput("输入密码"));
        config[PARAM_PASS] = pass;
        config[PARAM_CLOCKONLY] = "1";
        // 先确认是否能连上
        if (hal.tryConnectWiFi() == -1) {
            GUI::msgbox("连接失败", "请检查密码是否正确");
            return;
        } else {
            hal.saveConfig();
            GUI::msgbox("提示", "已写入配置，即将重启！");
            esp_restart();
        }
    } else {
        if (GUI::msgbox_yn("提示", "是否启动网页服务器配置密码", "确定",
                           "取消")) {
            hal.WiFiConfigManual();
            ESP.restart();
        }
    }
}
