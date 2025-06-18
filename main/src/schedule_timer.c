#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "driver/gptimer.h"

static const char *TAG = "TIMER";

static gptimer_handle_t gptimer = NULL;
static TaskHandle_t timer_task_handle = NULL;

gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1000000, // 1MHz, 1 tick = 1마이크로초
};

static void timer_task(void *arg)
{
    while (1)
    {
        if (xTaskNotifyWait(0, 0, NULL, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "Timer event occurred!");
            // 여기에 타이머 이벤트 처리 로직을 추가하세요.
        }
    }
}

// 타이머 콜백 함수
static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;

    // 타이머 처리 태스크에 알림 전송
    xTaskNotifyFromISR(timer_task_handle, 0, eNoAction, &high_task_awoken);

    // 다음 알람 설정 (1분 후)
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = edata->count_value + 60000000, // 1분 = 60,000,000 마이크로초
        .reload_count = 0,
        .flags.auto_reload_on_alarm = false,
    };
    gptimer_set_alarm_action(timer, &alarm_config);

    return high_task_awoken == pdTRUE;
}

void schedule_timer_init(void)
{
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_on_alarm_cb,
    };

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // 타이머 활성화
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // 타이머 활성화
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    // 알람 설정 (1분 = 60,000,000 마이크로초)
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 60000000, // 1분
        .reload_count = 0,
        .flags.auto_reload_on_alarm = false,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    // 타이머 시작
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}