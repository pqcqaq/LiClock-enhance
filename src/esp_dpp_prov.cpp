#include <A_Config.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_dpp.h"
#include "esp_log.h"
#include "nvs_flash.h"

#ifdef CONFIG_ESP_DPP_LISTEN_CHANNEL
#define EXAMPLE_DPP_LISTEN_CHANNEL_LIST     CONFIG_ESP_DPP_LISTEN_CHANNEL_LIST
#else
#define EXAMPLE_DPP_LISTEN_CHANNEL_LIST     "1,6,11"
#endif

#ifdef CONFIG_ESP_DPP_BOOTSTRAPPING_KEY
#define EXAMPLE_DPP_BOOTSTRAPPING_KEY   CONFIG_ESP_DPP_BOOTSTRAPPING_KEY
#else
#define EXAMPLE_DPP_BOOTSTRAPPING_KEY   0
#endif

#ifdef CONFIG_ESP_DPP_DEVICE_INFO
#define EXAMPLE_DPP_DEVICE_INFO      CONFIG_ESP_DPP_DEVICE_INFO
#else
#define EXAMPLE_DPP_DEVICE_INFO      0
#endif

static const char *TAG = "wifi dpp-enrollee";
wifi_config_t s_dpp_wifi_config;

static int s_retry_num = 0;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_dpp_event_group;

#define DPP_CONNECTED_BIT  BIT0
#define DPP_CONNECT_FAIL_BIT     BIT1
#define DPP_AUTH_FAIL_BIT           BIT2


void drawDPPQRCode(const char *str)
{
#include "images/dpp.h"
    display.clearScreen();
    display.drawXBitmap(0, 0, DPP_bits, 296, 128, GxEPD_BLACK);
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(7)];
    qrcode_initText(&qrcode, qrcodeData, 6, 0, str);
    Serial.println(qrcode.size);
    Serial.println(str);
    for (uint8_t y = 0; y < qrcode.size; y++)
    {
        // Each horizontal module
        for (uint8_t x = 0; x < qrcode.size; x++)
        {
            display.fillRect(2 * x + 20, 2 * y + 33, 2, 2, qrcode_getModule(&qrcode, x, y) ? GxEPD_BLACK : GxEPD_WHITE);
        }
    }
    display.display(true);
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_ERROR_CHECK(esp_supp_dpp_start_listen());
        ESP_LOGE(TAG, "Started listening for DPP Authentication");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 5) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGE(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_dpp_event_group, DPP_CONNECT_FAIL_BIT);
        }
        ESP_LOGE(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGE(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_dpp_event_group, DPP_CONNECTED_BIT);
    }
}

void dpp_enrollee_event_cb(esp_supp_dpp_event_t event, void *data)
{
    switch (event) {
    case ESP_SUPP_DPP_URI_READY:
        if (data != NULL) {

            ESP_LOGE(TAG, "Scan below QR Code to configure the enrollee:\n");
            drawDPPQRCode((const char *)data);
        }
        break;
    case ESP_SUPP_DPP_CFG_RECVD:
        memcpy(&s_dpp_wifi_config, data, sizeof(s_dpp_wifi_config));
        esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &s_dpp_wifi_config);
        ESP_LOGE(TAG, "DPP Authentication successful, connecting to AP : %s",
                 s_dpp_wifi_config.sta.ssid);
        s_retry_num = 0;
        esp_wifi_connect();
        break;
    case ESP_SUPP_DPP_FAIL:
        if (s_retry_num < 5) {
            ESP_LOGE(TAG, "DPP Auth failed (Reason: %s), retry...", esp_err_to_name((int)data));
            ESP_ERROR_CHECK(esp_supp_dpp_start_listen());
            s_retry_num++;
        } else {
            xEventGroupSetBits(s_dpp_event_group, DPP_AUTH_FAIL_BIT);
        }
        break;
    default:
        break;
    }
}

void dpp_enrollee_init(void)
{
    s_dpp_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));


    ESP_ERROR_CHECK(esp_supp_dpp_init(dpp_enrollee_event_cb));
    /* Currently only supported method is QR Code */
    ESP_ERROR_CHECK(esp_supp_dpp_bootstrap_gen(EXAMPLE_DPP_LISTEN_CHANNEL_LIST, DPP_BOOTSTRAP_QR_CODE,
                    EXAMPLE_DPP_BOOTSTRAPPING_KEY, EXAMPLE_DPP_DEVICE_INFO));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_dpp_event_group,
                                           DPP_CONNECTED_BIT | DPP_CONNECT_FAIL_BIT | DPP_AUTH_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & DPP_CONNECTED_BIT) {
        ESP_LOGE(TAG, "connected to ap SSID:%s password:%s",
                 s_dpp_wifi_config.sta.ssid, s_dpp_wifi_config.sta.password);
    } else if (bits & DPP_CONNECT_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to SSID:%s, password:%s",
                 s_dpp_wifi_config.sta.ssid, s_dpp_wifi_config.sta.password);
    } else if (bits & DPP_AUTH_FAIL_BIT) {
        ESP_LOGE(TAG, "DPP Authentication failed after %d retries", s_retry_num);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    esp_supp_dpp_deinit();
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_dpp_event_group);
}


void esp_dpp_start()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    dpp_enrollee_init();
    display.clearScreen();
    display.display(true);
}
