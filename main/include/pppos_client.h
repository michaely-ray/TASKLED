#ifndef _PPPOS_CLIENT_H_
#define _PPPOS_CLIENT_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_netif_ppp.h"
#include "esp_modem.h"
#include "esp_modem_netif.h"
#include "esp_log.h"
#include "sim800.h"
#include "bg96.h"
#include "sim7600.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif
  extern bool ppposConnectFlag;
  void getSignalQuantity(uint32_t *rssi, uint32_t *ber);
  void sim800lConfig(void);
  void sim800lReset(void);
  void ppposStart(void);
  void ppposConnect(void);
  void ppposStop(void);
  void getGprsInfo(char *oper);
  uint8_t gprsIsConnected(uint16_t timeout);

#ifdef __cplusplus
}
#endif

#endif