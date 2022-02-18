#include "internet.h"

void handleWifiStart(void);
void handleRouterConnected(void);
void handleRouterDisconnected(void);
void handlerIntenetConnected(void);
void handlerFirstConnection(void);
void handlerIntenetDisconnected(void);
void handlerCrendentialCB(void);
handle_args_t scHandlerArgs = {
    .wifiStart = &handleWifiStart,
    .routerGotIp = &handleRouterConnected,
    .routerDisconnected = &handleRouterDisconnected,
    .firstHandler = &handlerFirstConnection,
    .internetConnected = &handlerIntenetConnected,
    .internetDisconnected = &handlerIntenetDisconnected,
    .credentialCB = &handlerCrendentialCB,
};
uint8_t networkType = gprs;
uint8_t credentialStatus = 0;
void handlerCrendentialCB(void)
{
    credentialStatus = 1;
    vTaskDelay(pdMS_TO_TICKS(2000));
    credentialStatus = 0;
}
void handleWifiStart(void)
{
    ESP_LOGI(INTERNET_TAG, "START WIFI");
}
void handleRouterConnected(void)
{
    ESP_LOGI(INTERNET_TAG, "ROUTER CONNECTED");
}
void handleRouterDisconnected(void)
{
    ESP_LOGI(INTERNET_TAG, "ROUTER DISCONNECTED");
    // mqttStop();
    // ppposStart();
    // ESP_LOGI(INTERNET_TAG, "START TASKS");
    // vTaskDelay(pdMS_TO_TICKS(120000));
    // initSmartConfig((void *)&scHandlerArgs);
    // ESP_LOGI(INTERNET_TAG, "CONNECTING  MQTT GPRS");
    // mqttStart();

    if (!ppposConnectFlag)
    {
        ppposStart();
        vTaskDelay(pdMS_TO_TICKS(1 * 60 * 1000));
        ESP_LOGI(INTERNET_TAG, "START TASKS");
        //initSmartConfig((void *)&scHandlerArgs);
    }
    networkType = gprs;
}
void handlerIntenetConnected(void)
{
    ppposConnectFlag = false;
    ESP_LOGI(INTERNET_TAG, "INTERNET CONNECTED");
    mqttStop();
    ppposStop();
    ESP_LOGI(INTERNET_TAG, "CONNECTING  MQTT WIFI");
    mqttStart();
    networkType = wifi;
    // if (isConnected(10) == 0)
    // {

    // }
    // else
    // {
    //     ESP_LOGI(INTERNET_TAG, "MQTT ALREADY CONNECTED");
    // }
}
void handlerFirstConnection(void)
{
    ESP_LOGI(INTERNET_TAG, "FIRST CONNECTION - START GPRS");

    ppposStart();
}

void handlerIntenetDisconnected(void)
{
    ESP_LOGI(INTERNET_TAG, "INTERNET DISCONNECTED");

    if (!ppposConnectFlag)
    {
        ppposStart();
        vTaskDelay(pdMS_TO_TICKS(1 * 60 * 1000));
        ESP_LOGI(INTERNET_TAG, "START TASKS");
        //initSmartConfig((void *)&scHandlerArgs);
    }
    networkType = gprs;
}
void internetInit()
{

    // rxQueue = xQueueCreate(10, sizeof(rx_event_t));
    mqttConfig();
#if CONFIG_ACTIVE_GPRS
    // wifiManagerInit();
    initSmartConfig((void *)&scHandlerArgs);

#else
    //ppposStart();
    initSmartConfig(&scHandlerArgs);
// wifiManagerInit();
#endif
   // otaSetup();
}

void wifiManagerInit()
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    esp_netif_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifiAP = esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &eventHandler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &eventHandler, NULL, &instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, NULL, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    xTaskCreate(checkConnectionTask, "checkConnectionTask", 1024, NULL, 5, &xcheckConnectionHandler);
    startWebserver();
    raiseAP();
}

static void checkConnectionTask(void *pvParameter)
{
    static uint8_t offlineCount = 0;
    raiseAP();
    vTaskDelay(pdMS_TO_TICKS(20 * 60000));
    dropAP();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(60000));
        if (gprsIsConnected(10) == 0)
        {
            printf("disconnected\n");
            // ppposStop();
            ppposStart();
        }
        else
        {
            printf("connected\n");
        }
    }
}

void raiseAP()
{
    ESP_LOGI(INTERNET_TAG, "Raising AP");
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "BlueRaven",
            .ssid_len = strlen("BlueRaven"),
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

    esp_netif_ip_info_t ipInfo;
    IP4_ADDR(&ipInfo.ip, 192, 168, 0, 1);
    IP4_ADDR(&ipInfo.gw, 192, 168, 0, 1);
    IP4_ADDR(&ipInfo.netmask, 255, 255, 255, 0);
    esp_netif_dhcps_stop(wifiAP);
    esp_netif_set_ip_info(wifiAP, &ipInfo);
    esp_netif_dhcps_start(wifiAP);

    ESP_ERROR_CHECK(esp_wifi_start());
    apIsRaised = 1;
}

void dropAP()
{
    if (apIsRaised == 1)
    {
        ESP_LOGI(INTERNET_TAG, "Dropping AP");
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
        apIsRaised = 0;
    }
}

uint8_t connect(char *ssid, char *password)
{
    wifi_config_t wifi_config = {};
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);

    ESP_LOGI(INTERNET_TAG, "WiFi %s ", wifi_config.sta.ssid);
    ESP_LOGI(INTERNET_TAG, "PSW %s ", wifi_config.sta.password);
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    return isConnected(15000);
}

uint16_t scanNetworks(uint16_t number, wifi_ap_record_t *ap_info)
{
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    while (scanDone == false)
        ;
    scanDone = false;

    uint16_t amount = number;
    uint16_t ap_count = 0;
    memset(ap_info, 0, amount * sizeof(wifi_ap_record_t));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&amount, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(INTERNET_TAG, "Total APs scanned = %u", ap_count);
    return ap_count;
}

static httpd_handle_t startWebserver()
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(INTERNET_TAG, "Starting server on port: '%d'", config.server_port);

    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(INTERNET_TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &serveWifiPage);
        httpd_register_uri_handler(server, &setWifi);
        httpd_register_uri_handler(server, &scan);
        httpd_register_uri_handler(server, &drop);
        httpd_register_uri_handler(server, &serveBusPage);
        httpd_register_uri_handler(server, &setBus);
        return server;
    }

    ESP_LOGI(INTERNET_TAG, "Error starting server!");
    return NULL;
}

static esp_err_t serveWifiPageGetHandler(httpd_req_t *req)
{
    extern const unsigned char cadastro_html_start[] asm("_binary_cadastro_html_start");
    extern const unsigned char cadastro_html_end[] asm("_binary_cadastro_html_end");
    const unsigned int cadastro_html_bytes = cadastro_html_end - cadastro_html_start;

    httpd_resp_send_chunk(req, (char *)cadastro_html_start, cadastro_html_bytes);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

// static esp_err_t scanGetHandler(httpd_req_t *req)
// {
//     char buffer[512];
//     wifi_ap_record_t *ap_info = (wifi_ap_record_t *)calloc(10, sizeof(wifi_ap_record_t));
//     uint16_t ap_count;

//     if ((isConnected(10) == 0) && (networkAmt > 0))
//     {
//         ap_info = scannedAP;
//         ap_count = scannedAPAmt;
//     }
//     else
//     {
//         ap_count = scanNetworks(10, ap_info);
//     }

//     sprintf(buffer, "[");
//     for (int i = 0; (i < 10) && (i < ap_count); i++)
//     {
//         uint8_t alreadyScanned = 0;
//         ESP_LOGI(INTERNET_TAG, "SSID: \t%s", ap_info[i].ssid);
//         for (int j = 0; j < i; j++)
//         {
//             if (strcmp((char *)ap_info[i].ssid, (char *)ap_info[j].ssid) == 0)
//                 alreadyScanned = 1;
//         }
//         if (alreadyScanned == 0)
//             sprintf(buffer + strlen(buffer), "{\"id\":\"%s\",\"label\":\"%s\"},", ap_info[i].ssid, ap_info[i].ssid);
//     }
//     sprintf(buffer + strlen(buffer) - 1, "]");

//     ESP_LOGI(INTERNET_TAG, "JSON: \t%s", buffer);

//     httpd_resp_sendstr_chunk(req, buffer);
//     httpd_resp_send_chunk(req, NULL, 0);
//     return ESP_OK;
// }

static esp_err_t setWifiPostHandler(httpd_req_t *req)
{
    char buf[128];
    char SSID[32];
    char PWD[64];
    int ret, remaining = req->content_len;

    buf[remaining] = 0;

    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                                  MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == 0)
            {
                ESP_LOGI(INTERNET_TAG, "No content received please try again ...");
            }
            else if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        printf("%s\n", buf);
        cJSON *root = cJSON_Parse(buf);

        sprintf(SSID, "%s", cJSON_GetObjectItem(root, "ssid")->valuestring);
        sprintf(PWD, "%s", cJSON_GetObjectItem(root, "pwd")->valuestring);

        remaining -= ret;
    }

    if (connect(SSID, PWD) == 1)
    {
        httpd_resp_sendstr_chunk(req, "{\"result\": \"success\"}");
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
        // vTaskDelete(xcheckConnectionHandler);
    }
    else
    {
        esp_restart();
        httpd_resp_sendstr_chunk(req, "{\"result\": \"fail\"}");
    }
    // End response
    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

static esp_err_t dropGetHandler(httpd_req_t *req)
{
    httpd_resp_send_chunk(req, NULL, 0);
    dropAP();
    return ESP_OK;
}

static esp_err_t serveBusPageGetHandler(httpd_req_t *req)
{
    extern const unsigned char bus_html_start[] asm("_binary_bus_html_start");
    extern const unsigned char bus_html_end[] asm("_binary_bus_html_end");
    const unsigned int bus_html_bytes = bus_html_end - bus_html_start;

    httpd_resp_send_chunk(req, (char *)bus_html_start, bus_html_bytes);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t setBusPostHandler(httpd_req_t *req)
{
    char buf[1024];
    int ret, remaining = req->content_len;
    memset(buf, 0, 1024);

    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                                  MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == 0)
            {
                ESP_LOGI(INTERNET_TAG, "No content received please try again ...");
            }
            else if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        cJSON *reqBody = cJSON_Parse(buf);
        char *recString = cJSON_PrintUnformatted(reqBody);
        printf("json: %s\n\n\n", recString);
        cJSON *device = NULL;
        bool first = true;
        cJSON_ArrayForEach(device, reqBody)
        {
            cJSON *mfr = cJSON_GetObjectItemCaseSensitive(device, "mfr");
            cJSON *mdl = cJSON_GetObjectItemCaseSensitive(device, "mdl");
            cJSON *sn = cJSON_GetObjectItemCaseSensitive(device, "sn");
            // cJSON *baud = cJSON_GetObjectItemCaseSensitive(device, "baud");
            cJSON *address = cJSON_GetObjectItemCaseSensitive(device, "address");
            modbusDevice_t node;
            char format[] = "mfr: %s\nmdl: %s\nsn:%s\nbaud: %d\naddress: %d\nstandard:%d\n\n";
            node.baud = 9600;
            // node.baud = atoi(baud->valuestring);
            node.slaveID = atoi(address->valuestring);

            sprintf(node.manufacturer, "%s", mfr->valuestring);
            sprintf(node.model, "%s", mdl->valuestring);
            sprintf(node.serialNo, "%s", sn->valuestring);

            if (strcmp(mfr->valuestring, "Fronius") == 0)
            {
                node.standard = SunspecFronius;
            }
            else if (strcmp(mfr->valuestring, "Huawei") == 0)
            {
                node.standard = Huawei;
            }
            else if (strcmp(mfr->valuestring, "Sungrow") == 0)
            {
                node.standard = Sungrow;
            }
            else if (strcmp(mfr->valuestring, "Ecosolys") == 0)
            {
                node.standard = SunspecEcosolys;
                node.baud = 19200;
            }
            else if (strcmp(mfr->valuestring, "Canadian") == 0)
            {
                node.standard = Canadian;
            }
            else if (strcmp(mfr->valuestring, "SAJ") == 0)
            {
                node.standard = SAJ;
            }
            else if (strcmp(mfr->valuestring, "sofar") == 0)
            {
                node.standard = sofar;
            }
            else if (strcmp(mfr->valuestring, "Refusol") == 0)
            {
                node.standard = Refusol;
                if ((strcmp(mdl->valuestring, "20kW-3T") == 0) || (strcmp(mdl->valuestring, "40kW-3T") == 0))
                {
                    node.standard = SunspecRefusol;
                }
            }

            printf(format, node.manufacturer, node.model, node.serialNo, node.baud, node.slaveID, node.standard);
            if (first)
            {
                createBusFile(node);
                first = false;
            }
            else
                addBusData(node);
        }

        remaining -= ret;
        cJSON_Delete(reqBody);
    }

    httpd_resp_send_chunk(req, NULL, 0);
    createSignupFile(1);

    return ESP_OK;
}

static void eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(INTERNET_TAG, "SYSTEM_EVENT_STA_DISCONNECTED %d ", esp_wifi_connect());
        esp_wifi_connect();
        networkType = gprs;
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(INTERNET_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        networkType = wifi;
        // if (isConnected(10) == 0)
        //     mqttConnect();
        // else
        // {
        //     mqttReconnect();
        // }
    }
}

static void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_START)
    {
        ESP_ERROR_CHECK(mdns_init());
        ESP_ERROR_CHECK(mdns_hostname_set("blueraven"));
        ESP_ERROR_CHECK(mdns_instance_name_set("BlueRaven webserver"));
    }
    else if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(INTERNET_TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(INTERNET_TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        scanDone = true;
    }

    ESP_LOGI(INTERNET_TAG, "wifiEventHandler event ID: %d", event_id);
}

int mqttPublish(const char *topic, const char *data, int qos)
{
    ESP_LOGI(INTERNET_TAG, "-------->>>>> %s", topic);

    if (isConnected(10) == 1)
        return mqttPub(topic, data, qos);
    else
        return -1;
}

uint8_t isConnected(uint16_t timeout)
{
    return mqttIsConnected(timeout);
}

void forceOta()
{
    otaCheck();
}

uint8_t isOta()
{
    return getOtaFlag();
}

void getInternetInfo(char *info)
{
    uint8_t macAddr[6];
    esp_efuse_mac_get_default(macAddr);
    sprintf(macGlobal, "" MACSTR "", MAC2STR(macAddr));
    char format[] = "\"mac\":\"%s\",\"hw\":\"%d\",\"mode\":\"%s\",\"ssid\":\"%s\",\"rssi\":%d,\"authmode\":%d,\"oper\":\"%s\",\"version\":\"%d.%d.%03d\", \"associated\":\"%s\"";
    uint32_t rssi = 0, ber = 0;
    // getSignalQuantity(&rssi, &ber);
    // printf("----> rssi:%d, ber:%d <-----", rssi, ber);
    int8_t hwVersion = getHardware();
    if (hwVersion != 2)
        hwVersion = 1;

    char associate[5];
    if (nvs_str_read(associate, sizeof(associate), "ASSOCIATE") != ESP_OK)
    {
        strcpy(associate, "0");
    }
    if (networkType == gprs)
    {
        char oper[15];
        getGprsInfo(oper);
        sprintf(info, format, macGlobal, hwVersion, "GPRS", "", 0, 0, oper, CONFIG_FIRMWARE_MAJOR, CONFIG_FIRMWARE_MINOR, CONFIG_FIRMWARE_PATCH, associate);
    }
    else
    {
        wifi_ap_record_t apInfo;
        esp_wifi_sta_get_ap_info(&apInfo);
        sprintf(info, format, macGlobal, hwVersion, "WiFi", apInfo.ssid, apInfo.rssi, apInfo.authmode, "", CONFIG_FIRMWARE_MAJOR, CONFIG_FIRMWARE_MINOR, CONFIG_FIRMWARE_PATCH, associate);
    }
}
