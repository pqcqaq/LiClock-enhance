#include "AppManager.h"
#include "images/images.h"

class AppOOBE : public AppBase {
   private:
    /* data */
   public:
    AppOOBE() {
        name = "oobe";
        title = "OOBE";
        description = "OOBE";
        image = NULL;
        _reentrant = false;
        _showInList = false;
    }
    void set();
    void waitClick();
    void setup();
};
static AppOOBE app;
void AppOOBE::set() {
    _showInList = hal.pref.getBool(hal.get_char_sha_key(title), false);
}
void AppOOBE::waitClick() {
    while (hal.btnl.isPressing() == false && hal.btnr.isPressing() == false &&
           hal.btnc.isPressing() == false)
        delay(10);
}
void AppOOBE::setup() {
    hal.pref.putInt("lpt", 350);
    hal.pref.getInt("ppc", 7230);
    bool ret = false;
    int reti = 0;
    int stage = hal.pref.getInt("oobe", 0);
    if (stage == 3) {
        if (GUI::msgbox_yn("提示", "是否继续进入OOBE？") == false) {
            appManager.goBack();
            return;
        }
    } else if (stage == 1) {
        goto stage1;
    } else if (stage == 2) {
        goto stage2;
    }
    display.clearScreen();
    display.drawXBitmap(0, 0, OOBE_Start_bits, 296, 128, 0);
    display.display();
    waitClick();
    display.clearScreen();
    display.drawXBitmap(0, 0, OOBE_checkWiring_bits, 296, 128, 0);
    display.display(true);
    hal.pref.putInt("oobe", 1);
    peripherals.check();
stage1:
    display.clearScreen();
    display.drawXBitmap(0, 0, OOBE_BG_bits, 296, 128, 0);
    display.display(true);
    ret = GUI::msgbox_yn("提示", "是否跳过教学？");
    if (ret) goto jmp_tutorial;
    while (ret == false) {
        ret = GUI::msgbox_yn("这是一个提示框", "请选择“确定”", "确定", "取消");
    }
    while (ret == true) {
        ret = GUI::msgbox_yn("这是一个提示框", "请选择“取消”", "确定", "取消");
    }
    while (reti == 0) {
        reti = GUI::msgbox_number("请随便输入一个数字", 3, 0);
        GUI::msgbox("提示", (String("你输入的是") + String(reti)).c_str());
    }
jmp_tutorial:
stage2:
    display.clearScreen();
    display.drawXBitmap(0, 0, OOBE_BG_bits, 296, 128, 0);
    u8g2Fonts.drawUTF8(40, 85, "网络连接");
    display.display(true);
    hal.pref.putInt("oobe", 2);
    if (config[PARAM_CLOCKONLY] == "1" && hal.pref.getInt("oobe", 0) == 2) {
        display.clearScreen();
        u8g2Fonts.drawUTF8(95, 100, "使用离线模式，重启中...");
        display.display(true);
        hal.pref.putString(SETTINGS_PARAM_HOME_APP, "clock");
        hal.pref.putInt("oobe", 3);
        ESP.restart();
    }
    hal.autoConnectWiFi();
    u8g2Fonts.drawUTF8(40, 100, "NTP同步");
    display.display(true);
    NTPSync();
    u8g2Fonts.drawUTF8(95, 100, "完成，重启中...");
    display.display(true);
    hal.pref.putString(SETTINGS_PARAM_HOME_APP, "clock");
    hal.pref.putInt("oobe", 3);
    ESP.restart();
}