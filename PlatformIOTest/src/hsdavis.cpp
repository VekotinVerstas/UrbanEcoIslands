#include "tasksdefine.h"
#ifdef READ_WEATHER_DAVIS_8_ENABLED
#include <Arduino.h>
#include "hsdavis.h"


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
#endif
    return (-1);
}

int readDavis()
{
#ifdef READ_WEATHER_DAVIS_8_ENABLED
    char buffer[100];
    char buf[40];
    RDavisDATA davisData;

    Serial1.println("LOOP 1"); // Request data from Davis
    delay(100);
    if (Serial1.available())
    { // If anything comes in Serial1
        delay(100);
        while (Serial1.available() >= 100)
        { // If anything comes in Serial1
            Serial.print("Serial available: ");
            //Serial.println ( Serial1.available(), DEC);
            Serial1.readBytes(buffer, 1); //Read acc away
            Serial1.readBytes(buffer, 99);

            davisData.msg_type = 0;
            davisData.msg_lenght = sizeof(RDavisDATA);
            davisData.wBarometer = toUInt16(buffer, 7); /* uint16_t 7 Current barometer as (Hg / 1000)          */
            davisData.wInsideTemp = toInt16(buffer, 9);
            ;                                  /* int16_t 9 Inside Temperature as (DegF / 10)          */
            davisData.yInsideHum = buffer[11]; /* uint8_t 11 Inside Humidity as percentage             */
            davisData.wOutsideTemp = toInt16(buffer, 12);
            ;                                     /* int16_t 12 Outside Temperature as (DegF / 10)        */
            davisData.yWindSpeed = buffer[14];    /* uint8_t 14 Wind Speed                                */
            davisData.yAvgWindSpeed = buffer[15]; /* uint8_t 15 10-Minute Average Wind Speed              */
            davisData.wWindDir = toUInt16(buffer, 16);
            ;                                   /* uint16_t 16 Wind Direction in degress                */
            davisData.yOutsideHum = buffer[33]; /* uint8_t 33 Outside Humidity                          */
            davisData.wRainRate = toUInt16(buffer, 41);
            ;                                /* uint16_t 41 Rain Rate                                */
            davisData.yUVLevel = buffer[43]; /* uint8_t 43 UV Level                                  */
            davisData.wSolarRad = toUInt16(buffer, 44);
            ; /* uint16_t 44 Solar Radiation                          */
            davisData.wStormRain = toUInt16(buffer, 46);
            ; /* uint16_t 46 Total Storm Rain                         */
            davisData.wStormStart = toUInt16(buffer, 48);
            ; /* uint16_t 48 Start date of current storm              */
            davisData.wRainDay = toUInt16(buffer, 50);
            ;                                            /* uint16_t 50 Rain Today                               */
            davisData.wRainMonth = toUInt16(buffer, 52); /* uint16_t 52 Rain this Month                          */
            davisData.wRainYear = toUInt16(buffer, 54);  /* uint16_t 54 Rain this Year                           */
            davisData.yXmitBatt = buffer[86];            /* uint8_t 86 Transmitter battery status                */
            davisData.wBattLevel = toUInt16(buffer, 87); /* uint16_t 87 Console Battery Level:                   */
            /*            Voltage = ((wBattLevel * 300)/512)/100.0  */
            davisData.yForeIcon = buffer[89]; /* uint8_t 89 Forecast Icon                             */
            davisData.yRule = buffer[90];     /* uint8_t 90 Forecast rule number                      */

            sprintf(buf, "%s = %.4f\n", "Ilmanpaine", ((float)davisData.wBarometer / 1000) * 33.86389);
            Serial.print(buf);

            //     sprintf(buf, "%s = %.1f\n", "Sisälämpö", toTemp(buffer, 9) );
            //     Serial.print(buf);

            sprintf(buf, "%s = %u\n", "Kosteus", davisData.yOutsideHum);
            Serial.print(buf);

            sprintf(buf, "%s = %.1f\n", "Lämpö", toTemp((char *)&davisData.wOutsideTemp, 0));
            Serial.print(buf);

            sprintf(buf, "%s = %.1f\n", "Lämpö", toTemp(buffer, 12));
            Serial.print(buf);

            sprintf(buf, "%s = %u\n", "Tuulen nopeus", davisData.yWindSpeed);
            Serial.print(buf);

            /*
            Serial.print("Wind speed: " );
            Serial.println( rtdata.yWindSpeed, DEC );

            Serial.print("AvgWind: " );
            Serial.println( rtdata.yAvgWindSpeed, DEC );

            Serial.print("Wind dir: " );
            Serial.println( rtdata.wWindDir, DEC );
      */
            Serial.println("*************************************");
            Serial.flush();
        }

        if (Serial1.available())
        { // If anything comes in Serial1
            Serial.print("Serial available: ");
            Serial.println(Serial1.available(), DEC);
            Serial.write(Serial1.read()); // read it and send it out Serial (USB)
        }
    }
#endif
    return (-1);
}
