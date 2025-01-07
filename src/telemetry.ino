#include "telemetry.h"

void Telemetry::init() {
    // Init current sensor
    ina219.begin();
    if (!ina219.begin()) {
        PRINTLN(F("Couldn't find INA219 device"));
    }
}

void Telemetry::read() {

}
