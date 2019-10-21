#include "tasksdefine.h"
#ifdef READ_WEATHER_DAVIS_8_ENABLED
#include <Arduino.h>
#include "hsdavis.h"
#include "main.h"

uint16_t toUInt16(char *buffer, int8_t offset)
{
    return ((uint16_t)(buffer[offset + 1] << 8) + buffer[offset]);
}

int16_t toInt16(char *buffer, int8_t offset)
{
    return ((int16_t)toUInt16(buffer, offset));
}

float toTemp(char *buffer, int8_t offset)
{
    return (((toInt16(buffer, offset) / 10) - 32) / 1.8); // Divide int with 10 to get desimal and then suptract 32 and divide by 1.8 to get Celccius from fahrenhait
}
#endif

int setupDavis()
{
#ifdef READ_WEATHER_DAVIS_8_ENABLED
    Serial1.begin(19200, SERIAL_8N1, 14, 13); // Davis
    Serial.println("Using Serial1 for ESP to Davis communication.");
    return (0);
#endif
    return (-1);
}

int readDavis()
{
#ifdef READ_WEATHER_DAVIS_8_ENABLED
    char buffer[100];
    //t_DavisDATA davisData;
    Serial.println("Reading loop1 data from Davis.");
    Serial1.println(); // Wake up Davis
    delay(100);
    while (Serial1.available())
    { // If anything comes in Serial1
        Serial1.read(); // Dump unknown data
    }
    Serial1.println("LOOP 1"); // Request data from Davis
    delay(100);
    if (Serial1.available())
    { // If anything comes in Serial1
        delay(100);
        while (Serial1.available() >= 100)
        {                                 // If anything comes in Serial1
            Serial1.readBytes(buffer, 1); //Read acc away
            Serial1.readBytes(buffer, 99);

            DataOut.davisData.msg_type = 10;
            DataOut.davisData.msg_ver = 0;
            DataOut.davisData.wBarometer = toUInt16(buffer, 7);   /* uint16_t 7 Current barometer as (Hg / 1000)          */
            DataOut.davisData.wInsideTemp = toInt16(buffer, 9);   /* int16_t 9 Inside Temperature as (DegF / 10)          */
            DataOut.davisData.yInsideHum = buffer[11];            /* uint8_t 11 Inside Humidity as percentage             */
            DataOut.davisData.wOutsideTemp = toInt16(buffer, 12); /* int16_t 12 Outside Temperature as (DegF / 10)        */
            DataOut.davisData.yWindSpeed = buffer[14];            /* uint8_t 14 Wind Speed                                */
            DataOut.davisData.yAvgWindSpeed = buffer[15];         /* uint8_t 15 10-Minute Average Wind Speed              */
            DataOut.davisData.wWindDir = toUInt16(buffer, 16);    /* uint16_t 16 Wind Direction in degress                */
            DataOut.davisData.yOutsideHum = buffer[33];           /* uint8_t 33 Outside Humidity                          */
            DataOut.davisData.wRainRate = toUInt16(buffer, 41);   /* uint16_t 41 Rain Rate                                */
            DataOut.davisData.yUVLevel = buffer[43];              /* uint8_t 43 UV Level                                  */
            DataOut.davisData.wSolarRad = toUInt16(buffer, 44);   /* uint16_t 44 Solar Radiation                          */
            DataOut.davisData.wStormRain = toUInt16(buffer, 46);  /* uint16_t 46 Total Storm Rain                         */
            DataOut.davisData.wStormStart = toUInt16(buffer, 48); /* uint16_t 48 Start date of current storm              */
            DataOut.davisData.wRainDay = toUInt16(buffer, 50);    /* uint16_t 50 Rain Today                               */
            DataOut.davisData.wRainMonth = toUInt16(buffer, 52);  /* uint16_t 52 Rain this Month                          */
            DataOut.davisData.wRainYear = toUInt16(buffer, 54);   /* uint16_t 54 Rain this Year                           */
            DataOut.davisData.yXmitBatt = buffer[86];             /* uint8_t 86 Transmitter battery status                */
            DataOut.davisData.wBattLevel = toUInt16(buffer, 87);  /* uint16_t 87 Console Battery Level:                   */
            DataOut.davisData.yForeIcon = buffer[89];             /* uint8_t 89 Forecast Icon                             */
            DataOut.davisData.yRule = buffer[90];                 /* uint8_t 90 Forecast rule number                      */

            Serial.println("************************");
            Serial.printf("%s = %.1f\n", "Lämpö", toTemp((char *)&DataOut.davisData.wOutsideTemp, 0));
            Serial.printf("%s = %.4f\n", "Ilmanpaine", ((float)DataOut.davisData.wBarometer / 1000) * 33.86389);
            Serial.printf("%s = %u\n", "Kosteus", DataOut.davisData.yOutsideHum);
            Serial.printf("%s = %u\n", "Tuulen nopeus", DataOut.davisData.yWindSpeed);
            Serial.println("************************");
            Serial.flush();
        }
        return (0);
    }
    else
        Serial.println("No aswer from Davis");

#endif
    return (-1);
}
