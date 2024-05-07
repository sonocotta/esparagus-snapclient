/*
 * improv_wrapper.cpp
 *
 *  Created on: Apr 28, 2024
 *      Author: karl
 */
#include "improv_wrapper.h"

ImprovWiFi *c = NULL;

extern "C" void improv_wifi_create(void) {
  if (c != NULL) {
    delete c;
  }

  c = new ImprovWiFi();
}

extern "C" void improv_wifi_destroy(void) {
  if (c != NULL) {
    delete c;
    c = NULL;
  }
}

extern "C" int improv_wifi_handle_serial(const uint8_t *data, size_t length) {
  if (c != NULL) {
    c->handleSerial(data, length);

    return 0;
  }

  return -1;
}

extern "C" void improv_wifi_set_device_info(uint8_t chipFamily,
                                            const char *firmwareName,
                                            const char *firmwareVersion,
                                            const char *deviceName,
                                            const char *deviceUrl) {
  if (c != NULL) {
    c->setDeviceInfo((ImprovTypes::ChipFamily)chipFamily, firmwareName,
                     firmwareVersion, deviceName, deviceUrl);
  }
}

extern "C" void improv_wifi_serialWrite(void *serWriteCb) {
  ImprovWiFi::SerialWrite *cb = (ImprovWiFi::SerialWrite *)serWriteCb;
  if (c != NULL) {
    c->serialWrite(cb);
  }
}

extern "C" void improv_wifi_onImprovError(void *onImprovErrorCb) {
  ImprovWiFi::OnImprovError *cb = (ImprovWiFi::OnImprovError *)onImprovErrorCb;
  if (c != NULL) {
    c->onImprovError(cb);
  }
}

extern "C" void improv_wifi_onImprovConnected(void *onImprovConnectedCb) {
  ImprovWiFi::OnImprovConnected *cb =
      (ImprovWiFi::OnImprovConnected *)onImprovConnectedCb;
  if (c != NULL) {
    c->onImprovConnected(cb);
  }
}

extern "C" void improv_wifi_setCustomConnectWiFi(void *setCustomConnectWiFiCb) {
  ImprovWiFi::CustomConnectWiFi *cb =
      (ImprovWiFi::CustomConnectWiFi *)setCustomConnectWiFiCb;
  if (c != NULL) {
    c->setCustomConnectWiFi(cb);
  }
}

extern "C" void improv_wifi_setCustomScanWiFi(void *setCustomScanWiFiCb) {
  ImprovWiFi::CustomScanWiFi *cb =
      (ImprovWiFi::CustomScanWiFi *)setCustomScanWiFiCb;
  if (c != NULL) {
    c->setCustomScanWiFi(cb);
  }
}

extern "C" void improv_wifi_setCustomIsConnected(void *setCustomIsConnected) {
  ImprovWiFi::CustomIsConnected *cb =
      (ImprovWiFi::CustomIsConnected *)setCustomIsConnected;
  if (c != NULL) {
    c->setCustomisConnected(cb);
  }
}

extern "C" void improv_wifi_setCustomGetLocalIpCallback(
    void *getLocalIpCallback) {
  ImprovWiFi::CustomGetLocalIpCallback *cb =
      (ImprovWiFi::CustomGetLocalIpCallback *)getLocalIpCallback;
  if (c != NULL) {
    c->setCustomGetLocalIpCallback(cb);
  }
}
