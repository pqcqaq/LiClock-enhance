#include "AppManager.h"
#include "images/images.h"

class AppClockOnly : public AppBase {
   private:
    /* data */
   public:
    AppClockOnly() {
        name = "clockonly";
        title = "仅时钟";
        description = "仅时钟模式";
        image = clock_bits;
    }
    void set();
    void setup();
};
static AppClockOnly app;
extern const char* dayOfWeek[];
void AppClockOnly::set() {
    _showInList = hal.pref.getBool(hal.get_char_sha_key(title), true);
}
void AppClockOnly::setup() {
    int w;
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", hal.timeinfo.tm_hour, hal.timeinfo.tm_min);
    display.clearScreen();
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setForegroundColor(0);
    u8g2Fonts.setBackgroundColor(1);
    u8g2Fonts.setFont(u8g2_font_logisoso92_tn);
    w = u8g2Fonts.getUTF8Width(timeStr);
    u8g2Fonts.setCursor((296 - w) / 2, 104);
    u8g2Fonts.print(timeStr);
    u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
    display.drawFastHLine(0, 110, 296, 0);
    u8g2Fonts.setCursor(10, 125);
    u8g2Fonts.printf("%02d月%02d日 星期%s  ", hal.timeinfo.tm_mon + 1,
                     hal.timeinfo.tm_mday, dayOfWeek[hal.timeinfo.tm_wday]);

    if (peripherals.peripherals_current & PERIPHERALS_AHT20_BIT) {
        sensors_event_t humidity, temp;
        peripherals.load_append(PERIPHERALS_AHT20_BIT);
        xSemaphoreTake(peripherals.i2cMutex, portMAX_DELAY);
        peripherals.aht.getEvent(&humidity, &temp);
        xSemaphoreGive(peripherals.i2cMutex);
        u8g2Fonts.printf("温度:%.1f℃ 湿度:%.1f%%", temp.temperature,
                         humidity.relative_humidity);
    } else {
        if (peripherals.peripherals_current & PERIPHERALS_BMP280_BIT) {
            peripherals.load_append(PERIPHERALS_BMP280_BIT);
            xSemaphoreTake(peripherals.i2cMutex, portMAX_DELAY);

            float temperature = peripherals.bmp.readTemperature();
            float pressure = peripherals.bmp.readPressure() / 100.0F;  // hPa

            xSemaphoreGive(peripherals.i2cMutex);

            u8g2Fonts.printf("温度:%.1f℃  气压:%.1f hPa", temperature, pressure);
        }
    }

    // ===== 网络状态图标 =====
    const unsigned char* wifi_icon =
        WiFi.isConnected() ? wifi_connected_bits : wifi_disconnected_bits;
    display.drawXBitmap(296 - 50, 111, wifi_icon, 20, 16,
                        0);  // 电池图标左边 25px

    // ===== 电池图标 =====
    display.drawXBitmap(296 - 25, 111, getBatteryIcon(), 20, 16, 0);

    if (force_full_update || part_refresh_count > 20) {
        display.display(false);
        force_full_update = false;
        part_refresh_count = 0;
    } else {
        display.display(true);
        part_refresh_count++;
    }
    appManager.noDeepSleep = false;
    appManager.nextWakeup = 61 - hal.timeinfo.tm_sec;
}
