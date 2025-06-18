#ifndef COMMON_INFO_H
#define COMMON_INFO_H

typedef struct
{
    uint32_t is_on;
    uint32_t brightness;
    uint32_t color; // argb
} led_status_t;

typedef struct
{
    uint32_t hour;
    uint32_t minute;
} event_time_t;

typedef struct
{
    event_time_t on;
    event_time_t off;
} on_off_time_t;

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

void set_on_time(event_time_t on_time);
void set_off_time(event_time_t off_time);
#endif