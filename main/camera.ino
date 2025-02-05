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
const int wheelbase = 190;
const int camera_height = 165;
const double camera_dist_to_sensor = 50;
const double camera_angle = 13 * M_PI / 60; // 39deg
const double back_focal_length = 1.3; // BFL = 1.3mm, EFL = 4.6mm
// const double lookahead_distance = 500; // TODO: Set dynamically?

const Point3 rear_focal = {
  0,
  camera_height + camera_dist_to_sensor * sin(camera_angle) + back_focal_length * sin(camera_angle),
  camera_dist_to_sensor * cos(camera_angle) - back_focal_length * cos(camera_angle)
};

Point3 from_pixel_coords(Point2 px) {
  Point3 screen;

  double sensor_x = (px.x / PX - 0.5) * sensor_width;
  double sensor_y = (px.y / PY - 0.5) * sensor_height;

  screen.x = sensor_x;
  screen.y = camera_height + (camera_dist_to_sensor + sensor_y) * sin(camera_angle);
  screen.z = (camera_dist_to_sensor + sensor_y) * cos(camera_angle);

  return screen;
}

Point3 project_ground(Point3 screen) {
  Point3 ground;

  double scale = -screen.y / (screen.y - rear_focal.y);
  ground.x = screen.x + scale * (screen.x - rear_focal.x);
  ground.y = 0;
  ground.z = screen.z + scale * (screen.z - rear_focal.z);

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
  return atan2(2 * wheelbase * sin(compute_angle(target, vehicle)), lookahead_distance);
}

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

double Camera::get_servo_angle() {
  return map_double(this->steering_angle, -M_PI, M_PI, 10, 170);
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
  Serial.printf("Rear focal point: (%f, %f, %f)\n", rear_focal.x, rear_focal.y, rear_focal.z);
    if (this->camera.request() && this->camera.available()) {
        HUSKYLENSResult result = this->camera.read();
        Point2 target = { result.xTarget, PY - result.yTarget };
        Point2 origin = { result.xOrigin, PY - result.yOrigin };
        Serial.printf("Pixels: (%f, %f) to (%f, %f)\n", origin.x, origin.y, target.x, target.y);

        if (origin.y > target.y) {
          Serial.println("Swapping target and origin");
          std::swap(target, origin);
        }

        Point3 screen_target = from_pixel_coords(target);
        Point3 screen_origin = from_pixel_coords(origin);
        Serial.printf("Screen: from (%f, %f, %f) to (%f, %f, %f)\n", screen_origin.x, screen_origin.y, screen_origin.z, screen_target.x, screen_target.y, screen_target.z);

        Point3 ground_target = project_ground(screen_target);
        Point3 ground_origin = project_ground(screen_origin);
        Serial.printf("Ground: from (%f, %f, %f) to (%f, %f, %f)\n", ground_origin.x, ground_origin.y, ground_origin.z, ground_target.x, ground_target.y, ground_target.z);

        this->angle = compute_angle(ground_target, ground_origin) * 180.0 / M_PI;
        this->steering_angle = compute_steering_angle(ground_target, (Point3) {0, 0, -wheelbase}) * 180.0 / M_PI;
        return true;
    } else {
      return false;
    }
}
