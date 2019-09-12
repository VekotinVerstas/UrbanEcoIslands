// tässä määritellään mitkä taskit käytössä
#include "settings.h"

#include "esp_system.h"

#define MAX_TASK_COUNT 20

/*
0) pax counter wifi
1) pax counter ble
2) send data with LoRa
3) send data with wifi
4) OTA update firmware
5) read bme280 sensor values
6) read htu21D sensor value
7) syncronize ntp time
*/
// next start time of each task, initiated in startup

RTC_DATA_ATTR time_t next_run_time[MAX_TASK_COUNT];

#define UNDEFINED_TIME -1

#ifdef SEND_DATA_LORA_2_ENABLED
#include <lmic.h>
#include <hal/hal.h>
// AAPO private 0601

static PROGMEM u1_t NWKSKEY[16] = {0x25, 0xdb, 0x9f, 0x0e, 0x9b, 0xb6, 0x3a, 0xcf, 0x7c, 0x3d, 0x4d, 0x01, 0x6e, 0x47, 0x75, 0x5f};
static PROGMEM u1_t APPSKEY[16] = {0xc7, 0xff, 0x78, 0x02, 0x11, 0x37, 0x37, 0x73, 0xbd, 0x7a, 0xa7, 0x20, 0x7e, 0x33, 0x00, 0x0f};
static PROGMEM u1_t DEVEUI[8] = {0xd1, 0xc1, 0x14, 0xbf, 0x76, 0x4a, 0x06, 0x01};
static u4_t DEVADDR = 0x764a0601; // NOTE: last 8 characters from DEVEUI

//MDummy message to emulate 26 byte (AQBURK) payload ( bat counter probably has much shorter
static PROGMEM u1_t STATICMSG[26] = {0x24, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui(u1_t *buf) {}
void os_getDevEui(u1_t *buf) {}
void os_getDevKey(u1_t *buf) {}

#endif

#ifdef READ_TEMP_HUM_BME280_5_ENABLED
#include <hsbme280.h>
HSbme280 HSBME280Sensor;
RTC_DATA_ATTR HSbme280Data HSBME280SensorData;
#endif

#ifdef READ_HTU21D_6_ENABLED
#include <hshtu21d.h>
HShtu21d HShtu21dSensor;
RTC_DATA_ATTR HShtu21dData HSHTU21DSensorData;
#endif

#if defined(SEND_DATA_WIFI_3_ENABLED) || defined(OTA_UPDATE_4_ENABLED) || defined(SYNCRONIZE_NTP_TIME_7_ENABLED)
#define WIFI_REQUIRED
// wifi käytössä
// tähän wifin vaatimat includet ja vakio/muuttujamääritykset
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiType.h>

WiFiMulti wifiMulti;

#endif

#ifdef SEND_DATA_WIFI_3_ENABLED
next_run_time[3] = 0; // Muuttuja, jossa määritetään seuraava aloitusaika
#endif

#ifdef OTA_UPDATE_4_ENABLED
#include "HSOTA.H"                 // tässä viittaus tiedostoon, jossa  ota-päivitykseen liittyvät funktiot
#define OTA_UPDATE_INTERVAL_MIN 15 // Wifi ota update is tried this often, suurin osa on turhia yrityksiä, mutta jos joku odottaa hotspot-kännykän kanssa updatea on odottavan aika pitkä. Jos tätä tarvetta ei ole voisi olla vaikka 1 vrk välein
next_run_time[4] = 0;
#endif

#ifdef SYNCRONIZE_NTP_TIME_7_ENABLED
// see https://forum.arduino.cc/index.php?topic=595802.0 for code

const char *ntpServer = "pool.ntp.org";
//const long gmtOffset_sec = 3600;
//const int daylightOffset_sec = 0;

RTC_DATA_ATTR byte bootCount = 0;
RTC_DATA_ATTR time_t now;
RTC_DATA_ATTR uint64_t Mics = 0;
RTC_DATA_ATTR struct tm *timeinfo;

#endif

#define sleepPeriod 10000000ul // 10 seconds sleep
#define OFFSET 0               // us (takes into account the updateTime execution time)

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

void do_sleep(unsigned long chrono)
{
  // updateTime(sleepPeriod - (micros() - chrono) + OFFSET); // onkohan tarpeen
  esp_sleep_enable_timer_wakeup(sleepPeriod - (micros() - chrono));
  esp_deep_sleep_start();
}

#define SCL_PIN 22
#define SDA_PIN 21

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

bool time_to_run_task(int task_number)
{
  time_t next_run_time_task = next_run_time[task_number];

  time(&now);
  if ((next_run_time_task <= now))
  {
    Serial.printf("Task %d was due %d secs ago (now %ld, due %ld)\n", task_number, now - next_run_time_task, now, next_run_time_task);
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

int battery_voltage; // globaali, johon luetaan voltage sopivin (?) määrävälein

time_t time_to_earliest_run()
{
  // loop all tasks and return earliest run time
  time_t earliest_run_time = LONG_MAX;
  for (int i = 0; i < MAX_TASK_COUNT; i++)
  {
    if ((next_run_time[i] > 0) && (next_run_time[i] < earliest_run_time))
    {
      earliest_run_time = next_run_time[i];
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

  //Serial.printf("earliest_run_time %ld\n", earliest_run_time);
  return earliest_run_time;
}

/*


*/
void setup()
{
  Serial.begin(115200);
  delay(20);
  Serial.print("bootCount:");
  Serial.println(bootCount);
  printLocalTime();

  // siirrä koodi omiin fileihin
#ifdef SEND_DATA_LORA_2_ENABLED
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

// Set static session parameters. Instead of dynamically establishing a session
// by joining the network, precomputed session parameters are be provided.
#ifdef PROGMEM
  Serial.println("PROGMEM");
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession(0x1, DEVADDR, nwkskey, appskey);
#else
  Serial.println("NO PROGMEM");
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession(0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif

  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI); // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);   // g2-band

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // Set data rate and transmit power (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF12, 14);

  // Start job
  do_send(&sendjob);
#endif //SEND_DATA_LORA_2_ENABLED
}
void loop()
{
#ifdef SEND_DATA_LORA_2_ENABLED
  os_runloop_once();
#endif

#ifdef SYNCRONIZE_NTP_TIME_7_ENABLED
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
        ESP.restart(); // restart after 15 s
    }
    Serial.println(" CONNECTED");
    //init and get the time
    //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    configTime(0, 0, ntpServer);

    delay(1000);
    Serial.println(" Got new time");
    printLocalTime();
    // unsigned long chrono = micros();

    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    // updateTime(0); // Update for use during waking
    // unsigned long chrono = micros();
    /*
   * do what you want here
   */
    // printLocalTime(); // This sets the internal clock
    // bootCount++;

    schedule_next_task_run(7, 120, false);
  }

#endif

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
#endif

#ifdef READ_TEMP_HUM_BME280_5_ENABLED

  HSBME280SensorData = HSBME280Sensor.read_sensor_values();
  Serial.printf("BME280 temperature %f, humidity %f, pressure %f\n", HSBME280SensorData.temperature, HSBME280SensorData.humidity, HSBME280SensorData.pressure);
  delay(10000); // toistaiseksi näin
#endif

#ifdef READ_HTU21D_6_ENABLED
  HSHTU21DSensorData = HShtu21dSensor.read_sensor_values();
  Serial.printf("HTU temperature %f, humidity %f, dew point %f\n", HSHTU21DSensorData.temperature, HSHTU21DSensorData.humidity, dewPoint(HSHTU21DSensorData.humidity, HSHTU21DSensorData.temperature));
  delay(10000); // toistaiseksi näin
#endif

#ifdef DEVICE_SCAN_WIFI_0_ENABLED

  // käynnistä pax-wifi-scan jos käynnistysaika koittanut
  if (time_to_run_task(4))
  {
    //  kutsu skannausfuntiota, joka määritelty omassa
  }
#endif

#ifdef SEND_DATA_WIFI_3_ENABLED
  //next_run_time[3] voidaan asettaa halutuksi (->0) esim. sensorin luvun yhteydessä jos halutaan lähettää heti tuore tieto wifillä
  if (time_to_run_task(3))
  {
    //  tee paketti (json?) ja lähetä sensoridata wifillä
  }
#endif

#ifdef OTA_UPDATE_4_ENABLED
  if (time_to_run_task(4))
  {
    // kokeile päivitystä, samalla voisi myös hakea kellonajan kun ollaan wifissä, päivitysfunktiot omassda filessä
  }
#endif

#ifdef WIFI_REQUIRED
  // close wifi here is still open
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
#endif

#ifdef SEND_DATA_LORA_2_ENABLED
  os_runloop_once();
#endif

#ifdef SLEEP_ENABLED
  time_t now_local;
  time(&now_local);
  time_t time_to_next_run = (time_to_earliest_run() - now_local);

  // tässä voisi vielä olla joku wait_before_sleep tyyppinen muuttuja, joka asetetaan esim. lora-viestin muodostuksen yhteydess
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
    // just relax, but do not sleep
    //tähän loran purkuluuppi myöskin
    // tämä delay on temppi
    Serial.printf("Waiting %d seconds (temporary implementation)\n", (int)time_to_next_run);
    delay(time_to_next_run * 1000);
  }
  // tsekkaa/varmista, ettää loraviestit ehtivät lähteä maailmalle ennen sleeppiä

  // check ( time_to_next_sleep ) when we should do something next time, if more than MIN_SLEEPING_TIME_SECS, otherwise wait
  /*
  int next_sleep_time = time_to_next_sleep();
  // tsekaa myös undefined, voiko olla vai pakoteenta joko perustoiminto
  if (next_sleep_time - _current_time_jostain() > MIN_SLEEPING_TIME_SECS)
  {
    // tsekkaa/varmista, ettää loraviestit ehtivät lähteä maailmalle ennen sleeppiä
    // do sleep
  }
  else
  {
    // tässä vaan looppi, joka kiertää kunnes seuraava taski, puretaan samalla lora-jonoja
  }
  */
#endif
}
