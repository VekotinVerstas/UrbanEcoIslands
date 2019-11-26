#include <Arduino.h>
#include "hsvictron.h"
#include "main.h"

char buf[80];

float floatFromBuffer(String val) {
 val.toCharArray(buf, sizeof(buf));
 return atof(buf);
}
int intFromBuffer(String val) {
 val.toCharArray(buf, sizeof(buf));
 return atoi(buf);
}

int readVictron()
{
  //The device transmits blocks of data at 1 second intervals. Each field is sent using the following format:
  // Serial.setTimeout(); // ms default 1000
  String label, val;
  while(Serial1.available())
   {
        label = Serial1.readStringUntil('\t');    // this is the actual line that reads the label from the MPPT controller
        val = Serial1.readStringUntil('\n');  // this is the line that reads the value of the label

     if (label =="V") {
         DataOut.victronData.mainVoltage_V = floatFromBuffer(val);
     }
     else if (label =="VPV") {
         DataOut.victronData.panelVoltage_VPV = floatFromBuffer(val);
     }
     else if (label =="PPV") {
         DataOut.victronData.panelPower_PPV = floatFromBuffer(val);
     }
     else if (label =="I") {
         DataOut.victronData.batteryCurrent_I = floatFromBuffer(val);
     }
     else if (label =="H19") {
         DataOut.victronData.yieldTotal_H19 = floatFromBuffer(val);
     }
     else if (label =="H20") {
         DataOut.victronData.yieldToday_H20 = floatFromBuffer(val);
     }
     else if (label =="H21") {
         DataOut.victronData.maxPowerToday_H21 = floatFromBuffer(val);
     }
     else if (label =="H22") {
         DataOut.victronData.yieldYesterday_H22 = floatFromBuffer(val);
     }
     else if (label =="H23") {
         DataOut.victronData.maxPowerYesterday_H23 = floatFromBuffer(val);
     }
     else if (label =="ERR") {
         DataOut.victronData.errorCode_ERR = intFromBuffer(val);
     }
     else if (label =="CS") {
         DataOut.victronData.stateOfOperation_CS = intFromBuffer(val);
     }

   }
   return(0);
}