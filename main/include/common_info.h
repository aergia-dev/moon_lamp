#ifndef COMMON_INFO_H
#define COMMON_INFO_H

void init_common_info();
char *get_device_name();
uint32_t get_touch_gpio();
size_t get_device_name_length();

uint32_t get_ble_passkey();
bool set_device_name(char *name);
bool set_ble_passkey(uint32_t passkey);
#endif