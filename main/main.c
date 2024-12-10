// #include "common.h"
// #include "gap.h"
// #include "gatt_svc.h"
// #include "embedded_led.h"
// #include "nimble.h"

// void app_main(void)
// {
//     /* Local variables */
//     int rc = 0;
//     esp_err_t ret = ESP_OK;

//     /* LED initialization */
// embedded_led_init();

//     /* NVS flash initialization */
//     ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
//         ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
//     {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     if (ret != ESP_OK)
//     {
//         ESP_LOGE(TAG, "failed to initialize nvs flash, error code: %d ", ret);
//         return;
//     }

//     /* NimBLE stack initialization */
//     ret = nimble_port_init();
//     if (ret != ESP_OK)
//     {
//         ESP_LOGE(TAG, "failed to initialize nimble stack, error code: %d ",
//                  ret);
//         return;
//     }

//     /* GAP service initialization */
//     rc = gap_init();
//     if (rc != 0)
//     {
//         ESP_LOGE(TAG, "failed to initialize GAP service, error code: %d", rc);
//         return;
//     }

//     /* GATT server initialization */
//     rc = gatt_svc_init();
//     if (rc != 0)
//     {
//         ESP_LOGE(TAG, "failed to initialize GATT server, error code: %d", rc);
//         return;
//     }

//     /* NimBLE host configuration initialization */
//     nimble_host_config_init();

//     /* Start NimBLE host task thread and return */
//     xTaskCreate(nimble_host_task, "NimBLE Host", 4 * 1024, NULL, 5, NULL);
//     return;
// }

/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
// #include "driver/rmt_tx.h"
// #include "led_strip_encoder.h"
#include "sleep_light.h"
#include "gpio.h"
#include "nvs_storage.h"
#include "nimble.h"
#include "embedded_led.h"
#include "common.h"

// static const char *TAG = "main";

// 따뜻한 달빛색
#define MOON_WARM 0xFFF8DC // R:255, G:248, B:220
#define MOON_GOLD 0xFDEAA8 // R:253, G:234, B:168

// 황금빛 달빛
#define MOON_YELLOW 0xFAD610 // R:250, G:214, B:165

// 은은한 달빛
#define MOON_SOFT 0xF4F1C9 // R:244, G:241, B:201

// harvest moon(보름달)
#define MOON_HARVEST 0xFBB953 // R:251, G:185, B:83k

void app_main(void)
{
    nvs_init();
    gpio_init();
    light_init();
    light_on();
    int ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    init_nimble();
}
