#include "camera.h"
#include <Wire.h>
#include <math.h>

void Camera::init() {
    Wire.begin();
    this->camera.begin(Wire);
}

const int PX = 320;
const int PY = 240;
// const int sensor_height = 24;
// const int sensor_width = 36;
const int sensor_height = 2.7;
const int sensor_width = 3.6;

const int camera_height = 165;
const double camera_dist_to_sensor = 50;
const double camera_angle = 13 * M_PI / 60; // 39deg

const double back_focal_length = 1.3; // BFL = 1.3mm, EFL = 4.6mm

// All in inches
const double cam_z_min = 3.5;
const double cam_z_max = 16;
const double cam_x_max = 7.5;
const double cam_x_min = 3.25;

const double inch_to_mm = 25.4;
const int wheelbase = 190 / inch_to_mm;

const double lookahead_distance = 12;
const double forward_offset = 8;

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

const Point3 vehicle = {0, 0, -wheelbase};

Point3 from_pixel_coords(Point2 px) {
  Point3 screen;

  double sensor_x = (px.x / PX - 0.5) * sensor_width;
  double sensor_y = (px.y / PY - 0.5) * sensor_height;

  screen.x = sensor_x;
  screen.y = camera_height + (camera_dist_to_sensor + sensor_y) * sin(camera_angle);
  screen.z = (camera_dist_to_sensor + sensor_y) * cos(camera_angle);

  return screen;
}

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
  printf("Lookahead distance (real): %f\n", lookahead_distance);
  return -atan2(2.0 * wheelbase * sin(compute_angle(target, vehicle)), lookahead_distance);
}

double Camera::get_servo_angle() {
  return map_double(this->steering_angle, -26, 26, 10, 170);
}

double get_slope(Point3 target, Point3 origin){
  return (target.z - origin.z) / (target.x - origin.x);
}

double get_zintercept(Point3 target, Point3 origin){
  return origin.z - get_slope(target, origin) * origin.x;
}

Point3 get_lookahead_point(Point3 target, Point3 origin){
  Point3 lookahead;

  double m = get_slope(target, origin);
  double b = get_zintercept(target, origin);
  printf("m: %f, b: %f\n", m, b);
  double discriminant = 4 * m*m * b*b - 4 * (m*m + 1) * (b*b - lookahead_distance*lookahead_distance);
  if (discriminant < 0) {
    printf("DISCRIMINANT WAS NEGATIVE\n");
    target.z -= forward_offset;
    return target;
  }

  if (m > 0) {
    lookahead.x = (-2 * m * b + sqrt(discriminant))/(2 * (m*m + 1));
  } else if (m < 0) {
    lookahead.x = (-2 * m * b - sqrt(discriminant))/(2 * (m*m + 1));
  } else {
    lookahead.x = lookahead_distance;
  }

  lookahead.z = m * lookahead.x + b;
  lookahead.y = 0;

  lookahead.z -= forward_offset;
  return lookahead;
}

bool Camera::old_read() {
  if (this->camera.request() && this->camera.available()) {
    HUSKYLENSResult result = this->camera.read();
    if (result.yTarget < result.yOrigin) { // Swap values if arrow is pointing down
      int tmp = result.yTarget;
      result.yTarget = result.yOrigin;
      result.yOrigin = tmp;

      tmp = result.xTarget;
      result.xTarget = result.xOrigin;
      result.xOrigin = tmp;
    }

    this->offset = ((result.xTarget + result.xOrigin)/2) - 160;
    this->angle = 180.0 - (std::atan2(result.yTarget - result.yOrigin, result.xTarget - result.xOrigin))*(90 / std::acos(0.0));

    return true;
  } else {
    return false;
  }
}

bool Camera::read() {
  if (!this->camera.request() || ! this->camera.available()) return false;

  HUSKYLENSResult result = this->camera.read();
  this->target = (Point2) { result.xTarget, PY - result.yTarget };
  this->origin = (Point2) { result.xOrigin, PY - result.yOrigin };

  if (origin.y > target.y) {
    Serial.println("Swapping target and origin");
    std::swap(this->target, this->origin);
  }

  Serial.printf("Pixels: (%f, %f) to (%f, %f)\n", origin.x, origin.y, target.x, target.y);

  Point3 ground_target = project_ground(this->target);
  Point3 ground_origin = project_ground(this->origin);
  Serial.printf("Ground: from (%f, %f, %f) to (%f, %f, %f)\n", ground_origin.x, ground_origin.y, ground_origin.z, ground_target.x, ground_target.y, ground_target.z);

  this->angle = compute_angle(ground_target, ground_origin) * 180.0 / M_PI;
  Serial.printf("Wheelbase angle: %f, ", 180.0 / M_PI * compute_angle(ground_target, vehicle));
  Serial.printf("Lookahead distance: %f, ", distance_2d(vehicle, ground_target));
  Point3 lookahead_point = get_lookahead_point(ground_target, ground_origin);
  this->steering_angle = compute_steering_angle(lookahead_point, vehicle) * 180.0 / M_PI;
  
  Serial.printf("Angle: %f, Steering angle: %f\n", this->angle, this->steering_angle);
  Serial.printf("Lookahead Point: (%f, %f)\n", lookahead_point.x, lookahead_point.z);

  return true;
}

CameraView Camera::get_camera_view() {
  // printf("Camera view: target = (%f, %f), origin = (%f, %f)\n", this->target.x, this->target.y, this->origin.x, this->origin.y);
  return (CameraView) { .target = this->target, .origin = this->origin };
}

