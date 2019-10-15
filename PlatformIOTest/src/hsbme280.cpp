
#include <hsbme280.h>

HSbme280::HSbme280(){

};

HSbme280Data HSbme280::read_sensor_values()
{
    HSbme280Data result;
    while (!bme.begin(BME280_I2C))
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        delay(1000);
    }
    result.temperature = bme.readTemperature();
    result.humidity = bme.readHumidity();
    result.pressure = bme.readPressure();

    return result;
};

// from https://stackoverflow.com/questions/27288021/formula-to-calculate-dew-point-from-temperature-and-humidity/27289801
float dewPoint(float humi, float temp)
{
  return (temp - (14.55 + 0.114 * temp) * (1 - (0.01 * humi)) - pow(((2.5 + 0.007 * temp) * (1 - (0.01 * humi))), 3) - (15.9 + 0.117 * temp) * pow((1 - (0.01 * humi)), 14));
}

//BME280 @0x76 !