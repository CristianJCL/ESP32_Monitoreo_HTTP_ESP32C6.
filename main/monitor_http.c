#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_crt_bundle.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "monitor_logic.h"

static const char *TAG = "cloud_monitor";

typedef struct {
    char *buffer;
    size_t capacity;
    size_t length;
} http_response_t;

static esp_err_t http_event_handler(esp_http_client_event_t *event)
{
    http_response_t *response = (http_response_t *)event->user_data;

    if (event->event_id == HTTP_EVENT_ON_DATA && response != NULL &&
        event->data_len > 0 && response->length < response->capacity - 1) {
        size_t available = response->capacity - response->length - 1;
        size_t copy_size = (size_t)event->data_len < available
                               ? (size_t)event->data_len
                               : available;
        memcpy(response->buffer + response->length, event->data, copy_size);
        response->length += copy_size;
        response->buffer[response->length] = '\0';
    }
    return ESP_OK;
}

static void initialize_nvs(void)
{
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);
}

static esp_err_t get_wifi_rssi(int *rssi)
{
    wifi_ap_record_t access_point = {0};
    esp_err_t result = esp_wifi_sta_get_ap_info(&access_point);
    if (result == ESP_OK) {
        *rssi = access_point.rssi;
    }
    return result;
}

static esp_err_t send_cloud_sample(int rssi, unsigned long uptime_seconds)
{
    char payload[192] = {0};
    char response_buffer[64] = {0};
    http_response_t response = {
        .buffer = response_buffer,
        .capacity = sizeof(response_buffer),
        .length = 0,
    };

    if (!build_thingspeak_payload(payload, sizeof(payload),
                                  CONFIG_CLOUD_WRITE_API_KEY,
                                  rssi, uptime_seconds)) {
        ESP_LOGE(TAG, "No fue posible construir el payload HTTP");
        return ESP_ERR_INVALID_ARG;
    }

    esp_http_client_config_t config = {
        .url = CONFIG_CLOUD_UPDATE_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .event_handler = http_event_handler,
        .user_data = &response,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        return ESP_FAIL;
    }

    esp_http_client_set_header(client, "Content-Type",
                               "application/x-www-form-urlencoded");
    esp_http_client_set_post_field(client, payload, strlen(payload));

    esp_err_t result = esp_http_client_perform(client);
    if (result == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200 && thingspeak_response_is_success(response_buffer)) {
            ESP_LOGI(TAG,
                     "Muestra enviada: RSSI=%d dBm, uptime=%lu s, entrada=%s",
                     rssi, uptime_seconds, response_buffer);
        } else {
            ESP_LOGE(TAG, "Respuesta no valida: HTTP %d, cuerpo='%s'",
                     status_code, response_buffer);
            result = ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "Error en HTTP POST: %s", esp_err_to_name(result));
    }

    esp_http_client_cleanup(client);
    return result;
}

void app_main(void)
{
    initialize_nvs();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Conectando a la red Wi-Fi configurada");
    ESP_ERROR_CHECK(example_connect());

    esp_netif_t *station = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ip_info = {0};
    if (station != NULL && esp_netif_get_ip_info(station, &ip_info) == ESP_OK) {
        ESP_LOGI(TAG, "Direccion IP: " IPSTR, IP2STR(&ip_info.ip));
    }

    if (CONFIG_CLOUD_WRITE_API_KEY[0] == '\0') {
        ESP_LOGE(TAG,
                 "Configure la Write API Key en menuconfig antes de ejecutar");
        return;
    }

    while (true) {
        int rssi = 0;
        if (get_wifi_rssi(&rssi) == ESP_OK) {
            unsigned long uptime_seconds =
                (unsigned long)(esp_timer_get_time() / 1000000ULL);
            send_cloud_sample(rssi, uptime_seconds);
        } else {
            ESP_LOGW(TAG, "No se pudo leer el RSSI de la conexion Wi-Fi");
        }

        vTaskDelay(pdMS_TO_TICKS(CONFIG_CLOUD_SEND_INTERVAL_SECONDS * 1000));
    }
}
