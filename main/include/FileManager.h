#ifndef _FILEMANAGER_
#define _FILEMANAGER_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include <errno.h>
#include "cJSON.h"
#include "paramDefinition.h"
#include "utils.h"

#define storageNamespace "offline"

enum storageMode
{
    update,
    newData
};

#ifdef __cplusplus
extern "C"
{
#endif

    static const char *FMTAG = "FILE_MANAGER";

    // Handle of the wear levelling library instance
    static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
    // Mount path for the partition
    static const char *base_path = "/config";

    static uint8_t partitionTableVersion = 0;

    void mount();
    void unmount();
    int getBusData(char **data, unsigned int dataLen);
    void createBusFile(modbusDevice_t slaveConfig);
    void addBusData(modbusDevice_t slaveConfig);
    void createSignupFile(uint8_t signup);
    int8_t getSignup();
    void writeFile(const char *fileName, char *text);
    uint32_t getPartitionSize();
    uint32_t getFreeStorage();
    void createOutputFile(uint8_t output);
    int8_t getOutput();
    esp_err_t setCity(char *city);
    esp_err_t getCity(char *city);
    void setOfflineStatus(uint8_t first, double end);
    int8_t getOfflineStatus(char **data, uint16_t dataLen);
    void setHardware(uint8_t version);
    int8_t getHardware();
    void handleOffline(uint8_t hasOffline);
    int8_t getOffline();
    bool checkBusData(modbusDevice_t slaveConfig);

#ifdef __cplusplus
}
#endif

#endif