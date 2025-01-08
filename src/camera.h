#include "HUSKYLENS.h"

// Class to read and interpret data from the HuskyLens camera
class Camera {
public:
    int angle;
    int offset;

    void init();

    void read();

private:
    HUSKYLENS camera;
};
