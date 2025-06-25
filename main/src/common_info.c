#include "string.h"
#include "nvs_storage.h"
#include "common_info.h"
#include "time.h"

const uint32_t GPIO_INPUT_IO_0 = 10;
#define DEV_NAME_LEN 15

static const char *TAG = "common_info";

char device_name[DEV_NAME_LEN] = {};
static uint32_t ble_passkey = 0;
const uint32_t default_color = 0x00FEE033;

uint32_t get_default_color()
{
    return default_color;
}

static led_status_t _led_status = {
    .is_on = 0,
    .brightness = 0,
    .color = 0,
    .is_time_synced = false,
    .power_on_hour = 0,
    .power_on_minute = 0,
    .power_off_hour = 0,
    .power_off_minute = 0,
    .power_off_delay_min = 0,
};

led_status_t get_led_status(void)
{
    return _led_status;
}

void set_led_status(led_status_t s)
{
    _led_status.is_on = s.is_on;
    _led_status.brightness = s.brightness;
    _led_status.color = s.color;
    _led_status.power_on_hour = s.power_on_hour;
    _led_status.power_on_minute = s.power_on_minute;
    _led_status.power_off_hour = s.power_off_hour;
    _led_status.power_off_minute = s.power_off_minute;
    _led_status.power_off_delay_min = s.power_off_delay_min;
}

void init_common_info()
{
    memset(device_name, 0, DEV_NAME_LEN);
    read_device_name_nvs(device_name);
    ble_passkey = read_ble_pwd_nvs();

    led_status_t status = {
        .is_on = 1,
        .brightness = 100,
        .color = read_color_nvs(),
    };
    set_led_status(status);
}

char *get_device_name()
{
    return device_name;
}

uint32_t get_touch_gpio()
{
    return GPIO_INPUT_IO_0;
}

size_t get_device_name_length()
{
    return DEV_NAME_LEN;
}

bool set_device_name(char *name)
{
    write_device_name_nvs(name);
    return true;
}

uint32_t get_ble_passkey()
{
    return ble_passkey;
}

bool set_ble_passkey(uint32_t passkey)
{
    write_ble_pwd_nvs(passkey);
    return true;
}
void set_time_synced(bool is_synced)
{
    _led_status.is_time_synced = is_synced;
}

void get_local_time(struct tm *t)
{
    time_t now;
    time(&now);
    localtime_r(&now, t);
}
