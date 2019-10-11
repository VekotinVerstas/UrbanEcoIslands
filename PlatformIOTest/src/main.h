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

bool clear_to_sleep = true; // LoRa send can prevent sleep