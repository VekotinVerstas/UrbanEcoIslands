
#include <hsbme280.h>

HSbme280::HSbme280(){

};

HSbme280Data HSbme280::read_sensor_values()
{
    HSbme280Data result;
    if (!bme.begin(0x76))
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1)
            ;
    }

    result.temperature = bme.readTemperature();
    result.humidity = bme.readHumidity();
    result.pressure = bme.readPressure();

    return result;
};
