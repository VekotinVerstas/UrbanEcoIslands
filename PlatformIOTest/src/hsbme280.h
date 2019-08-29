#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

class HSbme280Data
{
public:
    float temperature;
    float humidity;
    float pressure;
};

class HSbme280
{
public:
    HSbme280();
    HSbme280Data read_sensor_values();

    Adafruit_BME280 bme;
};
