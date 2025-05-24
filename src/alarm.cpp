#include <A_Config.h>
#include <alarm.h>
#include <LittleFS.h>
#include "images/images.h"

Alarm alarms;

void Alarm::load()
{
    // 从LittleFS加载
    if (LittleFS.exists("/System/alarms.bin") == false)
    {
        clearAll();
        save();
        return;
    }
    File f = LittleFS.open("/System/alarms.bin");
    f.readBytes((char *)alarm_table, sizeof(alarm_table));
    f.close();
}

void Alarm::save()
{
    File f = LittleFS.open("/System/alarms.bin", "w");
    f.write((uint8_t *)alarm_table, sizeof(alarm_table));
    f.close();
}

int8_t Alarm::getNext(uint16_t week, uint16_t now)
{
    uint16_t bit_week = BIT(week);
    int next = -1;
    uint16_t next_time = 0;
    for (int i = 0; i < 5; ++i)
    {
        if ((alarm_table[i].enable & bit_week) || (alarm_table[i].enable & 0x80))
        {
            if (alarm_table[i].time > now)
            {
                // TODO!!!
                if (next == -1)
                {
                    next = i;
                    next_time = alarm_table[i].time;
                    Serial.printf("[闹钟]id=%d, time=%d", next, next_time);
                }
                else
                {
                    if (alarm_table[i].time < next_time)
                    {
                        next = i;
                        next_time = alarm_table[i].time;
                        Serial.printf("[闹钟]id=%d, time=%d", next, next_time);
                    }
                }
            }
        }
    }
    return next;
}

void Alarm::alarm()
{
    String filename = hal.pref.getString(SETTINGS_PARAM_ALARM_TONE, "");
    bool beep = false;
    if (filename == "")
    {
        beep = true;
        buzzer.append(2000, 500);
    }
    else
    {
        beep = false;
        buzzer.playFile(filename.c_str());
    }
    display.clearScreen();
    display.drawXBitmap(88, 4, alarm_clock_bits, alarm_clock_bits_width, alarm_clock_bits_height, 0);
    display.display();
    for (int16_t i = 0; i < 600; ++i)
    {
        if (hal.btnc.isPressing())
        {
            break;
        }
        delay(100);
        if (beep == true)
        {
            if (i % 5 == 0)
            {
                buzzer.append(2000, 500);
            }
        }
    }
    buzzer.forceStop();
    display.clearScreen();
    display.display(true);
}

int8_t RTC_DATA_ATTR next_alarm_to = -1;
// 闹钟检查原理
// next_alarm_to如果不是0：说明上次启动已经判断过下次时间
// 如果是0,则更新下次闹钟时间
void Alarm::check()
{
    if (next_alarm_to == -1)
    {
        int8_t nextIdx = getNext(hal.timeinfo.tm_wday, hal.timeinfo.tm_hour * 60 + hal.timeinfo.tm_min);
        if (nextIdx != -1)
        {
            // 今天有闹钟
            next_alarm_to = nextIdx;
        }
    }
    else
    {
        int8_t tmp = getNext(hal.timeinfo.tm_wday, hal.timeinfo.tm_hour * 60 + hal.timeinfo.tm_min);
        Serial.printf("[闹钟]存在下一个闹钟：id=%d, tmp=%d", next_alarm_to, tmp);
        if (tmp != next_alarm_to)
        {
            alarm();
            if(alarm_table[next_alarm_to].enable == 0x80)
            {
                alarm_table[next_alarm_to].enable = ALARM_DISABLE;
                save();
            }
            next_alarm_to = tmp;
        }
    }
}

int Alarm::getNextWakeupMinute()
{
    if (next_alarm_to != -1)
        return alarm_table[next_alarm_to].time;
    return 0;
}

extern const char *dayOfWeek[];
String Alarm::getEnable(alarm_t *alarm)
{
    if (alarm->enable == 0)
        return "关";
    if (alarm->enable == 0x80)
        return "单次";
    if (alarm->enable == 0x7F)
        return "每天";
    if (alarm->enable == 0b00111110)
        return "周一到周五";
    if (alarm->enable == 0b01000001)
        return "周六和周日";
    String result = "";
    for (uint8_t i = 0; i < 7; ++i)
    {
        if (BIT(i) & alarm->enable)
        {
            result += dayOfWeek[i];
        }
    }
    return result;
}
