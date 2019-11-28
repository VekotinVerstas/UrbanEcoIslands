#ifndef HSDAVIS_H
#define HSDAVIS_H

int setupDavis();
int readDavis();

#ifdef READ_WEATHER_DAVIS_8_ENABLED
/* Definition of Davis LOOP data packet FULL */
//struct t_RTDATA
//{
//    //uint8_t yACK;           /* -1 ACK from stream                           */
//    char cL;  /* 0  character "L"                             */
//    char cO;  /* 1  character "O"                             */
//    char cO1; /* 2  character "O"                             */
//    char cP;  /* 3  character "P" (RevA) or the current       */
//    /*    3-hour Barometer trend as follows:        */
//    /*    196 = Falling Rapidly                     */
//    /*    236 = Falling Slowly                      */
//    /*    0   = Steady                              */
//    /*    20  = Rising Slowly                       */
//    /*    60  = Rising Rapidly                      */
//    /* any other value is 3-hour data not available */
//    uint8_t yPacketType;   /* 4 Always zero for current firmware release   */
//    uint16_t wNextRec;     /* 5 loc in archive memory for next data packet */
//    uint16_t wBarometer;   /* 7 Current barometer as (Hg / 1000)           */
//    int16_t wInsideTemp;   /* 9 Inside Temperature as (DegF / 10)          */
//    uint8_t yInsideHum;    /* 11 Inside Humidity as percentage             */
//    int16_t wOutsideTemp;  /* 12 Outside Temperature as (DegF / 10)        */
//    uint8_t yWindSpeed;    /* 14 Wind Speed                                */
//    uint8_t yAvgWindSpeed; /* 15 10-Minute Average Wind Speed              */
//    uint16_t wWindDir;     /* 16 Wind Direction in degress                 */
//    int8_t yXtraTemps[7];  /* 18 Extra Temperatures                        */
//    int8_t ySoilTemps[4];  /* 25 Soil Temperatures                         */
//    int8_t yLeafTemps[4];  /* 29 Leaf Temperatures                         */
//    uint8_t yOutsideHum;   /* 33 Outside Humidity                          */
//    uint8_t yXtraHums[7];  /* 34 Extra Humidities                          */
//    uint16_t wRainRate;    /* 41 Rain Rate                                 */
//    uint8_t yUVLevel;      /* 43 UV Level                                  */
//    uint16_t wSolarRad;    /* 44 Solar Radiation                           */
//    uint16_t wStormRain;   /* 46 Total Storm Rain                          */
//    uint16_t wStormStart;  /* 48 Start date of current storm               */
//    uint16_t wRainDay;     /* 50 Rain Today                                */
//    uint16_t wRainMonth;   /* 52 Rain this Month                           */
//    uint16_t wRainYear;    /* 54 Rain this Year                            */
//    uint16_t wETDay;       /* 56 Day ET                                    */
//    uint16_t wETMonth;     /* 58 Month ET                                  */
//    uint16_t wETYear;      /* 60 Year ET                                   */
//    uint32_t wSoilMoist;   /* 62 Soil Moistures                            */
//    uint32_t wLeafWet;     /* 66 Leaf Wetness                              */
//    uint8_t yAlarmInside;  /* 70 Inside Alarm bits                         */
//    uint8_t yAlarmRain;    /* 71 Rain Alarm bits                           */
//    uint16_t yAlarmOut;    /* 72 Outside Temperature Alarm bits            */
//    uint8_t yAlarmXtra[8]; /* 74 Extra Temp/Hum Alarms                     */
//    uint32_t yAlarmSL;     /* 82 Soil and Leaf Alarms                      */
//    uint8_t yXmitBatt;     /* 86 Transmitter battery status                */
//    uint16_t wBattLevel;   /* 87 Console Battery Level:                    */
//    /*    Voltage = ((wBattLevel * 300)/512)/100.0  */
//    uint8_t yForeIcon; /* 89 Forecast Icon                             */
//    uint8_t yRule;     /* 90 Forecast rule number                      */
//    uint16_t wSunrise; /* 91 Sunrise time (BCD encoded, 24hr)          */
//    uint16_t wSunset;  /* 93 Sunset time  (BCD encoded, 24hr)          */
//    uint8_t yLF;       /* 95 Line Feed (\n) 0x0a                       */
//    uint8_t yCR;       /* 96 Carraige Return (\r) 0x0d                 */
//    uint16_t WCRC;     /* 97 CRC check bytes (CCITT-16 standard)       */
//} RTDATA;

/* Definition of Davis LOOP data packet STRIPPED */
/*COMPLER ADDS 8bit for single 8bit struct value!!!
 Have two 8bit values next to each to avoid fillings.
 And make life easyer in python parser */
struct t_DavisDATA
{
    uint8_t msg_type;
    int8_t msg_ver;
    uint16_t wBarometer;   /* 7 Current barometer as (Hg / 1000)           */
    int16_t wInsideTemp;   /* 9 Inside Temperature as (DegF / 10)          */
    uint8_t yInsideHum;    /* 11 Inside Humidity as percentage             */
    uint8_t yOutsideHum;   /* 33 Outside Humidity                          */
    int16_t wOutsideTemp;  /* 12 Outside Temperature as (DegF / 10)        */
    uint8_t yWindSpeed;    /* 14 Wind Speed                                */
    uint8_t yAvgWindSpeed; /* 15 10-Minute Average Wind Speed              */
    uint16_t wWindDir;     /* 16 Wind Direction in degress                 */
    uint16_t wRainRate;    /* 41 Rain Rate                                 */
    uint8_t yXmitBatt;     /* 86 Transmitter battery status                */
    uint8_t yUVLevel;      /* 43 UV Level                                  */
    uint16_t wSolarRad;    /* 44 Solar Radiation                           */
    uint16_t wStormRain;   /* 46 Total Storm Rain                          */
    uint16_t wStormStart;  /* 48 Start date of current storm               */
    uint16_t wRainDay;     /* 50 Rain Today                                */
    uint16_t wRainMonth;   /* 52 Rain this Month                           */
    uint16_t wRainYear;    /* 54 Rain this Year                            */
    uint16_t wBattLevel;   /* 87 Console Battery Level:                    */
    /*    Voltage = ((wBattLevel * 300)/512)/100.0  */
    uint8_t yForeIcon; /* 89 Forecast Icon                             */
    uint8_t yRule;     /* 90 Forecast rule number                      */
};
#endif //READ_WEATHER_DAVIS_8_ENABLED

#endif // HSDAVIS_H
