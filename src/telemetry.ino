#include "telemetry.h"

void Telemetry::init() {
    // Init current sensor
    ina219.begin();
    if (!ina219.begin()) {
        PRINTLN(F("Couldn't find INA219 device"));
    }
}

TelemetryData Telemetry::read() {
    TelemetryData result;
    result.current_ma = this.ina219.getCurrent_mA();
    result.battery_v = this.ina219.getBusVoltage_V();
    return result;
}
