#ifndef SETTINGS_H
#define SETTINGS_H

#include "identity.h"

#define SLEEP_ENABLED // do we sleep in the end of loop
#define DEBUG_ENABLED // debug print

#define MIN_SLEEPING_TIME_SECS 10   // if next activity is later than this amount of secs -> do sleep, otherwise just wait loop
#define MAX_SLEEPING_TIME_SECS 3600 // do not sleep longer than this

enum task 
{ 
    scan_wifi, 
    scan_ble, 
    send_data_lora,
    send_data_wifi,
    ota_update,
    read_temp_hum_bme280,
    syncronize_ntp_time,
    read_weather_davis,
    read_external_volt,
    restart 
};

// next start time of each task is in array in non-volatile memory
#undef DEVICE_SCAN_WIFI_0_ENABLED

#undef DEVICE_SCAN_BLE_1_ENABLED

#define SEND_DATA_LORA_2_ENABLED
const unsigned TX_INTERVAL = 180; // LoRa send interval in seconds.
// Airtime counter https://www.loratools.nl/#/airtime

#undef SEND_DATA_WIFI_3_ENABLED

#undef OTA_UPDATE_4_ENABLED

#undef READ_TEMP_HUM_BME280_5_ENABLED
#define BME280_I2C 0x76

#undef READ_HTU21D_6_ENABLED

#undef SYNCRONIZE_NTP_TIME_7_ENABLED

#define READ_WEATHER_DAVIS_8_ENABLED

#undef READ_EXTERNAL_VOLTAGE_9_ENABLED
#define EXTERNAL_VOLTAGE_9_GPIO 34
#define EXTERNAL_VOLTAGE_9_FACTOR 17.368

#define RESTART_10_ENABLED
#define RESTART_INTERVAL 86400
#endif //SETTINGS_H