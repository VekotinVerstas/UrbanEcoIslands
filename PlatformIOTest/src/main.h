#ifndef MAIN_H
#define MAIN_H
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
#endif //main_h