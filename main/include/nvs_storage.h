#include <stdio.h>
#include <stdbool.h>

void nvs_init();
bool nvs_write_uint32(char *key, uint32_t val);
uint32_t nvs_read_uint32(char *key, uint32_t default_val);

uint32_t read_color_nvs();
void write_color_nvs(uint32_t color);

uint32_t read_ble_pwd_nvs();
void write_ble_pwd_nvs(uint32_t ble_pwd);

// bool nvs_read_uint32(char *key, char *default_val, char *read_val, size_t *length);
// bool nvs_write_str(char *key, char *val);

void read_device_name();
bool write_device_name(char *device_name);