#include <stdio.h>
#include "utils.h"

static const char *TAG = "FUNCTIONS";

//___________________________________________ NVS Read Register ___________________________________________
esp_err_t nvs_reg_read(uint8_t key, uint8_t *data, size_t data_len)
{
  nvs_handle my_handle;
  char str[] = {key, '\0'};
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {

    ESP_LOGI(TAG, "Error (%d) opening NVS handle!\n", err);
  }
  else
  {
    err = nvs_get_blob(my_handle, str, data, &data_len);
    if (err != ESP_OK)
    {
      if (err == ESP_ERR_NVS_NOT_FOUND)
      {

        ESP_LOGI(TAG, "\nKey data not found.\n");
      }
    }
    else
    {
      //ESP_LOGI(TAG, "\n data is %s\n", data);
    }
    nvs_close(my_handle);
  }
  return err;
}
//___________________________________________ NVS Write Register ___________________________________________

esp_err_t nvs_reg_erase(uint8_t key)
{
  nvs_handle my_handle;
  char str[] = {key, '\0'};
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    ESP_LOGI(TAG, "Error (%d) opening NVS handle!\n", err);
  }
  err = nvs_erase_key(my_handle, str);
  if (err != ESP_OK)
  {
    printf("apagado\n");
  }
  return err;
}

esp_err_t nvs_reg_write(uint8_t key, uint8_t *value, size_t data_len)
{
  nvs_handle my_handle;
  char str[] = {key, '\0'};
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    ESP_LOGI(TAG, "Error (%d) opening NVS handle!\n", err);
  }
  else
  {
    err = nvs_set_blob(my_handle, str, value, data_len);
    if (err != ESP_OK)
    {
      ESP_LOGI(TAG, "\nError in %s : (%04X)\n", str, err);
    }
    /* Salva em nvs */
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
      printf("\nError in commit! (%04X)\n", err);
    }
    nvs_close(my_handle);
  }
  return err;
}
//___________________________________________ Quick Sort Function___________________________________________
void quicksort(float *number, int first, int last)
{
  int i, j, pivot;
  float temp;
  if (first < last)
  {
    pivot = first;
    i = first;
    j = last;

    while (i < j)
    {
      while (number[i] <= number[pivot] && i < last)
        i++;
      while (number[j] > number[pivot])
        j--;
      if (i < j)
      {
        temp = number[i];
        number[i] = number[j];
        number[j] = temp;
      }
    }

    temp = number[pivot];
    number[pivot] = number[j];
    number[j] = temp;
    quicksort(number, first, j - 1);
    quicksort(number, j + 1, last);
  }
}
//___________________________________________ NVS Read String ___________________________________________
esp_err_t nvs_str_read(char *data, size_t data_len, char *key)
{
  nvs_handle my_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {

    ESP_LOGI(TAG, "Error (%d) opening NVS handle!\n", err);
  }
  else
  {
    err = nvs_get_str(my_handle, key, data, &data_len);
    if (err != ESP_OK)
    {
      if (err == ESP_ERR_NVS_NOT_FOUND)
      {

        ESP_LOGI(TAG, "\nKey data not found.\n");
      }
    }
    else
    {
      ESP_LOGI(TAG, "Read NVS String\n");
    }
    nvs_close(my_handle);
  }
  return err;
}
//___________________________________________ NVS Write String___________________________________________
esp_err_t nvs_str_save(char *key, char *value)
{
  nvs_handle my_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    ESP_LOGI(TAG, "Error (%d) opening NVS handle!\n", err);
  }
  else
  {
    err = nvs_set_str(my_handle, key, value);
    if (err != ESP_OK)
    {
      ESP_LOGI(TAG, "\nError in %s : (%04X)\n", key, err);
    }
    /* Salva em nvs */
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
      printf("\nError in commit! (%04X)\n", err);
    }
    nvs_close(my_handle);
  }
  return err;
}

//___________________________________________ NVS Write integer___________________________________________
esp_err_t nvs_int16_save(char *key, int8_t value)
{
  ESP_LOGI(TAG, "vai gravar %d", value);
  nvs_handle my_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    ESP_LOGI(TAG, "Error (%d) opening NVS handle!\n", err);
  }
  else
  {
    err = nvs_set_i8(my_handle, key, value);
    if (err != ESP_OK)
    {
      ESP_LOGI(TAG, "\nError in %s : (%04X)\n", key, err);
    }
    /* Salva em nvs */
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
      printf("\nError in commit! (%04X)\n", err);
    }
    nvs_close(my_handle);
  }
  return err;
}
//___________________________________________ NVS Read integer____________________________________________
esp_err_t nvs_int16_read(char *key, int8_t *value)
{
  nvs_handle my_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {

    ESP_LOGI(TAG, "Error (%d) opening NVS handle!\n", err);
  }
  else
  {
    err = nvs_get_i8(my_handle, key, value);
    if (err != ESP_OK)
    {
      if (err == ESP_ERR_NVS_NOT_FOUND)
      {

        ESP_LOGI(TAG, "\nKey data not found.\n");
      }
    }
    else
    {
      ESP_LOGI(TAG, "Read NVS String\n");
    }
    nvs_close(my_handle);
  }
  return err;
}