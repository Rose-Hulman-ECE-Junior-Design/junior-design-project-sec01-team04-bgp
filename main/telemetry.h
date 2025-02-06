#pragma once

#include <Adafruit_INA219.h>

typedef struct {
    double current_a;
    double battery_v;
    double power_w;
} TelemetryData;

// Class to handle reading telemetry data from vehicle.
// This includes the current, voltage, and power.
class Telemetry {
public:
    void init();
    
    TelemetryData read();
private:
    Adafruit_INA219 ina219;
};

