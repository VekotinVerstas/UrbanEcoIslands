#include <Arduino.h>
// tässä määritellään mitkä taskit käytössä
#include "settings.h"

#include "esp_system.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#define MAX_TASK_COUNT 20

// next start time of each task, initiated in startup

RTC_DATA_ATTR time_t next_run_time[MAX_TASK_COUNT];

#define UNDEFINED_TIME -1

#include "hslora.h"

#ifdef READ_TEMP_HUM_BME280_5_ENABLED
#include <hsbme280.h>
HSbme280 HSBME280Sensor;
RTC_DATA_ATTR HSbme280Data HSBME280SensorData;
#endif //READ_TEMP_HUM_BME280_5_ENABLED

#ifdef READ_HTU21D_6_ENABLED
#include <hshtu21d.h>
HShtu21d HShtu21dSensor;
RTC_DATA_ATTR HShtu21dData HSHTU21DSensorData;
#endif //READ_HTU21D_6_ENABLED

#if defined(SEND_DATA_WIFI_3_ENABLED) || defined(OTA_UPDATE_4_ENABLED) || defined(SYNCRONIZE_NTP_TIME_7_ENABLED)
#define WIFI_REQUIRED
#endif

#ifdef WIFI_REQUIRED
// wifi käytössä
// tähän wifin vaatimat includet ja vakio/muuttujamääritykset
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiType.h>
WiFiMulti wifiMulti;
#endif //WIFI_REQUIRED

#ifdef OTA_UPDATE_4_ENABLED
#include "HSOTA.H"                 // tässä viittaus tiedostoon, jossa  ota-päivitykseen liittyvät funktiot
#define OTA_UPDATE_INTERVAL_MIN 15 // Wifi ota update is tried this often, suurin osa on turhia yrityksiä, mutta jos joku odottaa hotspot-kännykän kanssa updatea on odottavan aika pitkä. Jos tätä tarvetta ei ole voisi olla vaikka 1 vrk välein
next_run_time[4] = 0;
#endif //OTA_UPDATE_4_ENABLED

#ifdef READ_WEATHER_DAVIS_8_ENABLED
#include "hsdavis.h"
HSDavis WeatherStation;
RTC_DATA_ATTR DAVIS_LORA_OUT DavisLoraOut; // result in static memory
#endif                                     //READ_WEATHER_DAVIS_8_ENABLED

#ifdef SYNCRONIZE_NTP_TIME_7_ENABLED
#include "hstimesync.h"
HSTimeSync TimeSync;
// see https://forum.arduino.cc/index.php?topic=595802.0 for code
const char *ntpServer = "pool.ntp.org";
#endif //SYNCRONIZE_NTP_TIME_7_ENABLED

#ifdef READ_EXTERNAL_VOLTAGE_9_ENABLED

typedef struct t_EXTERNAL_VOLTAGE_OUT
{
  uint8_t msg_type; // 09
  uint8_t msg_length;
  float voltage;
} EXTERNAL_VOLTAGE_OUT;
RTC_DATA_ATTR float externalVoltage; // result in static memory

#endif

//END OF TASK  SPECIFIC DEFINITIONS

RTC_DATA_ATTR byte bootCount = 0;
RTC_DATA_ATTR time_t now;
RTC_DATA_ATTR uint64_t Mics = 0;
RTC_DATA_ATTR struct tm *timeinfo;

#ifdef WIFI_REQUIRED
bool connectWifi()
{
  if (wifiMulti.run() == WL_CONNECTED)
  {
    return true; // already connected
  }
  wifiMulti.addAP(AP1_SSID, AP1_PWD);
#ifdef AP2_SSID
  wifiMulti.addAP(AP2_SSID, AP2_PWD);
#endif
#ifdef AP3_SSID
  wifiMulti.addAP(AP3_SSID, AP3_PWD);
#endif
#ifdef AP4_SSID
  wifiMulti.addAP(AP4_SSID, AP4_PWD);
#endif

  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED)
  {
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  return false;
}
#endif // WIFI_REQUIRED

void printLocalTime()
{
  time(&now);
  timeinfo = localtime(&now);
  Serial.printf("%s\n", asctime(timeinfo));
  delay(2); // 26 bytes @ 115200 baud is less than 2 ms
}
/*
void updateTime(uint64_t elapsedTime)
{ // elapsedTime in us
  if (elapsedTime == 0)
    Mics += micros();
  else
    Mics += elapsedTime;
  if (Mics > 1000000)
  {
    Mics = Mics % 1000000;
    now += Mics / 1000000;
  }
}
*/
#define SCL_PIN 22
#define SDA_PIN 21

bool time_to_run_task(int task_number)
{
  time_t next_run_time_task = next_run_time[task_number];

  time(&now);
  if ((next_run_time_task <= now))
  {
    Serial.printf("Task %d was due %ld secs ago (now %ld, due %ld)\n", task_number, now - next_run_time_task, now, next_run_time_task);
    return true;
  }
  else
  {
    Serial.printf("Task %d is due after %ld secs \n", task_number, next_run_time_task - now);

    return false;
  }
};

void schedule_next_task_run(int task_number, time_t to_next_run_sec, bool from_now)
{
  time_t now_local;
  time(&now_local);
  if (from_now)
  {
    next_run_time[task_number] = now_local + to_next_run_sec;
  }
  else
  {
    if (next_run_time[task_number] < 1)
    {
      next_run_time[task_number] = now_local; // if undefined/0, initiate
    }
    next_run_time[task_number] += to_next_run_sec;

    if (next_run_time[task_number] < now_local)
    { //do not let tasks accumulate
      next_run_time[task_number] = now_local;
    }
  }
}

void onEvent(ev_t ev)
{
  if (ev == EV_TXCOMPLETE)
  {
    // Schedule nexvt transmission
    //os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
    Serial.printf("LoRa EV_TXCOMPLETE, next send in %d seconds\n", TX_INTERVAL);

    schedule_next_task_run(2, TX_INTERVAL, false);
    clear_to_sleep = true; // buffer clear, sleep ok
  }
}

// from https://stackoverflow.com/questions/27288021/formula-to-calculate-dew-point-from-temperature-and-humidity/27289801
float dewPoint(float humi, float temp)
{
  return (temp - (14.55 + 0.114 * temp) * (1 - (0.01 * humi)) - pow(((2.5 + 0.007 * temp) * (1 - (0.01 * humi))), 3) - (15.9 + 0.117 * temp) * pow((1 - (0.01 * humi)), 14));
}

//BME280 @0x76 !

/* 
yleiskysymyksiä 
- voisko loralla saada reaaliajan ,jos odotetaan loraviestejä niin silloin nukkumaan ei voi mennä kovin usein. OTAn yhteydessä voisi hakea ajan wifillä.
- missä säilytetään data sleepin aikana, esim. RTC_DATA_ATTR int bootCount = 0; , RTC-muisti (4k) ei dynaaminen, mutta struct onnistuu https://esp32.com/viewtopic.php?t=1536
- tässä mallissa ei oikein moniajoa, vaan esim. wifi skannaus varaa koko devicen, onko lopulta ongelma

taskin ominaisuudet:
- taskeilla oltava ajojärjestys (nyt loopissa oleva järjestys) , jos riippuvat toisistaan, esim. jos bat-counter toimii vain lämpimänä yönä, pitää lämpö ja valo sensoroida ennen 
- frequency (esim. 300 s), taski voi määritellä seuraavan ajoajan itselleen muun sensoridata (pitää olla luettuna) perusteella, esim. harvemmin, jos jännite alhaalla
- taski voi määrittää myös toisen taskin seuraavan ajoajan, esim. pax-counter onnistuneen skannauksen jälkeen "käskee" lähettämään datan

esimerkkitaskeja:
read_battery_voltage - jos käytetään erilaisia akkuja (lion, lyijy 6v jne) niin tulos voisi olla vakioitu luokkiin low/normal/high
read_temp_hum_bme280 - lukee bme280 sensorilta lämpötilan ja kosteuden globaaliin muuttujaan/tietorakenteeseen
read_chimney_temp
read_water_temp
read_water_distance - luetaan etäisyys veteen eli 
read_lightness_xyz - lukee tietyllä sensorilla valoisuuden globaaliin muuttujaan
listen_bat_sounds - kuuntelee annetun ajan UÄ-sensorilla lepakkojen aktiivisuutta 
scan_pax_wifi - skannaa wifi-laitteet
scan_pax_ble - skannaa ble-laitteet
scan_battery_charger - nykyinen Vasikkasaaren Victronin luku voisi olla oma moduulinsa, sieltä pukkaan rivejä tasaiseen tahtiin

set_power_switches - tämä voisi muuttaa relekytkinten asentoa sensoriarvojen perusteella, esim. jos jännite alhaalla otetaan jostain virta pois, tai ohjataan tuuletinta lämpötilan/kosteuden perusteella

send_sensordata_lora kasaa sensoroidut datat ja lähettää paketin loralla
send_sensordata_wifi ... wifillä


*/

int battery_voltage; // globaali, johon luetaan voltage sopivin (?) määrävälein

time_t time_to_earliest_run()
{
  int next_task_to_run = -1;
  // loop all tasks and return earliest run time
  time_t earliest_run_time = LONG_MAX;
  for (int i = 0; i < MAX_TASK_COUNT; i++)
  {
    if ((next_run_time[i] > 0) && (next_run_time[i] < earliest_run_time))
    {
      earliest_run_time = next_run_time[i];
      next_task_to_run = i;
      //  Serial.printf("earliest_run_time task %d = %ld\n", i, earliest_run_time);
    }
  }
  if (earliest_run_time == UNDEFINED_TIME)
  {
    time_t now_l;
    time(&now_l);
    earliest_run_time = now_l + MIN_SLEEPING_TIME_SECS;
    //  Serial.printf("earliest_run_time =MIN_SLEEPING_TIME_SECS \n");
  }
  Serial.printf("earliest_run_time %ld, task %d\n", earliest_run_time,next_task_to_run );
  return earliest_run_time;
}

void setup()
{
  Serial.begin(115200);
  delay(20);
  if (bootCount == 0)
  {
    for (int i = 0; i < MAX_TASK_COUNT; i++)
    {
      next_run_time[i] = UNDEFINED_TIME;
    }
    // disable ble, we do not need it (yet)
    esp_bluedroid_disable();
    esp_bt_controller_disable(); 
#ifdef RESTART_10_ENABLED
    schedule_next_task_run(10, RESTART_INTERVAL, true);
#endif
  }

  Serial.print("bootCount:");
  Serial.println(bootCount);
  printLocalTime();

#ifdef READ_WEATHER_DAVIS_8_ENABLED
#include "hsdavis.h"
  Serial1.begin(19200, SERIAL_8N1, 14, 13); // Davis
#endif                                      //READ_WEATHER_DAVIS_8_ENABLED

hslora_setup();  // siirrä koodi omiin fileihin
}

void loop()
{
#ifdef SEND_DATA_LORA_2_ENABLED
  os_runloop_once();
#endif

#ifdef SYNCRONIZE_NTP_TIME_7_ENABLED
  // Tämä viedään myöhemmin omaan luokkaan HSTimeSync
  if (time_to_run_task(7))
  {
    //connect to WiFi
    int wifiCounter = 0;
    bool connected = connectWifi();

    while (!connected)
    {
      delay(500);
      Serial.print(".");
      if (++wifiCounter > 30)
      {
        //ESP.restart(); // restart after 15 s
        break;
      }
    }
    if (connected)
    {
      Serial.println(" CONNECTED");
      //init and get the time
      //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      configTime(0, 0, ntpServer);
      delay(1000);
      Serial.println(" Got new time");
      printLocalTime();
    }
    else
    {
      Serial.println("Cannot connect wifi, no ntp time update");
    }
    schedule_next_task_run(7, 120, false);
  }

#endif //SYNCRONIZE_NTP_TIME_7_ENABLED

#ifdef READ_WEATHER_DAVIS_8_ENABLED
  // tähän luku, varmaan yritetään saada omaksi luokaksi
  // mieti saisiko tästä yleisen lorabufferin kasvatuksen. Jos kaksi eri taskia kasvattaa bufferia,
  // niin miten määritellään miten kasataan bufferiin. Olisiko lora-lähetyksessä vielä iffittelyt ja datapaketin kasaaminen kustakin lukutaskista.
  // tarvittaessa
  if (time_to_run_task(8))
  {
    Serial.printf("Read Davis here...\n");
    // muista myös liipaista lora-lähetys tyyliin schedule_next_task_run(2, 0,true);
    schedule_next_task_run(8, 41, false);
  }
#endif //READ_WEATHER_DAVIS_8_ENABLED

#ifdef READ_TEMP_HUM_BME280_5_ENABLED

  HSBME280SensorData = HSBME280Sensor.read_sensor_values();
  Serial.printf("BME280 temperature %f, humidity %f, pressure %f\n", HSBME280SensorData.temperature, HSBME280SensorData.humidity, HSBME280SensorData.pressure);
  delay(10000); // toistaiseksi näin
#endif          //READ_TEMP_HUM_BME280_5_ENABLED

#ifdef READ_HTU21D_6_ENABLED
  HSHTU21DSensorData = HShtu21dSensor.read_sensor_values();
  Serial.printf("HTU temperature %f, humidity %f, dew point %f\n", HSHTU21DSensorData.temperature, HSHTU21DSensorData.humidity, dewPoint(HSHTU21DSensorData.humidity, HSHTU21DSensorData.temperature));
  delay(10000); // toistaiseksi näin
#endif          //READ_HTU21D_6_ENABLED

#ifdef READ_EXTERNAL_VOLTAGE_9_ENABLED
  // EXTERNAL_VOLTAGE_9_GPIO 21
  // EXTERNAL_VOLTAGE_9_FACTOR
  if (time_to_run_task(9))
  {
    pinMode(EXTERNAL_VOLTAGE_9_GPIO, INPUT); // setup Voltmeter
    int val1, val2, val3;
    delay(1000);
    val1 = analogRead(EXTERNAL_VOLTAGE_9_GPIO);
    // Serial.println("Voltage val1");
    // Serial.println(val1);
   /* delay(100);
    val2 = analogRead(EXTERNAL_VOLTAGE_9_GPIO);
    delay(100);
    val3 = analogRead(EXTERNAL_VOLTAGE_9_GPIO);*/

    externalVoltage = (float) (((val1) * EXTERNAL_VOLTAGE_9_FACTOR)  / 4095);
    //Serial.println(externalVoltage);
    schedule_next_task_run(9, TX_INTERVAL, false); // same interval as lora
  }
#endif //READ_EXTERNAL_VOLTAGE_9_ENABLED

#ifdef DEVICE_SCAN_WIFI_0_ENABLED

  // käynnistä pax-wifi-scan jos käynnistysaika koittanut
  if (time_to_run_task(4))
  {
    //  kutsu skannausfuntiota, joka määritelty omassa
  }
#endif //DEVICE_SCAN_WIFI_0_ENABLED

#ifdef SEND_DATA_LORA_2_ENABLED
  if (time_to_run_task(2))
  {
    Serial.println("do_send");
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND)
    {
      Serial.println("OP_TXRXPEND, not sending");
    }
    else
    {
      // Prepare upstream data transmission at the next possible time.
// TODO: Kun tulee  paketteja, niin koostetaan koko paketti bufferiin
#ifdef READ_WEATHER_DAVIS_8_ENABLED
      // tähän joku sopiva konversio
      //
      LMIC_setTxData2(2, (unsigned char *)&DavisLoraOut, sizeof(DavisLoraOut), 0);

#endif //READ_WEATHER_DAVIS_8_ENABLED
#ifdef READ_EXTERNAL_VOLTAGE_9_ENABLED
      EXTERNAL_VOLTAGE_OUT voltageOut;
      voltageOut.msg_type = 9;

      voltageOut.msg_length = sizeof(EXTERNAL_VOLTAGE_OUT);
      voltageOut.voltage = externalVoltage;
     
      Serial.printf("Sending voltage: %f\n",voltageOut.voltage);
/*
      char buffer[8];
      memcpy( buffer, &voltageOut, 8 );
      for (int i = 0; i < sizeof(voltageOut); i++)
      {
        Serial.printf("%02X", buffer[i]);
      }   
*/
      LMIC_setTxData2(2, (unsigned char *)&voltageOut, sizeof(voltageOut), 0);
#endif

      //LMIC_setTxData2(2, STATICMSG, sizeof(STATICMSG), 0);

      Serial.println("Packet queued 2");
      clear_to_sleep = false; // do not sleep before the message is send
    }
    // Next TX is scheduled after TX_COMPLETE event.
  }
#endif //SEND_DATA_LORA_2_ENABLED

#ifdef SEND_DATA_WIFI_3_ENABLED
  //next_run_time[3] voidaan asettaa halutuksi (->0) esim. sensorin luvun yhteydessä jos halutaan lähettää heti tuore tieto wifillä
  if (time_to_run_task(3))
  {
    //  tee paketti (json?) ja lähetä sensoridata wifillä
  }
#endif //SEND_DATA_WIFI_3_ENABLED

#ifdef OTA_UPDATE_4_ENABLED
  if (time_to_run_task(4))
  {
    // kokeile päivitystä, samalla voisi myös hakea kellonajan kun ollaan wifissä, päivitysfunktiot omassa filessä
  }
#endif //OTA_UPDATE_4_ENABLED

#ifdef WIFI_REQUIRED
  // close wifi here is still open
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
#endif //WIFI_REQUIRED

#ifdef SEND_DATA_LORA_2_ENABLED
  // do it once anyway here
  os_runloop_once();
#endif

#ifdef RESTART_10_ENABLED

  //RESTART_INTERVAL 86400
  if (time_to_run_task(10))
  {
    time(&now);
    if ((next_run_time[10] - now) < -3600 * 24 * 30)
    {
      // probably clock was synched from ntp, no reboot now
      schedule_next_task_run(10, RESTART_INTERVAL, true);
    }
    else
    {
      ESP.restart();
    }
  }
#endif

#ifdef SLEEP_ENABLED
  time_t now_local;
  time(&now_local);
  time_t time_to_next_run = (time_to_earliest_run() - now_local);

  while (!clear_to_sleep)
  { //wait until previous send is complete
    os_runloop_once();
  }

  if (time_to_next_run >= MIN_SLEEPING_TIME_SECS)
  {
    if (time_to_next_run > MAX_SLEEPING_TIME_SECS)
    {
      time_to_next_run = MAX_SLEEPING_TIME_SECS; // do not sleep too long even there is nothing to execute
    }
    bootCount++;
    Serial.printf("Sleeping %d seconds\n", (int)time_to_next_run);
    uint64_t sleeptime = (time_to_next_run * 1000000);
    esp_sleep_enable_timer_wakeup(sleeptime);
    esp_deep_sleep_start();
  }
  else
  {
    // just relax, but do not   sleep
    Serial.printf("Waiting %d seconds\n", (int)time_to_next_run);
    static unsigned long wait_start_ms = millis();
    while ((time_t)(millis() - wait_start_ms) < (time_t)(time_to_next_run * 1000))
    {
#ifdef SEND_DATA_LORA_2_ENABLED
      os_runloop_once();
#endif
// Serial.printf("%ld\t %d\n",(millis() - wait_start_ms),(time_to_next_run * 1000));
    }
    /*
      if ((millis() - wait_start_ms) >= time_to_next_run * 1000)
      {
#ifdef SEND_DATA_LORA_2_ENABLED
      os_runloop_once();
#endif
    }
    */
  }
#else // no SLEEP_ENABLED
  Serial.printf("No Sleep enable - waiting %d seconds\n", (int)time_to_next_run);
  static unsigned long wait_start_ms = millis();
  if ((millis() - wait_start_ms) >= time_to_next_run * 1000)
  {
#ifdef SEND_DATA_LORA_2_ENABLED
    os_runloop_once();
#endif
  }
#endif // SLEEP_ENABLED
}
