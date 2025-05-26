#include "hal.h"
#include <LittleFS.h>
#include "images/images.h"

void task_hal_update(void *)
{
    while (1)
    {
        if (hal._hookButton)
        {
            while (hal.btnr.isPressing() || hal.btnl.isPressing() || hal.btnc.isPressing())
            {
                hal.btnr.tick();
                hal.btnl.tick();
                hal.btnc.tick();
                delay(20);
            }
            hal.btnr.tick();
            hal.btnl.tick();
            hal.btnc.tick();
            while (hal._hookButton)
            {
                while (hal.SleepUpdateMutex)
                    delay(10);
                hal.update();
                delay(20);
            }
            while (hal.btnr.isPressing() || hal.btnl.isPressing() || hal.btnc.isPressing())
            {
                delay(20);
            }
        }
        while (hal.SleepUpdateMutex)
            delay(10);
        hal.SleepUpdateMutex = true;
        hal.btnr.tick();
        hal.btnl.tick();
        hal.btnc.tick();
        hal.SleepUpdateMutex = false;
        delay(20);
        while (hal.SleepUpdateMutex)
            delay(10);
        hal.SleepUpdateMutex = true;
        hal.btnr.tick();
        hal.btnl.tick();
        hal.btnc.tick();
        hal.update();
        hal.SleepUpdateMutex = false;
        delay(20);
    }
}
void task_btn_buzzer(void *){
    bool buz_l = false, buz_r = false, buz_c = false;
    int buz_freq = hal.pref.getInt("btn_buz_freq", 150);
    int buz_time = hal.pref.getInt("btn_buz_time", 100);
    while(1){
        if (hal.btnl.isPressing() && !buz_l){
            buz_l = true;
            buzzer.append(buz_freq, buz_time);
        }else if (hal.btnr.isPressing() && !buz_r){
            buz_r = true;
            buzzer.append(buz_freq, buz_time);
        }else if (hal.btnc.isPressing() && !buz_c){
            buz_c = true;
            buzzer.append(buz_freq, buz_time);
        }
        if (!hal.btnl.isPressing() && buz_l)
            buz_l = false;
        else if(!hal.btnr.isPressing() && buz_r)
            buz_r = false;
        if (!hal.btnc.isPressing() && buz_c)
            buz_c = false;
        delay(50);
    }
}
void cfu_task(void *){
    File cfufile = LittleFS.open("/System/CFU.json", "r");
    bool file_true = true;
    if (!cfufile)
    {
        Serial.println("Failed to open cfu file");
        file_true = false;
    }
    deserializeJson(cfu, cfufile);
    cfufile.close();
    if (file_true){
        if (strcmp(cfu["version"].as<String>().c_str(), code_version) != 0){
            ESP_LOGW("HAL", "当前版本为 %s，最新版本为 %s，建议尽快升级以获得最佳体验。", code_version, cfu["version"].as<String>().c_str());
            strcpy(hal.update_version, cfu["version"].as<String>().c_str());
            hal.has_new_firmware = true;
        }
        int lognum = cfu["updateinfo"]["lognum"].as<int>(); // 转换为整数
        hal.update_info_num = lognum;
        String _update_log[lognum];
        
        if (hal.has_new_firmware){
            strcpy(hal.update_url[0], cfu["updateinfo"]["url"].as<String>().c_str());
            strcpy(hal.update_url[1], cfu["updateinfo"]["url1"].as<String>().c_str());
            JsonArray updatelog = cfu["updateinfo"]["log"];
            int i = 0;
            for (JsonVariant item : updatelog){
                _update_log[i] = item.as<String>();
                i++;
            }
            hal.is_big_update = cfu["updateinfo"]["bigupdate"].as<bool>();
        }
        int j = 0;
        while(j < lognum){ // 修改循环条件
            sprintf(hal.update_info[j], "%s", _update_log[j].c_str());
            j++;
        }
    }
    vTaskDelete(NULL);
}
void HAL::saveConfig()
{
    File configFile = LittleFS.open("/System/config.json", "w");
    if (!configFile)
    {
        Serial.println("Failed to open config file for writing");
        return;
    }
    serializeJson(config, configFile);
    configFile.close();
}
void HAL::loadConfig()
{
    File configFile = LittleFS.open("/System/config.json", "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file");
        return;
    }
    deserializeJson(config, configFile);
    configFile.close();
}

void HAL::getTime()
{
    int64_t tmp;
    if (peripherals.peripherals_current & PERIPHERALS_DS3231_BIT)
    {
        xSemaphoreTake(peripherals.i2cMutex, portMAX_DELAY);
        timeinfo.tm_year = peripherals.rtc.getYear() + 2000;
        timeinfo.tm_mon = peripherals.rtc.getMonth();
        timeinfo.tm_mday = peripherals.rtc.getDate();
        timeinfo.tm_hour = peripherals.rtc.getHour();
        timeinfo.tm_min = peripherals.rtc.getMinute();
        timeinfo.tm_sec = peripherals.rtc.getSecond();
        timeinfo.tm_wday = peripherals.rtc.getDoW() - 1;
        now = mktime(&timeinfo);
        xSemaphoreGive(peripherals.i2cMutex);
    }
    else
    {
        time(&now);
        if (delta != 0 && lastsync < now)
        {
            // 下面修正时钟频率偏移
            tmp = now - lastsync;
            tmp *= delta;
            tmp /= every;
            now -= tmp;
        }
        localtime_r(&now, &timeinfo);
    }
}

#include <esp32\rom\sha.h>
    char key[16];
char* HAL::get_char_sha_key(const char *str){
    SHA_CTX ctx;
    uint8_t temp[32];
    char hex_hash[65];  // 64 字节的十六进制字符串 + 1 字节的 null 终止符
    ets_sha_enable();
    ets_sha_init(&ctx);  // 初始化上下文
    ets_sha_update(&ctx, SHA2_256, (const uint8_t *)str, strlen(str) * 8); // 更新哈希值
    ets_sha_finish(&ctx, SHA2_256, temp); // 完成哈希计算
    // 将哈希值转换为十六进制字符串
    for (int j = 0; j < 32; j++) {
        sprintf(hex_hash + j * 2, "%02x", temp[j]);
    }
    // 截取前 15 个字符作为 key
    strncpy(key, hex_hash, 15);
    key[15] = '\0';  // 确保字符串以 null 结尾
    ets_sha_disable();
    return key;
}

static void cheak_freq()
{
    int freq = ESP.getCpuFreqMHz();
    if (freq < 80){
        bool cpuset = setCpuFrequencyMhz(80);
        Serial.begin(115200);
        ESP_LOGI("hal", "CpuFreq: %dMHZ -> 80MHZ", freq);
        F_LOG("CpuFreq: %dMHZ -> 80MHZ", freq);
        if(cpuset)
            {ESP_LOGI("hal", "ok");F_LOG("已调节CPU频率至可启用WIFI的状态");}
        else{ESP_LOGW("hal", "err");F_LOG("CPU频率调节失败");}
    }
}
#define is_test 1
#define url_is_test 0
#define url_test "http://192.168.101.12:5500/firmware-info.json"
#define url_firmware "https://kanfandelong.github.io/liclock-web-flash/firmware-info.json"
#define CAcert_file "/System/_.github.io.crt"
/* const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n" \
"2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n" \
"1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n" \
"q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n" \
"tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n" \
"vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n" \
"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n" \
"5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n" \
"1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n" \
"NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n" \
"Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n" \
"8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n" \
"pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n" \
"MrY=\n" \
"-----END CERTIFICATE-----"; */
bool firmware_cheak = false;
void HAL::cheak_firmware_update(){
    log_i("开始检查固件更新...");
    if (!WiFi.isConnected() || firmware_cheak)
        return;
    else
        GUI::info_msgbox("提示", "检查固件更新...");
    HTTPClient http;
    char* ca_cert;
    if (LittleFS.exists(CAcert_file)){
        File CAcert = LittleFS.open(CAcert_file, "r");
        // 计算动态缓冲区大小（考虑CRLF可能被替换为LF）
        size_t file_size = CAcert.size();
        // 假设每个CRLF可能被替换为LF，最大需要file_size * 2的空间（极端情况）
        ca_cert = new (std::nothrow) char[file_size + 1]; // +1为终止符
          // 读取证书内容并替换CRLF为LF
        size_t index = 0;
        while (CAcert.available()) {
            char c = CAcert.read();
            if (c == '\r' && CAcert.peek() == '\n') {
                // 遇到CRLF，替换为LF
                ca_cert[index++] = '\n';
                CAcert.read(); // 跳过下一个字符（\n）
            } else {
                ca_cert[index++] = c;
            }
            // 防止缓冲区溢出
            if (index >= file_size * 2) {
                Serial.println("缓冲区溢出，证书可能被截断");
                break;
            }
        }
        ca_cert[index] = '\0'; // 添加终止符
    }
    log_i("CAcert: \n%s", ca_cert);
    http.setTimeout(20000); 
    if (url_is_test)
        http.begin((String)url_test);
    else
        http.begin((String)url_firmware, ca_cert);
    int httpCode = http.GET();
    run:
    if (httpCode == HTTP_CODE_OK){
        firmware_cheak = true;
        DynamicJsonDocument doc(2048);
        String http_str = http.getString();
        deserializeJson(doc, http_str);
        Serial.println("正在写入固件版本检查文件...");
        File f = LittleFS.open("/System/CFU.json", "w");
        f.print(http_str);
        f.close();
        if (strcmp(doc["version"], code_version) == 0)
            hal.has_new_firmware = false;
        else{
            hal.has_new_firmware = true;
            strcpy(hal.update_version, doc["version"]);
        }
        int lognum = doc["updateinfo"]["lognum"].as<int>(); // 转换为整数
        hal.update_info_num = lognum;
        String _update_log[lognum];
        
        if (hal.has_new_firmware){
            strcpy(hal.update_url[0], doc["updateinfo"]["url"].as<String>().c_str());
            strcpy(hal.update_url[1], doc["updateinfo"]["url1"].as<String>().c_str());
            JsonArray updatelog = doc["updateinfo"]["log"];
            int i = 0;
            for (JsonVariant item : updatelog){
                _update_log[i] = item.as<String>();
                i++;
            }
            hal.is_big_update = doc["updateinfo"]["bigupdate"];
        }
        int j = 0;
        while(j < lognum){ // 修改循环条件
            sprintf(hal.update_info[j], "%s", _update_log[j].c_str());
            j++;
        }
    } else {
        for (int i = 0; i < 5; i++)
        {
            Serial.println("连接失败，正在重试...");
            delay(1000);
            httpCode = http.GET();
            if (httpCode != HTTP_CODE_OK) {
                Serial.printf("请求失败，http code: %d, 重试次数: %d\n", httpCode, i + 1);
                delay(1000); // 等待1秒后重试
            }else
                goto run;
        }
        
        log_e("无法获取固件更新状态,http code:%d", httpCode);
    }
    http.end();
    log_i("结束固件更新状态检查");
}
void HAL::WiFiConfigSmartConfig()
{
    ESP_LOGI("hal", "WiFiConfigManual\n");
    cheak_freq();
#include "images/esptouch.h"
    display.fillScreen(GxEPD_WHITE);
    display.drawXBitmap(0, 0, esptouch_bits, 296, 128, GxEPD_BLACK);
    display.display();
    WiFi.beginSmartConfig();
    int count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        ++count;
        if (count >= 240) // 120秒超时
        {
            Serial.println("SmartConfig超时");
            display.fillScreen(GxEPD_WHITE);
            u8g2Fonts.setCursor(70, 80);
            u8g2Fonts.print("SmartConfig超时");
            display.display();
            delay(100);
            hal.powerOff(false);
            ESP.restart();
        }
    }
    /*
    void esp_dpp_start();
    esp_dpp_start();
    */
    if (WiFi.waitForConnectResult() == WL_CONNECTED)
    {
        Serial.println("WiFi connected");
        config[PARAM_SSID] = WiFi.SSID();
        config[PARAM_PASS] = WiFi.psk();
        hal.saveConfig();
    }
}

#include <DNSServer.h>
void HAL::WiFiConfigManual()
{
    ESP_LOGI("hal", "WiFiConfigManual");
    cheak_freq();
    DNSServer dnsServer;
#include "images/manual.h"
    String passwd = String((esp_random() % 1000000000L) + 10000000L); // 生成随机密码
    String str = "WIFI:T:WPA2;S:WeatherClock;P:" + passwd + ";;";
    WiFi.softAP("WeatherClock", passwd.c_str());
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    beginWebServer();
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(7)];
    uint32_t last_millis = millis();
    bool show_qr = false, show_ssid = false, have_station = false;
    uint8_t StationNum = 0;
    while (1)
    {
        dnsServer.processNextRequest();
        updateWebServer();
        delay(5);
        if (WiFi.softAPgetStationNum() > 0)
        {
            last_millis = millis();
            if (!show_qr)
            {
                String str = "http://192.168.4.1";
                display.fillScreen(GxEPD_WHITE);
                //QRCode qrcode;
                //uint8_t qrcodeData[qrcode_getBufferSize(7)];
                qrcode_initText(&qrcode, qrcodeData, 6, 2, str.c_str());
                Serial.println(qrcode.size);
                for (uint8_t y = 0; y < qrcode.size; y++)
                {
                    // Each horizontal module
                    for (uint8_t x = 0; x < qrcode.size; x++)
                    {
                        display.fillRect(2 * x + 20, 2 * y + 20, 2, 2, qrcode_getModule(&qrcode, x, y) ? GxEPD_BLACK : GxEPD_WHITE);
                    }
                }
                u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
                u8g2Fonts.setCursor(120, ((128 - (14 * 6)) / 2) + 14);
                char buf[256];
                sprintf(buf, "如果使用的是电脑或手机未跳转至配置界面(移动数据可能会干扰跳转),请扫描二维码打开配置界面或浏览器打开http://192.168.4.1");
                GUI::autoIndentDraw(buf, 280, 120, 14);
                display.display();
                show_qr = true;
                have_station = true;
            }
        }
        if (WiFi.softAPgetStationNum() == 0 && have_station) {
            show_qr = false;
            show_ssid = false;
            have_station = false;
        }
        if (!show_ssid){
            display.fillScreen(GxEPD_WHITE);
            display.drawXBitmap(0, 0, wifi_manual_bits, 296, 128, GxEPD_BLACK);
            qrcode_initText(&qrcode, qrcodeData, 6, 0, str.c_str());
            Serial.println(qrcode.size);
            for (uint8_t y = 0; y < qrcode.size; y++)
            {
                // Each horizontal module
                for (uint8_t x = 0; x < qrcode.size; x++)
                {
                    display.fillRect(2 * x + 20, 2 * y + 20, 2, 2, qrcode_getModule(&qrcode, x, y) ? GxEPD_BLACK : GxEPD_WHITE);
                }
            }
            display.setFont(&FreeSans9pt7b);
            display.setCursor(192, 124);
            display.print(passwd);
            display.display();
            show_ssid = true;
        }
        if (millis() - last_millis > 300000) // 10分钟超时
        {
            Serial.println("手动配置超时");
            display.fillScreen(GxEPD_WHITE);
            u8g2Fonts.setCursor(70, 80);
            u8g2Fonts.print("手动配置超时");
            display.display();
            delay(100);
            hal.powerOff(false);
            ESP.restart();
        }
        if (LuaRunning)
            continue;
        if (hal.btnl.isPressing())
        {
            while (hal.btnl.isPressing())
                delay(20);
            LittleFS.end();
            ESP.restart();
            break;
        }
    }
}
void HAL::ReqWiFiConfig()
{
    display.fillScreen(GxEPD_WHITE);
    // u8g2Fonts.setCursor(0, 20);
    // u8g2Fonts.print("无法连接到WiFi");
    // u8g2Fonts.setCursor(0, 40);
    // u8g2Fonts.print("向左:网页配置");
    // u8g2Fonts.setCursor(0, 60);
    // u8g2Fonts.print("向右:SmartConfig");
    // u8g2Fonts.setCursor(0, 80);
    // u8g2Fonts.print("中间:离线模式");
    display.drawXBitmap(0, 0, no_wifi_connected_bits, no_wifi_connected_width, no_wifi_connected_height, 0);
    display.display();
    uint32_t last_millis = millis();
    int a = 0;
    while (1)
    {
        if (hal.btnl.isPressing())
        {
            WiFiConfigManual();
        }
        if (hal.btnr.isPressing())
        {
            WiFiConfigSmartConfig();
        }
        if (hal.btnc.isPressing())
        {
            WiFi.disconnect(true);
            config[PARAM_CLOCKONLY] = "1";
            hal.saveConfig();
            ESP.restart();
            break;
        }
        delay(5);
        if (millis() - last_millis > 60000) // 1分钟超时
        {
            Serial.println("\033[33mWiFi配置方式选择超时\033[32m");
            if (a < 4){
                Serial.println("尝试重连WiFi");
                autoConnectWiFi();
                a++;
                last_millis = millis();
            }
            else{
                break;
            }
        }
    }
    if (WiFi.isConnected() == false)
    {
        config[PARAM_CLOCKONLY] = "1";
        hal.saveConfig();
        ESP.restart();
    }else{
        a = 0;
    }
}
#include "esp_spi_flash.h"
#include "esp_rom_md5.h"
#include "esp_partition.h"
#define PARTITION_TOTAL 4
#define PARTITIONS_OFFSET 0x8000
#define PARTITION_SPIFFS (4 - 1)

void test_littlefs_size(bool format = true)
{
    uint32_t size_request; // 存储目的分区大小
    size_t size_physical = 0;
    esp_flash_get_physical_size(esp_flash_default_chip, &size_physical);
    size_request = size_physical - 0x300000 - 0x1000;
    if (hal.pref.getUInt("size", 0) != size_request)
    {
        Serial.println("检测到分区大小不一致，正在格式化");
        hal.pref.putUInt("size", size_request);
        LittleFS.format();
    }
}
void refresh_partition_table()
{
    md5_context_t ctx;
    static uint8_t table[16 * 20];
    static uint8_t table1[16 * 20];
    esp_rom_md5_init(&ctx);
    union
    {
        uint32_t size;
        uint8_t size_byte[4];
    } partition_size;
    uint32_t size_request; // 存储目的分区大小
    size_t size_physical = 0;
    esp_flash_get_physical_size(esp_flash_default_chip, &size_physical);
    size_request = size_physical - 0x300000 - 0x1000;
    esp_flash_read(esp_flash_default_chip, table, 0x8000, sizeof(table));
    memcpy(partition_size.size_byte, &table[16 * 2 * PARTITION_SPIFFS + 0x8], 4);
    Serial.printf("当前LittleFS分区大小%d\n期望LittleFS分区大小%d\n", partition_size.size, size_request);
    if (partition_size.size != size_request)
    {
        Serial.printf("正在修改分区表\n");
        partition_size.size = size_request;
        memcpy(&table[16 * 2 * PARTITION_SPIFFS + 0x8], partition_size.size_byte, 4);
        Serial.println("正在计算MD5\n");
        esp_rom_md5_update(&ctx, table, 16 * 2 * PARTITION_TOTAL);
        esp_rom_md5_final(&table[16 * (2 * PARTITION_TOTAL + 1)], &ctx);
        esp_flash_set_chip_write_protect(esp_flash_default_chip, false);
        Serial.println("\n正在写入");
        if (esp_flash_erase_region(esp_flash_default_chip, 0x8000, 0x1000) != ESP_OK)
        {
            Serial.println("擦除失败");
            while (1)
                vTaskDelay(1000);
        }
        if (esp_flash_write(esp_flash_default_chip, table, 0x8000, sizeof(table)) != ESP_OK)
        {
            Serial.println("写入失败");
            while (1)
                vTaskDelay(1000);
        }
        Serial.println("完成，正在校验结果");
        esp_flash_read(esp_flash_default_chip, table1, 0x8000, sizeof(table1));
        if (memcmp(table, table1, sizeof(table)) != 0)
        {
            Serial.println("校验失败");
            while (1)
                vTaskDelay(1000);
        }
        else
        {
            for (size_t i = 0; i < 16 * 12; i++)
            {
                Serial.printf("0x%02X ", table[i]);
                if ((i + 1) % 16 == 0)
                {
                    Serial.println();
                }
            }
        }
        ESP.restart();
    }
}
void HAL::wait_input(){
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    if (hal.btn_activelow){
        gpio_wakeup_enable((gpio_num_t)PIN_BUTTONC, GPIO_INTR_LOW_LEVEL);
        esp_sleep_enable_gpio_wakeup();
        esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTONL, 0);
        esp_sleep_enable_ext1_wakeup((1LL << PIN_BUTTONR), ESP_EXT1_WAKEUP_ALL_LOW);
    }else{
        //esp_sleep_enable_ext1_wakeup((1ULL << PIN_BUTTONC) | (1ULL << PIN_BUTTONL) | (1ULL << PIN_BUTTONR), ESP_EXT1_WAKEUP_ANY_HIGH);
        gpio_wakeup_enable((gpio_num_t)PIN_BUTTONC, GPIO_INTR_HIGH_LEVEL);
        esp_sleep_enable_gpio_wakeup();
        esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTONL, 1);
        esp_sleep_enable_ext1_wakeup((1LL << PIN_BUTTONR), ESP_EXT1_WAKEUP_ANY_HIGH);
    }
    log_i("进入lightsleep");
    esp_light_sleep_start();
}
static const char esp_rst_str[12][64] = {"UNKNOWN_RESRT", "POWERON_RESET", "EXT_RESET", "SW_RESET", "PANIC_RESET", "INT_WDT_RESET", "TASK_WDT_RESET", "WDT_RESET", "DEEPSLEEP_RESET", "BROWNOUT_RESET", "SDIO_RESET"};
bool HAL::init()
{
    int16_t total_gnd = 0;
    bool timeerr = false;
    bool initial = true;
    Serial.begin(115200);
    setenv("TZ", "CST-8", 1);
    tzset();
    // 读取时钟偏移
    pref.begin("clock");
    pinMode(PIN_BUTTONR, INPUT);
    pinMode(PIN_BUTTONL, INPUT);
    pinMode(PIN_BUTTONC, INPUT);
    total_gnd += digitalRead(PIN_BUTTONR);
    total_gnd += digitalRead(PIN_BUTTONL);
    total_gnd += digitalRead(PIN_BUTTONC);
    if (total_gnd <= 1)
    {
        btnl._buttonPressed = 1;
        btnr._buttonPressed = 1;
        btnc._buttonPressed = 1;
        btn_activelow = false;
    }
    else
    {
        ESP_LOGW("HAL", "此设备为旧版硬件，建议尽快升级以获得最佳体验。");
        btnl._buttonPressed = 0;
        btnr._buttonPressed = 0;
        btnc._buttonPressed = 0;
        btn_activelow = true;
    }
    esp_task_wdt_init(portMAX_DELAY, false);
    pinMode(PIN_CHARGING, INPUT);
    pinMode(PIN_SD_CARDDETECT, INPUT_PULLUP);
    pinMode(PIN_SDVDD_CTRL, OUTPUT);
    digitalWrite(PIN_SDVDD_CTRL, 1);
    digitalWrite(PIN_BUZZER, 0);
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, 0);

    const esp_partition_t *p = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "spiffs");
    if (pref.getUInt("size", 0) != p->size)
    {
        pref.putUInt("size", p->size);
    }
    refresh_partition_table();
    if (pref.getUInt("lastsync") == 0)
    {
        pref.putUInt("lastsync", 1);  // 上次同步时间的准确时间
        pref.putInt("every", 100);    // 两次校准间隔多久
        pref.putInt("delta", 0);      // 这两次校准之间时钟偏差秒数，时钟时间-准确时间
        pref.putInt("upint", 2 * 60); // NTP同步间隔
    }
    lastsync = pref.getUInt("lastsync", 1); // 上次同步时间的准确时间
    every = pref.getInt("every", 100);      // 两次校准间隔多久
    delta = pref.getInt("delta", 0);        // 这两次校准之间时钟偏差秒数，时钟时间-准确时间
    upint = pref.getInt("upint", 2 * 60);   // NTP同步间隔
    // 系统“自检”

    int date = pref.getInt("CpuFreq", 80);
    int freq = ESP.getCpuFreqMHz();
    if (freq != date)
    {
        bool cpuset = setCpuFrequencyMhz(date);
        Serial.begin(115200);
        ESP_LOGI("hal", "CpuFreq: %dMHZ -> %dMHZ ......", freq, date);
        if(cpuset){Serial.print("ok\n");}
        else {ESP_LOGI("hal", "err");}
    }

    getTime();
    if ((timeinfo.tm_year < (2016 - 1900)))
    {
        timeerr = true;              // 需要同步时间
        pref.putUInt("lastsync", 1); // 清除上次同步时间，但不清除时钟偏移信息
        lastsync = 1;
    }
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED)
        initial = false;
    // 下面进行初始化

    WiFi.mode(WIFI_OFF);
    display.epd2.startQueue();
    display.init(0, initial);
    display.setRotation(pref.getUChar(SETTINGS_PARAM_SCREEN_ORIENTATION, 3));
    display.setTextColor(GxEPD_BLACK);
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
    u8g2Fonts.begin(display);
    if (hal.btnl.isPressing() && (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED))
    {
        // 复位时检查左键是否按下，可以用于无限重启时临时关机
        powerOff(true);
        ESP.restart();
    }
    if (LittleFS.begin(false) == false)
    {
        display.fillScreen(GxEPD_WHITE);
        u8g2Fonts.setCursor(70, 80);
        u8g2Fonts.print("等待LittleFS格式化");
        display.display();
        LittleFS.format();
        if (LittleFS.begin(false) == false)
        {
            Serial.println("LittleFS格式化失败");
            display.fillScreen(GxEPD_WHITE);
            u8g2Fonts.setCursor(70, 80);
            u8g2Fonts.print("LittleFS格式化失败");
            display.display(true);
            delay(100);
            powerOff(false);
            ESP.restart();
        }
        test_littlefs_size(false);
    }
    test_littlefs_size(true);
    if(LittleFS.exists("/System") == false){LittleFS.mkdir("/System");}
    if(LittleFS.exists("/dat") == false){LittleFS.mkdir("/dat");}
    if (LittleFS.exists("/System/config.json") == false)
    {
        Serial.println("正在写入默认配置");
        File f = LittleFS.open("/System/config.json", "w");
        f.print(DEFAULT_CONFIG);
        f.close();
    }
    if(LittleFS.exists("/System/error log.txt"))
    {
        File log_file = LittleFS.open("/System/error log.txt", "r");
        if(log_file.size() > 1024 * 50){
            log_file.close();
            LittleFS.remove("/System/error log.txt");
        }
        log_file.close();
    }else{      
        File log_file = LittleFS.open("/System/error log.txt", "a");
        // 添加 BOM 头
        log_file.write(0xEF);
        log_file.write(0xBB);
        log_file.write(0xBF);
        log_file.close();
    }
    F_LOG("ESP32复位,原因:%s", esp_rst_str[esp_reset_reason()]);
    if(esp_reset_reason() == ESP_RST_DEEPSLEEP){
        F_LOG("复位为DeepSleep,唤醒源:%d", esp_sleep_get_wakeup_cause());
    }
    loadConfig();
    peripherals.init();
    weather.begin();
    buzzer.init();
    TJpgDec.setCallback(GUI::epd_output);
    if (hal.pref.getBool(get_char_sha_key("按键音"), false))
        xTaskCreate(task_btn_buzzer, "btn_buzzer", 2048, NULL, 9, NULL);
    xTaskCreate(task_hal_update, "hal_update", 2048, NULL, 10, NULL);
    xTaskCreate(cfu_task, "cfu_task", 4096, NULL, 10, NULL);
    if (initial == false && timeerr == false)
    {
        return false;
    }
    return true;
}

void HAL::autoConnectWiFi()
{
    ESP_LOGI("hal", "autoConnectWiFi\n");
    cheak_freq();
    if (WiFi.isConnected())
    {
        return;
    }
    // 下面连接WiFi
    if (config[PARAM_SSID] == "")
    {
        ReqWiFiConfig();
    }
    else
    {
        WiFi.setHostname("weatherclock");
        WiFi.mode(WIFI_STA);
        WiFi.begin(config[PARAM_SSID].as<const char *>(), config[PARAM_PASS].as<const char *>());
    }
    if (!WiFi.isConnected())
    {
        if (WiFi.waitForConnectResult(20000) != WL_CONNECTED)
        {
            hal.ReqWiFiConfig();
        }
    }
    F_LOG("成功连接:%s", WiFi.SSID().c_str());
    F_LOG("IP:%s", WiFi.localIP().toString().c_str());
    F_LOG("MAC:%s", WiFi.macAddress().c_str());
    F_LOG("信号强度:%d", WiFi.RSSI());
    sntp_stop();
}

int HAL::tryConnectWiFi()
{
    ESP_LOGI("hal", "tryConnectWiFi\n");
    cheak_freq();
    if (WiFi.isConnected())
    {
        return 0;
    }
    if (config[PARAM_SSID] == "")
    {
        GUI::info_msgbox("提示", "请先配置WiFi");
        return -1;
    }
    GUI::info_msgbox("提示", "正在连接WiFi");
    WiFi.setHostname("weatherclock");
    WiFi.mode(WIFI_STA);
    WiFi.begin(config[PARAM_SSID].as<const char *>(), config[PARAM_PASS].as<const char *>());
    if (!WiFi.isConnected())
    {
        if (WiFi.waitForConnectResult(20000) != WL_CONNECTED)
        {
            return -1;
        }
    }
    F_LOG("成功连接:%s", WiFi.SSID().c_str());
    F_LOG("IP:%s", WiFi.localIP().toString().c_str());
    F_LOG("MAC:%s", WiFi.macAddress().c_str());
    F_LOG("信号强度:%d", WiFi.RSSI());
    // sntp_stop();
    return 0;
}

void HAL::searchWiFi()
{
    ESP_LOGI("hal", "searchWiFi");
    cheak_freq();
    WiFi.mode(WIFI_STA);
    hal.numNetworks = WiFi.scanNetworks();
    if(hal.numNetworks == 0)
    {
        hal.numNetworks = WiFi.scanNetworks();
        if(hal.numNetworks == 0)
        {
            Serial.printf("没有搜索到WIFI");
            F_LOG("没有搜索到WIFI");
        }
    }
}
extern RTC_DATA_ATTR bool ebook_run;
static void set_sleep_set_gpio_interrupt()
{
    if (hal.btn_activelow)
    {
        esp_sleep_enable_ext0_wakeup((gpio_num_t)hal._wakeupIO[0], 0);
        esp_sleep_enable_ext1_wakeup((1LL << hal._wakeupIO[1]), ESP_EXT1_WAKEUP_ALL_LOW);
    }
    else
    {
        if (hal.pref.getBool(hal.get_char_sha_key("根据唤醒源翻页")) == true && ebook_run == true){
            esp_sleep_enable_ext0_wakeup((gpio_num_t)hal._wakeupIO[0], 1);
            esp_sleep_enable_ext1_wakeup((1LL << hal._wakeupIO[1]), ESP_EXT1_WAKEUP_ANY_HIGH);
        }else{
            esp_sleep_enable_ext1_wakeup((1ULL << PIN_BUTTONC) | (1ULL << PIN_BUTTONL) | (1ULL << PIN_BUTTONR), ESP_EXT1_WAKEUP_ANY_HIGH);
        }
    }
}

#include "driver/ledc.h"
static void pre_sleep()
{
    peripherals.sleep();
    set_sleep_set_gpio_interrupt();
    display.hibernate();
    buzzer.waitForSleep();
    LittleFS.end();
    delay(10);
    ledcDetachPin(PIN_BUZZER);
    digitalWrite(PIN_BUZZER, 0);
}
static void wait_display()
{
    while(uxQueueMessagesWaiting(display.epd2.getQueue()) > 0)
    {
        delay(10);
    }
    while(display.epd2.isBusy())
    {
        delay(10);
    }
}
void HAL::goSleep(uint32_t sec)
{
    hal.getTime();
    long nextSleep = 0;
    if (sec != 0)
        nextSleep = sec;
    else
    {
        nextSleep = 1;
    }
    Serial.printf("下次唤醒:%ld s\n", nextSleep);
    nextSleep = nextSleep * 1000000UL;
    pre_sleep();
    esp_sleep_enable_timer_wakeup(nextSleep);
    wait_display();
    delay(1);
    if (noDeepSleep)
    {
        esp_light_sleep_start();
        display.init(0, false);
        LittleFS.begin(false);
        peripherals.wakeup();
        ledcAttachPin(PIN_BUZZER, 0);
    }
    else
    {
        esp_deep_sleep_start();
    }
}



void HAL::powerOff(bool displayMessage)
{
    if (displayMessage)
    {
        display.setFullWindow();
        display.fillScreen(GxEPD_WHITE);
        // u8g2Fonts.setCursor(120, 70);
        // u8g2Fonts.print("已关机");
        display.drawXBitmap(0, 0, poweroff_bits, poweroff_width, poweroff_height, 0);
        display.display();
    }
    force_full_update = true;
    pre_sleep();
    WiFi.disconnect(true);
    set_sleep_set_gpio_interrupt();
    wait_display();
    delay(1);
    if (noDeepSleep)
    {
        esp_light_sleep_start();
        display.init(0, false);
        LittleFS.begin(false);
        peripherals.wakeup();
    }
    else
    {
        esp_deep_sleep_start();
    }
}
void HAL::update(void)
{
    static int count = 0;
    if (count++ % 30 == 0)
    {
        count = 0;
        getTime();
    }

    long adc;
    adc = analogRead(PIN_ADC);
    adc = adc * pref.getInt("ppc",7230) / 4096;
    VCC = adc;
    if (adc > 4400)
    {
        USBPluggedIn = true;
    }
    else
    {
        USBPluggedIn = false;
    }
    if (digitalRead(PIN_CHARGING) == 0)
    {
        isCharging = true;
    }
    else
    {
        isCharging = false;
    }
}
int HAL::getNTPMinute()
{
    int res[] = {
        0,
        2 * 60,
        4 * 60,
        6 * 60,
        12 * 60,
        24 * 60,
        36 * 60,
        48 * 60,
    };
    int val = pref.getUChar(SETTINGS_PARAM_NTP_INTERVAL, 1);
    return res[val];
}
#include "images/goodnightmorning.h"
uint8_t RTC_DATA_ATTR night_sleep_today = -1; // 用于判断今天是否退出过夜间模式
uint8_t RTC_DATA_ATTR night_sleep = 0;
void HAL::checkNightSleep()
{
    if (hal.timeinfo.tm_year < (2016 - 1900))
    {
        Serial.println("[夜间模式] 时间错误，直接返回");
        return;
    }
    if (config[PARAM_SLEEPATNIGHT].as<String>() == "0")
    {
        Serial.println("[夜间模式] 夜间模式已禁用");
        return;
    }
    if (night_sleep_today == hal.timeinfo.tm_mday)
    {
        Serial.println("[夜间模式] 当天暂时退出夜间模式");
        return;
    }
    if (hal.timeinfo.tm_year < (2016 - 1900))
    {
        Serial.println("[夜间模式] 时间错误");
        night_sleep = 0;
        night_sleep_today = -1;
        return;
    }
    String tmp = config[PARAM_SLEEPATNIGHT_START].as<String>();
    // 转换时间数据到分钟
    int sleepStart = tmp.substring(0, 2).toInt() * 60 + tmp.substring(3, 5).toInt();
    tmp = config[PARAM_SLEEPATNIGHT_END].as<String>();
    int sleepEnd = tmp.substring(0, 2).toInt() * 60 + tmp.substring(3, 5).toInt();
    bool end_at_nextday = sleepStart > sleepEnd; // 是否在第二天结束
    int now = hal.timeinfo.tm_hour * 60 + hal.timeinfo.tm_min;
    uint8_t night_sleep_pend = 0; // 当前夜间模式状态
    if (end_at_nextday)
    {
        if (now >= sleepStart)
        {
            // 晚安
            night_sleep_pend = 1;
        }
        else if (now < sleepEnd)
        {
            // 早上好
            night_sleep_pend = 2;
        }
        else
        {
            night_sleep_pend = 0;
        }
    }
    else
    {
        int mid = sleepStart + sleepEnd;
        mid = mid / 2;
        if (now >= sleepStart && now <= sleepEnd)
        {
            if (now < mid)
            {
                night_sleep_pend = 1;
            }
            else
            {
                night_sleep_pend = 2;
            }
        }
        else
        {
            night_sleep_pend = 0;
        }
    }
    // 判断当前屏幕显示
    if (night_sleep != night_sleep_pend)
    {
        Serial.println("[DEBUG] 夜间模式重绘");
        night_sleep = night_sleep_pend;
        display.clearScreen();
        if (night_sleep == 1)
        {
            // 晚安
            display.drawXBitmap(0, 0, goodnight_bits, 296, 128, 0);
        }
        else if (night_sleep == 2)
        {
            // 早上好
            display.drawXBitmap(0, 0, goodmorning_bits, 296, 128, 0);
        }
        display.display(false);
    }
    // 判断是否进入睡眠
    if (night_sleep != 0)
    {
        hal.goSleep(1800); // 休眠半小时再看
    }
}
void HAL::setWakeupIO(int io1, int io2)
{
    _wakeupIO[0] = io1;
    _wakeupIO[1] = io2;
}
bool HAL::copy(File &newFile, File &file)
{
    log_i("开始文件复制");

    // 分配缓冲区内存
    const size_t bufferSize = 512;
    char *buf = (char *)malloc(bufferSize);
    if (!buf) {
        log_e("内存分配失败");
        F_LOG("内存分配失败");
        return false;
    }

    int fileSize = file.size();
    int fileSize_kb = fileSize / 1024;
    char filename[256];
    sprintf(filename, "%s", file.name());
    size_t bytesRead = 0;
    size_t totalBytesRead = 0;
    float progress = 0.0;
    unsigned long time = 0;
    newFile.setBufferSize((size_t)8192); // 设置缓冲区大小为8KB
    while ((bytesRead = file.readBytes(buf, bufferSize)) > 0) {
        // 将缓冲区中的数据写入到目标文件中
        size_t bytesWritten = newFile.write((uint8_t *)buf, bytesRead);
        if (bytesWritten != bytesRead) {
            log_e("文件在写入过程中发生错误");
            for(int i = 0;i < 3;i++){
                newFile.seek(-bytesWritten,SeekCur);
                bytesWritten = newFile.write((uint8_t *)buf, bytesRead);
                log_e("尝试重新写入，bytesWritten = %d", bytesWritten);
                if(bytesWritten == bytesRead){
                    goto tray;
                }
            }
            for(int i = 0;i < 3;i++)
            {
                buzzer.append(3000,200);
                delay(350);
            }
            GUI::msgbox("警告", "写入过程中发生错误");
            free(buf);
            return false;
        }
        tray:
        totalBytesRead += bytesRead;
        // 计算进度百分比
        // 如果进度有变化，则更新显示
        if (millis() - time >= 2000) {
            progress = ((float)totalBytesRead * 100.0) / (float)fileSize;
            display.clearScreen();
            u8g2Fonts.setCursor(1, 20);
            u8g2Fonts.printf("正在复制：%s", filename);
            u8g2Fonts.setCursor(1, 35);
            u8g2Fonts.printf("总计：%dKB 剩余：%dKB", fileSize_kb, (fileSize - totalBytesRead) / 1024);
            u8g2Fonts.setCursor(1, 50);
            u8g2Fonts.printf("进度: %0.2f%%", progress);
            u8g2Fonts.setCursor(1, 65);
            u8g2Fonts.printf("提示:长按左键中止复制");
            u8g2Fonts.setCursor(1, 80);
            u8g2Fonts.printf("提示:长按中键暂停，暂停后按任意键恢复复制");
            display.display(true);
            time = millis();
            newFile.flush();
        }
        if (GUI::waitLongPress(PIN_BUTTONL))
            return false;
        if (GUI::waitLongPress(PIN_BUTTONC)){
            display.fillRect(0, 22, 296, 22, GxEPD_WHITE);
            u8g2Fonts.setCursor(1, 20);
            u8g2Fonts.printf("暂停复制：%s", filename);
            display.display(true);
            hal.wait_input();
        }
    }

    // 确保显示最终完成的进度
    if (totalBytesRead == fileSize) { 
        display.clearScreen();
        u8g2Fonts.setCursor(1, 20);
        u8g2Fonts.printf("复制完成：%s", filename);
        u8g2Fonts.setCursor(1, 35);
        u8g2Fonts.printf("总计：%dKB 剩余：%dKB", fileSize_kb, 0);
        u8g2Fonts.setCursor(1, 50);
        u8g2Fonts.printf("进度: 100%%", progress);
        display.display(true);
    } else {
        log_w("文件复制不完整");
        free(buf);
        return false;
    }
    // 释放缓冲区内存
    free(buf);
    log_i("文件复制完成");
    file.close();
    newFile.close();
    return true;
}
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

void HAL::rm_rf(const char *path)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    // 打开目录
    if ((dp = opendir(path)) == NULL)
    {
        perror("opendir");
        return;
    }

    // 迭代读取目录中的文件
    while ((entry = readdir(dp)) != NULL)
    {
        // 获取文件的完整路径
        char filePath[256];
        sprintf(filePath, "%s/%s", path, entry->d_name);

        // 获取文件信息
        if (stat(filePath, &statbuf) == -1)
        {
            perror("lstat");
            continue;
        }

        // 判断是否是目录
        if (S_ISDIR(statbuf.st_mode))
        {
            // 忽略.和..目录
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            // 递归删除子目录
            rm_rf(filePath);
        }
        else
        {
            // 删除文件
            if (remove(filePath) != 0)
            {
                perror("remove");
            }
        }
    }

    // 关闭目录
    closedir(dp);

    // 删除空目录
    if (rmdir(path) != 0)
    {
        perror("rmdir");
    }
}

HAL hal;