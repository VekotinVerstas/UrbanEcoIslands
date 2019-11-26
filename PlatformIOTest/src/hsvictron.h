#ifndef HSVICTORN_H
#define HSVICTORN_H
#include "tasksdefine.h"


int readVictron();

#ifdef READ_VICTRON_ENABLED
struct t_VictronDATA {
  int16_t batteryVoltageRaw;
  int16_t panelVoltageRaw;
  float mainVoltage_V;      // mV
  float panelVoltage_VPV;   // mV
  float panelPower_PPV;     // W
  float batteryCurrent_I;   // mA
  float yieldTotal_H19;     // 0.01 kWh
  float yieldToday_H20;     // 0.01 kWh
  float maxPowerToday_H21;  // W
  float yieldYesterday_H22; // 0.01 kWh
  float maxPowerYesterday_H23; // W
  int errorCode_ERR;
  int stateOfOperation_CS;
};

 //measurement status = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
#endif //READ_VICTORN_ENABLED

#endif // HSVICTORN_H