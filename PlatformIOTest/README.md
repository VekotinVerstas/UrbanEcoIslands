This is a - development in process - template for a ESP32 based modulat sensor software.

Main development ideas:
- There are several types of tasks, e.g.:
  - sensor read (temp/humidity, air quality, bat activity...)
  - send information, LoRa, WiFi
  - actuator tasks, e.g. switch power on/off on certain circuit if battery voltage is high/low
  - utility tasks, e.g. read time from ntp-server, OTA software update
- Next execution time of each task is stored in the global array (next_run_time) in the non-volatile memory 
- A task can set its' next execution time (next read in 60 secs) or it can be set by another task (e.g. sensor read can trigger LoRa transmit)
- After processing all tasks due, the program goes to deep-sleep-state (if enabled) to save battery
- Enabled tasks are selected (in settings.h) with preprocesser directives #define / #undef . This keep keep compiled package 
as compact as possible. Unfortunately makes source code harder to read.
- Global variables (also task results) are stored in non-volatile memory (RTC_DATA_ATTR 4k) which keeps data during sleep periods.
- Incomplete LoRa transmit blocks deep-sleep
- In the loop-function tasks are executed, if they are due (next_run_time[task number] <= current time ) . Generally sensor tasks
are before transmitting and actuator tasks (first we try to get all data in the memory)

