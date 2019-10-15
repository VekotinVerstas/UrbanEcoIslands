#ifndef SETTINGS_H
#define SETTINGS_H

#include "tasksdefine.h"
#include "identity.h"
#include <lmic.h>
#include <hal/hal.h>

#define MIN_SLEEPING_TIME_SECS 10   // if next activity is later than this amount of secs -> do sleep, otherwise just wait loop
#define MAX_SLEEPING_TIME_SECS 3600 // do not sleep longer than this

// next start time of each task is in array in non-volatile memory
extern const unsigned TX_INTERVAL = 60; // LoRa send interval in seconds.
// Airtime counter https://www.loratools.nl/#/airtime

// These settings seems to work with TTGO LORA OLED
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,

    //For board revision V1.5 use GPIO12 for LoRa RST
    //.rst = 12,
    //For board revision(s) newer than V1.5 use GPIO19 for LoRa RST
    .rst = 19,
    .dio = {26, 33, 32}};

#define EXTERNAL_VOLTAGE_9_GPIO 34
#define EXTERNAL_VOLTAGE_9_FACTOR 17.368

#define RESTART_INTERVAL 86400
#endif //SETTINGS_H