/*
 * improv_wrapper.h
 *
 *  Created on: Apr 29, 2024
 *      Author: karl
 */

#ifndef COMPONENTS_IMPROV_WIFI_INCLUDE_IMPROV_WRAPPER_H_
#define COMPONENTS_IMPROV_WIFI_INCLUDE_IMPROV_WRAPPER_H_

#ifdef __cplusplus
#include "improvWifi.h"
extern "C" {
#else
enum ChipFamily_e {
  CF_ESP32,
  CF_ESP32_C3,
  CF_ESP32_S2,
  CF_ESP32_S3,
  CF_ESP8266
};
#endif

void improv_wifi_create(void);
void improv_wifi_destroy(void);
int improv_wifi_handle_serial(const uint8_t *data, size_t length);
void improv_wifi_set_device_info(uint8_t chipFamily, const char *firmwareName,
                                 const char *firmwareVersion,
                                 const char *deviceName, const char *deviceUrl);
void improv_wifi_serialWrite(void *cb);
void improv_wifi_onImprovError(void *onImprovErrorCb);
void improv_wifi_onImprovConnected(void *onImprovConnectedCb);
void improv_wifi_setCustomConnectWiFi(void *setCustomConnectWiFiCb);
void improv_wifi_setCustomScanWiFi(void *setCustomScanWiFiCb);
void improv_wifi_setCustomIsConnected(void *setCustomIsConnected);
void improv_wifi_setCustomGetLocalIpCallback(void *getLocalIpCallback);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_IMPROV_WIFI_INCLUDE_IMPROV_WRAPPER_H_ */
