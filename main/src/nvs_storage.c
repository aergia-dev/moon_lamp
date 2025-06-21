
#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "string.h"
#include "common_info.h"

#define TAG "NVS"

void nvs_init()
{
    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
}

void open_nvs_handle(nvs_handle_t *handle)
{
    esp_err_t err;
    err = nvs_open("storage", NVS_READWRITE, handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
}

uint32_t nvs_read_uint32(char *key, uint32_t default_val)
{
    esp_err_t err;
    nvs_handle_t handle;
    open_nvs_handle(&handle);
    uint32_t saved_val = 0;
    err = nvs_get_u32(handle, key, &saved_val);

    switch (err)
    {
    case ESP_OK:
        ESP_LOGI(TAG, "Done");
        ESP_LOGI(TAG, "read saved val = %" PRIu32, saved_val);
        break;
    default:
        saved_val = default_val;
        ESP_LOGI(TAG, "not init or can't read. set as default %" PRIu32, saved_val);
    }

    nvs_close(handle);

    return saved_val;
}

bool nvs_write_uint32(char *key, uint32_t val)
{
    esp_err_t err;
    bool ret = true;
    nvs_handle_t handle;
    open_nvs_handle(&handle);

    err = nvs_set_u32(handle, key, val);

    if (err != ESP_OK)
        ret = false;

    ESP_LOGI(TAG, "Committing updates in NVS ... ");
    err = nvs_commit(handle);

    if (err == ESP_OK)
        ESP_LOGI(TAG, "Done");
    else
        ESP_LOGI(TAG, "Failed");
    // Close
    nvs_close(handle);

    return ret;
}

bool nvs_read_str(char *key, char *default_val, char *read_val, size_t *length)
{
    esp_err_t err;
    nvs_handle_t handle;
    open_nvs_handle(&handle);
    err = nvs_get_str(handle, key, read_val, length);

    switch (err)
    {
    case ESP_OK:
        ESP_LOGI(TAG, "Done");
        ESP_LOGI(TAG, "read saved val = %s", read_val);
        break;
    default:
        memcpy(read_val, default_val, *length);
        ESP_LOGI(TAG, "not init or can't read. set as default %s", read_val);
    }

    nvs_close(handle);

    return err;
}

bool nvs_write_str_nvs(char *key, char *val)
{
    esp_err_t err;
    bool ret = true;
    nvs_handle_t handle;
    open_nvs_handle(&handle);

    err = nvs_set_str(handle, key, val);

    if (err != ESP_OK)
        ret = false;

    ESP_LOGI(TAG, "Committing updates in NVS ... ");
    err = nvs_commit(handle);
    ESP_LOGI(TAG, "%s", (err != ESP_OK) ? "Failed!" : "Done");

    // Close
    nvs_close(handle);

    return ret;
}

uint32_t read_color_nvs()
{
    uint32_t default_color = get_default_color();
    uint32_t saved_color = nvs_read_uint32("saved_color", default_color);
    return saved_color;
}

void write_color_nvs(uint32_t color)
{
    ESP_LOGI(TAG, "write color to nvs %" PRIu32, color);
    nvs_write_uint32("saved_color", color);
}

uint32_t read_ble_pwd_nvs()
{
#define DEFAULT_BLE_PWD 1233456

    uint32_t saved_color = nvs_read_uint32("ble_pwd", DEFAULT_BLE_PWD);
    return saved_color;
}

void write_ble_pwd_nvs(uint32_t color)
{
    ESP_LOGI(TAG, "write ble_pwd to nvs %" PRIu32, color);
    nvs_write_uint32("ble_pwd", color);
}

void read_device_name_nvs(char *name)
{
    size_t length = get_device_name_length();
    char *default_name = "sleep_light";

    if (nvs_read_str("device_name", default_name, name, &length))
    {
        ESP_LOGI(TAG, "device name from nvs %s", name);
    }
    else
    {

        ESP_LOGI(TAG, "can't read device name from nvs so use %s", default_name);
    }
}

bool write_device_name_nvs(char *name)
{
    return nvs_write_str_nvs("device_name", name);
}

void read_onTime_nvs(uint32_t *on_time)
{
    uint32_t default_on_time = 10; // default on time in seconds
    *on_time = nvs_read_uint32("on_time", default_on_time);
    ESP_LOGI(TAG, "on time from nvs %" PRIu32, *on_time);
}

bool write_onTime_nvs(uint32_t on_time)
{
    ESP_LOGI(TAG, "write on time to nvs %" PRIu32, on_time);
    return nvs_write_uint32("on_time", on_time);
}

void read_offTime_nvs(uint64_t *off_time)
{
    uint32_t default_off_time = 10; // default off time in seconds
    *off_time = nvs_read_uint32("off_time", default_off_time);
    ESP_LOGI(TAG, "off time from nvs %" PRIu64, *off_time);
}

bool write_offTime_nvs(uint64_t off_time)
{
    ESP_LOGI(TAG, "write off time to nvs %" PRIu64, off_time);
    return nvs_write_uint32("off_time", off_time);
}