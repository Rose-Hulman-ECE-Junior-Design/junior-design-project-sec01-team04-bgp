#pragma once

#include "HUSKYLENS.h"

typedef struct {
  double x;
  double y;
} Point2;

typedef struct {
  double x;
  double y;
  double z;
} Point3;

// Class to read and interpret data from the HuskyLens camera
class Camera {
public:
    double angle;
    double steering_angle;
    double offset;

    void init();

    void read();

private:
    HUSKYLENS camera;
};
