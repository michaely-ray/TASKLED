#ifndef _MODBUSDRIVER_
#define _MODBUSDRIVER_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_system.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

#define UART_PORT UART_NUM_2
// Note: UART2 default pins IO16, IO17 do not work on ESP32-WROVER module
// because these pins connected to PSRAM
#define TXD (4)
#define RXD (13)
// RTS for RS485 Half-Duplex Mode manages DE/~RE
#define RTS (15)

#define CTS UART_PIN_NO_CHANGE

#define BUF_SIZE (127)

static const char *TAG = "ModbusDriver";

enum modbusFunctionCode
{
    coil = 1,
    discreteInput,
    holdingRegs,
    inputRegs,
    writeSingleCoil,
    writeSingleHoldingReg,
    writeMultipleHoldingReg = 16,
};

typedef struct
{
    uint16_t len;
    uint8_t data[40];
} uartRX_t;

typedef struct
{
    uint8_t slave_addr;
    uint8_t function_code;
    uint16_t reg_start;
    uint16_t reg_size;
} request_t;

typedef struct
{
    uint8_t id;
    uint32_t baud;
} deviceConfig_t;

class ModbusDriver
{
private:
    uint8_t deviceAddress;
    uint32_t currentBaud;
    bool waiting;
    QueueHandle_t uartQueue;
    QueueHandle_t uartRxQueue;
    uint16_t answerLen;
    uint8_t *answerData;
    deviceConfig_t *device;
    uint8_t busSize;
    void serialBegin(int baud);
    uint CRC16_2(unsigned char *buf, int len);
    esp_err_t send(uint16_t address, uint amount, uint functionCode);
    esp_err_t getData(uint functionCode, uint8_t **data);
    esp_err_t getConfirmation(uint8_t **data);

public:
    ModbusDriver();
    esp_err_t sendRaw(uint8_t id, uint16_t address, uint amount, uint functionCode);
    esp_err_t readReg(uint32_t baud, uint8_t id, uint16_t address, uint amount, uint functionCode, uint8_t **data);
    esp_err_t sendModBus(request_t *request, uint8_t *data);
    void setBus(uint8_t busSize, deviceConfig_t *deviceArray);
    esp_err_t readInputRegs(uint16_t address, uint amount, uint8_t **data);
    esp_err_t readHoldingRegs(uint16_t address, uint amount, uint8_t **data);
    esp_err_t writeSingleReg(uint16_t address, uint16_t value);
    esp_err_t writeMultHoldReg(uint16_t address, uint16_t value);
    esp_err_t writeSpecialReg(uint16_t address, uint amount, uint functionCode);
    esp_err_t sendEnableRefusol();
    void setSlaveAddress(uint slaveAddress);
    uint getSlaveAddress();
    static void uartEventTaskWrapper(void *);
    void uartEventTask();
    static void uartMessageHandlerTaskWrapper(void *);
    void uartMessageHandlerTask();
    void setDevice(uint8_t deviceId);
};

#endif