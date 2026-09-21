idf_component_register(
    SRCS "monitor_http.c" "monitor_logic.c"
    INCLUDE_DIRS "."
    REQUIRES esp_event esp_http_client esp_netif esp_timer esp_wifi mbedtls nvs_flash protocol_examples_common
)
