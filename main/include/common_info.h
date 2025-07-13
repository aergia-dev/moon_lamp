#ifndef COMMON_INFO_H
#define COMMON_INFO_H
#include <time.h>
typedef struct
{
    uint32_t is_on;
    uint32_t brightness;
    uint32_t color; // argb order
    bool is_time_synced;
    uint8_t power_on_hour;
    uint8_t power_on_minute;
    uint8_t power_off_hour;
    uint8_t power_off_minute;
    uint16_t delay_power_off_min;
} __attribute__((packed)) led_status_t;

void init_common_info();
char *get_device_name();
uint32_t get_touch_gpio();
size_t get_device_name_length();

led_status_t get_led_status(void);
void set_led_status(led_status_t s);

uint32_t get_ble_passkey();
bool set_device_name(char *name);
bool set_ble_passkey(uint32_t passkey);

uint32_t get_default_color();
void get_local_time(struct tm *time);

void set_time_synced(bool is_synced);
#endif