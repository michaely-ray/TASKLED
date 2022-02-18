#include "ModbusDriver.h"

ModbusDriver::ModbusDriver()
{
    uartRxQueue = xQueueCreate(5, sizeof(uartRX_t));

    this->answerLen = 0;
    this->waiting = false;
    setSlaveAddress(1);
    xTaskCreate(this->uartEventTaskWrapper, "uartEventTaskWrapper", 2048, this, 12, NULL);
    xTaskCreate(this->uartMessageHandlerTaskWrapper, "uartMessageHandlerTaskWrapper", 2048, this, 12, NULL);
    this->currentBaud = 9600;
    this->serialBegin(9600);
}

void ModbusDriver::setBus(uint8_t busSize, deviceConfig_t *deviceArray)
{
    this->device = (deviceConfig_t *)malloc(busSize * sizeof(deviceConfig_t));
    this->busSize = busSize;
    for (int i = 0; i < busSize; i++)
    {
        this->device[i].id = deviceArray[i].id;
        this->device[i].baud = deviceArray[i].baud;
        // printf("\nDriver:\nID:\t%d\nBaud:\t%d\n", this->device[i].id, this->device[i].baud);
    }
}

void ModbusDriver::serialBegin(int baud)
{
    uart_config_t uart_config = {
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, TXD, RXD, RTS, CTS);
    uart_driver_install(UART_PORT, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uartQueue, 0);
    uart_disable_tx_intr(UART_PORT);
    uart_set_rx_timeout(UART_PORT, 1);
    uart_disable_rx_intr(UART_PORT);
    uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX);
    //ESP_LOGI(TAG, "Modbus Serial Started, baud rate = %d", baud);
}

void ModbusDriver::uartMessageHandlerTask()
{
    uint8_t *dtmp = (uint8_t *)calloc(BUF_SIZE, 1);
    uartRX_t msg;
    uint16_t len = 0;

    while (1)
    {
        if (xQueueReceive(this->uartRxQueue, &msg, 200 / portTICK_RATE_MS))
        {
            for (int i = len; i < len + msg.len; i++)
            {
                dtmp[i] = msg.data[i - len];
            }
            len += msg.len;
        }
        else if (this->waiting && len > 0)
        {
            if (dtmp[0] == 0)
            {
                for (int i = 0; i < len; i++)
                    dtmp[i] = dtmp[i + 1];
                len--;
            }
            this->answerData = (uint8_t *)calloc(len, 1);
            memcpy(this->answerData, dtmp, len);
            this->answerLen = len;

            memset(dtmp, 0, BUF_SIZE);
            len = 0;
        }
    }
}

void ModbusDriver::uartEventTaskWrapper(void *_this)
{
    static_cast<ModbusDriver *>(_this)->uartEventTask();
}

void ModbusDriver::uartMessageHandlerTaskWrapper(void *_this)
{
    static_cast<ModbusDriver *>(_this)->uartMessageHandlerTask();
}

void ModbusDriver::uartEventTask()
{
    uart_event_t event;
    uartRX_t msg;
    uint8_t *dtmp = (uint8_t *)calloc(BUF_SIZE, 1);
    uart_flush(UART_PORT);
    while (1)
    {
        if (xQueueReceive(this->uartQueue, (void *)&event, (portTickType)portMAX_DELAY))
        {
            memset(dtmp, 0, BUF_SIZE);
            switch (event.type)
            {
            // Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
            case UART_DATA:
                // ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                msg.len = event.size;
                uart_read_bytes(UART_PORT, dtmp, event.size, portMAX_DELAY);
                memcpy(msg.data, dtmp, event.size);
                // printf("antes %d: ", event.size);
                // for(int i = 0; i<event.size; i++)
                //     printf("0x%02x ", msg.data[i]);
                // printf("\n");
                xQueueSend(uartRxQueue, &msg, 10 / portTICK_RATE_MS);
                break;
            // Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                //ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(UART_PORT);
                xQueueReset(this->uartQueue);
                break;
            // Event of UART ring buffer full
            case UART_BUFFER_FULL:
                //ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider encreasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(UART_PORT);
                xQueueReset(this->uartQueue);
                break;
            // Event of UART RX break detected
            case UART_BREAK:
                // ESP_LOGI(TAG, "uart rx break");
                break;
            // Event of UART parity check error
            case UART_PARITY_ERR:
                //ESP_LOGI(TAG, "uart parity error");
                break;
            // Event of UART frame error
            case UART_FRAME_ERR:
                //ESP_LOGI(TAG, "uart frame error");
                break;
            // UART_PATTERN_DET
            case UART_PATTERN_DET:
                //ESP_LOGI(TAG, "[UART PATTERN DETECTED]");
                break;
            // Others
            default:
                //ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
}

uint ModbusDriver::CRC16_2(unsigned char *buf, int len)
{
    uint crc = 0xFFFF;

    for (int pos = 0; pos < len; pos++)
    {
        crc ^= (uint)buf[pos]; // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--)
        { // Loop over each bit
            if ((crc & 0x0001) != 0)
            {              // If the LSB is set
                crc >>= 1; // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else           // Else LSB is not set
                crc >>= 1; // Just shift right
        }
    }
    // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
    return crc;
}

esp_err_t ModbusDriver::readInputRegs(uint16_t address, uint amount, uint8_t **data)
{
    this->send(address, amount, inputRegs);

    return (this->getData(inputRegs, data));
}

esp_err_t ModbusDriver::readHoldingRegs(uint16_t address, uint amount, uint8_t **data)
{
    this->send(address, amount, holdingRegs);

    return (this->getData(holdingRegs, data));
}

void ModbusDriver::setSlaveAddress(uint slaveAddress)
{
    this->deviceAddress = slaveAddress;
}

uint ModbusDriver::getSlaveAddress()
{
    return this->deviceAddress;
}

esp_err_t ModbusDriver::writeSingleReg(uint16_t address, uint16_t value)
{
    uint16_t *data = (uint16_t *)calloc(8, 1);
    esp_err_t errorCode = this->send(address, value, writeSingleHoldingReg);

    if (errorCode == ESP_OK)
    {
        //ESP_LOGI(TAG, "Register %d written value %d", address, value);
    }

    free(data);
    return errorCode;
}

esp_err_t ModbusDriver::writeMultHoldReg(uint16_t address, uint16_t value)
{
    uint16_t *data = (uint16_t *)calloc(8, 1);
    esp_err_t errorCode = this->send(address, value, writeMultipleHoldingReg);

    if (errorCode == ESP_OK)
    {
        //ESP_LOGI(TAG, "Register %d written value %d", address, value);
    }

    free(data);
    return errorCode;
}

esp_err_t ModbusDriver::writeSpecialReg(uint16_t address, uint amount, uint functionCode)
{
    esp_err_t errorCode = this->send(address, amount, functionCode);
    if (errorCode == ESP_OK)
    {
        //ESP_LOGI(TAG, "Register %d written value %d function %d", address, amount, functionCode);
    }
    return errorCode;
}

esp_err_t ModbusDriver::send(uint16_t address, uint amount, uint functionCode)
{
    unsigned char *buf = (unsigned char *)calloc(8, 1);
    esp_err_t errorCode = ESP_OK;

    buf[0] = address == 322 ? 0x88 : this->getSlaveAddress(); // 322 is the refusol adress to shutdown. This command  works only with broadcast device adress.
    buf[1] = functionCode;
    buf[2] = address / 256;
    buf[3] = address % 256;
    buf[4] = amount / 256;
    buf[5] = amount % 256;
    int crc = this->CRC16_2(buf, 6);
    buf[6] = crc % 256;
    buf[7] = crc / 256;
    //ESP_LOGI(TAG, "\n |DevAdress|func|RegAdress1|ReAdress2|Data1|Data2|CRC1|CRC2|\n|%d|%d|%d|%d|%d|%d|%d|%d|\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    uart_write_bytes(UART_PORT, (const char *)buf, 8);
    // ESP_LOGI(TAG, "Register %d written, amount: %d", address, amount);

    if (functionCode >= writeSingleCoil)
    {
        errorCode = this->getConfirmation(&buf);
    }
    free(buf);
    return errorCode;
}

esp_err_t ModbusDriver::sendEnableRefusol()
{
    unsigned char *buf = (unsigned char *)calloc(10, 1);
    esp_err_t errorCode = ESP_OK;

    buf[0] = this->getSlaveAddress();
    buf[1] = 0x13;
    buf[2] = 0x10;
    buf[3] = 0x40;
    buf[4] = 0x00;
    buf[5] = 0x01;
    buf[6] = 0x00;
    buf[7] = 0x02;
    int crc = this->CRC16_2(buf, 8);
    buf[8] = crc % 256;
    buf[9] = crc / 256;
    uart_write_bytes(UART_PORT, (const char *)buf, 10);
    //ESP_LOGI(TAG, "\n|DevAdress|func|RegAdress1|ReAdress2|Data1|Data2|Data3|Data4|CRC1|CRC2|\n|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);

    errorCode = this->getConfirmation(&buf);

    free(buf);
    return errorCode;
}
esp_err_t ModbusDriver::getData(uint functionCode, uint8_t **data)
{
    uint8_t *dtmp = (uint8_t *)calloc(BUF_SIZE, 1);
    esp_err_t errorCode = ESP_OK;
    memset(*data, 0, 35);
    int len = 0;
    uint64_t timeout = esp_timer_get_time();

    this->waiting = true;
    while (this->answerLen == 0 && (esp_timer_get_time() - timeout) <= 600000)
        esp_task_wdt_reset();
    len = this->answerLen;
    this->answerLen = 0;
    this->waiting = false;

    if (len > 0)
    {
        memcpy(dtmp, this->answerData, len);
        free(this->answerData);

        int crc = this->CRC16_2(dtmp, len - 2);
        if (dtmp[len - 1] != crc / 256 || dtmp[len - 2] != crc % 256)
        {
            errorCode = ESP_ERR_INVALID_CRC;
            //printf("erro no crc \n");
            // ESP_LOGE(TAG, "CRC Mismatch: %02x %02x %02x %02x %02x %02x %02x %02x %02x", dtmp[0], dtmp[1], dtmp[2], dtmp[3], dtmp[4], dtmp[5], dtmp[6], dtmp[7], dtmp[8]);
        }
        else if (dtmp[1] != functionCode)
        {
            errorCode = ESP_ERR_NOT_FOUND;
            //printf("erro na function code \n");
            //ESP_LOGE(TAG, "Function Code Error: %.02x, Exception: %d", dtmp[1], dtmp[2]);
        }
        else
        {
            for (int i = 0; i < dtmp[2]; i++)
            {
                (*data)[i] = dtmp[i + 3];
            }
        }
    }
    else
    {
        errorCode = ESP_ERR_TIMEOUT;
        //printf("erro timeout \n");
        // ESP_LOGE(TAG, "No response: %02x %02x %02x %02x %02x %02x %02x %02x ", dtmp[0], dtmp[1], dtmp[2], dtmp[3], dtmp[4], dtmp[5], dtmp[6], dtmp[7]);
    }
    uart_flush(UART_PORT);
    free(dtmp);
    return errorCode;
}

esp_err_t ModbusDriver::getConfirmation(uint8_t **data)
{
    uint8_t *dtmp = (uint8_t *)calloc(BUF_SIZE, 1);
    esp_err_t errorCode = ESP_OK;

    // int len = uart_read_bytes(UART_PORT, dtmp, BUF_SIZE, 100/portTICK_RATE_MS);
    int len = 0;
    uint64_t timeout = esp_timer_get_time();

    this->waiting = true;
    while (this->answerLen == 0 && (esp_timer_get_time() - timeout) <= 1000000)
        esp_task_wdt_reset();
    len = this->answerLen;
    this->answerLen = 0;
    this->waiting = false;

    if (len > 0)
    {
        memcpy(dtmp, this->answerData, len);
        free(this->answerData);
        for (int i = 0; i < len; i++)
        {
            if ((*data)[i] != dtmp[i])
            {
                errorCode = ESP_ERR_NOT_FOUND;
                //ESP_LOGE(TAG, "Confirmation failed, expected %d and received %d at position %d", (*data)[i], dtmp[i], i);
                break;
            }
        }
    }
    else
    {
        errorCode = ESP_ERR_TIMEOUT;
        //ESP_LOGE(TAG, "No response");
    }

    free(dtmp);
    return errorCode;
}

void ModbusDriver::setDevice(uint8_t deviceId)
{
    for (int i = 0; i < this->busSize; i++)
    {

        if (this->device[i].id == deviceId)
        {

            setSlaveAddress(this->device[i].id);
            // uart_set_baudrate(UART_PORT, this->device[i].baud); //
            if (this->device[i].baud != this->currentBaud)
            {

                uart_set_baudrate(UART_PORT, this->device[i].baud);
                this->currentBaud = this->device[i].baud;
                vTaskDelay(pdMS_TO_TICKS(20));
                printf("########################\nDriver:\nID:\t%d\nBaud:\t%d\n", this->device[i].id, this->device[i].baud);
            }
            break;
        }
    }
}

esp_err_t ModbusDriver::sendModBus(request_t *request, uint8_t *data)
{
    esp_err_t errorCode = ESP_OK;

    uint16_t full_size = (9 + (2 * request->reg_size));
    unsigned char *buf = (unsigned char *)calloc(full_size, 1);
    printf("------------------------------%d\n", full_size);
    buf[0] = this->getSlaveAddress();
    buf[1] = request->function_code;
    buf[2] = request->reg_start / 256;
    buf[3] = request->reg_start % 256;
    buf[4] = request->reg_size / 256;
    buf[5] = request->reg_size % 256;
    buf[6] = 2 * request->reg_size;
    for (uint8_t i = 0; i < buf[6]; i++)
    {
        buf[7 + i] = data[i];
    }
    uint16_t crc = this->CRC16_2(buf, full_size - 2);
    buf[full_size - 2] = crc % 256;
    buf[full_size - 1] = crc / 256;
    printf("%X:%X:%X:%X:%X:%X:%x:%x:%x:%x:%x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], crc % 256, crc / 256);
    uart_write_bytes(UART_PORT, (const char *)buf, full_size);
    if (request->function_code >= writeSingleCoil)
    {
        errorCode = this->getConfirmation(&buf);
    }
    free(buf);
    return errorCode;
}

esp_err_t ModbusDriver::sendRaw(uint8_t id, uint16_t address, uint amount, uint functionCode)
{

    unsigned char *buf = (unsigned char *)calloc(8, 1);
    esp_err_t errorCode = ESP_OK;

    buf[0] = id;
    buf[1] = functionCode;
    buf[2] = address / 256;
    buf[3] = address % 256;
    buf[4] = amount / 256;
    buf[5] = amount % 256;
    int crc = this->CRC16_2(buf, 6);
    buf[6] = crc % 256;
    buf[7] = crc / 256;
    //ESP_LOGI(TAG, "\n |DevAdress|func|RegAdress1|ReAdress2|Data1|Data2|CRC1|CRC2|\n|%d|%d|%d|%d|%d|%d|%d|%d|\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    uart_write_bytes(UART_PORT, (const char *)buf, 8);
    // ESP_LOGI(TAG, "Register %d written, amount: %d", address, amount);

    if (functionCode >= writeSingleCoil)
    {
        errorCode = this->getConfirmation(&buf);
    }
    free(buf);
    return errorCode;
}

esp_err_t ModbusDriver::readReg(uint32_t baud, uint8_t id, uint16_t address, uint amount, uint functionCode, uint8_t **data)
{
    uart_set_baudrate(UART_PORT, baud);
    this->currentBaud = baud;
    this->sendRaw(id, address, amount, functionCode);
    return (this->getData(functionCode, data));
}