#include <Adafruit_INA219.h>

// Class to handle reading telemetry data from vehicle
class Telemetry {
public:
    float current_ma;
    float battery_v;

    void read();

private:
    Adafruit_INA219 ina219;
};

