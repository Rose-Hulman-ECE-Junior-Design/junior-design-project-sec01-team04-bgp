#include "camera.h"
#include <Wire.h>

void Camera::init() {
    Wire.begin();
    this->camera.begin(Wire);
}

void Camera::read() {
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
    }
}
