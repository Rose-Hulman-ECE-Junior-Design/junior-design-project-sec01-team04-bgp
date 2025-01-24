#pragma once

#include "HUSKYLENS.h"

// Class to read and interpret data from the HuskyLens camera
class Camera {
public:
    float angle;
    float offset;

    void init();

    void read();

private:
    HUSKYLENS camera;
};
