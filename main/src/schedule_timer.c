#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "driver/gptimer.h"
#include "sleep_light.h"

static const char *TAG = "TIMER";

static gptimer_handle_t gptimer = NULL;
static TaskHandle_t timer_task_handle = NULL;

// gptimer_config_t timer_config = {
//     .clk_src = GPTIMER_CLK_SRC_DEFAULT,
//     .direction = GPTIMER_COUNT_UP,
//     .resolution_hz = 1000000, // 1MHz, 1 tick = 1마이크로초
// };

static void exec_event(void)
{
    on_off_time_t on_off_time;
    get_on_off_time(&on_off_time);

    if (!on_off_time.is_time_synced)
    {
        ESP_LOGW(TAG, "Time is not synced, skipping timer event execution.");
        return;
    }

    struct tm cur_time;
    get_local_time(&cur_time);
    ESP_LOGI(TAG, "Current time: %02d:%02d", cur_time.tm_hour, cur_time.tm_min);

    // do power on
    if (on_off_time.on.is_set && cur_time.tm_hour == on_off_time.on.hour && cur_time.tm_min == on_off_time.on.minute)
    {
        ESP_LOGI(TAG, "Turning light ON at scheduled time: %" PRIu32 ":%" PRIu32, on_off_time.on.hour, on_off_time.on.minute);
        const uint32_t blink_times = 3;
        const uint32_t blink_term_ms = 100;
        light_on_blink(blink_times, blink_term_ms);
    }

    // power off
    if (on_off_time.off.is_set && cur_time.tm_hour == on_off_time.off.hour && cur_time.tm_min == on_off_time.off.minute)
    {
        ESP_LOGI(TAG, "Turning light OFF at scheduled time: %" PRIu32 ":%" PRIu32, on_off_time.off.hour, on_off_time.off.minute);
        const uint32_t blink_times = 3;
        const uint32_t blink_term_ms = 100;
        light_off_blink(blink_times, blink_term_ms);
    }
}

static void timer_event_task(void *arg)
{
    while (1)
    {
        if (xTaskNotifyWait(0, 0, NULL, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "Timer event occurred!");
            exec_event();
            vTaskDelay(pdMS_TO_TICKS(1000)); // 1초 대기
        }
    }
}

static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;

    // 타이머 처리 태스크에 알림 전송
    xTaskNotifyFromISR(timer_task_handle, 0, eNoAction, &high_task_awoken);

    return high_task_awoken == pdTRUE;
}

void schedule_timer_init(void)
{

    xTaskCreate(timer_event_task, "timer_event_task", 2048, NULL, 5, &timer_task_handle);

    // 타이머 설정
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick = 1마이크로초
    };
    // 타이머 생성
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_on_alarm_cb,
    };

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    // 알람 설정 (1분 = 60,000,000 마이크로초)
    gptimer_alarm_config_t alarm_config = {
        // .alarm_count = 60000000, // 1분
        .alarm_count = 1000000, // 1분
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    ESP_ERROR_CHECK(gptimer_start(gptimer));
}