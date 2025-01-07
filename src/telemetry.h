#include <Adafruit_INA219.h>

class Telemetry {
public:
    void read(tl);

private:
    float current_ma;
    float battery_v;
    Adafruit_INA219 ina219;
};

