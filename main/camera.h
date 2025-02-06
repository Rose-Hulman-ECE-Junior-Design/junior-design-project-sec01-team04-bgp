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

typedef struct {
  Point2 target;
  Point2 origin;
} CameraView;

double map_double(double, double, double, double, double);

// Class to read and interpret line data from the HuskyLens camera.
class Camera {
public:
    Point2 origin;
    Point2 target;
    double angle; // Angle of line in degrees [-90, 90]
    double steering_angle; // Angle to steer to in degrees [-90, 90]
    double offset;

    void init();

    bool old_read(); // Uses naive angle based on screen coordinates
    bool read(); // Uses angle projected onto ground to remove distortion

    double get_servo_angle(); // Scales steering angle to [10, 170] for servo

    CameraView get_camera_view();
private:
    HUSKYLENS camera;
};
