#include <stdio.h>
#include <stdbool.h>

void nvs_init();
bool nvs_write_uint32(char *key, uint32_t val);
uint32_t nvs_read_uint32(char *key, uint32_t default_val);

uint32_t read_color_nvs();
void write_color_nvs(uint32_t color);

uint32_t read_ble_pwd_nvs();
void write_ble_pwd_nvs(uint32_t ble_pwd);