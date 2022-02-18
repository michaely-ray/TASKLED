#ifndef _INVERTER_
#define _INVERTER_

#include "ModbusDriver.h"
#include "cJSON.h"
#include <math.h>

#include "paramDefinition.h"
#include "FileManager.h"
#include "sungrow.h"
#include "sunspecFronius.h"
#include "huawei.h"
#include "sunspecEcosolys.h"
#include "canadian.h"
#include "canadianSolar.h"
#include "refusol.h"
#include "sofar.h"
#include "utils.h"
#include "saj.h"
// #include "sunspecRefusol.h" //todo

static const char *INVTAG = "inverterClass";

typedef union
{
    float f;
    struct
    {
        // Order is important.
        // Here the members of the union data structure
        // use the same memory (32 bits).
        // The ordering is taken
        // from the LSB to the MSB.
        unsigned int mantissa : 23;
        unsigned int exponent : 8;
        unsigned int sign : 1;
    } raw;
} myfloat;

typedef struct
{
    uint8_t size;
    uint8_t mode;
    registerDefinition *table;
} deviceModel_t;

class Inverter
{
private:
    ModbusDriver modbus;
    cJSON *data = cJSON_CreateObject();
    cJSON **nodeData;
    uint16_t errors;
    modbusDevice_t *devices = NULL;
    uint8_t busSize;
    uint16_t maxErrors = 0;
    char deviceName[10];
    uint8_t busSet = 0;
    double lastEnd = 0;
    uint16_t handleU16(unsigned char **data, uint8_t endianess);
    int16_t handleS16(unsigned char **data, uint8_t endianess);
    uint32_t handleU32(unsigned char **data, uint8_t endianess);
    uint64_t handleU64(unsigned char **data, uint8_t endianess);
    int32_t handleS32(unsigned char **data, uint8_t endianess);
    float handleF32(unsigned char **data);
    float handleSunssf(unsigned char **data);
    deviceModel_t findStandard(uint8_t standard, uint8_t model);

public:
    char softError[240];
    uint8_t busStatus = 0;
    Inverter();
    void setBus(uint8_t busSize, modbusDevice_t *busDevices);
    int readSungrowStatus();
    int readSunspecFroniusStatus();
    void read(uint8_t device, uint8_t standard, uint8_t model);
    void readPolling();
    esp_err_t readModbus(uint8_t readMode, uint16_t address, uint amount, uint8_t **data);
    esp_err_t outputControlSunspecFronius(uint8_t value);
    esp_err_t outputControlSunspecEcosolys(uint8_t value);
    esp_err_t outputControlHuawei(uint8_t value);
    esp_err_t outputControlSungrow(uint8_t value);
    esp_err_t outputControlCanadian(uint8_t value);
    esp_err_t outputControlCanadianSolar(uint8_t value);
    esp_err_t outputControlRefusol(uint8_t value);
    esp_err_t outputControlSofar(uint8_t value);
    esp_err_t outputControlSAJ(uint8_t value);
    esp_err_t outputControlSunspecRefusol(uint8_t value);
    esp_err_t outputControl(uint8_t value, uint8_t standard);
    char *getJSON();
    char *getJSONUnformatted();
    uint16_t getErrorAmount();
    void zeroErrorAmount();
    void readPromiscuous();
    uint16_t getMaxErrorAmount();
    uint8_t handleState(uint8_t output);
    void monitorStatePolling(int8_t **result);
    int32_t readState(uint8_t readMode, uint16_t address);
    uint8_t getBusSet();
    void setBusSet(uint8_t value);
    double getLastEnd();
    char *handleStrSN(uint8_t standard, unsigned char **data, int address);
    registerDefinition *getRegister(char *label, registerDefinition *table, uint8_t length);
    bool validateFrequency(float frequency);
    float handleFloatData(unsigned char **data, uint8_t type, float scale, uint8_t endianness);
    bool validatePF(float pf);
    void getBus();
};

#endif