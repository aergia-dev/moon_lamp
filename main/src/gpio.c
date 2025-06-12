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

#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;
static TimerHandle_t brightness_mode_timer = NULL;

static uint32_t press_start_time = 0;
static bool is_pressed = false;
static bool brightness_mode = false;
static uint8_t current_brightness_level = 5;

#define LONG_PRESS_TIME_MS 2000
#define BRIGHTNESS_MODE_TIMEOUT_MS 5000

static void handle_brightness_adjustment(void)
{
    current_brightness_level = (current_brightness_level + 1) % 6;
    uint8_t brightness_percent = current_brightness_level * 20;

    led_status_t status = get_led_status();
    status.brightness = brightness_percent;

    if (brightness_percent == 0)
    {
        status.is_on = 0;
        set_led_status(status);
        light_off_dimming();
    }
    else
    {
        status.is_on = 1;
        set_led_status(status);

        if (!get_light_state())
        {
            light_on_dimming();
        }
        else
        {
            change_color_with_status(&status);
        }
    }

    xTimerReset(brightness_mode_timer, 0);
    printf("Brightness: %d%% (timer reset)\n", brightness_percent);
}

static void brightness_mode_timeout_callback(TimerHandle_t xTimer)
{
    brightness_mode = false;
    printf("Brightness mode OFF (5 seconds timeout)\n");
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
                press_start_time = event_time;
                printf("Touch started at %lu\n", event_time);
            }
            else if (current_level == 0 && is_pressed)
            {
                is_pressed = false;
                uint32_t press_duration = event_time - press_start_time;
                printf("Touch ended, duration: %lu ms\n", press_duration);

                if (press_duration >= LONG_PRESS_TIME_MS)
                {
                    if (brightness_mode)
                    {
                        brightness_mode = false;
                        xTimerStop(brightness_mode_timer, 0);
                        printf("Brightness mode OFF\n");
                    }
                    else
                    {
                        brightness_mode = true;
                        current_brightness_level = get_brightness() / 20;
                        xTimerStart(brightness_mode_timer, 0);
                        printf("Brightness mode ON\n");
                    }
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
                        printf("Toggle light\n");
                    }
                }
                else
                {
                    printf("Touch too short, ignored\n");
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
    printf("GPIO initialized with long press detection\n");
}