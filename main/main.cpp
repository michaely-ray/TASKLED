#include "string.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "Inverter.h"
#include "paramDefinition.h"
#include "FileManager.h"
#include "internet.h"
#include "utils.h"

#define RED (gpio_num_t)14
#define GREEN (gpio_num_t)12
#define BLUE (gpio_num_t)27
#define sendDataPeriod_min 5
#define getDataPeriod_s 60
#define commFailAlertPeriod_min 20
#define sendDataPeriod_us (60 * 1000000 * sendDataPeriod_min)
#define getDataPeriod_us (1000000 * getDataPeriod_s)
#define commFailAlertPeriod_us (60 * 1000000 * commFailAlertPeriod_min)

static void acquisitionTask(void *arg);
static const char *MAINTAG = "main";

TaskHandle_t xAquisitionTaskHandle;
TaskHandle_t xLedTaskHandle;
Inverter inverter;
uint8_t output = 1;
uint8_t daytime = 1;
uint8_t commFlag = 1;
uint8_t connectedState = 0;
uint8_t internetStarted = 0;
// uint8_t credentialStatus = 0;

char city[50];
void setDaytime(char *msg)
{
    if (strcmp("day", msg) == 0)
    {
        daytime = 1;
    }
    else if (strcmp("night", msg) == 0)
    {
        daytime = 0;
    }
}

void busCheck()
{
    uint8_t signupFlag = 0;
    if (getSignup() == 1 && connectedState == 1)
    {
        signupFlag = 1;
    }
    else if (inverter.getBusSet() == 1)
    {
        return;
    }

    cJSON *bus = NULL;

    char *busJSON = (char *)calloc(512, 1);
    modbusDevice_t *devices;
    if (getBusData(&busJSON, 512) > 0)
    {
        printf("%s", busJSON);
        if (signupFlag == 1)
        {
            char *sendMsg = (char *)calloc(1000, 1);
            char info[500];
            getInternetInfo(info);
            sprintf(sendMsg, "{\"info\":{%s},\"bus\":%s}", info, busJSON);
            mqttPublish("/signup", sendMsg, 2);
            createSignupFile(0);
            free(sendMsg);
            if (inverter.getBusSet() == 1)
            {
                free(busJSON);
                cJSON_Delete(bus);
                return;
            }
        }
        bus = cJSON_Parse(busJSON);
        cJSON *amount = cJSON_GetObjectItemCaseSensitive(bus, "amt");
        if (amount == NULL)
        {
            ESP_LOGE(MAINTAG, "Failed to acquire JSON amt param");
        }
        uint8_t amt = amount->valueint;
        ESP_LOGI(MAINTAG, "amount: %d", amt);
        devices = (modbusDevice_t *)malloc(amt * sizeof(modbusDevice_t));
        cJSON *nodes = cJSON_GetObjectItemCaseSensitive(bus, "Bus");
        if (nodes == NULL)
        {
            ESP_LOGE(FMTAG, "Failed to acquire JSON Array");
        }
        cJSON *node = NULL;
        int indexNode = 0;
        cJSON_ArrayForEach(node, nodes)
        {
            cJSON *id = cJSON_GetObjectItemCaseSensitive(node, "id");
            cJSON *std = cJSON_GetObjectItemCaseSensitive(node, "std");
            cJSON *mfr = cJSON_GetObjectItemCaseSensitive(node, "mfr");
            cJSON *mdl = cJSON_GetObjectItemCaseSensitive(node, "mdl");
            cJSON *sn = cJSON_GetObjectItemCaseSensitive(node, "sn");
            cJSON *baud = cJSON_GetObjectItemCaseSensitive(node, "baud");

            devices[indexNode].slaveID = id->valueint;
            devices[indexNode].standard = std->valueint;
            devices[indexNode].baud = baud->valueint;
            sprintf(devices[indexNode].manufacturer, "%s", mfr->valuestring);
            sprintf(devices[indexNode].model, "%s", mdl->valuestring);
            sprintf(devices[indexNode].serialNo, "%s", sn->valuestring);

            indexNode++;
        }
        inverter.setBus(amt, devices);
        free(devices);
    }
    free(busJSON);
    cJSON_Delete(bus);
}

void send(uint8_t type)
{
    uint8_t getData = 0;
    static double prevEnd = 100000;
    char sendTopic[50];
    char info[500];
    getInternetInfo(info);
    uint8_t macAddr[6];
    esp_efuse_mac_get_default(macAddr);

    switch (type)
    {
    case 0:
        getData = 1;
        strcpy(sendTopic, "/data");
        break;

    case 1:
        getData = 1;
        strcpy(sendTopic, "/fault");
        break;

    default:
        strcpy(sendTopic, "/error");
        break;
    }

    if (getData == 1)
    {
        char *readData = NULL;
        readData = inverter.getJSONUnformatted();
        // printf("%s\n\nData Size: %d\n", readData, strlen(readData));
        char sendMsg[strlen(readData) + 150];
        sprintf(sendMsg, "{\"info\":{%s},\"data\":%s}", info, readData);
        free(readData);
        static int8_t hasOffline = getOffline();
        ESP_LOGW(MAINTAG, " HAS OFFLINE: %d", hasOffline);
        if (connectedState == 1)
        {
            if (daytime == 1)
            {
                if (hasOffline == 1)
                {
                    mqttPublish("/offline", sendMsg, 2);
                    hasOffline = 0;
                    handleOffline(0);
                }
                else
                {
                    if (type == 0)
                    {
                        mqttPublish("/data", sendMsg, 2);
                    }
                    else if (type == 1)
                    {
                        mqttPublish("/fault", sendMsg, 2);
                    }
                }
            }
        }
        else
        {
            hasOffline = 1;
            handleOffline(1);
        }
    }
    // else if (type == 2 && daytime == 1)
    else if (type == 2)
    {
        char msg[760];
        sprintf(msg, "{\"info\":{%s},\"error\":%s}", info, inverter.softError);
        mqttPublish("/error", msg, 2);
    }
    else if (type == 3 && daytime == 1)
    {
        char msg[600];
        sprintf(msg, "{\"info\":{%s},\"error\":\"Inverter output inconsistency, should be %d, instead is %d\"}", info, output, 1 - output);
        mqttPublish(sendTopic, msg, 2);
    }
}

uint8_t monitor(uint8_t alarm)
{
    uint8_t outputAlarm = 0;
    static uint8_t lastOutput = 1;
    uint8_t changeOutputFlag;

    changeOutputFlag = (lastOutput != output) ? 1 : 0;
    uint8_t inverterStatus = inverter.handleState(output);
    if (inverterStatus & 0x01)
    { // fault
        inverter.readPolling();
        send(1);
        commFlag = 1;
    }
    if (inverterStatus & 0x02)
    {
        commFlag = 0;
    }
    if (inverterStatus & 0x04)
    {
        if (alarm == 0 && changeOutputFlag == 0)
        {
            send(3);
            outputAlarm = 1;
        }
        commFlag = 1;
    }
    if (inverterStatus & 0x08)
    {
        commFlag = 1;
    }
    //ESP_LOGI(MAINTAG, "Monitor: %d\t-\tHeap: %d", inverterStatus, xPortGetFreeHeapSize());
    lastOutput = output;
    return outputAlarm;
}

static void mqttRxTask(void *arg)
{
    rx_event_t evt;
    char *pEnd;
    uint8_t macAddr[6];
    esp_efuse_mac_get_default(macAddr);
    char outputTopic[50];
    char ackTopic[50];
    char cityTopic[50];
    char otaTopic[50];
    char city[50];
    char daytimeTopic[60];
    char statusTopic[50];
    char signupTopic[50];
    char dataTopic[50];
    char dataRxTopic[50];
    char associatedTopic[50];
    char getBusTopic[50];
    char eraseTopic[50];
    char credentialTopic[50];
    char setBusTopic[50];

    sprintf(outputTopic, "/command/" MACSTR "", MAC2STR(macAddr));
    sprintf(ackTopic, "/response/" MACSTR "", MAC2STR(macAddr));
    sprintf(cityTopic, "/city/" MACSTR "", MAC2STR(macAddr));
    sprintf(otaTopic, "/ota/" MACSTR "", MAC2STR(macAddr));
    sprintf(statusTopic, "/status/" MACSTR "", MAC2STR(macAddr));
    sprintf(signupTopic, "/signup/" MACSTR "", MAC2STR(macAddr));
    sprintf(dataTopic, "/data/" MACSTR "", MAC2STR(macAddr));
    sprintf(dataRxTopic, "/response/" MACSTR "", MAC2STR(macAddr));
    sprintf(associatedTopic, "/associate/" MACSTR "", MAC2STR(macAddr));
    sprintf(getBusTopic, "/getbus/" MACSTR "", MAC2STR(macAddr));
    sprintf(eraseTopic, "/erase/" MACSTR "", MAC2STR(macAddr));
    sprintf(credentialTopic, "/credential/" MACSTR "", MAC2STR(macAddr));
    sprintf(setBusTopic, "/setbus/" MACSTR "", MAC2STR(macAddr));
    while (true)
    { // wait queue and send data
        xQueueReceive(rxQueue, &evt, portMAX_DELAY);
        ESP_LOGI(MAINTAG, "Topic: %s // Message: %s", evt.topic, evt.msg);

        if (getCity(city) == ESP_OK)
        {
            sprintf(daytimeTopic, "/daytime/%s", city);
        }

        if (strcmp(outputTopic, evt.topic) == 0)
        {
            if (strcmp("tell me", evt.msg) == 0)
            {
                char response[] = "desligado";
                if (output == 1)
                    sprintf(response, "ligado");

                mqttPublish(ackTopic, response, 2);
            }
            else if (strcmp("0", evt.msg) == 0 || strcmp("1", evt.msg) == 0)
            {
                vTaskSuspend(xAquisitionTaskHandle);
                esp_task_wdt_delete(xAquisitionTaskHandle);
                output = atoi(evt.msg);
                createOutputFile(output);
                mqttPublish(ackTopic, "recebido", 2);
                monitor(1);
                vTaskResume(xAquisitionTaskHandle);
                esp_task_wdt_add(xAquisitionTaskHandle);
            }
        }
        else if (strcmp(daytimeTopic, evt.topic) == 0)
        {
            if (strcmp("day", evt.msg) == 0 || strcmp("night", evt.msg) == 0)
                setDaytime(evt.msg);
            ESP_LOGI(MAINTAG, "Daytime: %s", evt.msg);
        }
        else if (strcmp(cityTopic, evt.topic) == 0)
        {
            setCity(evt.msg);
            char daytimeTopic[410];
            sprintf(daytimeTopic, "/daytime/%s", evt.msg);
            mqttSub(daytimeTopic);
        }
        else if ((strcmp(otaTopic, evt.topic) == 0) || (strcmp("/ota/blueraven", evt.topic) == 0))
        {
            if (strcmp("update", evt.msg) == 0)
                forceOta();
        }
        else if ((strcmp(statusTopic, evt.topic) == 0))
        {
            if (strcmp("status", evt.msg) == 0)
            {
                char info[500];
                char response[600];
                getInternetInfo(info);
                sprintf(response, "{%s,\"status\":\"%s\",\"com\":%d}", info, output == 1 ? "ligado" : "desligado", commFlag);
                mqttPublish(ackTopic, response, 2);
            }
        }
        else if (strcmp(signupTopic, evt.topic) == 0)
        {
            if (strcmp("signup", evt.msg) == 0)
            {
                char *busJSON = (char *)calloc(512, 1);
                if (getBusData(&busJSON, 512) > 0)
                {
                    char *sendMsg = (char *)calloc(1000, 1);
                    char info[500];
                    getInternetInfo(info);
                    sprintf(sendMsg, "{\"info\":{%s},\"bus\":%s}", info, busJSON);
                    mqttPublish("/signup", sendMsg, 2);
                    free(sendMsg);
                }
                else
                {
                    mqttPublish("/signup", "no configured inverter", 2);
                }
                free(busJSON);
            }
        }
        else if (strcmp(dataTopic, evt.topic) == 0)
        {

            if (strcmp("data", evt.msg) == 0)
            {
                char info[500];
                getInternetInfo(info);
                char *readData = NULL;
                readData = inverter.getJSONUnformatted();
                // printf("%s\n\nData Size: %d\n", readData, strlen(readData));
                char sendMsg[strlen(readData) + 150];
                sprintf(sendMsg, "{\"info\":{%s},\"data\":%s}", info, readData);
                mqttPublish(dataRxTopic, sendMsg, 2);
                free(readData);
            }
        }
        else if (strcmp(associatedTopic, evt.topic) == 0)
        {
            if (strcmp("1", evt.msg) == 0 || strcmp("0", evt.msg) == 0)
            {
                nvs_handle my_handle;
                esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (err != ESP_OK)
                {
                    ESP_LOGI(TAG, "Error (%d) opening NVS handle!\n", err);
                }
                else
                {
                    err = nvs_set_str(my_handle, "ASSOCIATE", evt.msg);
                    if (err != ESP_OK)
                    {
                        ESP_LOGI(TAG, "\nError in %s : (%04X)\n", "ASSOCIATE", err);
                    }
                    /* Salva em nvs */
                    err = nvs_commit(my_handle);
                    if (err != ESP_OK)
                    {
                        printf("\nError in commit! (%04X)\n", err);
                    }
                    nvs_close(my_handle);
                }
            }
        }
        else if (strcmp(getBusTopic, evt.topic) == 0)
        {
            if (strcmp("getbus", evt.msg) == 0)
            {
                vTaskDelete(xAquisitionTaskHandle);
                inverter.getBus();
                xTaskCreate(acquisitionTask, "acquisitionTask", 1024 * 7, NULL, 8, &xAquisitionTaskHandle);
                // vTaskResume(xAquisitionTaskHandle);
                if (getSignup() == 1)
                {
                    char *busJSON = (char *)calloc(512, 1);
                    if (getBusData(&busJSON, 512) > 0)
                    {
                        char *sendMsg = (char *)calloc(1000, 1);
                        char info[500];
                        getInternetInfo(info);
                        sprintf(sendMsg, "{\"info\":{%s},\"bus\":%s}", info, busJSON);
                        mqttPublish("/signup", sendMsg, 2);
                        createSignupFile(0);
                        free(sendMsg);
                    }
                    free(busJSON);
                }
            }
        }
        else if (strcmp(setBusTopic, evt.topic) == 0)
        {
            nvs_str_save("bus", evt.msg);
        }
        else if (strcmp(eraseTopic, evt.topic) == 0)
        {
            if (strcmp("erase", evt.msg) == 0)
            {
                nvs_flash_deinit();
                nvs_flash_erase_partition(NVS_DEFAULT_PART_NAME);
                nvs_flash_erase();
                vTaskDelay(pdMS_TO_TICKS(500));
                esp_restart();
                ESP_LOGI(TAG, "NVS ERASED");
            }
        }
        else if (strcmp(credentialTopic, evt.topic) == 0)
        {
            cJSON *cred = NULL;
            cJSON *ssid = NULL;
            cJSON *pwd = NULL;
            cred = cJSON_Parse(evt.msg);

            if (strlen(evt.msg) > 26)
            {
                ssid = cJSON_GetObjectItem(cred, "ssid");
                pwd = cJSON_GetObjectItem(cred, "pwd");
                nvs_str_save("SSID", ssid->valuestring);
                nvs_str_save("PASSWORD", pwd->valuestring);
                ESP_LOGI(TAG, "CREDENTIAL CHANGED SSID:%s - PWD:%s", ssid->valuestring, pwd->valuestring);

                vTaskDelay(pdMS_TO_TICKS(1000));
                cJSON_Delete(cred);
                esp_restart();
            }
            {

                ESP_LOGI(TAG, "FAILED TO PARSE JSON");
            }
        }
    }
}
static void acquisitionTask(void *arg)
{
    uint8_t commFail = 0;
    uint64_t commFailTimeout = 0;
    uint64_t sendDataTimeout = esp_timer_get_time() + sendDataPeriod_us;
    while (1)
    {
        busCheck();

        inverter.readPolling();

        int errorAmount = inverter.getErrorAmount();
        if (errorAmount <= 0.15 * inverter.getMaxErrorAmount())
        {
            commFail = 0;
            commFlag = 1;
            ESP_LOGI(MAINTAG, "Energia diária: %.1f", inverter.getLastEnd());
            if (esp_timer_get_time() >= sendDataTimeout)
            {
                ESP_LOGI(MAINTAG, "SEND");
                send(0);
                sendDataTimeout = esp_timer_get_time() + sendDataPeriod_us;
            }
        }
        else
        {
            if (errorAmount >= 0)
            {
                if (esp_timer_get_time() >= sendDataTimeout)
                {
                    ESP_LOGI(MAINTAG, "SEND ERROR");
                    send(0);
                    send(2);

                    sendDataTimeout = esp_timer_get_time() + sendDataPeriod_us;
                }
                // send(0);
                // send(2);
                // commFlag = 0;
                // if (commFail == 0)
                // {
                //     commFailTimeout = esp_timer_get_time() + commFailAlertPeriod_us;
                //     commFail = 1;
                // }
                // else if (esp_timer_get_time() > commFailTimeout)
                // {
                //     commFail = 0;
                //     send(0);
                //     send(2);
                // }
            }
            printf("\nCommunication error, found %d mistakes\n", errorAmount);
        }

        uint8_t monitorLoop = 0;
        uint64_t monitorTimeout = esp_timer_get_time() + getDataPeriod_us;
        while (esp_timer_get_time() < monitorTimeout)
        {
            esp_task_wdt_reset();
            vTaskDelay(pdMS_TO_TICKS(500));
            if (monitor(monitorLoop) == 1)
                monitorLoop = 1;
        }

        inverter.zeroErrorAmount();
        // printf("Free Heap: %d bytes\nFree Storage: %d bytes\nErrors: %d\nMax Error: %d\n", xPortGetFreeHeapSize(), getFreeStorage(), errorAmount, inverter.getMaxErrorAmount());
        printf("\n---------------------------------------------------------\n");
    }
}

static void ledIndicatorTask(void *arg)
{
    gpio_pad_select_gpio(RED);
    gpio_pad_select_gpio(GREEN);
    gpio_pad_select_gpio(BLUE);
    gpio_set_direction(RED, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(GREEN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(BLUE, GPIO_MODE_INPUT_OUTPUT);
    uint8_t i = 0;
    uint8_t tick = 5;

    while (1)
    {
        esp_task_wdt_reset();
        i++;
        connectedState = isConnected(10);
        if (credentialStatus == 1)
        {
            gpio_set_level(RED, 0);
            gpio_set_level(GREEN, i % 2);
            gpio_set_level(BLUE, 0);
            tick = 1;
        }
        else if (isOta() == 1)
        {
            gpio_set_level(RED, 0);
            gpio_set_level(GREEN, 0);
            gpio_set_level(BLUE, i % 2);
            tick = 1;
        }
        else if (inverter.busStatus == 1)
        {
            gpio_set_level(RED, 0);
            gpio_set_level(GREEN, i % 2);
            gpio_set_level(BLUE, i % 2);
            tick = 5;
        }
        else if (connectedState == 1 && commFlag == 1)
        {
            if (networkType == wifi)
            {
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 1);
                gpio_set_level(BLUE, 0);
            }
            else if (networkType == gprs)
            {
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 1);
            }
            tick = 5;
        }
        else if (connectedState == 0 && commFlag == 0)
        {
            gpio_set_level(RED, 1);
            gpio_set_level(GREEN, 0);
            gpio_set_level(BLUE, 0);
            tick = 5;
        }
        else if (connectedState == 1 && commFlag == 0)
        {
            gpio_set_level(RED, 1 - gpio_get_level(RED));
            gpio_set_level(GREEN, 0);
            gpio_set_level(BLUE, 0);
            tick = 5;
        }
        else if (connectedState == 0 && commFlag == 1)
        {
            gpio_set_level(RED, 0);
            gpio_set_level(GREEN, 1 - gpio_get_level(GREEN));
            gpio_set_level(BLUE, 0);
            tick = 5;
        }
        else
        {
            gpio_set_level(RED, 1);
        }

        vTaskDelay(pdMS_TO_TICKS(tick * 100));
    }
}

extern "C" void app_main()
{
    gpio_pad_select_gpio(RED);
    gpio_pad_select_gpio(GREEN);
    gpio_pad_select_gpio(BLUE);
    gpio_set_direction(RED, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(GREEN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(BLUE, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(GREEN, 1);
    gpio_set_level(RED, 0);
    gpio_set_level(BLUE, 0);
    char associated[5];
    uint8_t macAddr[6];
    esp_efuse_mac_get_default(macAddr);
    sprintf(macGlobal, "" MACSTR "", MAC2STR(macAddr));
    mount();

    int8_t storedOutput = getOutput();
    if (storedOutput != -1 && storedOutput != 255)
        output = storedOutput;

    esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(0));
    esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(1));

    // wifiManagerInit();
    mqttSetup();

    // initSmartConfig(&mqttConnect);
    //xTaskCreate(ledIndicatorTask, "ledIndicatorTask", 2048, NULL, 5, &xLedTaskHandle);

    internetInit();
    if (nvs_str_read(associated, sizeof(associated), "ASSOCIATE") == ESP_OK)
    {
        if (strcmp(associated, "0") == 0)
        {
            //inverter.getBus();
        }
    }
    else
    {
        //inverter.getBus();
    }

    // inverter.readPromiscuous();
    xTaskCreate(mqttRxTask, "mqttRxTask", 7168, NULL, 6, NULL);
    xTaskCreate(acquisitionTask, "acquisitionTask", 1024 * 7, NULL, 8, &xAquisitionTaskHandle);
    internetStarted = 1;
    // esp_task_wdt_add(xAquisitionTaskHandle);
    // esp_task_wdt_add(xLedTaskHandle);
}
