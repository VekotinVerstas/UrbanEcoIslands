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
*/
// next start time of each task, initiated in startup
RTC_DATA_ATTR int next_run_time[MAX_TASK_COUNT];

#define UNDEFINED_TIME -1

#ifdef SEND_DATA_LORA_2_ENABLED
#include <lmic.h>
#include <hal/hal.h>
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

#if defined(SEND_DATA_WIFI_3_ENABLED) || defined(OTA_UPDATE_4_ENABLED)
// wifi käytössä
// tähän wifin vaatimat includet ja vakio/muuttujamääritykset
#endif

#ifdef SEND_DATA_WIFI_3_ENABLED
next_run_time[3] = 0; // Muuttuja, jossa määritetään seuraava aloitusaika
#endif

#ifdef OTA_UPDATE_4_ENABLED
#include "HSOTA.H"                 // tässä viittaus tiedostoon, jossa  ota-päivitykseen liittyvät funktiot
#define OTA_UPDATE_INTERVAL_MIN 15 // Wifi ota update is tried this often, suurin osa on turhia yrityksiä, mutta jos joku odottaa hotspot-kännykän kanssa updatea on odottavan aika pitkä. Jos tätä tarvetta ei ole voisi olla vaikka 1 vrk välein
next_run_time[4] = 0;
#endif

// general definations
#define MIN_SECS_TO_SLEEP 30 // if next activity is later than this amount of secs -> do sleep, otherwise just wait loop

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

int _current_time_jostain()
{
  return 0; // tämän palauttaa kellonajan jotenkin
};
int battery_voltage; // globaali,johon luetaan voltage sopivin (?) määrävälein

int time_to_next_sleep()
{
  // loop all tasks and return earliest start time
  int next_sleep_time = UNDEFINED_TIME;
  for (int i = 0; i < MAX_TASK_COUNT; i++)
  {
    if ((next_run_time[i] != UNDEFINED_TIME) && (next_run_time[i] < next_sleep_time))
    {

      next_sleep_time = next_run_time[i];
    }
  }
  return next_sleep_time;
}

void setup()
{
  Serial.begin(115200);
  /*
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  // oletetaan full reset eli ei nousta unesta, initioidaan taulukko
  if (wakeup_reason < 1 || wakeup_reason > 5)
  {

    for (int i = 0; i < MAX_TASK_COUNT; i++)
    {
      next_run_time[i] = -1;
    }
  }
*/
  // read status from nonvotlatile memory
  // read real time clock?
}
void loop()
{

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
  if (next_run_time[4] > _current_time_jostain())
  {
    //  kutsu skannausfuntiota, joka määritelty omassa
  }
#endif

#ifdef SEND_DATA_WIFI_3_ENABLED
  //next_run_time[3] voidaan asettaa halutuksi (->0) esim. sensorin luvun yhteydessä jos halutaan lähettää heti tuore tieto wifillä
  if (next_run_time[3] > _current_time_jostain())
  {
    //  tee paketti (json?) ja lähetä sensoridata wifillä
  }
#endif

#ifdef OTA_UPDATE_4_ENABLED
  if (next_run_time[4] > _current_time_jostain())
  {
    // kokeile päivitystä, samalla voisi myös hakea kellonajan kun ollaan wifissä, päivitysfunktiot omassda filessä
  }
#endif

  // check ( time_to_next_sleep ) when we should do something next time, if more than MIN_SECS_TO_SLEEP, otherwise wait
  /*
  int next_sleep_time = time_to_next_sleep();
  // tsekaa myös undefined, voiko olla vai pakoteenta joko perustoiminto
  if (next_sleep_time - _current_time_jostain() > MIN_SECS_TO_SLEEP)
  {
    // tsekkaa/varmista, ettää loraviestit ehtivät lähteä maailmalle ennen sleeppiä
    // do sleep
  }
  else
  {
    // tässä vaan looppi, joka kiertää kunnes seuraava taski, puretaan samalla lora-jonoja
  }
  */
}
