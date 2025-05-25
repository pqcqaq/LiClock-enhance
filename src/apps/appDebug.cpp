#include "AppManager.h"
#include "images/images.h"

class AppDebug : public AppBase
{
public:
    AppDebug()
    {
        name = "debug";
        title = "Debug";
        description = "Debug专用";
        image = debug_bits;
        _showInList = false;
        _reentrant = false;
    }
    void set();
    void setup();
};

static AppDebug app;

void AppDebug::set()
{
    _showInList = hal.pref.getBool(hal.get_char_sha_key(title), false);
}

void AppDebug::setup()
{
    alarms.alarm();  // 蜂鸣器提示音

    // 1. 获取用户输入
    int res = GUI::msgbox_number("请输入数字", 4, 1234);

    // 2. 显示输入结果
    display.fillScreen(1);
    GUI::drawWindowsWithTitle("输入结果", 10, 10, 280, 50);
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(0);
    display.setCursor(30, 45);
    display.printf("你输入了: %d", res);
    display.display();
    delay(2000);

    // 3. 循环等待用户长按按键
    struct {
        const char *label;
        int pin;
    } steps[] = {
        {"请长按 中间键 (Center)", PIN_BUTTONC},
        {"请长按 左键 (Left)", PIN_BUTTONL},
        {"请长按 右键 (Right)", PIN_BUTTONR},
    };

    for (int i = 0; i < 3; ++i) {
        bool longPressed = false;
        while (!longPressed) {
            display.fillScreen(1);
            GUI::drawWindowsWithTitle("按键测试", 10, 10, 280, 50);
            display.setFont(&FreeSans9pt7b);
            display.setTextColor(0);
            display.setCursor(20, 45);
            display.println(steps[i].label);
            display.setCursor(20, 70);
            display.println("等待长按...");
            display.display();

            // 如果检测到长按就跳出循环
            if (GUI::waitLongPress(steps[i].pin)) {
                longPressed = true;
                display.setCursor(20, 95);
                display.println("检测到长按 ✔");
                display.display();
                delay(1000);
            }

            delay(100);  // 减少 CPU 占用
        }
    }

    // 4. 结束提示
    display.fillScreen(1);
    GUI::drawWindowsWithTitle("完成", 10, 10, 280, 50);
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(0);
    display.setCursor(40, 45);
    display.println("调试完成，即将返回...");
    display.display();
    delay(2000);

    appManager.goBack();
} 
