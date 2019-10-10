int readDavis();

#ifdef READ_WEATHER_DAVIS_8_ENABLED
#include <Arduino.h>

typedef struct t_DAVIS_LORA_OUT
{
    uint8_t msg_type; // 07
    uint8_t msg_length;
} DAVIS_LORA_OUT;

RTC_DATA_ATTR DAVIS_LORA_OUT DavisLoraOut; // result in static memory
#endif                                     //READ_WEATHER_DAVIS_8_ENABLED
