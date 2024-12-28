#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sleep_light.h"
#include "common_info.h"

#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;

static uint32_t last_trigger_time = 0;
#define DEBOUNCE_TIME_MS 500 // 디바운스 시간

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    uint32_t current_time = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;

    if ((current_time - last_trigger_time) > DEBOUNCE_TIME_MS)
    {
        // 이벤트 처리
        xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
        last_trigger_time = current_time;
    }
}

static void gpio_task(void *arg)
{
    uint32_t io_num = 0;
    uint32_t gpio_num = get_touch_gpio();

    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            if (io_num == gpio_num)
            {
                toggle_light();
                // printf("in gpio handler\n");
            }
        }
    }
}

void gpio_init()
{
    gpio_config_t io_conf = {};
    uint32_t gpio_num = get_touch_gpio();

    io_conf.intr_type = GPIO_INTR_POSEDGE;
    // bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = (1ULL << gpio_num);
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_task, "gpio_task", 4096, NULL, 10, NULL);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    gpio_isr_handler_add(gpio_num, gpio_isr_handler, (void *)gpio_num);
}