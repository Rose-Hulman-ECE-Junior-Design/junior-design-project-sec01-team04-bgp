#pragma once

#include <stdbool.h>
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

double map_double(double, double, double, double, double);

// Class to read and interpret data from the HuskyLens camera
class Camera {
public:
    double angle;
    double steering_angle;
    double offset;

    void init();

    bool old_read();
    bool read();

    double get_servo_angle();
private:
    HUSKYLENS camera;
};
