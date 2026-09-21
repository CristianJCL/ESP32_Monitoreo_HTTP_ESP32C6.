#include "monitor_logic.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

bool build_thingspeak_payload(char *buffer, size_t buffer_size,
                              const char *api_key, int rssi,
                              unsigned long uptime_seconds)
{
    if (buffer == NULL || buffer_size == 0 || api_key == NULL || api_key[0] == '\0') {
        return false;
    }

    int written = snprintf(buffer, buffer_size,
                           "api_key=%s&field1=%d&field2=%lu",
                           api_key, rssi, uptime_seconds);
    return written >= 0 && (size_t)written < buffer_size;
}

bool thingspeak_response_is_success(const char *response)
{
    if (response == NULL) {
        return false;
    }

    while (isspace((unsigned char)*response)) {
        response++;
    }

    char *end = NULL;
    long entry_id = strtol(response, &end, 10);
    if (response == end || entry_id <= 0) {
        return false;
    }

    while (isspace((unsigned char)*end)) {
        end++;
    }
    return *end == '\0';
}
