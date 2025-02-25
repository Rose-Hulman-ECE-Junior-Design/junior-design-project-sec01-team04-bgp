#include <Wire.h>
#include <math.h>
#include "camera.h"
#include "state.h"

void Camera::init() {
  Wire.begin();
  this->camera.begin(Wire);
}

const int PX = 320;
const int PY = 240;

// All in inches
const double cam_z_min = 3.5;
const double cam_z_max = 16;
const double cam_x_max = 7.5;
const double cam_x_min = 3.25;

const double inch_to_mm = 25.4;
const int wheelbase = 190 / inch_to_mm;


double map_double(double value, double from_low, double from_high, double to_low, double to_high) {
  value -= from_low;
  value *= (to_high - to_low) / (from_high - from_low);
  value += to_low;

  if (value > to_high) {
    return to_high;
  } else if (value < to_low) {
    return to_low;
  } else {
    return value;
  }
}

const Point3 vehicle = { 0, 0, -wheelbase };

Point3 project_ground(Point2 screen) {
  Point3 ground;

  ground.z = map_double(screen.y, 0, PY, cam_z_min, cam_z_max);
  double x_offset = cam_x_min + (((cam_x_max - cam_x_min) / (cam_z_max - cam_z_min)) * (ground.z - cam_z_min));
  ground.x = map_double(screen.x, 0, PX, -x_offset, x_offset);
  ground.y = 0;


  return ground;
}

double compute_angle(Point3 target, Point3 origin) {
  return atan2(target.x - origin.x, target.z - origin.z);
}

double distance_2d(Point3 from, Point3 to) {
  return sqrt((to.x - from.x) * (to.x - from.x) + (to.z - from.z) * (to.z - from.z));
}

double compute_steering_angle(Point3 target, Point3 vehicle) {
  double lookahead_distance = distance_2d(vehicle, target);
  return -atan2(2.0 * wheelbase * sin(compute_angle(target, vehicle)), lookahead_distance);
}

double Camera::get_servo_angle() {
  return map_double(this->steering_angle, -26, 26, 10, 170);
}

double get_slope(Point3 target, Point3 origin) {
  if (target.x == origin.x) return 0;
  return (target.z - origin.z) / (target.x - origin.x);
}

double get_zintercept(Point3 target, Point3 origin) {
  return origin.z - get_slope(target, origin) * origin.x;
}

Point3 Camera::get_lookahead_point(Point3 target, Point3 origin) {
  Point3 lookahead;
  double lookahead_distance = this->state->data.lookahead_distance.apply(abs(this->steering_angle));
  double forward_offset = this->state->data.forward_offset.apply(abs(this->steering_angle));
  Serial.printf("Lookahead = %f, Forward Offset = %f\n", lookahead_distance, forward_offset);

  double m = get_slope(target, origin);
  double b = get_zintercept(target, origin);
  double discriminant = 4 * m * m * b * b - 4 * (m * m + 1) * (b * b - lookahead_distance * lookahead_distance);
  if (discriminant < 0) {
    target.z -= forward_offset;
    return target;
  }

  if (m == 0) {  // slope is NaN
    lookahead.x = 0;
  } else if (m > 0) {
    lookahead.x = (-2 * m * b + sqrt(discriminant)) / (2 * (m * m + 1));
  } else if (m < 0) {
    lookahead.x = (-2 * m * b - sqrt(discriminant)) / (2 * (m * m + 1));
  } else {
    lookahead.x = lookahead_distance;
  }

  lookahead.z = m * lookahead.x + b;
  lookahead.y = 0;

  lookahead.z -= forward_offset;
  return lookahead;
}

bool Camera::read() {
  if (!this->camera.request() || !this->camera.available()) return false;

  HUSKYLENSResult result = this->camera.read();
  this->target = (Point2){ result.xTarget, PY - result.yTarget };
  this->origin = (Point2){ result.xOrigin, PY - result.yOrigin };

  if (origin.y > target.y) {
    Serial.println("Swapping target and origin");
    std::swap(this->target, this->origin);
  }

  Point3 ground_target = project_ground(this->target);
  Point3 ground_origin = project_ground(this->origin);

  this->angle = compute_angle(ground_target, ground_origin) * 180.0 / M_PI;
  Point3 lookahead_point = this->get_lookahead_point(ground_target, ground_origin);
  this->steering_angle = compute_steering_angle(lookahead_point, vehicle) * 180.0 / M_PI;

  return true;
}

CameraView Camera::get_camera_view() {
  return (CameraView){ .target = this->target, .origin = this->origin };
}