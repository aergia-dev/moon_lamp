#include "string.h"
#include "nvs_storage.h"

const uint32_t GPIO_INPUT_IO_0 = 10;
#define DEV_NAME_LEN 15
// const uint32_t LED_CONT = 10;
// const uint32_t LED_CONT_GPIO = 21;

char device_name[DEV_NAME_LEN] = {};

void init_common_info()
{
    memset(device_name, 0, DEV_NAME_LEN);
    read_device_name(device_name);
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

// uint32_t get_led_count()
// {
//     return LED_CONT;
// }

// uint32_t get_led_cont_gpio()
// {
//     return LED_CONT_GPIO;
// }