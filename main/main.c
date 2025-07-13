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
#include "common_info.h"
#include "schedule_timer.h"
#include "light_controller.h"

void app_main(void)
{
    nvs_init();
    init_common_info();

    light_init();
    light_on();

    light_controller_init();

    gpio_init();

    init_nimble();
    schedule_timer_init();
}