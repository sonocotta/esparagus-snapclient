/*
 * wifi_provisioning.c
 *
 *  Created on: Apr 28, 2024
 *      Author: karl
 */

#include "wifi_provisioning.h"

#include <string.h>

#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "improv_wrapper.h"
#include "wifi_interface.h"

#define TAG "IMPROV"

#define RD_BUF_SIZE (UART_FIFO_LEN)
#define PATTERN_CHR_NUM (3)

static TaskHandle_t t_improv_task = NULL;

static const int uart_buffer_size = 2 * RD_BUF_SIZE;
static QueueHandle_t uart0_queue;

void uart_event_handler(void) {
  uart_event_t event;
  uint8_t dtmp[RD_BUF_SIZE];
  size_t buffered_size;

  // Waiting for UART event.
  if (xQueueReceive(uart0_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
    bzero(dtmp, RD_BUF_SIZE);
    // ESP_LOGI(TAG, "uart[%d] event:", UART_NUM_0);
    switch (event.type) {
      // Event of UART receving data
      /*We'd better handler data event fast, there would be much more data
      events than other types of events. If we take too much time on data event,
      the queue might be full.*/
      case UART_DATA:
        // ESP_LOGI(TAG, "[UART DATA]: %d", event.size);

        uart_read_bytes(UART_NUM_0, dtmp, event.size, portMAX_DELAY);
        //	ESP_LOGI(TAG, "[DATA EVT]:");

        improv_wifi_handle_serial(dtmp, event.size);
        break;
      // Event of HW FIFO overflow detected
      case UART_FIFO_OVF:
        // ESP_LOGI(TAG, "hw fifo overflow");

        // If fifo overflow happened, you should consider adding flow control
        // for your application. The ISR has already reset the rx FIFO, As an
        // example, we directly flush the rx buffer here in order to read more
        // data.
        uart_flush_input(UART_NUM_0);
        xQueueReset(uart0_queue);
        break;
      // Event of UART ring buffer full
      case UART_BUFFER_FULL:
        // ESP_LOGI(TAG, "ring buffer full");
        // If buffer full happened, you should consider increasing your buffer
        // size As an example, we directly flush the rx buffer here in order to
        // read more data.
        uart_flush_input(UART_NUM_0);
        xQueueReset(uart0_queue);
        break;
      // Others
      default:
        // ESP_LOGI(TAG, "uart event type: %d", event.type);
        break;
    }
  }
}

static void improv_task(void *pvParameters) {
  while (1) {
    uart_event_handler();
  }
}

void uart_write(const unsigned char *txData, int length) {
  uart_write_bytes(UART_NUM_0, txData, length);
}

void improv_wifi_scan(unsigned char *scanResponse, int bufLen,
                      uint16_t *count) {
  uint16_t number = 16;
  wifi_ap_record_t ap_info[16];

  memset(ap_info, 0, sizeof(ap_info));

  if (esp_wifi_scan_start(NULL, true) == ESP_ERR_WIFI_STATE) {
    wifi_ap_record_t ap_info_tmp;

    do {
      esp_wifi_disconnect();
      vTaskDelay(pdMS_TO_TICKS(500));
    } while (esp_wifi_sta_get_ap_info(&ap_info_tmp) !=
             ESP_ERR_WIFI_NOT_CONNECT);

    esp_wifi_scan_start(NULL, true);
  }
  //  ESP_LOGI(TAG, "Max AP number ap_info can hold = %u", number);
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(count));
  //  ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds =
  //  %u",
  //           *count, number);

  scanResponse[0] = 0;
  for (int i = 0; i < number; i++) {
    char rssiStr[8] = {
        0,
    };
    char cipherStr[8] = {
        0,
    };
    uint16_t neededLen;

    itoa(ap_info[i].rssi, rssiStr, 10);
    if (ap_info[i].authmode != WIFI_AUTH_OPEN) {
      strcat(cipherStr, "YES");
    } else {
      strcat(cipherStr, "NO");
    }
    neededLen = strlen((const char *)ap_info[i].ssid) + strlen(rssiStr) +
                strlen(cipherStr) + 3;

    if ((bufLen - neededLen) > 0) {
      strcat((char *)scanResponse, (char *)ap_info[i].ssid);
      strcat((char *)scanResponse, (char *)",");
      strcat((char *)scanResponse, (char *)rssiStr);
      strcat((char *)scanResponse, (char *)",");
      strcat((char *)scanResponse, (char *)cipherStr);
      strcat((char *)scanResponse, (char *)"\n");

      bufLen -= neededLen;
    }
  }

  //  ESP_LOGI(TAG, "APs \t\t%s", scanResponse);
}

bool improv_wifi_connect(const char *ssid, const char *password) {
  uint8_t count = 0;
  wifi_ap_record_t apRec;
  esp_err_t err;

  while ((err = esp_wifi_sta_get_ap_info(&apRec)) != ESP_ERR_WIFI_NOT_CONNECT) {
    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  wifi_config_t wifi_config;
  ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifi_config));
  strcpy((char *)wifi_config.sta.ssid, ssid);
  strcpy((char *)wifi_config.sta.password, password);
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  esp_wifi_connect();
  while (esp_wifi_sta_get_ap_info(&apRec) != ESP_OK) {
    vTaskDelay(pdMS_TO_TICKS(500));
    if (count > 20) {
      esp_wifi_disconnect();
      return false;
    }
    count++;
  }

  return true;
}

bool improv_wifi_is_connected(void) {
  wifi_ap_record_t apRec;

  if (esp_wifi_sta_get_ap_info(&apRec) == ESP_OK) {
    //    	printf("connected\n");

    return true;
  }

  //    printf("NOT connected\n");

  return false;
}

void improv_wifi_get_local_ip(uint8_t *address) {
  esp_netif_ip_info_t ip_info;

  // TODO: find a better way to do this
  do {
    esp_netif_get_ip_info(get_current_netif(), &ip_info);
    vTaskDelay(pdMS_TO_TICKS(200));
  } while (ip_info.ip.addr == 0);

  address[0] = ip_info.ip.addr >> 0;
  address[1] = ip_info.ip.addr >> 8;
  address[2] = ip_info.ip.addr >> 16;
  address[3] = ip_info.ip.addr >> 24;

  // ESP_LOGI(TAG, "%d.%d.%d.%d", address[0], address[1], address[2],
  // address[3]);
}

void improv_init(void) {
  uint8_t webPortStr[6] = {0};
  uint16_t webPort = CONFIG_WEB_PORT;
  uint8_t urlStr[26] = "http://{LOCAL_IPV4}:";

  utoa(webPort, (char *)webPortStr, 10);
  strcat((char *)urlStr, (char *)webPortStr);

  improv_wifi_create();
  improv_wifi_serialWrite(uart_write);
  improv_wifi_set_device_info(CF_ESP32, "esp32_snapclient", "0.0.3",
                              "snapclient", (const char *)urlStr);

  improv_wifi_setCustomConnectWiFi(improv_wifi_connect);
  improv_wifi_setCustomScanWiFi(improv_wifi_scan);
  improv_wifi_setCustomIsConnected(improv_wifi_is_connected);
  improv_wifi_setCustomGetLocalIpCallback(improv_wifi_get_local_ip);

  // Set UART pins(TX: IO4, RX: IO5, RTS: IO18, CTS: IO19)
  // ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, 1, 3, -1, -1));

  // Install UART driver using an event queue here
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, uart_buffer_size,
                                      uart_buffer_size, 10, &uart0_queue, 0));

  xTaskCreatePinnedToCore(&improv_task, "improv", 4 * 1024, NULL, 4,
                          &t_improv_task, tskNO_AFFINITY);
}

void improv_deinit(void) {
  if (t_improv_task) {
    vTaskDelete(t_improv_task);
    uart_driver_delete(UART_NUM_0);

    t_improv_task = NULL;
  }
  improv_wifi_destroy();
}
