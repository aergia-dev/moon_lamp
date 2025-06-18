
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "ble_protocol.h"
#include <esp_log.h>
#include <string.h>
#include "sleep_light.h"
#include "common_info.h"
#include <sys/time.h>

static const char *TAG = "BLE_PROTOCOL";

typedef void (*cmd_handler_t)(handler_req_t *req, handler_rsp_t *rsp);

typedef struct
{
    uint16_t cmd;
    cmd_handler_t handler;
} cmd_map_t;

static void handler_reset(handler_req_t *req, handler_rsp_t *rsp)
{
    ESP_LOGI(TAG, "reset");
}

static void handler_write_status(handler_req_t *req, handler_rsp_t *rsp)
{
    if (req->len != sizeof(led_status_t))
    {
        ESP_LOGE(TAG, "handler_write_status - data size is not matched, got %d but should be %d", req->len, sizeof(led_status_t));
        return;
    }

    led_status_t src;
    memcpy(&src, req->data, sizeof(led_status_t));
    ESP_LOGI(TAG, "write - led status");
    ESP_LOGI(TAG, "is_on: %d, brightness: %d, color: %lu", (int)src.is_on, (int)src.brightness, src.color);
    ble_cont_light_write(&src);

    rsp->is_success = true;
    rsp->len = 0;
}

static void handler_read_status(handler_req_t *req, handler_rsp_t *rsp)
{
    led_status_t status;

    status.is_on = get_light_state();
    status.brightness = get_brightness();
    status.color = get_saved_color_uint32();

    rsp->is_success = true;
    if (HANDLER_RSP_SZ >= sizeof(led_status_t))
    {
        memcpy(rsp->data, &status, sizeof(led_status_t));
        rsp->len = sizeof(led_status_t);
        ESP_LOGI(TAG, "read led status");
        ESP_LOGI(TAG, "is_on: %d, brightness: %d, color: %lu", (int)status.is_on, (int)status.brightness, status.color);
    }
    else
    {
        rsp->len = 0;
        ESP_LOGE(TAG, "error: should increase rsp data size to %d", sizeof(led_status_t));
    }
}

static void handler_sync_time(handler_req_t *req, handler_rsp_t *rsp)
{
    if (req->len == sizeof(struct timeval))
    {
        struct timeval tv;
        memcpy(&tv, req->data, sizeof(struct timeval));
        settimeofday(&tv, NULL);

        struct tm timeinfo;
        localtime_r(&tv.tv_sec, &timeinfo);

        ESP_LOGI(TAG, "시간 동기화 완료: %04d-%02d-%02d %02d:%02d:%02d",
                 timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        rsp->is_success = true;
    }
    else
    {
        rsp->is_success = false;
    }

    rsp->len = 0;
}

static struct timeval uint64_to_timeval_seconds(uint64_t second_since_epoch)
{
    struct timeval tv;
    tv.tv_sec = second_since_epoch; // Integer division for seconds
    tv.tv_usec = 0;
    return tv;
}

static void convert_second_to_event_time(uint64_t seconds, event_time_t *event_time)
{
    time_t rawtime = seconds;
    struct tm *info;
    info = localtime(&rawtime); // 또는 gmtime(&rawtime)

    if (info == NULL)
    {
        perror("Failed to convert time_t to struct tm");
        return;
    }

    event_time->hour = info->tm_hour;
    event_time->minute = info->tm_min;
}

static void handler_on_time(handler_req_t *req, handler_rsp_t *rsp)
{
    if (req->len == sizeof(uint64_t))
    {
        uint64_t seconds = *(uint64_t *)req->data;
        event_time_t on_time;
        convert_second_to_event_time(seconds, &on_time);
        set_on_time(on_time);
        rsp->is_success = true;
    }
    else
    {
        rsp->is_success = false;
    }

    rsp->len = 0;
}
static void handler_off_time(handler_req_t *req, handler_rsp_t *rsp)
{
    if (req->len == sizeof(struct timeval))
    {
        uint64_t seconds = *(uint64_t *)req->data;
        event_time_t off_time;
        convert_second_to_event_time(seconds, &off_time);
        set_off_time(off_time); // milliseconds
        rsp->is_success = true;
    }
    else
    {
        rsp->is_success = false;
    }

    rsp->len = 0;
}

static void handler_test_color(handler_req_t *req, handler_rsp_t *rsp)
{
    if (req->len == sizeof(led_status_t))
    {
        led_status_t status;
        memcpy(&status, req->data, sizeof(led_status_t));
        ESP_LOGI(TAG, "test led status");
        ESP_LOGI(TAG, "is_on: %d, brightness: %d, color: %lu", (int)status.is_on, (int)status.brightness, status.color);
        ble_change_color(&status);
        rsp->is_success = true;
    }
    else
    {
        rsp->is_success = false;
    }

    rsp->len = 0;
}

static void handler_write_dev_name(handler_req_t *req, handler_rsp_t *rsp)
{
    ESP_LOGI(TAG, "handler_write_dev_name");
    if (req->len == sizeof(led_status_t))
    {
        write_dev_name_t name;
        memcpy(&name, req->data, sizeof(write_dev_name_t));
        ESP_LOGI(TAG, "write dev name: %s", name.dev_name);
        set_device_name(name.dev_name);
        rsp->is_success = true;
    }
    else
    {
        ESP_LOGI(TAG, "received pkt size is not matched");
    }
}

static void handler_write_passkey(handler_req_t *req, handler_rsp_t *rsp)
{
    ESP_LOGI(TAG, "handler_write_passkey");
    if (req->len == sizeof(write_passkey_t))
    {
        write_passkey_t passkey;
        memcpy(&passkey, req->data, sizeof(write_passkey_t));
        ESP_LOGI(TAG, "write passkey: %" PRIu32, passkey.passkey);

        set_ble_passkey(passkey.passkey);
        rsp->is_success = true;
    }
    else
    {
        ESP_LOGI(TAG, "received pkt size is not matched");
    }
}

const cmd_map_t cmd_handlers[] = {
    {RESET, handler_reset},
    {WRITE_STATUS, handler_write_status},
    {READ_STATUS, handler_read_status},
    {TEST_STATUS, handler_test_color},
    {WRITE_DEV_NAME, handler_write_dev_name},
    {WRITE_PASSKEY, handler_write_passkey},
    {SYNC_TIME, handler_sync_time},     // data type: timeval
    {POWER_ON_TIME, handler_on_time},   // data type: timeval
    {POWER_OFF_TIME, handler_off_time}, // data type: timeval
    {0, NULL},
};

void process_command(handler_req_t *req, handler_rsp_t *rsp)
{
    for (const cmd_map_t *map = cmd_handlers; map->handler != NULL; map++)
    {
        if (map->cmd == req->cmd)
        {
            map->handler(req, rsp);
            return;
        }
    }

    ESP_LOGE(TAG, "couldn't find a handler of cmd: 0x%x", req->cmd);
}