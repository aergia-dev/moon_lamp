#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "sleep_light.h"
#include "common_info.h"
#include "esp_log.h"

#define TAG "GPIO"
#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;
static TimerHandle_t brightness_mode_timer = NULL;
static TimerHandle_t long_press_timer = NULL;

static uint32_t press_start_time = 0;
static bool is_pressed = false;
static bool brightness_mode = false;
static uint8_t current_brightness_level = 1;
static bool long_press_triggered = false;

#define LONG_PRESS_TIME_MS 1500
#define BRIGHTNESS_MODE_TIMEOUT_MS 5000

static void brightness_mode_blink(void)
{
    bool current_state = get_light_state();

    light_off();
    vTaskDelay(pdMS_TO_TICKS(100));
    light_on();
    vTaskDelay(pdMS_TO_TICKS(100));
    light_off();
    vTaskDelay(pdMS_TO_TICKS(100));
    light_on();
}

static void handle_brightness_adjustment(void)
{
    current_brightness_level = (current_brightness_level % 5) + 1;
    uint8_t brightness_percent = current_brightness_level * 20;

    led_status_t status = get_led_status();
    status.brightness = brightness_percent;
    status.is_on = 1;
    set_led_status(status);

    change_color_with_status(&status);

    xTimerReset(brightness_mode_timer, 0);
    ESP_LOGI(TAG, "Brightness: %d", brightness_percent);
}

static void long_press_timer_callback(TimerHandle_t xTimer)
{
    if (is_pressed && get_light_state() && !brightness_mode)
    {
        brightness_mode = true;
        long_press_triggered = true;
        current_brightness_level = get_brightness() / 20;
        if (current_brightness_level == 0)
            current_brightness_level = 1;

        brightness_mode_blink();

        xTimerStart(brightness_mode_timer, 0);
        ESP_LOGI(TAG, "Brightness mode ON (during press)");
    }
}

static void brightness_mode_timeout_callback(TimerHandle_t xTimer)
{
    brightness_mode = false;

    brightness_mode_blink();

    ESP_LOGI(TAG, "Brightness mode OFF (timeout)");
}

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t current_time = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    xQueueSendFromISR(gpio_evt_queue, &current_time, NULL);
}

static void gpio_task(void *arg)
{
    uint32_t event_time = 0;
    uint32_t gpio_num = get_touch_gpio();

    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &event_time, portMAX_DELAY))
        {
            int current_level = gpio_get_level(gpio_num);

            if (current_level == 1 && !is_pressed)
            {
                is_pressed = true;
                long_press_triggered = false;
                press_start_time = event_time;

                if (get_light_state() && !brightness_mode)
                {
                    xTimerStart(long_press_timer, 0);
                }

                ESP_LOGI(TAG, "Touch started");
            }
            else if (current_level == 0 && is_pressed)
            {
                is_pressed = false;
                uint32_t press_duration = event_time - press_start_time;

                xTimerStop(long_press_timer, 0);

                ESP_LOGI(TAG, "Touch ended, duration: %" PRIu32 " ms", press_duration);

                if (long_press_triggered)
                {
                    ESP_LOGI(TAG, "Long press already processed");
                    long_press_triggered = false;
                }
                else if (press_duration >= 100)
                {
                    if (brightness_mode)
                    {
                        handle_brightness_adjustment();
                    }
                    else
                    {
                        toggle_light();
                        ESP_LOGI(TAG, "Toggle light");
                    }
                }
                else
                {
                    ESP_LOGI(TAG, "Touch too short, ignored");
                }
            }
        }
    }
}

void gpio_init()
{
    gpio_config_t io_conf = {};
    uint32_t gpio_num = get_touch_gpio();

    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL << gpio_num);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    long_press_timer = xTimerCreate(
        "LongPressTimer",
        pdMS_TO_TICKS(LONG_PRESS_TIME_MS),
        pdFALSE,
        NULL,
        long_press_timer_callback);

    brightness_mode_timer = xTimerCreate(
        "BrightnessModeTimer",
        pdMS_TO_TICKS(BRIGHTNESS_MODE_TIMEOUT_MS),
        pdFALSE,
        NULL,
        brightness_mode_timeout_callback);

    xTaskCreate(gpio_task, "gpio_task", 4096, NULL, 10, NULL);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(gpio_num, gpio_isr_handler, (void *)gpio_num);

    current_brightness_level = get_brightness() / 20;
    if (current_brightness_level == 0)
        current_brightness_level = 1;

    ESP_LOGI(TAG, "GPIO initialized with hold-press detection");
}