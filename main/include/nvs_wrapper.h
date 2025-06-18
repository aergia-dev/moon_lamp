#ifndef NVS_WRAPPER_H
#define NVS_WRAPPER_H
#endif

#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t nvs_read_uint32(const char *key, uint32_t *out_val);
esp_err_t nvs_read_uint64(const char *key, uint64_t *out_val);
esp_err_t nvs_read_uint64(const char *key, uint64_t *out_val);
esp_err_t nvs_write_uint64(const char *key, uint64_t val);
esp_err_t nvs_read_str(const char *key, char *out_val, size_t *length);
esp_err_t nvs_write_uint64(const char *key, uint64_t val);

#ifdef __cplusplus
}
#endif