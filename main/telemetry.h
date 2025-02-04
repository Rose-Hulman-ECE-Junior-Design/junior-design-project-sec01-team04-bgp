#pragma once

#include <Adafruit_INA219.h>

typedef struct {
    float current_ma;
    float battery_v;
    float power_mw;
} TelemetryData;

// Class to handle reading telemetry data from vehicle
class Telemetry {
public:
    void init();
    
    TelemetryData read();
private:
    Adafruit_INA219 ina219;
};

