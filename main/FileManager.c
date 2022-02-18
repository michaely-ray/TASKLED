#include "FileManager.h"

void mount()
{
    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 10,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE};
    esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(FMTAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return;
    }

    esp_err_t ret = nvs_flash_init();

    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    // {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }

    ESP_ERROR_CHECK(ret);
    if (nvs_flash_init_partition("offline") == ESP_OK)
    {
        ESP_LOGI(FMTAG, "Partition type NVS");
    }
    else
    {
        ESP_LOGW(FMTAG, "No NVS Partition named 'offline'");
        partitionTableVersion = 1;
    }
}

void unmount()
{
    ESP_LOGI(FMTAG, "Unmounting FAT filesystem");
    ESP_ERROR_CHECK(esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle));

    ESP_LOGI(FMTAG, "Done");
}

int getBusData(char **data, unsigned int dataLen)
{
    // FILE *f = fopen("/config/bus.txt", "rb");
    // if (f == NULL)
    // {
    //     ESP_LOGE(FMTAG, "Failed to open file 'Bus' for reading");
    //     fclose(f);
    //     return -1;
    // }
    // memset(*data, 0, dataLen);
    // if (fgets(*data, dataLen, f) == NULL)
    // {
    //     ESP_LOGE(FMTAG, "Empty file: bus.txt");
    //     fclose(f);
    //     return 0;
    // }

    // fclose(f);
    // return 1;

    if (nvs_str_read(*data, dataLen, "bus") == ESP_OK)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void createBusFile(modbusDevice_t slaveConfig)
{
    cJSON *bus = cJSON_CreateObject();
    cJSON *node = NULL;

    if (cJSON_AddNumberToObject(bus, "amt", 1) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add amount");
    }

    node = cJSON_AddArrayToObject(bus, "Bus");
    if (node == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to create JSON Array");
    }

    cJSON *device = cJSON_CreateObject();
    if (cJSON_AddNumberToObject(device, "id", slaveConfig.slaveID) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add slaveID");
    }
    if (cJSON_AddNumberToObject(device, "std", slaveConfig.standard) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add standard");
    }
    if (cJSON_AddNumberToObject(device, "baud", slaveConfig.baud) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add baud");
    }
    if (cJSON_AddStringToObject(device, "mfr", slaveConfig.manufacturer) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add manufacturer");
    }
    if (cJSON_AddStringToObject(device, "mdl", slaveConfig.model) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add model");
    }
    if (cJSON_AddStringToObject(device, "sn", slaveConfig.serialNo) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add serial number");
    }
    cJSON_AddItemToArray(node, device);

    char *jsonString = cJSON_PrintUnformatted(bus);
    // writeFile("/config/bus.txt", jsonString);

    nvs_str_save("bus", jsonString);

    free(jsonString);
    cJSON_Delete(bus);
}

void addBusData(modbusDevice_t slaveConfig)
{
    cJSON *bus = NULL;
    cJSON *node = NULL;

    char *busJSON = (char *)calloc(512, 1);
    getBusData(&busJSON, 512);
    bus = cJSON_Parse(busJSON);

    cJSON *amount = cJSON_GetObjectItemCaseSensitive(bus, "amt");
    if (amount == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to acquire JSON amt param");
    }
    uint8_t amt = amount->valueint;
    ESP_LOGI(FMTAG, "amount: %d", amt);
    cJSON_ReplaceItemInObjectCaseSensitive(bus, "amt", cJSON_CreateNumber(++amt));

    node = cJSON_GetObjectItemCaseSensitive(bus, "Bus");
    if (node == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to acquire JSON Array");
    }

    cJSON *device = cJSON_CreateObject();
    if (cJSON_AddNumberToObject(device, "id", slaveConfig.slaveID) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add slaveID");
    }
    if (cJSON_AddNumberToObject(device, "std", slaveConfig.standard) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add standard");
    }
    if (cJSON_AddNumberToObject(device, "baud", slaveConfig.baud) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add baud");
    }
    if (cJSON_AddStringToObject(device, "mfr", slaveConfig.manufacturer) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add manufacturer");
    }
    if (cJSON_AddStringToObject(device, "mdl", slaveConfig.model) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add model");
    }
    if (cJSON_AddStringToObject(device, "sn", slaveConfig.serialNo) == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to add serial number");
    }
    cJSON_AddItemToArray(node, device);

    char *jsonString = cJSON_PrintUnformatted(bus);
    // writeFile("/config/bus.txt", jsonString);
    nvs_str_save("bus", jsonString);
    free(jsonString);

    cJSON_Delete(bus);
    free(busJSON);
}

bool checkBusData(modbusDevice_t slaveConfig)
{
    bool result = false;
    cJSON *bus = NULL;
    cJSON *nodes = NULL;
    cJSON *node = NULL;
    cJSON *item = NULL;
    char *busJSON = (char *)calloc(512, 1);
    if (getBusData(&busJSON, 512) < 0)
    {
        return false;
    }
    bus = cJSON_Parse(busJSON);

    cJSON *amount = cJSON_GetObjectItemCaseSensitive(bus, "amt");
    if (amount == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to acquire JSON amt param");
    }
    uint8_t amt = amount->valueint;
    ESP_LOGI(FMTAG, "amount: %d", amt);
    // cJSON_ReplaceItemInObjectCaseSensitive(bus, "amt", cJSON_CreateNumber(++amt));

    nodes = cJSON_GetObjectItemCaseSensitive(bus, "Bus");
    if (nodes == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to acquire JSON Array");
    }
    cJSON_ArrayForEach(node, nodes)
    {
        item = cJSON_GetObjectItem(node, "sn");
        if (strcmp(slaveConfig.serialNo, item->valuestring) == 0)
        {
            result = true;
        }
    }

    cJSON_Delete(bus);
    free(busJSON);
    return result;
}

void createSignupFile(uint8_t signup)
{
    char buf[6];
    sprintf(buf, "%d\n", signup);
    nvs_str_save("sugnUpFile", buf);
}

int8_t getSignup()
{
    // FILE *f = fopen("/config/signup.txt", "rb");
    // if (f == NULL)
    // {
    //     ESP_LOGE(FMTAG, "Failed to open file 'Signup' for reading");
    //     fclose(f);
    //     return -1;
    // }

    // uint8_t dataLen = 5;
    // char data[dataLen];
    // if (fgets(data, dataLen, f) == NULL)
    // {
    //     ESP_LOGE(FMTAG, "Empty file: signup.txt");
    //     fclose(f);
    //     return -1;
    // }

    // fclose(f);
    char data[6];
    int signup;
    if (nvs_str_read(data, (sizeof(char) * 6), "signUpFile") == ESP_OK)
    {
        sscanf(data, "%d\n", &signup);
        ESP_LOGI(FMTAG, "Stored Signup: %d", signup);
        return signup;
    }
    else
    {
        return -1;
    }
}

void writeFile(const char *fileName, char *text)
{
    FILE *f = fopen(fileName, "wb");
    if (f == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to open file '%s' for writing, error no: %d", fileName, errno);
        return;
    }
    fprintf(f, "%s", text);
    fclose(f);
    ESP_LOGI(FMTAG, "File '%s' written", fileName);
}

uint32_t getPartitionSize()
{
    uint32_t totalSize = 0;
    const esp_partition_t *part1 = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, "storage");
    nvs_stats_t nvsStats;
    nvs_get_stats("offline", &nvsStats);
    uint32_t part2 = nvsStats.total_entries * 32;
    totalSize = part1->size + part2;
    return totalSize;
}

uint32_t getFileSize(const char *fileName)
{
    uint32_t result = 0;
    FILE *f = fopen(fileName, "r");
    if (f == NULL)
    {
        ESP_LOGE(FMTAG, "Failed to open file '%s' for reading file size", fileName);
    }
    else
    {
        fseek(f, 0L, SEEK_END);
        result = ftell(f);
    }
    fclose(f);
    return result;
}

uint32_t getFreeStorage()
{
    uint32_t result = 0;
    result += getFileSize("/config/bus.txt");
    result += getFileSize("/config/output.txt");
    result += getFileSize("/config/offline.txt");
    result += getFileSize("/config/hardware.txt");

    nvs_stats_t nvsStats;
    nvs_get_stats("offline", &nvsStats);
    printf("Count: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)\n",
           nvsStats.used_entries, nvsStats.free_entries, nvsStats.total_entries);
    result += nvsStats.used_entries * 32;

    return getPartitionSize() - result;
}

void createOutputFile(uint8_t output)
{
    // char buf[6];
    // sprintf(buf, "%d\n", output);
    // writeFile("/config/output.txt", buf);
    char buf[6];
    sprintf(buf, "%d\n", output);
    nvs_str_save("outputFile", buf);
}

int8_t getOutput()
{
    // FILE *f = fopen("/config/output.txt", "rb");
    // if (f == NULL)
    // {
    //     ESP_LOGE(FMTAG, "Failed to open file 'Output' for reading");
    //     fclose(f);
    //     return -1;
    // }

    // uint8_t dataLen = 5;
    // char data[dataLen];
    // if (fgets(data, dataLen, f) == NULL)
    // {
    //     ESP_LOGE(FMTAG, "Empty file: output.txt");
    //     fclose(f);
    //     return -1;
    // }

    // fclose(f);

    char data[6];
    int output;
    if (nvs_str_read(data, (sizeof(char) * 6), "outputFile") == ESP_OK)
    {
        sscanf(data, "%d\n", &output);
        ESP_LOGI(FMTAG, "Stored output: %d", output);
        return output;
    }
    else
    {
        return -1;
    }

    // int output;
    // sscanf(data, "%d\n", &output);
    // ESP_LOGI(FMTAG, "Stored Output: %d", output);
    // return output;
}

esp_err_t setCity(char *city)
{
    nvs_handle_t myHandle;
    esp_err_t err;

    err = nvs_open_from_partition("offline", storageNamespace, NVS_READWRITE, &myHandle);
    if (err != ESP_OK)
        return err;

    err = nvs_set_str(myHandle, "city", city);
    if (err != ESP_OK)
        return err;

    nvs_close(myHandle);

    return ESP_OK;
}

esp_err_t getCity(char *city)
{
    nvs_handle_t myHandle;
    esp_err_t err;

    err = nvs_open_from_partition("offline", storageNamespace, NVS_READONLY, &myHandle);
    if (err != ESP_OK)
        return err;

    size_t requiredSize = 0;
    err = nvs_get_str(myHandle, "city", NULL, &requiredSize);
    if (err != ESP_OK)
        return err;

    err = nvs_get_str(myHandle, "city", city, &requiredSize);
    if (err != ESP_OK)
        return err;

    nvs_close(myHandle);

    return ESP_OK;
}

void handleOffline(uint8_t hasOffline)
{
    // char buf[6];
    // sprintf(buf, "%d\n", hasOffline);
    // writeFile("/config/offline.txt", buf);

    char buf[6];
    sprintf(buf, "%d\n", hasOffline);
    nvs_str_save("offlineFile", buf);
}

int8_t getOffline()
{
    // FILE *f = fopen("/config/offline.txt", "rb");
    // if (f == NULL)
    // {
    //     ESP_LOGE(FMTAG, "Failed to open file 'Offline' for reading");
    //     fclose(f);
    //     return -1;
    // }

    // uint8_t dataLen = 5;
    // char data[dataLen];
    // if (fgets(data, dataLen, f) == NULL)
    // {
    //     ESP_LOGE(FMTAG, "Empty file: offline.txt");
    //     fclose(f);
    //     return -1;
    // }

    // fclose(f);
    // int offline;
    // sscanf(data, "%d\n", &offline);
    // ESP_LOGI(FMTAG, "Stored Offline: %d", offline);
    // return offline;

    char data[6];
    int offline;
    if (nvs_str_read(data, (sizeof(char) * 6), "offlineFile") == ESP_OK)
    {
        sscanf(data, "%d\n", &offline);
        ESP_LOGI(FMTAG, "Stored Signup: %d", offline);
        return offline;
    }
    else
    {
        return -1;
    }
}

void setHardware(uint8_t version)
{
    char buf[6];
    sprintf(buf, "%d\n", version);
    nvs_str_save("hardwareFile", buf);
}

int8_t getHardware()
{
    // FILE *f = fopen("/config/hardware.txt", "rb");
    // if (f == NULL)
    // {
    //     ESP_LOGE(FMTAG, "Failed to open file \"hardware.txt\" for reading");
    //     fclose(f);
    //     return -1;
    // }

    // uint8_t dataLen = 5;
    // char data[dataLen];
    // if (fgets(data, dataLen, f) == NULL)
    // {
    //     ESP_LOGE(FMTAG, "Empty file: hardware.txt");
    //     fclose(f);
    //     return -1;
    // }

    // fclose(f);
    // int hardware;
    // sscanf(data, "%d\n", &hardware);
    // ESP_LOGI(FMTAG, "Stored Hardware: %d", hardware);
    // return hardware;

    char data[6];
    int hardware;
    if (nvs_str_read(data, (sizeof(char) * 6), "hardwareFile") == ESP_OK)
    {
        sscanf(data, "%d\n", &hardware);
        ESP_LOGI(FMTAG, "Stored Signup: %d", hardware);
        return hardware;
    }
    else
    {
        return -1;
    }
}
