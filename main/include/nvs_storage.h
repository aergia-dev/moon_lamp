#include <stdio.h>
#include <stdbool.h>

void nvs_init();
// bool nvs_write_uint32(char *key, uint32_t val);
// uint32_t nvs_read_uint32(char *key, uint32_t default_val);

uint32_t read_color_nvs();
void write_color_nvs(uint32_t color);

uint32_t read_ble_pwd_nvs();
void write_ble_pwd_nvs(uint32_t ble_pwd);

void read_device_name_nvs(char *name);
bool write_device_name_nvs(char *device_name);

void read_onTime_nvs(uint64_t *on_time);
bool write_onTime_nvs(uint64_t on_tme);

void read_offTime_nvs(uint64_t *off_time);
bool write_offTime_nvs(uint64_t off_time);
