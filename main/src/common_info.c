#include "string.h"
#include "nvs_storage.h"

const uint32_t GPIO_INPUT_IO_0 = 10;
#define DEV_NAME_LEN 15
// const uint32_t LED_CONT = 10;
// const uint32_t LED_CONT_GPIO = 21;

char device_name[DEV_NAME_LEN] = {};
static uint32_t ble_passkey = 0;

void init_common_info()
{
    memset(device_name, 0, DEV_NAME_LEN);
    read_device_name_nvs(device_name);
    ble_passkey = read_ble_pwd_nvs();
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

// uint32_t get_led_count()
// {
//     return LED_CONT;
// }

// uint32_t get_led_cont_gpio()
// {
//     return LED_CONT_GPIO;
// }