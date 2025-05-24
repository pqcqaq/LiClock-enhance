#include "AppManager.h"
#include "images/images.h"

const char *dayOfWeek[] = {"日", "一", "二", "三", "四", "五", "六"};
RTC_DATA_ATTR int part_refresh_count = 0;
RTC_DATA_ATTR bool force_full_update = false;
RTC_DATA_ATTR int last_alertPubTime = 0;
static void appclock_exit()
{
    force_full_update = true;
}
static void appclock_deepsleep()
{
}

class AppClock : public AppBase
{
private:
    /* data */
public:
    AppClock()
    {
        name = "clock";
        title = "天气时钟";
        description = "默认主页应用";
        image = clock_cloud_bits;
    }
    void set();
    void setup();

private:
    // 获取分钟对应图表位移
    inline int getMinuteShift(int minute)
    {
        return minute * PX_PER_SAMPLE / 60;
    }
    void drawWeatherIcon(int pos, int code, int y = 90)
    {
        display.drawXBitmap(pos, y, weather_icons_day[code].data, weather_icons_day[code].width, weather_icons_day[code].height, GxEPD_BLACK);
    }
    // 在屏幕底部显示时间、降水、天气
    // 每格占37像素
    // 右边多打一个时间保证滚动显示效果
    void drawPlot(int hour = 0, int minute = 0)
    {
        int minute_shift = getMinuteShift(minute);
        display.fillRect(0, 122, SCREEN_WIDTH, 6, GxEPD_BLACK);
        display.setFont(&Picopixel);
        display.setTextColor(GxEPD_WHITE);
        for (int i = 0; i < SAMPLE_COUNT; i++)
        {
            int x = (i * SCREEN_WIDTH) / 8 - minute_shift;
            display.setCursor(x, 127);
            display.printf("%02d:00", (i + hour) % 24);
        }
        display.setTextColor(GxEPD_BLACK);
        // 准备降水数据
        uint16_t rainmax = 0;
        for (int i = 0; i < SAMPLE_COUNT; ++i)
        {
            if (weather.hour24[i + hal.global_hour_offset].rain > rainmax)
                rainmax = weather.hour24[i + hal.global_hour_offset].rain;
            rain_data_raw[i] = weather.hour24[i + hal.global_hour_offset].rain;
        }
        if (rainmax < 500)
            rainmax = 500;
        processRain((float)(rainmax));
        // 绘制降水图表，纵坐标范围82-122
        display.fillRect(0, 122 - GRAPH_HEIGHT, SCREEN_WIDTH, GRAPH_HEIGHT, GxEPD_WHITE);
        for (int i = 0; i < SCREEN_WIDTH; ++i)
        {
            // 绘制虚线阴影
            // 偶数列只填充偶数像素，否则填充奇数像素
            if (ydata[i + minute_shift] != 0)
            {
                display.drawPixel(i, 122 - ydata[i + minute_shift], GxEPD_BLACK);
                for (int j = 122 - ydata[i + minute_shift]; j < 122; j++)
                {
                    if ((i % 2) == (j % 2))
                        display.drawPixel(i, j, GxEPD_BLACK);
                }
            }
        }
        // 绘制天气图标
        const void *last_symb = weather_icons_day[weather.hour24[hal.global_hour_offset].weathernum].data;
        int i = 0;
        if ((last_symb == weather_icons_day[weather.hour24[1 + hal.global_hour_offset].weathernum].data) || (minute_shift < 10))
        {
            drawWeatherIcon(0, weather.hour24[hal.global_hour_offset].weathernum); // 特殊处理，第一个图标放在屏幕最左侧防止看不到
            i = 1;
        }
        for (; i < SAMPLE_COUNT; ++i)
        {
            if (weather_icons_day[weather.hour24[i + hal.global_hour_offset].weathernum].data != last_symb)
            {
                last_symb = weather_icons_day[weather.hour24[i + hal.global_hour_offset].weathernum].data;
                drawWeatherIcon(i * PX_PER_SAMPLE - minute_shift, weather.hour24[i + hal.global_hour_offset].weathernum);
            }
        }
    }

    void drawTime(int hour, int minute)
    {
        display.setFont(&FreeSans18pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(7, 28);
        display.fillRect(7, 0, 86, 32, GxEPD_WHITE);
        display.printf("%02d:%02d", hour, minute);
        display.drawFastHLine(5, 30, 89, GxEPD_BLACK);
    }

    void drawDateAndDesc(int month, int date, int day)
    {
        u8g2Fonts.setCursor(2, 43);
        u8g2Fonts.printf("%02d月%02d日 星期%s", month, date, dayOfWeek[day]);
        display.drawFastHLine(5, 45, 89, GxEPD_BLACK);
        u8g2Fonts.setCursor(2 + 24, 43 + 14);
        GUI::autoIndentDraw(weather.desc1, 88);
        u8g2Fonts.setCursor(100, 12);
        u8g2Fonts.print(weather.desc2);
    }
    void drawTemp(int max, int min, int x)
    {
        int w;
        char s[12];
        sprintf(s, "%d/%d", max, min);
        w = u8g2Fonts.getUTF8Width(s);
        u8g2Fonts.setCursor(96 + 35 + x - w, 77);
        u8g2Fonts.print(s);
    }
    void drawFrame()
    {
        // display.drawLine(0, 82, SCREEN_WIDTH, 82, GxEPD_BLACK);
        display.fillRect(0, 96, 85, 200, GxEPD_WHITE); // 清空天气图标区域
        display.drawXBitmap(96, 2, weather_frames[0].data, weather_frames[0].width, weather_frames[0].height, GxEPD_BLACK);
        display.drawXBitmap(96 + 50, 2, weather_frames[1].data, weather_frames[1].width, weather_frames[1].height, GxEPD_BLACK);
        display.drawXBitmap(96 + 100, 2, weather_frames[2].data, weather_frames[2].width, weather_frames[2].height, GxEPD_BLACK);
        display.drawXBitmap(96 + 150, 2, weather_frames[3].data, weather_frames[3].width, weather_frames[3].height, GxEPD_BLACK);
        display.setFont(&FreeSans9pt7b);
        display.setCursor(96 + 12, 54);
        display.print(weather.realtime.temperature / 10);
        if (weather.realtime.humidity == 100)
            display.setCursor(96 + 8, 75);
        else if (weather.realtime.humidity == 0)
            display.setCursor(96 + 16, 75);
        else
            display.setCursor(96 + 12, 75);
        display.print(weather.realtime.humidity);
        drawWeatherIcon(96 + 50 + 12, weather.five_days[0].weathernum, 35);
        drawWeatherIcon(96 + 100 + 12, weather.five_days[1].weathernum, 35);
        drawWeatherIcon(96 + 150 + 12, weather.five_days[2].weathernum, 35);
        drawTemp(weather.five_days[0].max / 10, weather.five_days[0].min / 10, 50);
        drawTemp(weather.five_days[1].max / 10, weather.five_days[1].min / 10, 100);
        drawTemp(weather.five_days[2].max / 10, weather.five_days[2].min / 10, 150);
    }
    // 绘制全部内容
    void drawLayout()
    {
        display.fillScreen(GxEPD_WHITE);
        hal.getTime();
        drawFrame();
        drawDateAndDesc(hal.timeinfo.tm_mon + 1, hal.timeinfo.tm_mday, hal.timeinfo.tm_wday);
        drawPlot(hal.timeinfo.tm_hour, hal.timeinfo.tm_min);
        drawTime(hal.timeinfo.tm_hour, hal.timeinfo.tm_min);
    }
};
static AppClock app;
static RTC_DATA_ATTR uint8_t NTPCounter = 0;
void appclock_wakeup()
{
    app.setup();
}
void AppClock::set(){
    _showInList = hal.pref.getBool(hal.get_char_sha_key(title), true);
}
void AppClock::setup()
{
    exit = appclock_exit;
    deepsleep = appclock_deepsleep;
    wakeup = appclock_wakeup;
    int ntp_interval = hal.getNTPMinute();
    if (hal.wakeUpFromDeepSleep)
    {
        ++NTPCounter;
        if (NTPCounter < ntp_interval)
        {
            if (force_full_update == false && part_refresh_count < 15)
            {
                // 局部刷新
                drawLayout();
                display.display(true);
                ++part_refresh_count;
                appManager.noDeepSleep = false;
                appManager.nextWakeup = 61 - hal.timeinfo.tm_sec;
                Serial.println("Finished part");
                //Serial.printf("%d.%d.%d %d %d:%d:%d\n", hal.timeinfo.tm_year + 1900, hal.timeinfo.tm_mon + 1, hal.timeinfo.tm_mday, hal.timeinfo.tm_wday, hal.timeinfo.tm_hour, hal.timeinfo.tm_min, hal.timeinfo.tm_sec);
                return;
            }
        }
    }
    // 局部刷新不需要下面这些初始化操作
    force_full_update = false;
    part_refresh_count = 0;
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    hal.getTime();
    if (hal.timeinfo.tm_year < (2016 - 1900) || NTPCounter >= ntp_interval) // 等待NTP同步
    {
        //GUI::info_msgbox("提示", "NTP正在同步，请稍等...");
        NTPCounter = 0;
        delay(20);
        hal.autoConnectWiFi();
        NTPSync();
        hal.getTime();
    }
    if (hal.now < weather.lastupdate || hal.now - weather.lastupdate > 60 * atoi(config[PARAM_FULLUPDATE].as<const char *>()))
    {
        //GUI::info_msgbox("提示", "正在更新天气数据...");
        hal.autoConnectWiFi();
        weather.refresh();
    }
    if (weather.hasAlert && weather.alertPubTime != last_alertPubTime)
    {
        last_alertPubTime = weather.alertPubTime;
        appManager.gotoApp("warning");
        force_full_update = true;
        return;
    }
    drawLayout();
    display.display(false);
    appManager.noDeepSleep = false;
    appManager.nextWakeup = 61 - hal.timeinfo.tm_sec;
    Serial.println("Finished full");
    //Serial.printf("%d.%d.%d %d %d:%d:%d\n", hal.timeinfo.tm_year + 1900, hal.timeinfo.tm_mon + 1, hal.timeinfo.tm_mday, hal.timeinfo.tm_wday, hal.timeinfo.tm_hour, hal.timeinfo.tm_min, hal.timeinfo.tm_sec);
    return;
}
