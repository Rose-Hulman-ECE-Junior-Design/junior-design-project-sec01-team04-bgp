#include "camera.h"
#include <Wire.h>
#include <math.h>

void Camera::init() {
    Wire.begin();
    this->camera.begin(Wire);
}

const int PX = 320;
const int PY = 240;
const int sensor_height = 24;
const int sensor_width = 36;
const int wheelbase = 190;
const int camera_height = 165;
const double camera_dist_to_sensor = 50;
const double camera_angle = 13 * M_PI / 60; // 39deg
const double back_focal_length = 1.3;
// const double lookahead_distance = 500; // TODO: Set dynamically?

const Point3 rear_focal = {
  0,
  camera_dist_to_sensor * sin(camera_angle) + back_focal_length * sin(camera_angle),
  camera_dist_to_sensor * cos(camera_angle) - back_focal_length * cos(camera_angle)
};

Point3 from_pixel_coords(Point2 px) {
  Point3 screen;

  double sensor_x = (px.x * (2 / PX) - 1) * sensor_width / 2;
  double sensor_y = (px.y * (2 / PY) - 1) * sensor_height / 2;

  screen.x = sensor_x;
  screen.y = camera_height + (camera_dist_to_sensor + sensor_y) * sin(camera_angle);
  screen.z = (camera_dist_to_sensor + sensor_y) * cos(camera_angle);

  return screen;
}

Point3 project_ground(Point3 screen) {
  Point3 ground;

  ground.x = screen.x + (screen.x - rear_focal.x) / (screen.y - rear_focal.y) * -screen.y;
  ground.y = 0;
  ground.z = screen.z + (screen.z - rear_focal.z) / (screen.y - rear_focal.y) * -screen.y;

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

void Camera::read() {
    if (this->camera.request() && this->camera.available()) {
        HUSKYLENSResult result = this->camera.read();
        Point2 target = { result.xTarget, PY - result.yTarget };
        Point2 origin = { result.xOrigin, PY - result.yOrigin };

        if (target.y > origin.y) {
          std::swap(target, origin);
        }

        Point3 screen_target = from_pixel_coords(target);
        Point3 screen_origin = from_pixel_coords(origin);

        Point3 ground_target = project_ground(screen_target);
        Point3 ground_origin = project_ground(screen_origin);

        this->angle = compute_angle(ground_target, ground_origin);
        this->steering_angle = compute_steering_angle(ground_target, (Point3) {0, -wheelbase, 0});
        // if (result.yTarget < result.yOrigin) { // Swap values if arrow is pointing down
        //   int tmp = result.yTarget;
        //   result.yTarget = result.yOrigin;
        //   result.yOrigin = tmp;

        //   tmp = result.xTarget;
        //   result.xTarget = result.xOrigin;
        //   result.xOrigin = tmp;
        // }

        // this->offset = ((result.xTarget + result.xOrigin)/2) - 160;
        // this->angle = 180.0 - (atan2(result.yTarget - result.yOrigin, result.xTarget - result.xOrigin))*(90 / std::acos(0.0));
    }
}
