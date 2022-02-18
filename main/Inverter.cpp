#include "Inverter.h"

Inverter::Inverter() : modbus()
{
    strcpy(this->deviceName, "Fronius");

    this->errors = 0;
}

void Inverter::setBus(uint8_t busSize, modbusDevice_t *busDevices)
{
    this->busSize = busSize;
    this->nodeData = (cJSON **)calloc(busSize, sizeof(cJSON *));
    this->devices = (modbusDevice_t *)malloc(busSize * sizeof(modbusDevice_t));
    deviceConfig_t *driverNodes = (deviceConfig_t *)malloc(busSize * sizeof(deviceConfig_t));
    for (int i = 0; i < busSize; i++)
    {
        this->nodeData[i] = cJSON_CreateObjectReference(this->nodeData[i]);
        driverNodes[i].baud = busDevices[i].baud;
        driverNodes[i].id = busDevices[i].slaveID;

        this->devices[i].baud = busDevices[i].baud;
        this->devices[i].slaveID = busDevices[i].slaveID;
        this->devices[i].standard = busDevices[i].standard;
        sprintf(this->devices[i].serialNo, "%s", busDevices[i].serialNo);
        switch (busDevices[i].standard)
        {
        case SunspecFronius:
            this->maxErrors += froniusTotalRegAmount;
            break;
        case Sungrow:
            this->maxErrors += sungrowRegAmount;
            break;
        case Huawei:
            this->maxErrors += huaweiRegAmount;
            break;

        case SunspecEcosolys:
            this->maxErrors += ecosolysTotalRegAmount;
            break;

        case Canadian:
            this->maxErrors += canadianRegAmount;
            break;
        case SAJ:
            this->maxErrors += sajRegAmount;
            break;
        case Refusol:
            this->maxErrors += refusolRegAmount;
            break;
        case sofar:
            this->maxErrors += sofarRegAmount;
            break;
        case CanadianSolar:
            this->maxErrors += canadianSolarRegAmount;
            break;
            // TODO uncomment
            //  case SunspecRefusol:
            //      this->maxErrors += sunspecRefusolRegAmount;
            //      break;

        default:
            break;
        }
    }
    modbus.setBus(busSize, driverNodes);
    free(driverNodes);
    this->busSet = 1;
}

void Inverter::read(uint8_t device, uint8_t standard, uint8_t model)
{
    unsigned char *msg = (unsigned char *)calloc(35, 1);
    int8_t firstLoop = 1;

    if (this->data == NULL)
    {
        //ESP_LOGE(INVTAG, "Failed to create JSON object");
    }
    deviceModel_t readSchema = this->findStandard(standard, model);
    cJSON *readJSON = this->nodeData[device];
    registerDefinition *registerTable = readSchema.table;
    uint8_t tableSize = readSchema.size;
    uint8_t readMode = readSchema.mode;

    if (readJSON == NULL)
    {
        //ESP_LOGE(INVTAG, "Failed to create readJSON");
    }

    cJSON *check = cJSON_GetObjectItemCaseSensitive(this->data, this->deviceName);
    if (check == NULL)
        firstLoop = 1;
    else
    {
        if (cJSON_GetObjectItemCaseSensitive(check, registerTable[0].label) == NULL)
            firstLoop = 0;
        else
            firstLoop = -1;
    }

    if (firstLoop >= 0)
    {
        for (int i = 0; i < tableSize; i++)
        {
            cJSON_AddStringToObject(readJSON, registerTable[i].label, "");
        }
        if (firstLoop == 1)
            cJSON_AddItemToObject(this->data, this->deviceName, readJSON);
    }

    bool first = true;
    for (int i = 0; i < tableSize; i++)
    {
        esp_task_wdt_reset();
        //printf("\rReading %.2f %c", 100 * ((float)i / tableSize), 37);

        if (this->readModbus(readMode, registerTable[i].address, registerTable[i].size, &msg) == ESP_OK)
        {
            first = true;
            double response = 0;
            char buf[15];
            if (registerTable[i].type == U16)
            {
                response = this->handleU16(&msg, registerTable[i].endianness) * registerTable[i].scaleFactor;
                sprintf(buf, "%g", response);
                cJSON_ReplaceItemInObjectCaseSensitive(readJSON, registerTable[i].label, cJSON_CreateString(buf));
            }
            else if (registerTable[i].type == S16)
            {
                response = this->handleS16(&msg, registerTable[i].endianness) * registerTable[i].scaleFactor;
                sprintf(buf, "%g", response);
                cJSON_ReplaceItemInObjectCaseSensitive(readJSON, registerTable[i].label, cJSON_CreateString(buf));
            }
            else if (registerTable[i].type == U32)
            {
                response = this->handleU32(&msg, registerTable[i].endianness) * registerTable[i].scaleFactor;
                sprintf(buf, "%g", response);
                cJSON_ReplaceItemInObjectCaseSensitive(readJSON, registerTable[i].label, cJSON_CreateString(buf));
            }
            else if (registerTable[i].type == S32)
            {
                response = this->handleS32(&msg, registerTable[i].endianness) * registerTable[i].scaleFactor;
                sprintf(buf, "%g", response);
                cJSON_ReplaceItemInObjectCaseSensitive(readJSON, registerTable[i].label, cJSON_CreateString(buf));
            }
            else if (registerTable[i].type == F32)
            {
                response = this->handleF32(&msg) * registerTable[i].scaleFactor;
                sprintf(buf, "%g", response);
                cJSON_ReplaceItemInObjectCaseSensitive(readJSON, registerTable[i].label, cJSON_CreateString(buf));
            }
            else if (registerTable[i].type == U64)
            {
                response = this->handleU64(&msg, registerTable[i].endianness) * registerTable[i].scaleFactor;
                sprintf(buf, "%g", response);
                cJSON_ReplaceItemInObjectCaseSensitive(readJSON, registerTable[i].label, cJSON_CreateString(buf));
            }
            else if (registerTable[i].type == str)
            {
                if (standard == SunspecEcosolys || standard == Refusol)
                {
                    if (strcmp(registerTable[i].label, "sn") == 0)
                    {
                        cJSON_ReplaceItemInObjectCaseSensitive(readJSON, "sn", cJSON_CreateString(this->devices[device].serialNo));
                    }
                    else
                    {
                        char finalMsg[35];
                        memset(finalMsg, 0, 35);
                        finalMsg[0] = msg[0];
                        finalMsg[1] = msg[1];
                        for (int j = 1; j < registerTable[i].size; j++)
                        {
                            if (this->readModbus(readMode, (registerTable[i].address) + j, 1, &msg) == ESP_OK)
                            {
                                finalMsg[j * 2] = msg[0];
                                finalMsg[j * 2 + 1] = msg[1];
                            }
                        }
                        cJSON_ReplaceItemInObjectCaseSensitive(readJSON, registerTable[i].label, cJSON_CreateString(finalMsg));
                    }
                }
                else if (standard == Canadian)
                {
                    char finalMsg[35];
                    for (int j = 0; j < 8; j++)
                    {
                        msg[j] = ((msg[j] & 0xF0) >> 4) | ((msg[j] & 0x0F) << 4);
                    }
                    sprintf(finalMsg, "%x%x%x%x%x%x%x%x", msg[1], msg[0], msg[3], msg[2], msg[5], msg[4], msg[7], msg[6]);
                    cJSON_ReplaceItemInObjectCaseSensitive(readJSON, registerTable[i].label, cJSON_CreateString(finalMsg));
                }
                else
                {
                    cJSON_ReplaceItemInObjectCaseSensitive(readJSON, registerTable[i].label, cJSON_CreateString((char *)msg));
                }
            }
            else if (registerTable[i].type == sunssf)
            {
                response = this->handleSunssf(&msg);
                char buf[15];
                sprintf(buf, "%g", response);
                cJSON_ReplaceItemInObjectCaseSensitive(readJSON, registerTable[i].label, cJSON_CreateString(buf));
            }

            if (device == 0 && (strcmp(registerTable[i].label, "end") == 0))
            {
                this->lastEnd = response;
            }
        }
        else
        {
            //printf("###############################\n erro\n ###############################\n ");
            if (first)
            {
                i--;
                first = false;
            }
            else
            {
                this->errors++;
                first = true;
            }
        }
    }
    printf("\n");
    free(msg);
    cJSON_ReplaceItemInObjectCaseSensitive(this->data, deviceName, readJSON);
}

esp_err_t Inverter::readModbus(uint8_t readMode, uint16_t address, uint amount, uint8_t **data)
{
    if (readMode == 3)
        return modbus.readHoldingRegs(address, amount, data);
    else if (readMode == 4)
        return modbus.readInputRegs(address, amount, data);
    return ESP_FAIL;
}

deviceModel_t Inverter::findStandard(uint8_t standard, uint8_t model)
{
    deviceModel_t result;

    switch (standard)
    {
    case SunspecFronius:
        result.mode = 3;
        switch (model)
        {
        case 0:
            result.table = froniusCommon;
            result.size = froniusCommonRegAmount;
            break;
        case 1:
            result.table = froniusInverter;
            result.size = froniusInverterRegAmount;
            break;
        case 2:
            result.table = froniusMultiMPPT;
            result.size = froniusMultiMPPTRegAmount;
            break;
        default:
            ESP_LOGE(INVTAG, "Invalid Model");
            break;
        }
        break;
    case Huawei:
        result.table = huaweiRegisters;
        result.size = huaweiRegAmount;
        result.mode = 3;
        break;
    case Sungrow:
        result.table = sungrowRegisters;
        result.size = sungrowRegAmount;
        result.mode = 4;
        break;
    case sofar:
        result.table = sofarRegisters;
        result.size = sofarRegAmount;
        result.mode = 3;
        break;
    case Refusol:
        result.table = refusolRegisters;
        result.size = refusolRegAmount;
        result.mode = 3;
        break;
        // TODO uncomment check mode
        //  case SunspecRefusol:
        //      result.table = sunspecRefusolRegisters;
        //      result.size = sunspecRefusolRegAmount;
        //      result.mode = 3;
        //      break;

    case SunspecEcosolys:
        result.mode = 4;
        switch (model)
        {
        case 0:
            result.table = ecosolysCommon;
            result.size = ecosolysCommonRegAmount;
            break;
        case 1:
            result.table = ecosolysInverter;
            result.size = ecosolysInverterRegAmount;
            break;
        case 2:
            result.table = ecosolysMultiMPPT;
            result.size = ecosolysMultiMPPTRegAmount;
            break;
        default:
            ESP_LOGE(INVTAG, "Invalid Model");
            break;
        }
        break;

    case Canadian:
        result.table = canadianRegisters;
        result.size = canadianRegAmount;
        result.mode = 4;
        break;
    case SAJ:
        result.table = sajRegisters;
        result.size = sajRegAmount;
        result.mode = 3;
        break;

    case CanadianSolar:
        result.table = canadianSolarRegisters;
        result.size = canadianSolarRegAmount;
        result.mode = 4;
        break;

    default:
        ESP_LOGE(INVTAG, "Invalid Standard");
        break;
    }
    return result;
}

uint16_t Inverter::handleU16(unsigned char **data, uint8_t endianess)
{
    if ((*data)[0] == 0xFF && (*data)[1] == 0xFF)
        return 0;
    if (endianess == bigEndian)
    {
        return (uint16_t)(((*data)[0] << 8) + (*data)[1]);
    }
    else if (endianess == littleEndian)
    {
        return (uint16_t)(((*data)[1] << 8) + (*data)[0]);
    }
    return 0;
}

int16_t Inverter::handleS16(unsigned char **data, uint8_t endianess)
{
    if ((*data)[0] == 0xFF && (*data)[1] == 0xFF)
        return 0;
    if (endianess == bigEndian)
    {
        return (int16_t)(((*data)[0] << 8) + (*data)[1]);
    }
    else if (endianess == littleEndian)
    {
        return (int16_t)(((*data)[1] << 8) + (*data)[0]);
    }
    return 0;
}

uint32_t Inverter::handleU32(unsigned char **data, uint8_t endianess)
{
    if ((*data)[0] == 0xFF && (*data)[1] == 0xFF && (*data)[2] == 0xFF && (*data)[3] == 0xFF)
        return 0;
    if (endianess == bigEndian)
    {
        return (uint32_t)(((*data)[0] << 24) + ((*data)[1] << 16) + ((*data)[2] << 8) + (*data)[3]);
    }
    else if (endianess == littleEndian)
    {
        return (uint32_t)(((*data)[3] << 24) + ((*data)[2] << 16) + ((*data)[1] << 8) + (*data)[0]);
    }
    return 0;
}

uint64_t Inverter::handleU64(unsigned char **data, uint8_t endianess)
{
    if ((*data)[0] == 0xFF && (*data)[1] == 0xFF && (*data)[2] == 0xFF && (*data)[3] == 0xFF && (*data)[4] == 0xFF && (*data)[5] == 0xFF && (*data)[6] == 0xFF && (*data)[7] == 0xFF)
        return 0;
    if (endianess == bigEndian)
    {
        return (uint64_t)(((*data)[0] << 56) + ((*data)[1] << 48) + ((*data)[2] << 40) + ((*data)[3] << 32) + ((*data)[4] << 24) + ((*data)[5] << 16) + ((*data)[6] << 8) + ((*data)[7]));
    }
    else if (endianess == littleEndian)
    {
        return (uint64_t)(((*data)[7] << 56) + ((*data)[6] << 48) + ((*data)[5] << 40) + ((*data)[4] << 32) + ((*data)[3] << 24) + ((*data)[2] << 16) + ((*data)[1] << 8) + ((*data)[0]));
    }
    return 0;
}

int32_t Inverter::handleS32(unsigned char **data, uint8_t endianess)
{
    if ((*data)[0] == 0xFF && (*data)[1] == 0xFF && (*data)[2] == 0xFF && (*data)[3] == 0xFF)
        return 0;
    if (endianess == bigEndian)
    {
        return (int32_t)(((*data)[0] << 24) + ((*data)[1] << 16) + ((*data)[2] << 8) + (*data)[3]);
    }
    else if (endianess == littleEndian)
    {
        return (int32_t)(((*data)[3] << 24) + ((*data)[2] << 16) + ((*data)[1] << 8) + (*data)[0]);
    }
    return 0;
}

float Inverter::handleF32(unsigned char **data)
{
    myfloat num;
    uint32_t numToConvert = (((*data)[0] << 24) + ((*data)[1] << 16) + ((*data)[2] << 8) + (*data)[3]);

    if (numToConvert == 0xFFFFFFFF)
        return 0;

    num.raw.mantissa = numToConvert & 0x7FFFFF;
    num.raw.exponent = (numToConvert >> 23) & 0xFF;
    num.raw.sign = (numToConvert >> 31) & 0x01;

    return num.f;
}
float Inverter::handleSunssf(unsigned char **data)
{
    int16_t sf = (uint16_t)(((*data)[0] << 8) + (*data)[1]) - 65536;
    return pow(10, sf);
}

esp_err_t Inverter::outputControlSunspecFronius(uint8_t value)
{
    if (value == 0)
    {
        while (modbus.writeSingleReg(40246, 1) != ESP_OK)
            ;
        return modbus.writeSingleReg(40241, value);
    }
    else
    {
        while (modbus.writeSingleReg(40241, value) != ESP_OK)
            ;
        return modbus.writeSingleReg(40246, 0);
    }
}
esp_err_t Inverter::outputControlSunspecEcosolys(uint8_t value)
{
    if (value == 0)
    {
        while (modbus.writeSingleReg(40132, 1) != ESP_OK)
            ;
        return modbus.writeSingleReg(40127, value);
    }
    else
    {
        while (modbus.writeSingleReg(40127, value) != ESP_OK)
            ;
        return modbus.writeSingleReg(40132, 0);
    }
}
esp_err_t Inverter::outputControlHuawei(uint8_t value)
{
    esp_err_t result = ESP_OK;
    if (value == 0)
    {
        result = modbus.writeSingleReg(40201, 0);
    }
    else
    {
        result = modbus.writeSingleReg(40200, 1);
    }
    return result;
}
uint8_t trySaj = 0;
uint8_t lockSaj = 0;
esp_err_t Inverter::outputControlSAJ(uint8_t value)
{
    esp_err_t result = ESP_FAIL;
    // printf("\n**************\nooooxxxxeeeoooxxee:%d \n*****************\n", value);
    if (value == 0)
    {
        trySaj = 0;
        if (this->readState(3, 256) == 2)
        {
            printf("\n**************\nComando de desligar enviado\n***************\n");
            // if (this->modbus.writeSpecialReg(4151, 0, 3) == 1)
            // {
            // printf("\n**************\nfoi - %d\n*****************", this->modbus.writeSpecialReg(4151, 0, 3));
            request_t req = {
                .slave_addr = 1,
                .function_code = 16,
                .reg_start = 4151,
                .reg_size = 1,
            };
            uint8_t data[] = {0x00, 0x00};
            result = modbus.sendModBus(&req, data);
            // }
        }
    }
    else
    {
        uint8_t state = this->readState(3, 256);
        trySaj++;
        printf("\n**************\nCount: %d\n***************\n", trySaj);
        if (trySaj > 250 && lockSaj == 1)
        {

            lockSaj = 0;
            trySaj = 0;
        }
        if (state == 2)
        {
            trySaj = 0;
        }
        if (state != -1 && state != 2 && lockSaj == 0)
        {
            lockSaj = 1;
            // if (this->modbus.writeSpecialReg(4151, 0, 3) == 0)
            // {
            printf("\n**************\nComando de ligar enviado\n*****************\n");
            request_t req = {
                .slave_addr = 1,
                .function_code = 16,
                .reg_start = 4151,
                .reg_size = 1,
            };
            uint8_t data[] = {0x00, 0x01};
            result = modbus.sendModBus(&req, data);
            // vTaskDelay(pdMS_TO_TICKS(1000));
            // }
        }
    }

    return result;
}

esp_err_t Inverter::outputControlRefusol(uint8_t value)
{
    uint16_t response = 0;
    esp_err_t result = ESP_OK;
    if (value == 0)
    {
        result = this->modbus.writeSpecialReg(0x0142, 0x66, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        response = this->readState(4, 4160);
        ESP_LOGI(INVTAG, "Response Refusol: %d", response);

        if ((response & 0x0002) != 2)
        {

            result = this->modbus.sendEnableRefusol();
            if (result == ESP_OK)
            {
                vTaskDelay(pdMS_TO_TICKS(500));
                ESP_LOGI(INVTAG, "Remote flag write");
                result = this->modbus.writeSpecialReg(0x0142, 0x66, 1);
            }
            else
            {
                ESP_LOGI(INVTAG, "ERROOO - Remote flag write");
            }
        }
    }
    else
    {
        result = this->modbus.writeSpecialReg(322, 0x55, 1);
    }
    return result;
}

esp_err_t Inverter::outputControlSofar(uint8_t value)
{
    uint16_t response = 0;
    esp_err_t result = ESP_OK;
    if (value == 0)
    {
        result = this->modbus.writeSpecialReg(0x0142, 0x66, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        response = this->readState(4, 4160);
        ESP_LOGI(INVTAG, "Response Sofar: %d", response);

        if ((response & 0x0002) != 2)
        {

            result = this->modbus.sendEnableRefusol();
            if (result == ESP_OK)
            {
                vTaskDelay(pdMS_TO_TICKS(500));
                ESP_LOGI(INVTAG, "Remote flag write");
                result = this->modbus.writeSpecialReg(0x0142, 0x66, 1);
            }
            else
            {
                ESP_LOGI(INVTAG, "ERROOO - Remote flag write");
            }
        }
    }
    else
    {
        result = this->modbus.writeSpecialReg(322, 0x55, 1);
    }
    return result;
}

// TODO: check registers adress
esp_err_t Inverter::outputControlSunspecRefusol(uint8_t value)
{
    if (value == 0)
    {
        while (modbus.writeSingleReg(40132, 1) != ESP_OK)
            ;
        return modbus.writeSingleReg(40127, value);
    }
    else
    {
        while (modbus.writeSingleReg(40127, value) != ESP_OK)
            ;
        return modbus.writeSingleReg(40132, 0);
    }
}
esp_err_t Inverter::outputControlSungrow(uint8_t value)
{
    esp_err_t result = ESP_OK;
    if (value == 0)
    {
        result = modbus.writeSingleReg(5005, 0xCE);
    }
    else
    {
        result = modbus.writeSingleReg(5005, 0xCF);
    }
    return result;
}
esp_err_t Inverter::outputControlCanadian(uint8_t value)
{
    esp_err_t result = ESP_OK;
    if (value == 0)
    {
        result = modbus.writeSingleReg(3006, 0xDE);
    }
    else
    {
        result = modbus.writeSingleReg(3006, 0xBE);
    }
    return result;
}

esp_err_t Inverter::outputControlCanadianSolar(uint8_t value)
{
    esp_err_t result = ESP_OK;
    if (value == 0)
    {
        result = this->modbus.writeSpecialReg(1000, 0xFF00, 5);
    }
    else
    {
        result = this->modbus.writeSpecialReg(1000, 0x0000, 5);
    }
    return result;
}

esp_err_t Inverter::outputControl(uint8_t value, uint8_t standard)
{
    switch (standard)
    {
    case Huawei:
        return this->outputControlHuawei(value);
        break;
    case Sungrow:
        return this->outputControlSungrow(value);
        break;
    case SunspecFronius:
        return this->outputControlSunspecFronius(value);
        break;
    case SunspecEcosolys:
        return this->outputControlSunspecEcosolys(value);
        break;
    case Canadian:
        return this->outputControlCanadian(value);
        break;
    case Refusol:
        return this->outputControlRefusol(value);
        break;
    case SAJ:
        return this->outputControlSAJ(value);
        break;
    case CanadianSolar:
        return this->outputControlCanadianSolar(value);
        break;
    case sofar:
        return this->outputControlSofar(value);
        break;
    case SunspecRefusol:
        return this->outputControlSunspecRefusol(value);
        break;
    default:
        return ESP_ERR_INVALID_ARG;
        break;
    }
}

char *Inverter::getJSON()
{
    return cJSON_Print(this->data);
}

char *Inverter::getJSONUnformatted()
{
    return cJSON_PrintUnformatted(this->data);
}

uint16_t Inverter::getErrorAmount()
{
    return this->errors;
}

void Inverter::zeroErrorAmount()
{
    this->errors = 0;
}

uint16_t Inverter::getMaxErrorAmount()
{
    return this->maxErrors;
}

void Inverter::readPolling()
{
    cJSON *obj = cJSON_CreateObject();
    cJSON *sub = NULL;
    cJSON *item = NULL;
    cJSON *sn = NULL;
    uint8_t aux = 0;
    char buf[5];
    char *strBuff = NULL;
    bool erroFlag = false;
    esp_task_wdt_reset();
    for (int i = 0; i < this->busSize; i++)
    {
        modbus.setDevice(this->devices[i].slaveID);
        sprintf(this->deviceName, "%d", this->devices[i].slaveID);
        this->read(i, this->devices[i].standard, 0);
        if (this->devices[i].standard == SunspecFronius)
        {
            this->read(i, this->devices[i].standard, 1);
            this->read(i, this->devices[i].standard, 2);
        }
        else if (this->devices[i].standard == SunspecEcosolys)
        {
            this->read(i, this->devices[i].standard, 1);
        }
        aux = this->errors - aux;
        item = cJSON_CreateNumber(aux);
        sn = cJSON_CreateString(this->devices[i].serialNo);
        sub = cJSON_CreateObject();
        cJSON_AddItemToObject(sub, "sn", sn);
        cJSON_AddItemToObject(sub, "error", item);
        sprintf(buf, "%d", this->devices[i].slaveID);
        cJSON_AddItemToObject(obj, buf, sub);
        if (aux >= 0)
        {
            erroFlag = true;
        }
    }
    strBuff = cJSON_PrintUnformatted(obj);
    ESP_LOGE(INVTAG, "%s", strBuff);
    if (erroFlag)
    {
        strcpy(this->softError, strBuff);
    }
    free(strBuff);
}

uint8_t Inverter::handleState(uint8_t output)
{
    int8_t *states = (int8_t *)calloc(this->busSize, sizeof(int8_t));
    this->monitorStatePolling(&states);
    uint8_t faultCount = 0, commFailCount = 0, successCount = 0, switchOutputCount = 0;
    for (int i = 0; i < this->busSize; i++)
    {
        esp_task_wdt_reset();
        switch (states[i])
        {
        case -2:
            commFailCount++;
            break;

        case -1:
            faultCount++;
            break;

        default:
            // this->outputControl(output, this->devices[i].standard);
            if (states[i] != output)
            {
                modbus.setDevice(this->devices[i].slaveID);
                this->outputControl(output, this->devices[i].standard);
                switchOutputCount++;
            }
            else
            {
                successCount++;
            }
            break;
        }
    }
    uint8_t result = 0;
    if (faultCount > 0)
        result += 1;
    if (commFailCount > 0)
        result += 2;
    if (switchOutputCount > 0)
        result += 4;
    if (successCount > 0)
        result += 8;

    free(states);
    return result;
}

void Inverter::monitorStatePolling(int8_t **result)
{

    for (int i = 0; i < this->busSize; i++)
    {
        int32_t state;
        int32_t error;

        esp_task_wdt_reset();
        modbus.setDevice(this->devices[i].slaveID);
        switch (this->devices[i].standard)
        {
        case Huawei:
            state = this->readState(3, 32089);
            if (state == -1)
            { // communication error
                (*result)[i] = -2;
            }
            else if (state == 0x0300)
            { // fault
                (*result)[i] = -1;
            }
            else if (state > 0x0300 && state < 0x0401)
            { // off
                (*result)[i] = 0;
            }
            else
            { // on
                (*result)[i] = 1;
            }
            break;

        case Sungrow:
            state = this->readState(4, 5037);
            if (state == -1)
            { // comm error
                (*result)[i] = -2;
            }
            else if (state == 0x5500 || state == 0x9100)
            { // fault
                (*result)[i] = -1;
            }
            else if (state > 0 && state < 0x1600)
            { // off
                (*result)[i] = 0;
            }
            else
            { // on
                (*result)[i] = 1;
            }
            break;

        case SunspecFronius:
            state = this->readState(3, 40117);
            if (state == -1)
            { // comm error
                (*result)[i] = -2;
            }
            else if (state == 7)
            { // fault
                (*result)[i] = -1;
            }
            else if (state == 8)
            { // off
                (*result)[i] = 0;
            }
            else
            { // on
                (*result)[i] = 1;
            }
            break;

        case SunspecEcosolys:
            state = this->readState(4, 40109);
            if (state == -1)
            { // comm error
                (*result)[i] = -2;
            }
            else if (state == 7)
            { // fault
                (*result)[i] = -1;
            }
            else if (state == 8 || state == 1)
            { // off
                (*result)[i] = 0;
            }
            else
            { // on
                (*result)[i] = 1;
            }
            break;

        case Canadian:
            state = this->readState(4, 3071);
            error = this->readState(4, 3066) | this->readState(4, 3067) | this->readState(4, 3068) | this->readState(4, 3069) | this->readState(4, 3070);
            if (state == -1)
            { // comm error
                (*result)[i] = -2;
            }
            else if (error != 0)
            { // fault
                (*result)[i] = -1;
            }
            else if (state == 4 || state == 8)
            { // off
                (*result)[i] = 0;
            }
            else
            { // on
                (*result)[i] = 1;
            }
            break;

        case sofar:
            state = this->readState(3, 0);
            if (state == -1)
            { // comm error
                (*result)[i] = -2;
            }
            else if (state == 0x0003 || state == 0x0004)
            { // fault
                (*result)[i] = -1;
            }
            else if (state == 0)
            { // off
                (*result)[i] = 0;
            }
            else
            { // on
                (*result)[i] = 1;
            }
            break;

        case Refusol:
            state = this->readState(3, 0);
            if (state == -1)
            { // comm error
                (*result)[i] = -2;
            }
            else if (state == 0x0003 || state == 0x0004)
            { // fault
                (*result)[i] = -1;
            }
            else if (state == 0)
            { // off
                (*result)[i] = 0;
            }
            else
            { // on
                (*result)[i] = 1;
            }
            break;
        case SAJ: // TODO

            uint8_t *msg = (uint8_t *)malloc(2 * sizeof(uint8_t));

            state = this->readState(3, 256);
            printf("STATE>>>>%d<<<<<\n", state);
            // error = this->readState(2, 4151);
            // this->readModbus(4, 4151, 2, &msg);
            // printf("ON/OFF>>>>%d<<<<<\n", *msg);
            //  printf("ERROR>>>>%d<<<<<\n", this->readState(3, 310));
            if (state == 0 || state == -1)
            { // comm error
                (*result)[i] = -2;
            }
            else if (state == 3)
            { // fault
                (*result)[i] = -1;
            }
            else if (state == 1)
            { // off
                (*result)[i] = 0;
            }
            // else if (state == 2)
            // { // off
            //     trySaj = 0;
            // }
            else
            { // on
                (*result)[i] = 1;
            }
            free(msg);

            break;

            // case CanadianSolar:
            //     state = this->readState(4, 3090);
            //     if (state == -1)
            //     { //comm error
            //         (*result)[i] = -2;
            //     }
            //     else if (state == 2)
            //     { //fault
            //         (*result)[i] = -1;
            //     }
            //     else if (state == 0)
            //     { //off
            //         (*result)[i] = 0;
            //     }
            //     else
            //     { //on
            //         (*result)[i] = 1;
            //     }
            //     break;

            // default:
            //     break;
        }
    }
}

int32_t Inverter::readState(uint8_t readMode, uint16_t address)
{
    unsigned char *msg = (unsigned char *)calloc(35, 1);
    int status;
    int i = -1;
    while (i <= 0)
    {
        if (this->readModbus(readMode, address, 1, &msg) == ESP_OK)
        {
            status = (msg[0] << 8) + msg[1];
            i = 1;
        }
        else
        {
            i++;
            status = -1;
        }
    }
    free(msg);
    return status;
}

uint8_t Inverter::getBusSet()
{
    return this->busSet;
}

void Inverter::setBusSet(uint8_t value)
{
    this->busSet = value;
}

double Inverter::getLastEnd()
{
    return this->lastEnd;
}

void Inverter::readPromiscuous()
{
    unsigned char *msg = (unsigned char *)calloc(35, 1);
    modbus.setDevice(1);

    ESP_LOGI(INVTAG, "Reading Prostitute");
    // ESP_LOGW(INVTAG, "READING INPUT");
    // for (int i = 0; i < 255; i++)
    // {
    //     if (modbus.readInputRegs(i, 1, &msg) == ESP_OK)
    //     {
    //         ESP_LOGW(INVTAG, "PEGOOOOOOOOOOUUUUUUUUUUUUUUUUCARAIAIAFJDSBKVKJASDBV-------------------------- :: %d", i);
    //         printf("%02x %02x %02x %02x\n", msg[0], msg[1], msg[2], msg[3]);
    //     }
    //     else
    //     {
    //         this->errors++;
    //     }
    //     //vTaskDelay(1000/portTICK_RATE_MS);
    // }
    ESP_LOGW(INVTAG, "READING HOLDING");
    for (int i = 40004; i < 40400; i++)
    {
        // vTaskDelay(pdMS_TO_TICKS(1000));
        if (modbus.readReg(9600, 1, i, 1, 3, &msg) == ESP_OK)
        {
            ESP_LOGW(INVTAG, "PEGOOOOOOOOOOUUUUUUUUUUUUUUUUCARAIAIAFJDSBKVKJASDBV-------------------------- :: %d", i);
            printf("%02x %02x %02x %02x\n", msg[0], msg[1], msg[2], msg[3]);
        }
        else
        {
            ESP_LOGW(INVTAG, "erroooor %d", i);
            this->errors++;
        }
        // vTaskDelay(1000/portTICK_RATE_MS);
    }
    vTaskDelay(pdMS_TO_TICKS(10000));
    if (modbus.readReg(9600, 0x88, 14, 1, 3, &msg) == ESP_OK)
    {
        ESP_LOGW(INVTAG, "PEGOOOOOOOOOOUUUUUUUUUUUUUUUUCARAIAIAFJDSBKVKJASDBV-------------------------- :: %d", 100);
        printf("%02x %02x %02x %02x\n", msg[0], msg[1], msg[2], msg[3]);
    }
    if (modbus.readReg(9600, 1, 14, 1, 3, &msg) == ESP_OK)
    {
        ESP_LOGW(INVTAG, "PEGOOOOOOOOOOUUUUUUUUUUUUUUUUCARAIAIAFJDSBKVKJASDBV-------------------------- :: %d", 100);
        printf("%02x %02x %02x %02x\n", msg[0], msg[1], msg[2], msg[3]);
    }
    if (modbus.readReg(9600, 1, 14, 1, 3, &msg) == ESP_OK)
    {
        ESP_LOGW(INVTAG, "PEGOOOOOOOOOOUUUUUUUUUUUUUUUUCARAIAIAFJDSBKVKJASDBV-------------------------- :: %d", 100);
        printf("%02x %02x %02x %02x\n", msg[0], msg[1], msg[2], msg[3]);
    }
    modbus.writeSingleReg(3006, 0xBE);
    free(msg);
}

char *Inverter::handleStrSN(uint8_t standard, unsigned char **data, int address)
{
    if (standard == SunspecEcosolys || standard == Refusol || standard == sofar)
    {  
        int adr = address;
        uint8_t macAddr[6];
        esp_efuse_mac_get_default(macAddr);
        char macAddr_with_id[20];
        sprintf(macAddr_with_id, "" MACSTR "" "-" "%d", MAC2STR(macAddr), adr);
        char *ret = macAddr_with_id;
        return ret;
    }
    else if (standard == Canadian)
    {
        static char finalMsg[30];
        for (int j = 0; j < 8; j++)
        {
            (*data)[j] = (((*data)[j] & 0xF0) >> 4) | (((*data)[j] & 0x0F) << 4);
        }
        sprintf(finalMsg, "%x%x%x%x%x%x%x%x", (*data)[1], (*data)[0], (*data)[3], (*data)[2], (*data)[5], (*data)[4], (*data)[7], (*data)[6]);
        ESP_LOGI(INVTAG, "FINAL MSG --->>%s", finalMsg);
        return finalMsg;
        // return "SNCANADIAN";
    }
    else
    {
        return (char *)*data;
    }
}

registerDefinition *Inverter::getRegister(char *label, registerDefinition *table, uint8_t length)
{
    for (unsigned int i = 0; i < length; i++)
    {
        if (strcmp(table[i].label, label) == 0)
        {
            return &table[i];
        }
    }
    return NULL;
}
bool Inverter::validateFrequency(float frequency)
{
    const float max = 63;
    const float min = 57;
    if ((frequency < max) && (frequency > min))
    {
        return true;
    }
    else
    {
        return false;
    }
}
float Inverter::handleFloatData(unsigned char **data, uint8_t type, float scale, uint8_t endianness)
{

    switch (type)
    {
    case U16:
        return this->handleU16(data, endianness) * scale;
        break;
    case U32:
        return this->handleU32(data, endianness) * scale;
        break;
    case S16:
        return this->handleS16(data, endianness) * scale;
        break;
    case S32:
        return this->handleS32(data, endianness) * scale;
        break;
    case F32:
        return this->handleF32(data) * scale;
        break;
    case str:
        return this->handleU16(data, endianness) * scale;
        break;
    case sunssf:
        return this->handleSunssf(data) * scale;
        break;
    default:
        return 1.0;
        break;
    }
}
bool Inverter::validatePF(float pf)
{
    const float max = 1.0;
    const float min = 0.0;
    if ((pf <= max) && (pf >= min))
    {
        return true;
    }
    else
    {
        return false;
    }
}
void Inverter::getBus()
{
    this->busStatus = 1;
    this->setBusSet(0);
    unsigned char *msg = (unsigned char *)calloc(35, 1);
    char *mft[] = {
        "CanadianSolar",
        "Fronius",
        "Huawei",
        "Sungrow",
        "Ecosolys",
        "Canadian",
        "SAJ",
        "sofar",
        "Refusol",
    };

    uint8_t founded[] = {0, 0, 0, 0, 0, 0, 0};
    float response,regFactor;
    esp_err_t status;
    deviceModel_t readSchema;
    deviceModel_t readSchemaSN;
    registerDefinition *reg, *regsf;
    modbusDevice_t node;
    uint32_t baudRate = 9600;
    bool first = true;
    bool change = false;
    
    do
    {
        for (int i = CanadianSolar; i <= Refusol; i++)
        {
            baudRate = 9600;
            switch (i)
            {
            case SunspecFronius:
                readSchema = this->findStandard(SunspecFronius, 1);
                break;
            case Huawei:
                readSchema = this->findStandard(Huawei, 0);
                break;
            case Sungrow:
                readSchema = this->findStandard(Sungrow, 0);
                break;
            case SunspecEcosolys:
                readSchema = this->findStandard(SunspecEcosolys, 1);
                baudRate = 19200;
                break;
            case Canadian:
                readSchema = this->findStandard(Canadian, 0);
                break;
            case Refusol:
                readSchema = this->findStandard(Refusol, 0);
                break;
            case SAJ:
                readSchema = this->findStandard(SAJ, 0);
                break;
            case sofar:
                readSchema = this->findStandard(sofar, 0);
                break;
            case CanadianSolar:
                readSchema = this->findStandard(CanadianSolar, 0);
                break;
            }
            for (int adress = 1; adress <= 6; adress++)
            {
                if (founded[adress] == 1)
                    continue;
                ESP_LOGI(INVTAG, "Standard: %s - Adress:  %d \n", mft[i], adress);
                reg = this->getRegister("hz", readSchema.table, readSchema.size);
                ESP_LOGI(INVTAG, "Registerr:  %d \n", reg->address);
                regsf = this->getRegister("hzsf", readSchema.table, readSchema.size);
                if (reg != NULL)
                {
                    ESP_LOGI(INVTAG, "baud: %d - mode: %d - reg:  %d - size: %d - scale: %f\n", baudRate, readSchema.mode, reg->address, reg->size, reg->scaleFactor);
                    if (this->modbus.readReg(baudRate, adress, reg->address, reg->size, readSchema.mode, &msg) == ESP_OK)
                    {
                        response = handleFloatData(&msg, reg->type, reg->scaleFactor, reg->endianness);
                        if (regsf != NULL)
                        {
                            ESP_LOGI(INVTAG, "mode: %d - reg:  %d - size: %d - scale: %f\n", readSchema.mode, regsf->address, regsf->size, regsf->scaleFactor);
                            this->modbus.readReg(baudRate, adress, regsf->address, regsf->size, readSchema.mode, &msg);
                            regFactor = handleFloatData(&msg, regsf->type, regsf->scaleFactor, regsf->endianness);
                            response = response * regFactor;
                        }
                        ESP_LOGI(INVTAG, "freq: %f\n", response);
                        if (this->validateFrequency(response))
                        {
                            reg = NULL;
                            regsf = NULL;
                            reg = this->getRegister("pf", readSchema.table, readSchema.size);
                            regsf = this->getRegister("pfsf", readSchema.table, readSchema.size);
                            ESP_LOGI(INVTAG, "mode: %d - reg:  %d - size: %d - scale: %f\n", readSchema.mode, reg->address, reg->size, reg->scaleFactor);
                            if (this->modbus.readReg(baudRate, adress, reg->address, reg->size, readSchema.mode, &msg) == ESP_OK)
                            {
                                response = handleFloatData(&msg, reg->type, reg->scaleFactor, reg->endianness);
                                if (regsf != NULL)
                                {
                                    ESP_LOGI(INVTAG, "mode: %d - reg:  %d - size: %d - scale: %f\n", readSchema.mode, regsf->address, regsf->size, regsf->scaleFactor);
                                    this->modbus.readReg(baudRate, adress, regsf->address, regsf->size, readSchema.mode, &msg);
                                    regFactor = handleFloatData(&msg, regsf->type, regsf->scaleFactor, regsf->endianness);
                                    ESP_LOGI(INVTAG, "scale: %f\n", regFactor);
                                    response = response * regFactor;
                                }
                                ESP_LOGI(INVTAG, "FP: %f\n", response);
                                if (this->validatePF(response))
                                {
                                    readSchemaSN= this->findStandard(i, 0);
                                    reg = this->getRegister("sn", readSchemaSN.table, readSchemaSN.size);
                                    if (reg != NULL)
                                    {
                                        this->modbus.readReg(baudRate, adress, reg->address, reg->size, readSchemaSN.mode, &msg);
                                    }
                                    strcpy(node.manufacturer, mft[i]);
                                    strcpy(node.model, "Model");
                                    strcpy(node.serialNo, (const char *)this->handleStrSN(i, &msg, adress));
                                    node.baud = baudRate;
                                    node.slaveID = adress;
                                    node.standard = i;
                                    // if (!checkBusData(node))
                                    // {
                                    //     ESP_LOGI(INVTAG, "########### Mudança no barramento  ##########");
                                    //     change = true;
                                    // }
                                    change = true;
                                    if (first)
                                    {
                                        createBusFile(node);
                                        first = false;
                                    }
                                    else
                                        addBusData(node);

                                    founded[adress] = 1;
                                    ESP_LOGI(INVTAG, "###########  FINDED  ##########| Std-> %s SN->%s\n", mft[i], node.serialNo);
                                }
                            }
                        }
                    }
                    else
                    {
                        ESP_LOGI(INVTAG, "erro de leitura\n");
                        printf("%02x %02x %02x %02x\n", msg[0], msg[1], msg[2], msg[3]);
                    }
                }
                else
                {
                    ESP_LOGI(INVTAG, "não achou reg \n");
                }
            }
        }

    } while (first);
    if (change)
    {
        ESP_LOGI(INVTAG, "########### Primeiro siignup ##########");
        createSignupFile(1);
        nvs_str_save("SIGNUP", "1");
    }
    this->busStatus = 0;
}