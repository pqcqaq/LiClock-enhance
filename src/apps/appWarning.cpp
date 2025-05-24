#include "AppManager.h"
#include "images/images.h"

class AppWarning : public AppBase
{
private:
    /* data */
public:
    AppWarning()
    {
        name = "warning";
        title = "天气预警";
        description = "天气预警信息";
        image = weather_warning_bits;
        _showInList = true;
        _reentrant = false;
    }
    void set();
    void setup();
};
static AppWarning app;

void AppWarning::set(){
    _showInList = hal.pref.getBool(hal.get_char_sha_key(title), true);
}
void AppWarning::setup()
{
    if(hal.btnl.isPressing() || hal.btnr.isPressing() || hal.btnc.isPressing())
    {
        appManager.goBack();
        return;
    }
    display.clearScreen();
    if (weather.hasAlert)
    {
        GUI::drawWindowsWithTitle(weather.alertTitle, 0, 0, 296, 128);
        u8g2Fonts.setBackgroundColor(1);
        u8g2Fonts.setForegroundColor(0);
        u8g2Fonts.setCursor(18 + 24, 31);
        char *str = weather.alert;
        while (*str)
        {
            if (u8g2Fonts.getCursorX() >= 296 - 16)
            {
                u8g2Fonts.setCursor(18, u8g2Fonts.getCursorY() + 15);
            }
            u8g2Fonts.print(*str);
            str++;
        }
        display.display(false);
    }
    else
    {
        u8g2Fonts.drawUTF8(75, 100, "无预警，3秒后自动退出");
        display.display(false);
        delay(3000);
        appManager.goBack();
        return;
    }
    appManager.noDeepSleep = false;
    appManager.nextWakeup = 10;
}
