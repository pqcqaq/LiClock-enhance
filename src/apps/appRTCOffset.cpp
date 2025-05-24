#include "AppManager.h"
extern DS3231 Srtc;
#include "images/images.h"

class AppRTCOffset : public AppBase
{
private:
    /* data */
public:
    AppRTCOffset()
    {
        name = "rtcoffset";
        title = "误差补偿";
        description = "RTC线性误差补偿查看工具";
        image = RTCOffset_bits;
        _showInList = true;
    }
    void set();
    void setup();
};
static AppRTCOffset app;

static void RTCOffsetTimer()
{
    display.setFullWindow();
    display.clearScreen();
    GUI::drawWindowsWithTitle("RTC线性误差补偿", 0, 0, 296, 128);
    u8g2Fonts.setCursor(0, 30);
    time_t now;
    tm rtctime;
    time(&now);
    localtime_r(&now, &rtctime);
    u8g2Fonts.printf("  %02d:%02d:%02d -> %02d:%02d:%02d\n", rtctime.tm_hour, rtctime.tm_min, rtctime.tm_sec, hal.timeinfo.tm_hour, hal.timeinfo.tm_min, hal.timeinfo.tm_sec);
    if (hal.lastsync != 1)
    {
        u8g2Fonts.printf("  上次同步Unix时间：%d\n", hal.lastsync);
    }
    else
    {
        u8g2Fonts.printf("  从未同步时间或时间丢失\n");
    }
    if (hal.every != 100)
    {
        u8g2Fonts.printf("  当前误差数据：每%d分钟误差%d秒\n", hal.every / 60, hal.delta);
    }
    else
    {
        u8g2Fonts.printf("  RTC误差未知\n");
    }
    if(peripherals.peripherals_current & PERIPHERALS_DS3231_BIT)
    {
        u8g2Fonts.printf(" [ 误差数据为内置RTC ]\n [ 正在使用外部DS3231时钟源，误差补偿保持计算 ]\n");
    }
    display.display();
}
void AppRTCOffset::set(){
    _showInList = hal.pref.getBool(hal.get_char_sha_key(title), true);
}
void AppRTCOffset::setup()
{
    RTCOffsetTimer();
    appManager.setTimer(15, RTCOffsetTimer);
    appManager.noDeepSleep = true;
}
