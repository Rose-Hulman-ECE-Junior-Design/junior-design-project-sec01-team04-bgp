#include "telemetry.h"

void Telemetry::init() {
    // Init current sensor
    ina219.begin();
    if (!ina219.begin()) {
        Serial.println("Couldn't find INA219 device");
    }
}

TelemetryData Telemetry::read() {
    TelemetryData result;
    result.current_a = (double)this->ina219.getCurrent_mA() / 1000;
    result.battery_v = (double)this->ina219.getBusVoltage_V();
    result.power_w = (double)this->ina219.getPower_mW() / 1000;
    return result;
}
