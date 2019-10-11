#ifndef SETTINGS_H
#define SETTINGS_H

#include "tasksdefine.h"
#include "identity.h"

#define SLEEP_ENABLED // do we sleep in the end of loop
#define DEBUG_ENABLED // debug print

#define MIN_SLEEPING_TIME_SECS 10   // if next activity is later than this amount of secs -> do sleep, otherwise just wait loop
#define MAX_SLEEPING_TIME_SECS 3600 // do not sleep longer than this

// next start time of each task is in array in non-volatile memory
const unsigned TX_INTERVAL = 180; // LoRa send interval in seconds.
// Airtime counter https://www.loratools.nl/#/airtime

#define EXTERNAL_VOLTAGE_9_GPIO 34
#define EXTERNAL_VOLTAGE_9_FACTOR 17.368

#define RESTART_INTERVAL 86400
#endif //SETTINGS_H