#include "light_controller.h"
#include "touch_events.h"
#include "sleep_light.h"
#include "common_info.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "LIGHT_CONTROLLER";

static bool is_brightness_mode = false;

static void brightness_mode_indication(void)
{
    bool current_state = get_light_state();

    light_off();
    vTaskDelay(pdMS_TO_TICKS(100));
    light_on();
    vTaskDelay(pdMS_TO_TICKS(100));
    light_off();
    vTaskDelay(pdMS_TO_TICKS(100));

    if (current_state)
    {
        light_on();
    }
}

static void handle_touch_event(touch_event_t *event)
{
    switch (event->type)
    {
    case TOUCH_EVENT_SHORT_PRESS:
        ESP_LOGI(TAG, "Short press - Toggle light");

        bool current_state = get_light_state();
        bool new_state = !current_state;

        set_light_state(new_state);

        led_status_t status = get_led_status();
        status.is_on = new_state ? 1 : 0;
        set_led_status(status);

        if (new_state)
        {
            light_on_dimming();
        }
        else
        {
            light_off_dimming();
        }

        ESP_LOGI(TAG, "Light toggled: %s", new_state ? "ON" : "OFF");
        break;

    case TOUCH_EVENT_LONG_PRESS:
        ESP_LOGI(TAG, "Long press - Enter brightness mode");
        is_brightness_mode = true;
        brightness_mode_indication();
        break;

    case TOUCH_EVENT_BRIGHTNESS_STEP:
        ESP_LOGI(TAG, "Brightness step: %d%%", event->brightness_level);

        if (is_brightness_mode)
        {
            led_status_t status = get_led_status();
            status.brightness = event->brightness_level;
            status.is_on = 1;
            set_led_status(status);

            set_light_state(true);

            change_color_with_status(&status);
        }
        break;

    case TOUCH_EVENT_BRIGHTNESS_EXIT:
        ESP_LOGI(TAG, "Exit brightness mode");
        is_brightness_mode = false;
        brightness_mode_indication();
        break;

    default:
        ESP_LOGW(TAG, "Unknown touch event: %d", event->type);
        break;
    }
}

void light_controller_init(void)
{
    ESP_LOGI(TAG, "Initializing light controller");

    touch_register_callback(handle_touch_event);

    ESP_LOGI(TAG, "Light controller initialized");
}