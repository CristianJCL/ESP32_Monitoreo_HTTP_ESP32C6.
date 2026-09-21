#ifndef MONITOR_LOGIC_H
#define MONITOR_LOGIC_H

#include <stdbool.h>
#include <stddef.h>

bool build_thingspeak_payload(char *buffer, size_t buffer_size,
                              const char *api_key, int rssi,
                              unsigned long uptime_seconds);

bool thingspeak_response_is_success(const char *response);

#endif
