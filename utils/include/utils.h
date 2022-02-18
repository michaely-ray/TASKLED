
#include "nvs_flash.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C"
{
#endif
  esp_err_t nvs_reg_read(uint8_t key, uint8_t *data, size_t data_len);
  esp_err_t nvs_reg_write(uint8_t key, uint8_t *value, size_t data_len);
  esp_err_t nvs_str_read(char *data, size_t data_len, char *key);
  esp_err_t nvs_str_save(char *key, char *value);
  esp_err_t nvs_reg_erase(uint8_t key);
  esp_err_t nvs_int16_save(char *key, int8_t value);
  esp_err_t nvs_int16_read(char *key, int8_t *value);
  void quicksort(float *number, int first, int last);
#ifdef __cplusplus
}
#endif