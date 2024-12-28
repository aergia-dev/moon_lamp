
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "ble_protocol.h"
#include <esp_log.h>
#include <string.h>
#include "sleep_light.h"

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

static void handler_test_color(handler_req_t *req, handler_rsp_t *rsp)
{

    if (req->len == sizeof(led_status_t))
    {
        led_status_t status;
        memcpy(&status, req->data, sizeof(led_status_t));
        ESP_LOGI(TAG, "test led status");
        ESP_LOGI(TAG, "is_on: %d, brightness: %d, color: %lu", (int)status.is_on, (int)status.brightness, status.color);
        ble_cont_light(&status);
        rsp->is_success = true;
    }
    else
    {
        rsp->is_success = false;
    }

    rsp->len = 0;
}

static void handler_write_color(handler_req_t *req, handler_rsp_t *rsp) {}

const cmd_map_t cmd_handlers[] = {
    {RESET, handler_reset},
    {WRITE_STATUS, handler_write_status},
    {READ_STATUS, handler_read_status},
    {TEST_COLOR, handler_test_color},
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