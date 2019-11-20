#ifndef MAIN_H
#define MAIN_H

#include "hsdavis.h"
extern bool clear_to_sleep; // LoRa send can prevent sleep
extern RTC_DATA_ATTR byte bootCount;

#ifdef SEND_DATA_LORA_2_ENABLED
extern uint8_t NWKSKEY[16];
extern PROGMEM uint8_t APPSKEY[16];
extern uint32_t DEVADDR;
extern const unsigned TX_INTERVAL;
#endif

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
  restart,
  MAX_TASK_COUNT // Do not assign values manually and this must be last
};

void schedule_next_task_run(int task_number, time_t to_next_run_sec, bool from_now);

#if defined SEND_DATA_LORA_2_ENABLED || defined SEND_DATA_WIFI_3_ENABLED
struct t_EXTERNAL_VOLTAGE_OUT
{
  uint8_t msg_type; // 09
  uint8_t msg_ver; // 00
  float voltage;
};

struct DATA_OUT
{
#ifdef READ_WEATHER_DAVIS_8_ENABLED
t_DavisDATA davisData; // Add Davis data packet
#endif // READ_WEATHER_DAVIS_8_ENABLED

#ifdef READ_EXTERNAL_VOLTAGE_9_ENABLED
t_EXTERNAL_VOLTAGE_OUT externalVoltageData;  // Add external volltage data packet
#endif // READ_EXTERNAL_VOLTAGE_9_ENABLE
};

extern struct DATA_OUT DataOut; // result in static memory
#endif // defined SEND_DATA_LORA_2_ENABLED || defined SEND_DATA_WIFI_3_ENABLED

#endif //main_h