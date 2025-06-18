#include "esp_log.h"
#include "nvs.h"
#include "nvs_storage.h"
#include <stdio.h>
#include <type_traits>

static const char *TAG = "NVS_WRAPPER";

template <typename T> esp_err_t nvs_read_wrapper(const char *key, T &out_val) {
  esp_err_t err;
  nvs_handle_t handle;
  open_nvs_handle(&handle);

  if constexpr (std::is_same_v<T, uint32_t>) {
    err = nvs_get_u32(handler, key, reinterpret_cast<uint32_t *>(&out_val));
  } else if constexpr (std::is_same_v<T, uint64_t>) {
    err = nvs_get_u64(handler, key, reinterpret_cast<uint64_t *>(&out_val));
  } else if constexpr (std::is_same_v<T, char *>) {
    size_t length = 0;
    err = nvs_get_str(handle, key, out_val, &length);
  } else {
    ESP_LOGE(TAG, "Unsupported type for NVS read");
    return ESP_ERR_INVALID_ARG;
  }

  switch (err) {
  case ESP_OK:
    ESP_LOGI(TAG, "Read key '%s: %lld(type: %s)' successfully", key,
             (long long)out_val, typeid(T).name());
    break;
  case ESP_ERR_NVS_NOT_FOUND:
    ESP_LOGW(TAG, "Key '%s' not found, returning default value", key);
    break;
  case ESP_ERR_NVS_INVALID_HANDLE:
    ESP_LOGE(TAG, "Invalid NVS handle for key %s", key);
    break;
  case ESP_ERR_NVS_INVALID_NAME:
    ESP_LOGE(TAG, "Invalid key name: %s", key);
    break;
  default:
    ESP_LOGE(TAG, "Error reading key '%s': %s", key, esp_err_to_name(err));
    break;
  }
}

extern "C" {

esp_err_t nvs_read_uint32(const char *key, uint32_t *out_val) {
  return nvs_read_wrapper(handle, key, out_val);
}
esp_err_t nvs_read_uint64(const char *key, uint64_t *out_val) {
  return nvs_read_wrapper(handle, key, out_val);
}

esp_err_t nvs_write_uint64(const char *key, uint64_t val) {
  esp_err_t err = nvs_set_u64(handle, key, val);
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "Wrote key '%s: %llu' successfully", key,
             (unsigned long long)val);
  } else {
    ESP_LOGE(TAG, "Error writing key '%s': %s", key, esp_err_to_name(err));
  }
  return err;
}

esp_err_t nvs_read_str(const char *key, char *out_val, size_t *length) {
  esp_err_t err = nvs_get_str(handle, key, out_val, length);
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "Read key '%s: %s' successfully", key, out_val);
  } else {
    ESP_LOGE(TAG, "Error reading key '%s': %s", key, esp_err_to_name(err));
  }
  return err;
}
}
