#pragma once

#include <Arduino.h>
#include <GxEPD2.h>
#include <GxEPD2_BW.h>
#include <Fonts/Picopixel.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TJpg_Decoder.h>
#include "qrcode.h"
#include <esp_sntp.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "esp_task_wdt.h"
#include <driver/rtc_io.h>
#include <esp_netif.h>
#include <LittleFS.h>
#include "images/images.h"

// pre-build-auto-generated: timestamp
#define built_at "2025-05-27 21:38:48"
// pre-build-auto-generated: version
#define code_version "2.0.11"

#define SCREEN_WIDTH 296
#define SCREEN_HEIGHT 128
#define PIN_ADC 33

#define PIN_BUTTONL 35
#define PIN_BUTTONC 34
#define PIN_BUTTONR 39

#define PIN_RTC_IRQ 25
#define PIN_SDVDD_CTRL 27
#define PIN_CHARGING 26
#define PIN_SD_CS 14
#define PIN_SD_MOSI 12
#define PIN_SD_SCLK 13
#define PIN_SD_MISO 15
#define PIN_SD_CARDDETECT 2
#define PIN_BUZZER 21
#define PIN_SDA 23
#define PIN_SCL 22

#define F_LOG(fmt, ...) \
  do { \
    File file_log = LittleFS.open("/System/error log.txt", "a"); \
    if (file_log) { \
      file_log.printf("[%06d]", esp_log_timestamp());\
      file_log.printf("[%s:%d] ", __FILE__, __LINE__); \
      file_log.printf(fmt, ##__VA_ARGS__); \
      file_log.println(); \
    } else { \
      Serial.println("无法打开日志文件"); \
    } \
    file_log.close(); \
  } while (0)
// 下面这些尽量不要修改，因为改了不完全有效
#define GRAPH_HEIGHT 37
#define SAMPLE_COUNT 10
#define SAMPLE_STEP 1
#define PX_PER_SAMPLE (SCREEN_WIDTH / SAMPLE_STEP / (SAMPLE_COUNT - 2))
#define DEFAULT_CONFIG "{\"p1\":\"116.3975,39.9091\",\"p2\":\"15\",\"p3\":\"1\",\"p4\":\"23:30\",\"p5\":\"05:00\",\"p6\":\"\",\"p7\":\"\",\"p8\":\"0\"}"
#define TFmode "1"
#define autontpsync "0"
#define WeatherClocksource "0"

extern float rain_data_raw[];
extern int ydata[];
extern const image_desc weather_icons_day[];
extern const image_desc weather_frames[4];
extern const image_desc weather_icons_night[];

extern esp_ip6_addr_t ipv6global;
extern esp_ip6_addr_t ipv6local;
extern const char *ipv6_to_str(const esp_ip6_addr_t *addr);
extern void enableIPv6();
void refreshIPV6Addr();
bool file_exist(const char *path);

extern DynamicJsonDocument config;
extern DynamicJsonDocument cfu;
extern GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display;
extern U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
extern TJpg_Decoder TJpgDec;

extern bool force_full_update;
extern int part_refresh_count;
extern uint8_t night_sleep;          // 夜间模式屏幕状态，0：不在夜间模式，1：晚安，2：早上好
extern uint8_t night_sleep_today; // 今天是否进入过夜间模式
extern bool LuaRunning;            //全局变量，表示Lua服务器是否运行，用于防止调试时误退出

#define PARAM_GPS "p1"
#define PARAM_FULLUPDATE "p2"
#define PARAM_SLEEPATNIGHT "p3"
#define PARAM_SLEEPATNIGHT_START "p4"
#define PARAM_SLEEPATNIGHT_END "p5"
#define PARAM_SSID "p6"
#define PARAM_PASS "p7"
#define PARAM_CLOCKONLY "p8"

void processRain(float max);
void beginWebServer();
void updateWebServer();
const uint8_t *getBatteryIcon();

#include "hal.h"
#include "weather.h"
#include "myNTP.h"
#include "AppManager.h"
#include "GUI.h"
#include "settings.h"
#include "alarm.h"
#include "peripherals.h"
#include "Buzzer.h"
#include "lua_trans.h"
extern const char *getRealPath(const char *fpath);
extern void setPath(const char *path);