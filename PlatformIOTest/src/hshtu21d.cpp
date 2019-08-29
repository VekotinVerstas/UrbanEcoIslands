
#include <hshtu21d.h>

HShtu21d::HShtu21d(){

};

HShtu21dData HShtu21d::read_sensor_values()
{
  HShtu21dData result;
  if (!htu21d.begin())
  {
    Serial.println("Could not find a valid HTU21D sensor, check wiring!");
    while (1)
      ;
  }

  result.temperature = htu21d.readTemperature();
  result.humidity = htu21d.readHumidity();

  return result;
};
