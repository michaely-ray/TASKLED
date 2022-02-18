
#include "driver/gpio.h"
#include "pppos_client.h"

#define RED (gpio_num_t)14
#define GREEN (gpio_num_t)12
#define BLUE (gpio_num_t)27
static const char *TAG = "GPRS";
static EventGroupHandle_t event_group = NULL;
static const int CONNECT_BIT = BIT0;
static const int STOP_BIT = BIT1;
bool onlyOncePPPOS = true;
modem_dte_t *dte = NULL;
modem_dce_t *dce = NULL;
void *modem_netif_adapter = NULL;
esp_netif_t *esp_netif = NULL;
bool ppposConnectFlag = false;
bool ppposProcessingFlag = false;
void ppposReconnect(void);
void sim800lConfig(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << GPIO_NUM_21;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void sim800lReset(void)
{
    gpio_set_level(GPIO_NUM_21, 0);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_NUM_21, 1);
    vTaskDelay(10000 / portTICK_PERIOD_MS);
}

void ppposStart(void)
{
    if (!ppposConnectFlag && !ppposProcessingFlag)
    {
        mqttStop();
        ppposProcessingFlag = true;
        sim800lConfig();
        sim800lReset();
        vTaskDelay(pdMS_TO_TICKS(20000));
        ppposConnect();
    }
}

static void modem_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case ESP_MODEM_EVENT_PPP_START:
        ESP_LOGI(TAG, "Modem PPP Started");
        break;
    case ESP_MODEM_EVENT_PPP_STOP:
        ESP_LOGI(TAG, "Modem PPP Stopped");
        xEventGroupSetBits(event_group, STOP_BIT);
        break;
    case ESP_MODEM_EVENT_UNKNOWN:
        ESP_LOGW(TAG, "Unknow line received: %s", (char *)event_data);
        break;
    default:
        break;
    }
}

static void on_ppp_changed(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "PPP state changed event %d", event_id);
    if (event_id == NETIF_PPP_ERRORUSER)
    {
        /* User interrupted event from esp-netif */
        esp_netif_t *netif = event_data;
        ESP_LOGI(TAG, "User interrupted event from netif:%p", netif);
    }
}

static void on_ip_event(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "IP event! %d", event_id);
    if (event_id == IP_EVENT_PPP_GOT_IP)
    {
        esp_netif_dns_info_t dns_info;

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        esp_netif_t *netif = event->esp_netif;

        ESP_LOGI(TAG, "Modem Connect to PPP Server");
        ESP_LOGI(TAG, "~~~~~~~~~~~~~~");
        ESP_LOGI(TAG, "IP          : " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Netmask     : " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGI(TAG, "Gateway     : " IPSTR, IP2STR(&event->ip_info.gw));
        esp_netif_get_dns_info(netif, 0, &dns_info);
        ESP_LOGI(TAG, "Name Server1: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
        esp_netif_get_dns_info(netif, 1, &dns_info);
        ESP_LOGI(TAG, "Name Server2: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
        ESP_LOGI(TAG, "~~~~~~~~~~~~~~");
        xEventGroupSetBits(event_group, CONNECT_BIT);
        ppposConnectFlag = true;

        ESP_LOGI(TAG, "Starting MQTT via GPRS");
        mqttStop();
        // mqttDestroy();
        // ESP_LOGI(TAG, "Config ok");
        // mqttConfigGPRS();
        mqttConfig();
        mqttStart();
        setHardware(2);
        ppposProcessingFlag = false;
        ESP_LOGI(TAG, "GOT ip event!!!");
    }
    else if (event_id == IP_EVENT_PPP_LOST_IP)
    {
        ppposProcessingFlag = false;
        ESP_LOGI(TAG, "Modem Disconnect from PPP Server");
        xEventGroupClearBits(event_group, CONNECT_BIT);
        ppposReconnect();
        // ppposStart();
        ppposConnectFlag = false;
    }
    else if (event_id == IP_EVENT_GOT_IP6)
    {
        ESP_LOGI(TAG, "GOT IPv6 event!");

        ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
        ESP_LOGI(TAG, "Got IPv6 address " IPV6STR, IPV62STR(event->ip6_info.ip));
    }
}

void ppposConnect(void)
{
    event_group = xEventGroupCreate();
#if CONFIG_LWIP_PPP_PAP_SUPPORT
    esp_netif_auth_type_t auth_type = NETIF_PPP_AUTHTYPE_PAP;
#elif CONFIG_LWIP_PPP_CHAP_SUPPORT
    esp_netif_auth_type_t auth_type = NETIF_PPP_AUTHTYPE_CHAP;
#elif !defined(CONFIG_MODEM_PPP_AUTH_NONE)
// #error "Unsupported AUTH Negotiation"
#endif

    ESP_ERROR_CHECK(esp_netif_init());
#if !CONFIG_ACTIVE_GPRS
    ESP_ERROR_CHECK(esp_event_loop_create_default());
#endif
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &on_ip_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(NETIF_PPP_STATUS, ESP_EVENT_ANY_ID, &on_ppp_changed, NULL));
    if (onlyOncePPPOS)
    {

        /* create dte object */
        esp_modem_dte_config_t config = ESP_MODEM_DTE_DEFAULT_CONFIG();
        /* setup UART specific configuration based on kconfig options */
        config.tx_io_num = CONFIG_MODEM_UART_TX_PIN;
        config.rx_io_num = CONFIG_MODEM_UART_RX_PIN;
        config.rts_io_num = CONFIG_MODEM_UART_RTS_PIN;
        config.cts_io_num = CONFIG_MODEM_UART_CTS_PIN;
        config.rx_buffer_size = CONFIG_MODEM_UART_RX_BUFFER_SIZE;
        config.tx_buffer_size = CONFIG_MODEM_UART_TX_BUFFER_SIZE;
        config.pattern_queue_size = CONFIG_MODEM_UART_PATTERN_QUEUE_SIZE;
        config.event_queue_size = CONFIG_MODEM_UART_EVENT_QUEUE_SIZE;
        config.event_task_stack_size = CONFIG_MODEM_UART_EVENT_TASK_STACK_SIZE;
        config.event_task_priority = CONFIG_MODEM_UART_EVENT_TASK_PRIORITY;
        config.line_buffer_size = CONFIG_MODEM_UART_RX_BUFFER_SIZE / 2;

        dte = esp_modem_dte_init(&config);
        /* Register event handler */
        ESP_ERROR_CHECK(esp_modem_set_event_handler(dte, modem_event_handler, ESP_EVENT_ANY_ID, NULL));

        // Init netif object
        esp_netif_config_t cfg = ESP_NETIF_DEFAULT_PPP();
        esp_netif = esp_netif_new(&cfg);
        assert(esp_netif);

        modem_netif_adapter = esp_modem_netif_setup(dte);
        esp_modem_netif_set_default_handlers(modem_netif_adapter, esp_netif);
        onlyOncePPPOS = false;
    }

    /* create dce object */
#if CONFIG_MODEM_DEVICE_SIM800
    dce = sim800_init(dte);
#elif CONFIG_MODEM_DEVICE_BG96
    dce = bg96_init(dte);
#elif CONFIG_MODEM_DEVICE_SIM7600
    dce = sim7600_init(dte);
#else
// #error "Unsupported DCE"
#endif
    if (dce == NULL)
        return;
    gpio_pad_select_gpio(RED);
    gpio_pad_select_gpio(GREEN);
    gpio_pad_select_gpio(BLUE);
    gpio_set_direction(RED, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(GREEN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(BLUE, GPIO_MODE_INPUT_OUTPUT);
    ESP_ERROR_CHECK(dce->set_flow_ctrl(dce, MODEM_FLOW_CONTROL_NONE));
    ESP_ERROR_CHECK(dce->store_profile(dce));
    /* Print Module ID, Operator, IMEI, IMSI */
    ESP_LOGI(TAG, "Module: %s", dce->name);
    ESP_LOGI(TAG, "Operator: %s", dce->oper);
    if(strcmp(dce->oper,"")==0){
        while(1){
            
            gpio_set_level(RED, 1);
            gpio_set_level(GREEN, 0);
            gpio_set_level(BLUE, 0);
            vTaskDelay(pdMS_TO_TICKS(5000));
            int stop = 0;
            while(1){
                stop++;
            }
        }
    }else{
        while(1){
            
            gpio_set_level(RED, 0);
            gpio_set_level(GREEN, 0);
            gpio_set_level(BLUE, 1);
            vTaskDelay(pdMS_TO_TICKS(5000));
            int stop = 0;
            while(1){
                stop++;
            }
        }
    }
    ESP_LOGI(TAG, "IMEI: %s", dce->imei);
    ESP_LOGI(TAG, "IMSI: %s", dce->imsi);
    /* Get signal quality */
    uint32_t rssi = 0, ber = 0;
    ESP_ERROR_CHECK(dce->get_signal_quality(dce, &rssi, &ber));
    ESP_LOGI(TAG, "rssi: %d, ber: %d", rssi, ber);
    /* Get battery voltage */
    uint32_t voltage = 0, bcs = 0, bcl = 0;
    ESP_ERROR_CHECK(dce->get_battery_status(dce, &bcs, &bcl, &voltage));
    ESP_LOGI(TAG, "Battery voltage: %d mV", voltage);
    /* setup PPPoS network parameters */
#if !defined(CONFIG_MODEM_PPP_AUTH_NONE) && (defined(CONFIG_LWIP_PPP_PAP_SUPPORT) || defined(CONFIG_LWIP_PPP_CHAP_SUPPORT))
    ////esp_netif_ppp_set_auth(esp_netif, auth_type, CONFIG_MODEM_PPP_AUTH_USERNAME, CONFIG_MODEM_PPP_AUTH_PASSWORD);
#endif
    /* attach the modem to the network interface */
    // esp_netif_attach(esp_netif, modem_netif_adapter);
    if (esp_netif_attach(esp_netif, modem_netif_adapter) != ESP_OK)
    {
        ESP_LOGI(TAG, "RECONNCT PPPoS");
        ppposReconnect();
    }
    /* Wait for IP address */
    // xEventGroupWaitBits(event_group, CONNECT_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
}
void ppposReconnect(void)
{
    mqttStop();
    // ESP_ERROR_CHECK(esp_modem_stop_ppp(dte));

    //xEventGroupWaitBits(event_group, STOP_BIT, pdTRUE, pdTRUE, portMAX_DELAY);

    // Power down module
    ESP_ERROR_CHECK(dce->power_down(dce));
    ESP_LOGI(TAG, "Power down");
    ESP_ERROR_CHECK(dce->deinit(dce));

    // Unregister events, destroy the netif adapter and destroy its esp-netif instance
    // esp_modem_netif_clear_default_handlers(modem_netif_adapter);
    // esp_modem_netif_teardown(modem_netif_adapter);
    // esp_netif_destroy(esp_netif);

    ESP_ERROR_CHECK(dte->deinit(dte));

    // reset
    sim800lConfig();
    sim800lReset();
    vTaskDelay(pdMS_TO_TICKS(20000));
    //configuration
#if CONFIG_LWIP_PPP_PAP_SUPPORT
    esp_netif_auth_type_t auth_type = NETIF_PPP_AUTHTYPE_PAP;
#elif CONFIG_LWIP_PPP_CHAP_SUPPORT
    esp_netif_auth_type_t auth_type = NETIF_PPP_AUTHTYPE_CHAP;
#elif !defined(CONFIG_MODEM_PPP_AUTH_NONE)
// #error "Unsupported AUTH Negotiation"
#endif

    // ESP_ERROR_CHECK(esp_netif_init());
#if !CONFIG_ACTIVE_GPRS
    ESP_ERROR_CHECK(esp_event_loop_create_default());
#endif
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &on_ip_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(NETIF_PPP_STATUS, ESP_EVENT_ANY_ID, &on_ppp_changed, NULL));

    /* create dte object */
    esp_modem_dte_config_t config = ESP_MODEM_DTE_DEFAULT_CONFIG();
    /* setup UART specific configuration based on kconfig options */
    config.tx_io_num = CONFIG_MODEM_UART_TX_PIN;
    config.rx_io_num = CONFIG_MODEM_UART_RX_PIN;
    config.rts_io_num = CONFIG_MODEM_UART_RTS_PIN;
    config.cts_io_num = CONFIG_MODEM_UART_CTS_PIN;
    config.rx_buffer_size = CONFIG_MODEM_UART_RX_BUFFER_SIZE;
    config.tx_buffer_size = CONFIG_MODEM_UART_TX_BUFFER_SIZE;
    config.pattern_queue_size = CONFIG_MODEM_UART_PATTERN_QUEUE_SIZE;
    config.event_queue_size = CONFIG_MODEM_UART_EVENT_QUEUE_SIZE;
    config.event_task_stack_size = CONFIG_MODEM_UART_EVENT_TASK_STACK_SIZE;
    config.event_task_priority = CONFIG_MODEM_UART_EVENT_TASK_PRIORITY;
    config.line_buffer_size = CONFIG_MODEM_UART_RX_BUFFER_SIZE / 2;

    dte = esp_modem_dte_init(&config);
    /* Register event handler */
    ESP_ERROR_CHECK(esp_modem_set_event_handler(dte, modem_event_handler, ESP_EVENT_ANY_ID, NULL));

    // Init netif object
    // esp_netif_config_t cfg = ESP_NETIF_DEFAULT_PPP();
    // esp_netif = esp_netif_new(&cfg);
    // assert(esp_netif);

    modem_netif_adapter = esp_modem_netif_setup(dte);
    esp_modem_netif_set_default_handlers(modem_netif_adapter, esp_netif);

    /* create dce object */
#if CONFIG_MODEM_DEVICE_SIM800
    dce = sim800_init(dte);
#elif CONFIG_MODEM_DEVICE_BG96
    dce = bg96_init(dte);
#elif CONFIG_MODEM_DEVICE_SIM7600
    dce = sim7600_init(dte);
#else
// #error "Unsupported DCE"
#endif
    if (dce == NULL)
        return;
    ESP_ERROR_CHECK(dce->set_flow_ctrl(dce, MODEM_FLOW_CONTROL_NONE));
    ESP_ERROR_CHECK(dce->store_profile(dce));
    /* Print Module ID, Operator, IMEI, IMSI */
    ESP_LOGI(TAG, "Module: %s", dce->name);
    ESP_LOGI(TAG, "Operator: %s", dce->oper);
    ESP_LOGI(TAG, "IMEI: %s", dce->imei);
    ESP_LOGI(TAG, "IMSI: %s", dce->imsi);
    /* Get signal quality */
    uint32_t rssi = 0, ber = 0;
    ESP_ERROR_CHECK(dce->get_signal_quality(dce, &rssi, &ber));
    ESP_LOGI(TAG, "rssi: %d, ber: %d", rssi, ber);
    /* Get battery voltage */
    uint32_t voltage = 0, bcs = 0, bcl = 0;
    ESP_ERROR_CHECK(dce->get_battery_status(dce, &bcs, &bcl, &voltage));
    ESP_LOGI(TAG, "Battery voltage: %d mV", voltage);
    /* setup PPPoS network parameters */
#if !defined(CONFIG_MODEM_PPP_AUTH_NONE) && (defined(CONFIG_LWIP_PPP_PAP_SUPPORT) || defined(CONFIG_LWIP_PPP_CHAP_SUPPORT))
    ////esp_netif_ppp_set_auth(esp_netif, auth_type, CONFIG_MODEM_PPP_AUTH_USERNAME, CONFIG_MODEM_PPP_AUTH_PASSWORD);
#endif
    /* attach the modem to the network interface */
    if (esp_netif_attach(esp_netif, modem_netif_adapter) != ESP_OK)
    {
        ESP_LOGI(TAG, "RECONNCT PPPoS");
        ppposReconnect();
    }
    /* Wait for IP address */
    // xEventGroupWaitBits(event_group, CONNECT_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
}
void ppposStop(void)
{
    ESP_LOGE(TAG, "Test");
    // Exit PPP mode
    if (dce != NULL)
    {

        ESP_ERROR_CHECK(esp_modem_stop_ppp(dte));

        //xEventGroupWaitBits(event_group, STOP_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
        ESP_LOGE(TAG, "Test3333");
        // Power down module
        ESP_ERROR_CHECK(dce->power_down(dce));
        ESP_LOGI(TAG, "Power down");
        ESP_ERROR_CHECK(dce->deinit(dce));

        // Unregister events, destroy the netif adapter and destroy its esp-netif instance
        // esp_modem_netif_clear_default_handlers(modem_netif_adapter);
        // esp_modem_netif_teardown(modem_netif_adapter);
        // esp_netif_destroy(esp_netif);

        // ESP_ERROR_CHECK(dte->deinit(dte));
    }
}
void getGprsInfo(char *oper)
{
    int8_t operIndex = 0;
    if (dce != NULL)
    {
        operIndex = espModemFindOperator(dce->oper);
    }
    else
    {
        operIndex = -1;
    }

    if (operIndex == -1)
    {
        ESP_LOGE(TAG, "Unknown operator :(");
        sprintf(oper, "%s", "Unk");
    }
    else
    {
        sprintf(oper, "%s", operators[operIndex].name);
    }
}

void getSignalQuantity(uint32_t *rssi, uint32_t *ber)
{
    if (dce == NULL)
    {
        ESP_ERROR_CHECK(dce->get_signal_quality(dce, rssi, ber));
    }
}

uint8_t gprsIsConnected(uint16_t timeout)
{
    EventBits_t uxBits = xEventGroupWaitBits(event_group, CONNECT_BIT, false, true, pdMS_TO_TICKS(timeout));
    if ((uxBits & CONNECT_BIT) == 0)
        return 0;
    else
    {
        return 1;
    }
}
