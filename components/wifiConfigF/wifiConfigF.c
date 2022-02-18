#include <stdio.h>
#include "wifiConfigF.h"

#include "ping/ping_sock.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
TaskHandle_t pintTaskHandle;
TaskHandle_t SCTaskHandle = NULL;
/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/* FreeRTOS event group to signal when we are connected & ready to make a request */

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int SC_CONNECTED_BIT = BIT0;
static const int SC_ESPTOUCH_DONE_BIT = BIT1;
static const int SC_DISCONNECTED_BIT = BIT2;
static const int SC_FIRST_CONNETION_BIT = BIT3;
static const int SC_START_CONNETION_BIT = BIT4;
static const int SC_FOUND_CHANNEL_BIT = BIT5;
static const int PING_DONE_BIT = BIT6;
static const char *TAG = "wifiConfig";
static uint8_t ssid[33] = {0};
static uint8_t password[65] = {0};
static uint8_t bssid[65] = {0};
uint8_t routerConnectedFlag = 0;
// wifi_config_t wifi_config;
wifi_config_t wifi_config;
smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
handleFunc firstHandler = NULL;
handleFunc internetConnected = NULL;
handleFunc routerGotIp = NULL;
handleFunc internetDisconnected = NULL;
handleFunc wifiStart = NULL;
handleFunc routerDisconnected = NULL;
handleFunc credentialCB = NULL;
static void smartconfig_example_task(void *parm);
esp_netif_t *sta_netif = NULL;
bool withoutInternetFLag = true;
bool onlyOnceSC = true;
bool internetConnectedFlag = false;
void pingTask(void *args);

static void test_on_ping_success(esp_ping_handle_t hdl, void *args)
{
  // optionally, get callback arguments
  // const char* str = (const char*) args;
  // printf("%s\r\n", str); // "foo"
  uint8_t ttl;
  uint16_t seqno;
  uint32_t elapsed_time, recv_len;
  ip_addr_t target_addr;
  esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
  esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
  printf("%d bytes from %s icmp_seq=%d ttl=%d time=%d ms\n",
         recv_len, inet_ntoa(target_addr.u_addr.ip4), seqno, ttl, elapsed_time);
}

static void test_on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
  uint16_t seqno;
  ip_addr_t target_addr;
  esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
  printf("From %s icmp_seq=%d timeout\n", inet_ntoa(target_addr.u_addr.ip4), seqno);
}

static void test_on_ping_end(esp_ping_handle_t hdl, void *args)
{
  uint32_t transmitted;
  uint32_t received;
  uint32_t total_time_ms;

  esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
  esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
  esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
  printf("%d packets transmitted, %d received, time %dms\n", transmitted, received, total_time_ms);
  xEventGroupSetBits(s_wifi_event_group, PING_DONE_BIT);
  if ((received > 1) && !internetConnectedFlag)
  {
    withoutInternetFLag = true;

    ESP_LOGI(TAG, "HANDLE PING CONNECT");
    internetConnectedFlag = true;
    (internetConnected)();
  }
  if ((received == 0) && (internetConnectedFlag == true))
  {
    ESP_LOGI(TAG, "HANDLE PING DISCONNECT");
    withoutInternetFLag = false;
    internetConnectedFlag = false;
    // vTaskDelete(pintTaskHandle);
    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskDelete(SCTaskHandle);
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_wifi_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_wifi_deinit();
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_netif_destroy(sta_netif); //não é necessário
    vTaskDelay(pdMS_TO_TICKS(1000));
    (internetDisconnected)();

    // vTaskDelay(pdMS_TO_TICKS(120000));
    // ESP_LOGI(TAG, "STARTING WIFI TASKS");
    // vTaskResume(SCTaskHandle);
    // vTaskResume(pintTaskHandle);
    // ESP_ERROR_CHECK(esp_wifi_stop());
    // ESP_ERROR_CHECK(esp_wifi_deinit());
  }

  // if ((received == 0) && internetConnectedFlag)
  // {
  //   ESP_LOGI(TAG, "HANDLE PING DISCONNECT");
  //   (internetDisconnected)();
  //   internetConnectedFlag = false;
  // }
}

esp_err_t initialize_ping(esp_ping_handle_t *ping)
{

  /* convert URL to IP address */
  ip_addr_t target_addr;
  struct addrinfo hint;
  struct addrinfo *res = NULL;
  memset(&hint, 0, sizeof(hint));
  memset(&target_addr, 0, sizeof(target_addr));
  getaddrinfo("www.google.com", NULL, &hint, &res);
  if (res == NULL)
  {
    return ESP_FAIL;
  }
  else
  {
    struct in_addr addr4 = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
    inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);

    printf("%s\n", inet_ntoa(target_addr.u_addr.ip4));
    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.task_stack_size = 2 * 2048;
    ping_config.target_addr = target_addr; // target IP address
    ping_config.count = 5;                 // ping in infinite mode, esp_ping_stop can stop it

    /* set callback functions */
    esp_ping_callbacks_t cbs;
    cbs.on_ping_success = test_on_ping_success;
    cbs.on_ping_timeout = test_on_ping_timeout;
    cbs.on_ping_end = test_on_ping_end;
    cbs.cb_args = NULL; // arguments that will feed to all callback functions, can be NULL
    // cbs.cb_args = eth_event_group;
    freeaddrinfo(res);

    if (esp_ping_new_session(&ping_config, &cbs, ping) == -1)
    {
      ESP_LOGE(TAG, "ERROR SESSION CREATION");
      return ESP_FAIL;
    }

    return ESP_OK;
  }
}
// static const esp_ping_handle_t emptyPing;

void pingTask(void *args)
{
  ESP_LOGI(TAG, "PING TASK INIT");
  esp_ping_handle_t ping;
  // EventBits_t bits;
  for (;;)
  {
    ESP_LOGI(TAG, "TEST 1");
    vTaskDelay(pdMS_TO_TICKS(5000));
    // bits = xEventGroupWaitBits(s_wifi_event_group,
    //                            SC_CONNECTED_BIT,
    //                            pdFALSE,
    //                            pdFALSE,
    //                            pdMS_TO_TICKS(5000));
    // ESP_LOGI(TAG, "Heap Memory --> %d", esp_get_free_heap_size());
    if (routerConnectedFlag == 1)
    {
      ESP_LOGI(TAG, "TEST 2");
      if (initialize_ping(&ping) == ESP_OK)
      {
        esp_ping_start(ping);
        xEventGroupWaitBits(s_wifi_event_group,
                            PING_DONE_BIT,
                            pdFALSE,
                            pdFALSE,
                            pdMS_TO_TICKS(10000));
        xEventGroupClearBits(s_wifi_event_group, PING_DONE_BIT);
        if (esp_ping_stop(ping) == ESP_OK)
        {
          esp_ping_delete_session(ping);
        };
      }
      else
      {
        ESP_LOGI(TAG, "CONNECTED ON ROUTER WITHOUT INTERNET");
        if (withoutInternetFLag)
        {
          withoutInternetFLag = false;
          // internetConnectedFlag = false;
          // vTaskDelete(pintTaskHandle);
          vTaskDelay(pdMS_TO_TICKS(1000));
          vTaskDelete(SCTaskHandle);
          vTaskDelay(pdMS_TO_TICKS(1000));
          esp_wifi_disconnect();
          vTaskDelay(pdMS_TO_TICKS(1000));
          esp_wifi_stop();
          vTaskDelay(pdMS_TO_TICKS(1000));
          esp_wifi_deinit();
          vTaskDelay(pdMS_TO_TICKS(1000));
          esp_netif_destroy(sta_netif); //não é necessário
          vTaskDelay(pdMS_TO_TICKS(1000));
          (internetDisconnected)();
        }
      }
    }
  }
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    ESP_LOGI(TAG, "WIFI START");
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    xEventGroupSetBits(s_wifi_event_group, SC_START_CONNETION_BIT);
    (wifiStart)();
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    // esp_wifi_connect();
    xEventGroupSetBits(s_wifi_event_group, SC_DISCONNECTED_BIT);
    xEventGroupClearBits(s_wifi_event_group, SC_CONNECTED_BIT);
    ESP_LOGI(TAG, "connect to the AP fail");
    routerConnectedFlag = 0;
    if (withoutInternetFLag)
    {
      withoutInternetFLag = false;
      vTaskDelay(pdMS_TO_TICKS(1000));
      vTaskDelete(SCTaskHandle);
      vTaskDelay(pdMS_TO_TICKS(1000));
      esp_wifi_disconnect();
      vTaskDelay(pdMS_TO_TICKS(1000));
      esp_wifi_stop();
      vTaskDelay(pdMS_TO_TICKS(1000));
      esp_wifi_deinit();
      vTaskDelay(pdMS_TO_TICKS(1000));
      esp_netif_destroy(sta_netif); //não é necessário
      vTaskDelay(pdMS_TO_TICKS(1000));
      (routerDisconnected)();
    }
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    xEventGroupClearBits(s_wifi_event_group, SC_DISCONNECTED_BIT);
    xEventGroupSetBits(s_wifi_event_group, SC_CONNECTED_BIT);
    (routerGotIp)();
    routerConnectedFlag = 1;
  }
  else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
  {
    ESP_LOGI(TAG, "Scan done");
  }
  else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
  {
    ESP_LOGI(TAG, "Found channel");
    xEventGroupSetBits(s_wifi_event_group, SC_FOUND_CHANNEL_BIT);
  }
  else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
  {
    smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
    bzero(&wifi_config, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
    wifi_config.sta.bssid_set = evt->bssid_set;
    if (wifi_config.sta.bssid_set == true)
    {
      memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
    }
    memcpy(ssid, evt->ssid, sizeof(evt->ssid));
    memcpy(password, evt->password, sizeof(evt->password));
    ESP_LOGI(TAG, "SSID:%s", ssid);
    ESP_LOGI(TAG, "PASSWORD:%s", password);
    ESP_ERROR_CHECK(nvs_str_save("SSID", (char *)ssid));
    ESP_ERROR_CHECK(nvs_str_save("PASSWORD", (char *)password));
    ESP_ERROR_CHECK(nvs_str_save("BSSID", (char *)evt->ssid));
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    (credentialCB)();
    xEventGroupClearBits(s_wifi_event_group, SC_FOUND_CHANNEL_BIT);
  }
  else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
  {
    xEventGroupClearBits(s_wifi_event_group, SC_FOUND_CHANNEL_BIT);
    xEventGroupSetBits(s_wifi_event_group, SC_ESPTOUCH_DONE_BIT);
  }
}
uint8_t onlyOnce = true;
static void initialise_wifi(handle_args_t *args)
{

  if (onlyOnce)
  {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    onlyOnce = false;
  }
  sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, (void *)args));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, (void *)args));
  ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, (void *)args));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

  if (nvs_str_read((char *)ssid, sizeof(ssid), "SSID") == ESP_OK && nvs_str_read((char *)password, sizeof(password), "PASSWORD") == ESP_OK && nvs_str_read((char *)bssid, sizeof(bssid), "BSSID") == ESP_OK)
  {
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
  }
}

static void smartconfig_example_task(void *parm)
{
  bool selector = true;
  EventBits_t uxBits;

  while (1)
  {
    uxBits = xEventGroupWaitBits(s_wifi_event_group, SC_CONNECTED_BIT | SC_ESPTOUCH_DONE_BIT, true, false, pdMS_TO_TICKS(20000));
    if (uxBits & SC_CONNECTED_BIT)
    {

      ESP_LOGI(TAG, "WiFi Connected to ap");
    }
    if (uxBits & SC_ESPTOUCH_DONE_BIT)
    {
      ESP_LOGI(TAG, "smartconfig over");
      esp_smartconfig_stop();
      // vTaskDelete(NULL);
    }
    if ((uxBits & SC_DISCONNECTED_BIT) && (uxBits & SC_START_CONNETION_BIT) && !(uxBits & SC_FOUND_CHANNEL_BIT))
    {
      if (selector)
      {
        ESP_LOGI(TAG, "SCAN PAPAI");
        ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
        selector = !selector;
      }
      else
      {
        ESP_LOGI(TAG, "TENTANDO CONECTAR");
        esp_smartconfig_stop();
        // bzero(&wifi_config, sizeof(wifi_config_t));
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_connect());
        selector = !selector;
      }
    }
    if ((uxBits & SC_FIRST_CONNETION_BIT) && (uxBits & SC_START_CONNETION_BIT))
    {
      ESP_LOGI(TAG, "FIRST SCAN");
      ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
      xEventGroupClearBits(s_wifi_event_group, SC_FIRST_CONNETION_BIT);
      (firstHandler)();
    }
  }
}

void initSmartConfig(handle_args_t *args)
{

  firstHandler = ((handle_args_t *)args)->firstHandler;
  internetConnected = ((handle_args_t *)args)->internetConnected;
  internetDisconnected = ((handle_args_t *)args)->internetDisconnected;
  routerGotIp = ((handle_args_t *)args)->routerGotIp;
  routerDisconnected = ((handle_args_t *)args)->routerDisconnected;
  wifiStart = ((handle_args_t *)args)->wifiStart;
  credentialCB = ((handle_args_t *)args)->credentialCB;

  ESP_ERROR_CHECK(nvs_flash_init());
  s_wifi_event_group = xEventGroupCreate();

  if (onlyOnceSC)
  {
    xTaskCreate(pingTask, "pingTASK", 5 * 2048, NULL, 6, &pintTaskHandle);
    onlyOnceSC = false;
  }

  xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, &SCTaskHandle);

  if (nvs_str_read((char *)ssid, sizeof(ssid), "SSID") == ESP_OK && nvs_str_read((char *)password, sizeof(password), "PASSWORD") == ESP_OK && nvs_str_read((char *)bssid, sizeof(bssid), "BSSID") == ESP_OK)
  {
    ESP_LOGI(TAG, "SSID:%s", (char *)ssid);
    ESP_LOGI(TAG, "PASSWORD:%s", (char *)password);
    ESP_LOGI(TAG, "BSSID:%s", (char *)bssid);
  }
  else
  {
    ESP_LOGI(TAG, "FIRST CONNECTION");
    xEventGroupSetBits(s_wifi_event_group, SC_FIRST_CONNETION_BIT);
  }
  initialise_wifi(args);
}
// void app_main(void)
// {
//
//   initSmartConfig();
// }
