
#include <hsbme280.h>
#include <settings.h>

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
