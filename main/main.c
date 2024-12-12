#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sleep_light.h"
#include "gpio.h"
#include "nvs_storage.h"
#include "nimble.h"
#include "embedded_led.h"
#include "common.h"

// static const char *TAG = "main";

void app_main(void)
{
    nvs_init();
    gpio_init();
    light_init();
    light_on();

    // int ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
    //     ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    // {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }

    init_nimble();
}
