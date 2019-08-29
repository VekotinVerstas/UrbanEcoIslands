#include <Wire.h>
#include <SPI.h>
#include <Adafruit_HTU21DF.h>

class HShtu21dData
{
public:
    float temperature;
    float humidity;
};

class HShtu21d
{
public:
    HShtu21d();
    HShtu21dData read_sensor_values();
    Adafruit_HTU21DF htu21d;
};
