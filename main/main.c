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

void app_main(void)
{
    nvs_init();
    init_common_info();
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

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/ledc.h"
// #include "esp_err.h"

// // LEDC PWM 설정
// #define LEDC_TIMER LEDC_TIMER_0
// #define LEDC_MODE LEDC_LOW_SPEED_MODE
// #define LEDC_OUTPUT_IO 21 // GPIO 번호, IRFZ44N의 게이트 핀 연결
// #define LEDC_CHANNEL LEDC_CHANNEL_0
// #define LEDC_DUTY_RES LEDC_TIMER_13_BIT          // 해상도 설정
// #define LEDC_FREQUENCY 5000                      // 주파수 (Hz)
// #define LEDC_DUTY_MAX ((1 << LEDC_DUTY_RES) - 1) // 최대 듀티 값

// // MOSFET 전류 제어 함수
// void set_mosfet_current(uint32_t duty)
// {
//     // 듀티 값을 0 ~ LEDC_DUTY_MAX 사이로 제한
//     if (duty > LEDC_DUTY_MAX)
//     {
//         duty = LEDC_DUTY_MAX;
//     }

//     ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
//     ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
//     printf("duty: %" PRIu32, duty);
//     printf(", percent: (%.1f%%)\n", (float)duty * 100 / LEDC_DUTY_MAX);
// }

// // LEDC PWM 초기화 함수
// void ledc_init(void)
// {
//     // LEDC 타이머 설정
//     ledc_timer_config_t ledc_timer = {
//         .speed_mode = LEDC_MODE,
//         .timer_num = LEDC_TIMER,
//         .duty_resolution = LEDC_DUTY_RES,
//         .freq_hz = LEDC_FREQUENCY,
//         .clk_cfg = LEDC_AUTO_CLK};
//     ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

//     // LEDC 채널 설정
//     ledc_channel_config_t ledc_channel = {
//         .speed_mode = LEDC_MODE,
//         .channel = LEDC_CHANNEL,
//         .timer_sel = LEDC_TIMER,
//         .intr_type = LEDC_INTR_DISABLE,
//         .gpio_num = LEDC_OUTPUT_IO,
//         .duty = 0, // 초기 듀티 사이클 0%
//         .hpoint = 0};
//     ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
// }

// void app_main(void)
// {
//     // LEDC PWM 초기화
//     ledc_init();

//     printf("IRFZ44N MOSFET PWM 제어 시작\n");

//     while (1)
//     {
//         printf("전류 증가 중...\n");
//         printf("LEDC_DUTY_MAX: %d", LEDC_DUTY_MAX);
//         printf(", inc term: %d \n", (LEDC_DUTY_MAX / 20));
//         // 전류 점진적 증가
//         for (int duty = 0; duty <= LEDC_DUTY_MAX; duty += LEDC_DUTY_MAX / 20)
//         {
//             printf("duty: %d\n", duty);
//             set_mosfet_current(duty);
//             vTaskDelay(100 / portTICK_PERIOD_MS);
//         }

//         vTaskDelay(1000 / portTICK_PERIOD_MS);

//         printf("전류 감소 중...\n");
//         // 전류 점진적 감소
//         for (int duty = LEDC_DUTY_MAX; duty >= 0; duty -= LEDC_DUTY_MAX / 20)
//         {
//             set_mosfet_current(duty);
//             vTaskDelay(100 / portTICK_PERIOD_MS);
//         }

//         vTaskDelay(1000 / portTICK_PERIOD_MS);

//         // 3단계 전류 레벨 테스트 (25%, 50%, 75%)
//         printf("전류 레벨 테스트\n");
//         uint32_t test_levels[] = {
//             LEDC_DUTY_MAX / 4,    // 25%
//             LEDC_DUTY_MAX / 2,    // 50%
//             LEDC_DUTY_MAX * 3 / 4 // 75%
//         };

//         for (int i = 0; i < 3; i++)
//         {
//             set_mosfet_current(test_levels[i]);
//             vTaskDelay(2000 / portTICK_PERIOD_MS);
//         }

//         // 전류 끄기
//         set_mosfet_current(0);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
