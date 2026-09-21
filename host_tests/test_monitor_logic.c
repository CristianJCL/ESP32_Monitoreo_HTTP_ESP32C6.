#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "monitor_logic.h"

int main(void)
{
    char payload[128];

    assert(build_thingspeak_payload(payload, sizeof(payload), "ABC123", -57, 125));
    assert(strcmp(payload, "api_key=ABC123&field1=-57&field2=125") == 0);
    assert(!build_thingspeak_payload(payload, sizeof(payload), "", -57, 125));

    char small_buffer[8];
    assert(!build_thingspeak_payload(small_buffer, sizeof(small_buffer),
                                    "ABC123", -57, 125));

    assert(thingspeak_response_is_success("42"));
    assert(thingspeak_response_is_success("  42\n"));
    assert(!thingspeak_response_is_success("0"));
    assert(!thingspeak_response_is_success(""));
    assert(!thingspeak_response_is_success("error"));

    puts("Pruebas de logica superadas.");
    return 0;
}
